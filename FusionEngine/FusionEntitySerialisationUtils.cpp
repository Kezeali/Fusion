/*
*  Copyright (c) 2011 Fusion Project Team
*
*  This software is provided 'as-is', without any express or implied warranty.
*  In noevent will the authors be held liable for any damages arising from the
*  use of this software.
*
*  Permission is granted to anyone to use this software for any purpose,
*  including commercial applications, and to alter it and redistribute it
*  freely, subject to the following restrictions:
*
*    1. The origin of this software must not be misrepresented; you must not
*    claim that you wrote the original software. If you use this software in a
*    product, an acknowledgment in the product documentation would be
*    appreciated but is not required.
*
*    2. Altered source versions must be plainly marked as such, and must not
*    be misrepresented as being the original software.
*
*    3. This notice may not be removed or altered from any source distribution.
*
*
*  File Author(s):
*
*    Elliot Hayward
*/

#include "FusionStableHeaders.h"

#include "FusionEntitySerialisationUtils.h"

#include "FusionEntity.h"
#include "FusionEntityComponent.h"
#include "FusionEntityDeserialiser.h"
#include "FusionEntityFactory.h"
#include "FusionEntityManager.h"
#include "FusionInstanceSynchroniser.h"

#include <BitStream.h>
#include <StringCompressor.h>

#include <boost/crc.hpp>

namespace FusionEngine
{

	namespace EntitySerialisationUtils
	{

		bool SerialiseEntity(RakNet::BitStream& out, const EntityPtr& entity, IComponent::SerialiseMode mode)
		{
			bool dataWritten = false;

			//entity->SerialiseReferencedEntitiesList(out);

			auto compressor = RakNet::StringCompressor::Instance();

			auto& components = entity->GetComponents();
			size_t numComponents = components.size() - 1; // minus transform

			auto transform = dynamic_cast<IComponent*>(entity->GetTransform().get());
			{
				std::string type = transform->GetType();
				compressor->EncodeString(type.c_str(), type.length() + 1, &out);
				
				RakNet::BitStream tempStream;
				bool conData = transform->SerialiseContinuous(tempStream);
				//FSN_ASSERT(conData || tempStream.GetNumberOfBitsUsed() == 0);
				if (!conData)
					tempStream.Reset();

				const auto bitsUsedBeforeWriting = tempStream.GetNumberOfBitsUsed();
				bool occData = transform->SerialiseOccasional(tempStream, mode);
				//FSN_ASSERT(occData || tempStream.GetNumberOfBitsUsed() == bitsUsedBeforeWriting);
				if (!occData)
					tempStream.SetWriteOffset(bitsUsedBeforeWriting);

				out.Write(conData);
				out.Write(occData);
				out.Write(tempStream);

				dataWritten |= conData || occData;
			}

			out.Write(numComponents);
			if (components.size() > 1)
			{
				auto begin = ++components.begin();
				auto end = components.end();
				for (auto it = begin; it != end; ++it)
				{
					auto& component = *it;
					FSN_ASSERT(component.get() != transform);

					auto dist = std::distance(components.begin(), it);
					FSN_ASSERT(dist > 0);
					size_t index = (size_t)dist;
					out.Write(index);

					std::string type = component->GetType();
					std::string identifier = component->GetIdentifier();
					compressor->EncodeString(type.c_str(), type.length() + 1, &out);
					// TODO: give IDs from an IDStack to components and use those to identify them
					//  (the interface identifier are really ment as a scripting convinience thing - i.e. for #uses <Interface> <identifier> directives)
					compressor->EncodeString(identifier.c_str(), identifier.length() + 1, &out);
				}
				for (auto it = begin; it != end; ++it)
				{
					auto& component = *it;
					FSN_ASSERT(component.get() != transform);

					RakNet::BitStream tempStream;
					bool conData = component->SerialiseContinuous(tempStream);
					FSN_ASSERT(conData || tempStream.GetNumberOfBitsUsed() == 0);
					if (!conData)
						tempStream.Reset();

					const auto bitsUsedBeforeWriting = tempStream.GetNumberOfBitsUsed();
					bool occData = component->SerialiseOccasional(tempStream, mode);
					FSN_ASSERT(occData || tempStream.GetNumberOfBitsUsed() == bitsUsedBeforeWriting);
					if (!occData)
						tempStream.SetWriteOffset(bitsUsedBeforeWriting);

					out.Write(conData);
					out.Write(occData);
					out.Write(tempStream);

					dataWritten |= (conData || occData);
				}
			}

			return dataWritten;
		}

		EntityPtr DeserialiseEntity(RakNet::BitStream& in, EntityFactory* factory, EntityManager* manager)
		{
			EntityPtr entity;

			auto mode = IComponent::All;

			//entity->DeserialiseReferencedEntitiesList(in, EntityDeserialiser(manager));

			auto stringCompressor = RakNet::StringCompressor::Instance();

			ComponentPtr transform;
			{
				char typeNameBuf[256u];
				stringCompressor->DecodeString(&typeNameBuf[0], 256, &in);
				std::string typeName(typeNameBuf);

				//FSN_ASSERT(entity->GetTransform()->GetType() == typeName);
				//transform = entity->GetTransform();

				transform = factory->InstanceComponent(typeName);

				bool conData = in.ReadBit();
				bool occData = in.ReadBit();
				if (conData)
					transform->DeserialiseContinuous(in);
				if (occData)
					transform->DeserialiseOccasional(in, mode);
			}

			transform->SynchronisePropertiesNow();

			entity = std::make_shared<Entity>(manager, &manager->m_PropChangedQueue, transform);

			std::vector<ComponentPtr> localComponents;

			//auto& existingComponents = entity->GetComponents();

			size_t numComponents;
			in.Read(numComponents);
			localComponents.reserve(numComponents);
			for (size_t i = 0; i < numComponents; ++i)
			{
				size_t index;
				in.Read(index);

				char typeNameBuf[256u];
				stringCompressor->DecodeString(&typeNameBuf[0], 256, &in);
				std::string typeName(typeNameBuf);
				char identifierBuf[256u];
				stringCompressor->DecodeString(&identifierBuf[0], 256, &in);
				std::string identifier(identifierBuf);
				
				//auto componentEntry = std::find_if(existingComponents.begin(), existingComponents.end(), [&typeName, &identifier](const ComponentPtr& com)
				//{
				//	return com->GetType() == typeName && com->GetIdentifier() == identifier;
				//});
				ComponentPtr component;
				//if (componentEntry != existingComponents.end())
				//	component = *componentEntry;
				//else
				{
					component = factory->InstanceComponent(typeName);
					if (component)
					{
						entity->AddComponent(component, identifier);
					}
					else
					{
						FSN_EXCEPT(InvalidArgumentException, "Unknown component type used: " + identifier);
					}
				}
				localComponents.push_back(component);
			}
			if (numComponents != 0)
			{
				auto& components = localComponents;
				for (auto it = components.begin(), end = components.end(); it != end; ++it)
				{
					auto& component = *it;
					FSN_ASSERT(component != transform);

					bool conData = in.ReadBit();
					bool occData = in.ReadBit();
					if (conData)
						component->DeserialiseContinuous(in);
					if (occData)
						component->DeserialiseOccasional(in, mode);
				}
			}

			return entity;
		}

		bool SerialiseContinuous(RakNet::BitStream& out, const EntityPtr& entity, IComponent::SerialiseMode mode)
		{
			bool dataWritten = false;

			//entity->SerialiseReferencedEntitiesList(out);

			auto compressor = RakNet::StringCompressor::Instance();

			auto& components = entity->GetComponents();
			size_t numComponents = components.size() - 1; // minus transform

			auto transform = dynamic_cast<IComponent*>(entity->GetTransform().get());
			{			
				RakNet::BitStream tempStream;
				bool conData = transform->SerialiseContinuous(tempStream);
				//FSN_ASSERT(conData || tempStream.GetNumberOfBitsUsed() == 0);
				if (!conData)
					tempStream.Reset();

				out.Write(conData);
				out.Write(tempStream);

				dataWritten |= conData;
			}

			out.Write(numComponents);
			if (components.size() > 1)
			{
				auto begin = ++components.begin();
				auto end = components.end();
				for (auto it = begin; it != end; ++it)
				{
					auto& component = *it;
					FSN_ASSERT(component.get() != transform);

					RakNet::BitStream tempStream;
					bool conData = component->SerialiseContinuous(tempStream);
					//FSN_ASSERT(conData || tempStream.GetNumberOfBitsUsed() == 0);
					if (conData)
					{
						out.Write1();
						out.Write(tempStream);
					}
					else
						out.Write0();

					dataWritten |= conData;
				}
			}

			return dataWritten;
		}

		void DeserialiseContinuous(RakNet::BitStream& in, const EntityPtr& entity, IComponent::SerialiseMode mode, EntityFactory* factory, EntityManager* manager)
		{
			//entity->DeserialiseReferencedEntitiesList(in, EntityDeserialiser(manager));

			auto stringCompressor = RakNet::StringCompressor::Instance();

			ComponentPtr transform;
			{
				transform = entity->GetTransform();

				bool conData = in.ReadBit();
				if (conData)
					transform->DeserialiseContinuous(in);
			}

			auto& existingComponents = entity->GetComponents();

			size_t numComponents;
			in.Read(numComponents);
			// TEMP - just ignore states for incomplete entities (TODO: synchronise entities properly)
			if (numComponents != existingComponents.size() - 1) return;
			//FSN_ASSERT(numComponents == existingComponents.size() - 1);
			if (numComponents != 0)
			{
				auto it = existingComponents.begin(), end = existingComponents.end();
				for (++it; it != end; ++it)
				{
					auto& component = *it;
					FSN_ASSERT(component != transform);

					bool conData = in.ReadBit();
					if (conData)
						component->DeserialiseContinuous(in);
				}
			}
		}

		bool SerialiseComponent(RakNet::BitStream& out, uint16_t& storedChecksum, IComponent* component, IComponent::SerialiseMode mode)
		{
			RakNet::BitStream tempStream;
			bool conData = component->SerialiseOccasional(tempStream, IComponent::All);
			//FSN_ASSERT(conData || tempStream.GetNumberOfBitsUsed() == 0);

			conData = tempStream.GetNumberOfBitsUsed() > 0;

			if (conData)
			{
				boost::crc_16_type crc;
				crc.process_bytes(tempStream.GetData(), tempStream.GetNumberOfBytesUsed());
				uint16_t checksum = crc.checksum();

				if (mode == IComponent::Changes)
				{
					conData = checksum != storedChecksum; // Ignore unchanged states
				}
				storedChecksum = checksum;
			}

			if (conData)
			{
				out.Write1();
#ifdef _DEBUG
				out.Write(RakNet::RakString(component->GetType().c_str()));
				out.Write(tempStream.GetNumberOfBitsUsed());
#endif
				out.Write(tempStream);
			}
			else
				out.Write0();

			return conData;
		}

		bool SerialiseOccasional(RakNet::BitStream& out, std::vector<uint16_t>& checksums, const EntityPtr& entity, IComponent::SerialiseMode mode)
		{
			bool dataWritten = false;

			//entity->SerialiseReferencedEntitiesList(out);

			auto compressor = RakNet::StringCompressor::Instance();

			auto& components = entity->GetComponents();
			size_t numComponents = components.size() - 1; // minus transform

			checksums.resize(components.size());

			auto transform = dynamic_cast<IComponent*>(entity->GetTransform().get());
			{
				dataWritten |= SerialiseComponent(out, checksums[0], transform, mode);
			}

			out.Write(numComponents);
			if (components.size() > 1)
			{
				auto begin = ++components.begin();
				auto end = components.end();
				auto checksumEntry = checksums.begin() + 1;
				for (auto it = begin; it != end; ++it)
				{
					auto& component = *it;
					FSN_ASSERT(component.get() != transform);
					FSN_ASSERT(checksumEntry != checksums.end());


					dataWritten |= SerialiseComponent(out, *checksumEntry, component.get(), mode);
					++checksumEntry;
				}
			}

			return dataWritten;
		}

		void DeserialiseComponent(RakNet::BitStream& in, uint16_t& checksum, IComponent* component)
		{
			//bool conData = in.ReadBit();
			//if (conData)
			{
#ifdef _DEBUG
				RakNet::RakString expectedType;
				in.Read(expectedType);
				FSN_ASSERT(component->GetType() == expectedType.C_String());
				RakNet::BitSize_t expectedNumBits;
				in.Read(expectedNumBits);
#endif
				auto start = in.GetReadOffset();
				component->DeserialiseOccasional(in, IComponent::All);

				const auto dataEnd = in.GetReadOffset();
				const auto dataBits = dataEnd - start;
				//const size_t dataBytes = BITS_TO_BYTES(dataBits);
				in.SetReadOffset(start);

				RakNet::BitStream checksumBitstream;
				in.Read(checksumBitstream, dataBits);

				boost::crc_16_type crc;
				crc.process_bytes(checksumBitstream.GetData(), checksumBitstream.GetNumberOfBytesUsed());
				checksum = crc.checksum();

				in.SetReadOffset(dataEnd);

				FSN_ASSERT(dataBits == expectedNumBits);
			}
		}

		void DeserialiseOccasional(RakNet::BitStream& in, std::vector<uint16_t>& checksums, const EntityPtr& entity, IComponent::SerialiseMode mode, EntityFactory* factory, EntityManager* manager)
		{
			//entity->DeserialiseReferencedEntitiesList(in, EntityDeserialiser(manager));

			auto stringCompressor = RakNet::StringCompressor::Instance();

			auto& existingComponents = entity->GetComponents();
			// Make sure checksums for every component can be stored
			checksums.resize(existingComponents.size());

			std::vector<bool> comsRead;

			ComponentPtr transform;
			{
				transform = entity->GetTransform();

				bool conData = in.ReadBit();
				if (conData)
				{
					DeserialiseComponent(in, checksums[0], transform.get());
				}
				comsRead.push_back(conData);
			}

			size_t numComponents;
			in.Read(numComponents);
			// TEMP - just ignore states for incomplete entities (TODO: synchronise entities properly)
			if (numComponents != existingComponents.size() - 1) return;
			//FSN_ASSERT(numComponents == existingComponents.size() - 1);
			if (numComponents != 0)
			{
				auto it = existingComponents.begin(), end = existingComponents.end();
				auto checksumEntry = checksums.begin() + 1;
				for (++it; it != end; ++it)
				{
					auto& component = *it;
					FSN_ASSERT(component != transform);
					FSN_ASSERT(checksumEntry != checksums.end());

					bool conData = in.ReadBit();
					if (conData)
						DeserialiseComponent(in, *checksumEntry, component.get());
					++checksumEntry;
					comsRead.push_back(conData);
				}
			}
		}

		void WriteComponent(CL_IODevice& out, IComponent* component)
		{
			FSN_ASSERT(component);

			RakNet::BitStream stream;
			const bool conData = component->SerialiseContinuous(stream);
			const bool occData = component->SerialiseOccasional(stream, IComponent::All);

			out.write_uint8(conData ? 0xFF : 0x00); // Flag indicating data presence
			out.write_uint8(occData ? 0xFF : 0x00);

			out.write_uint32(stream.GetNumberOfBytesUsed());
			out.write(stream.GetData(), stream.GetNumberOfBytesUsed());
		}

		void ReadComponent(CL_IODevice& in, IComponent* component)
		{
			FSN_ASSERT(component);

			const bool conData = in.read_uint8() != 0x00;
			const bool occData = in.read_uint8() != 0x00;

			const auto dataLen = in.read_uint32();
			std::vector<unsigned char> data(dataLen);
			in.read(data.data(), data.size());

			RakNet::BitStream stream(data.data(), data.size(), false);

			if (conData)
				component->DeserialiseContinuous(stream);
			if (occData)
				component->DeserialiseOccasional(stream, IComponent::All);

			//stream.AssertStreamEmpty();
			if (stream.GetNumberOfUnreadBits() >= 8)
				SendToConsole("Not all serialised data was used when reading a " + component->GetType());
		}

		void SaveEntity(CL_IODevice& out, EntityPtr entity, bool id_included)
		{
			if (id_included)
			{
				ObjectID id = entity->GetID();
				out.write(&id, sizeof(ObjectID));
			}

			{
				RakNet::BitStream stream;
				entity->SerialiseReferencedEntitiesList(stream);

				out.write_uint32(stream.GetNumberOfBytesUsed());
				out.write(stream.GetData(), stream.GetNumberOfBytesUsed());
			}

			auto& components = entity->GetComponents();
			size_t numComponents = components.size() - 1; // - transform

			auto transform = dynamic_cast<IComponent*>(entity->GetTransform().get());
			out.write_string_a(transform->GetType());
			WriteComponent(out, transform);

			out.write(&numComponents, sizeof(size_t));
			for (auto it = components.begin(), end = components.end(); it != end; ++it)
			{
				auto& component = *it;
				if (component.get() != transform)
				{
					out.write_string_a(component->GetType());
					out.write_string_a(component->GetIdentifier());
				}
			}
			for (auto it = components.begin(), end = components.end(); it != end; ++it)
			{
				auto& component = *it;
				if (component.get() != transform)
					WriteComponent(out, component.get());
			}
		}

		EntityPtr LoadEntity(CL_IODevice& in, bool id_included, EntityFactory* factory, EntityManager* manager, InstancingSynchroniser* synchroniser)
		{
			ObjectID id = 0;
			if (id_included)
			{
				in.read(&id, sizeof(ObjectID));
				synchroniser->TakeID(id);
			}

			const auto dataLen = in.read_uint32();
			std::vector<unsigned char> referencedEntitiesData(dataLen);
			in.read(referencedEntitiesData.data(), referencedEntitiesData.size());

			ComponentPtr transform;
			{
				std::string transformType = in.read_string_a();
				transform = factory->InstanceComponent(transformType);

				ReadComponent(in, transform.get());
			}

			auto entity = std::make_shared<Entity>(manager, &manager->m_PropChangedQueue, transform);
			entity->SetID(id);

			transform->SynchronisePropertiesNow();

			{
				RakNet::BitStream stream(referencedEntitiesData.data(), referencedEntitiesData.size(), false);
				entity->DeserialiseReferencedEntitiesList(stream, EntityDeserialiser(manager));
			}

			size_t numComponents;
			in.read(&numComponents, sizeof(size_t));
			for (size_t i = 0; i < numComponents; ++i)
			{
				std::string type = in.read_string_a();
				std::string ident = in.read_string_a();
				auto component = factory->InstanceComponent(type);
				entity->AddComponent(component, ident);
			}
			if (numComponents != 0)
			{
				auto& components = entity->GetComponents();
				auto it = components.begin(), end = components.end();
				for (++it; it != end; ++it)
				{
					auto& component = *it;
					FSN_ASSERT(component != transform);

					ReadComponent(in, component.get());
				}
			}

			return entity;
		}
	}
}
