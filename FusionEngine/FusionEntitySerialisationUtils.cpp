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

	// TODO: put this in a more common header
	template <int num_bits, typename IntegerT>
	void WriteIntegerInBits(RakNet::BitStream& out, IntegerT value)
	{
		FSN_ASSERT(std::is_integral<IntegerT>());
		FSN_ASSERT(std::is_unsigned<IntegerT>());
		// Make sure the given value can fit in the allowed number of bits
		FSN_ASSERT(num_bits >= BYTES_TO_BITS(sizeof(IntegerT))-NumberOfLeadingZeroes(IntegerT(value)));

		if (value != 0)
		{
			out.Write1();
			if (RakNet::BitStream::IsBigEndian())
			{
				unsigned char output[sizeof(IntegerT)];
				RakNet::BitStream::ReverseBytes(reinterpret_cast<unsigned char*>(&value), output, sizeof(templateType));
				out.WriteBits(&output, num_bits);
			}
			else
			{
				out.WriteBits(reinterpret_cast<unsigned char*>(&value), num_bits);
			}
		}
		else
			out.Write0();
	}

	template <int num_bits, typename IntegerT>
	bool ReadIntegerInBits(RakNet::BitStream& in, IntegerT& outValue)
	{
		if (out.ReadBit())
		{
			unsigned char rawBits[sizeof(IntegerT)];
			memset(rawBits, 0, sizeof(rawBits));
			const bool success = in.ReadBits(rawBits, num_bits);
			if (success)
			{
				if (RakNet::BitStream::IsBigEndian())
					RakNet::BitStream::ReverseBytesInPlace(rawBits, sizeof(rawBits));
				memcpy(&outValue, rawBits, sizeof(rawBits));
			}
			else
				return false;
		}
		else
			value = 0;

		return true;
	}

	namespace EntitySerialisationUtils
	{

		Vector2 StdDeserialisePosition(RakNet::BitStream& stream, const Vector2& origin, float radius)
		{
			if (stream.ReadBit())
			{
				Vector2 position;
				stream.ReadFloat16(position.x, -radius, radius);
				stream.ReadFloat16(position.y, -radius, radius);
				return origin + position;
			}
			else
			{
				Vector2 position;
				stream.Read(position.x);
				stream.Read(position.y);
				return position;
			}
		}

		void StdSerialisePosition(RakNet::BitStream& stream, ITransform* tf, const Vector2& origin, float radius)
		{
			auto pos = tf->GetPosition();
			auto offset = pos - origin;
			if (offset.length() < radius - 0.01f)
			{
				stream.Write1();
				stream.WriteFloat16(offset.x, -radius, radius);
				stream.WriteFloat16(offset.y, -radius, radius);
			}
			else
			{
				stream.Write0();
				stream.Write(pos.x);
				stream.Write(pos.y);
			}
		}

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

		bool WriteComponentState(RakNet::BitStream& out, RakNet::BitStream& state, const IComponent* component)
		{
			if (state.GetNumberOfBitsUsed() > 0)
			{
				if (state.GetNumberOfBitsUsed() < (1 << 14))
				{
					//WriteIntegerInBits<14>(out, state.GetNumberOfBitsUsed());
					out.Write1();
					out.WriteBitsFromIntegerRange(state.GetNumberOfBitsUsed(), 0u, (1u << 14));
					out.Write(state);

					return true;
				}
				else
					FSN_EXCEPT(InvalidArgumentException, "Serialised state for " + component->GetType() + " is too large");
			}
			else
			{
				out.Write0();
			}

			return false;
		}

		RakNet::BitSize_t ReadStateLength(RakNet::BitStream& in)
		{
			//ReadIntegerInBits<14>(in, length);
			if (in.ReadBit())
			{
				RakNet::BitSize_t length;
				in.ReadBitsFromIntegerRange(length, 0u, (1u << 14));
				return length;
			}
			else
				return RakNet::BitSize_t(0);
		}

		bool SerialiseContinuous(RakNet::BitStream& out, const EntityPtr& entity, IComponent::SerialiseMode mode)
		{
			bool dataWritten = false;

			//entity->SerialiseReferencedEntitiesList(out);

			auto compressor = RakNet::StringCompressor::Instance();

			auto& components = entity->GetComponents();
			size_t numComponents = components.size() - 1; // minus transform

			auto transformComponent = entity->GetTransform();
			{
				Vector2 origin; float radius = 0.f;
				auto transform = dynamic_cast<ITransform*>(entity->GetTransform().get());
				if (transform->HasContinuousPosition())
				{
					out.Write1();
					StdSerialisePosition(out, transform, origin, radius);
				}
				else
					out.Write0();

				RakNet::BitStream tempStream;
				transformComponent->SerialiseContinuous(tempStream);
				dataWritten = WriteComponentState(out, tempStream, transformComponent.get());
			}

			out.Write(numComponents);
			if (components.size() > 1)
			{
				auto begin = ++components.begin();
				auto end = components.end();
				for (auto it = begin; it != end; ++it)
				{
					auto& component = *it;
					FSN_ASSERT(component != transformComponent);

					RakNet::BitStream tempStream;
					component->SerialiseContinuous(tempStream);

					dataWritten |= WriteComponentState(out, tempStream, component.get());
				}
			}

			return dataWritten;
		}

		void DeserialiseContinuous(RakNet::BitStream& in, const EntityPtr& entity, IComponent::SerialiseMode mode)
		{
			//entity->DeserialiseReferencedEntitiesList(in, EntityDeserialiser(manager));

			auto stringCompressor = RakNet::StringCompressor::Instance();

			ComponentPtr transform;
			{
				transform = entity->GetTransform();

				Vector2 origin; float radius = 0;
				auto tf = dynamic_cast<ITransform*>(transform.get());
				auto result = DeserialisePosition(in, origin, radius);
				if (result.first)
					tf->SetPosition(result.second);

				const auto stateLength = ReadStateLength(in);
				if (stateLength > 0)
				{
					const auto startPos = in.GetReadOffset();

					transform->DeserialiseContinuous(in);

					if (in.GetReadOffset() - startPos != stateLength)
						FSN_EXCEPT(Exception, "Component read too much data");
				}
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

					const auto stateLength = ReadStateLength(in);
					if (stateLength > 0)
						component->DeserialiseContinuous(in);
				}
			}
		}

		bool SerialiseComponentOccasional(RakNet::BitStream& out, uint32_t& storedChecksum, IComponent* component, IComponent::SerialiseMode mode)
		{
			RakNet::BitStream tempStream;
			component->SerialiseOccasional(tempStream, IComponent::All);

			bool conData = tempStream.GetNumberOfBitsUsed() > 0;

			if (conData)
			{
				boost::crc_32_type crc;
				crc.process_bytes(tempStream.GetData(), tempStream.GetNumberOfBytesUsed());
				uint32_t checksum = crc.checksum();

				if (mode == IComponent::Changes)
				{
					conData = checksum != storedChecksum; // Ignore unchanged states
				}
				storedChecksum = checksum;
			}

			WriteComponentState(out, tempStream, component);

			return conData;
		}

		bool SerialiseOccasional(RakNet::BitStream& out, std::vector<uint32_t>& checksums, const EntityPtr& entity, IComponent::SerialiseMode mode)
		{
			bool dataWritten = false;

			//entity->SerialiseReferencedEntitiesList(out);

			auto compressor = RakNet::StringCompressor::Instance();

			auto& components = entity->GetComponents();
			size_t numComponents = components.size() - 1; // minus transform

#ifdef _DEBUG
			std::string componentsNames;
			for (auto it = components.begin(), end = components.end(); it != end; ++it)
			{
				componentsNames += (*it)->GetType();
			}
			boost::crc_32_type crc;
			crc.process_block(componentsNames.data(), componentsNames.data() + componentsNames.length());
			out.Write(crc.checksum());
#endif

			checksums.resize(components.size());

			auto transformComponent = entity->GetTransform();
			{
				Vector2 origin; float radius = 0.f;
				auto transform = dynamic_cast<ITransform*>(entity->GetTransform().get());
				if (transform->HasContinuousPosition())
				{
					out.Write1();
					StdSerialisePosition(out, transform, origin, radius);
				}
				else
					out.Write0();

				dataWritten |= SerialiseComponentOccasional(out, checksums[0], transformComponent.get(), mode);
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
					FSN_ASSERT(component != transformComponent);
					FSN_ASSERT(checksumEntry != checksums.end());

					dataWritten |= SerialiseComponentOccasional(out, *checksumEntry, component.get(), mode);
					++checksumEntry;
				}
			}

			return dataWritten;
		}

		void DeserialiseComponentOccasional(RakNet::BitStream& in, uint32_t& checksum, IComponent* component)
		{
			//const auto stateLength = ReadStateLength(in);
			//if (stateLength)
			{
				auto start = in.GetReadOffset();
				component->DeserialiseOccasional(in, IComponent::All);

				const auto dataEnd = in.GetReadOffset();
				const auto dataBits = dataEnd - start;
				//const size_t dataBytes = BITS_TO_BYTES(dataBits);
				in.SetReadOffset(start);

				RakNet::BitStream checksumBitstream;
				in.Read(checksumBitstream, dataBits);

				boost::crc_32_type crc;
				crc.process_bytes(checksumBitstream.GetData(), checksumBitstream.GetNumberOfBytesUsed());
				checksum = crc.checksum();

				in.SetReadOffset(dataEnd);

				//FSN_ASSERT(dataBits == stateLength);
			}
		}

		void DeserialiseOccasional(RakNet::BitStream& in, std::vector<uint32_t>& checksums, const EntityPtr& entity, IComponent::SerialiseMode mode)
		{
			//entity->DeserialiseReferencedEntitiesList(in, EntityDeserialiser(manager));

			auto stringCompressor = RakNet::StringCompressor::Instance();

			auto& existingComponents = entity->GetComponents();

#ifdef _DEBUG
			std::string componentsNames;
			for (auto it = existingComponents.begin(), end = existingComponents.end(); it != end; ++it)
			{
				componentsNames += (*it)->GetType();
			}
			boost::crc_32_type crc;
			crc.process_block(componentsNames.data(), componentsNames.data() + componentsNames.length());
			auto localNameSum = crc.checksum();
			uint32_t remoteNameSum;
			in.Read(remoteNameSum);
			FSN_ASSERT(localNameSum == remoteNameSum);
#endif

			// Make sure checksums for every component can be stored
			checksums.resize(existingComponents.size());

			ComponentPtr transform;
			{
				transform = entity->GetTransform();

				Vector2 origin; float radius = 0;
				auto tf = dynamic_cast<ITransform*>(transform.get());
				auto result = DeserialisePosition(in, origin, radius);
				if (result.first)
					tf->SetPosition(result.second);

				const auto stateLength = ReadStateLength(in);
				if (stateLength > 0)
				{
					DeserialiseComponentOccasional(in, checksums[0], transform.get());
				}
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

					const auto stateLength = ReadStateLength(in);
					if (stateLength > 0)
						DeserialiseComponentOccasional(in, *checksumEntry, component.get());
					++checksumEntry;
				}
			}
		}

		std::pair<bool, Vector2> DeserialisePosition(RakNet::BitStream& in, const Vector2& origin, const float radius)
		{
			// First bit indicates presence of position data, as some
			//  transform components send this in the continuous stream
			//  while others use the occasional stream (the given stream
			//  can be either):
			if (in.ReadBit())
			{
				// TODO: Could use ~3 bits here indicating a non-standard position-(de)serialisation functor to use
				return std::make_pair(true, StdDeserialisePosition(in, origin, radius));
			}
			else
			{
				return std::make_pair(false, origin);
			}
		}

		//void SerialisePosition(RakNet::BitStream& out, ComponentPtr tf, const Vector2& origin, const float radius)
		//{
		//	auto sf = std::function<void (RakNet::BitStream&, const Vector2&, float)>();//tf->GetSerialisationFunctor();
		//	sf(out, origin, radius);
		//}

		static void MergeComponentData(CL_IODevice& in, CL_IODevice& out, RakNet::BitStream& continuous, RakNet::BitStream& occasional)
		{
			auto existingConDataBytes = in.read_uint16();
			auto existingOccDataBytes = in.read_uint16();

			auto conDataBits = ReadStateLength(continuous);
			RakNet::BitStream tempStream;
			if (conDataBits > 0)
			{
				continuous.Read(tempStream, conDataBits);

				out.write_uint16(tempStream.GetNumberOfBytesUsed());
				out.write(tempStream.GetData(), tempStream.GetNumberOfBytesUsed());
				
				tempStream.Reset();
			}
			else
			{
				std::vector<unsigned char> tempData(existingConDataBytes);
				in.read(tempData.data(), existingConDataBytes);

				out.write_uint16(existingConDataBytes);
				out.write(tempData.data(), existingConDataBytes);
			}

			auto occDataBits = ReadStateLength(occasional);
			if (occDataBits > 0)
			{
				occasional.Read(tempStream, occDataBits);

				out.write_uint16(tempStream.GetNumberOfBytesUsed());
				out.write(tempStream.GetData(), tempStream.GetNumberOfBytesUsed());
			}
			else
			{
				std::vector<unsigned char> tempData(existingOccDataBytes);
				in.read(tempData.data(), existingOccDataBytes);

				out.write_uint16(existingOccDataBytes);
				out.write(tempData.data(), existingOccDataBytes);
			}
		}

		void MergeEntityData(CL_IODevice& in, CL_IODevice& out, RakNet::BitStream& incomming_c, RakNet::BitStream& incomming_o)
		{
			// Copy transform component type name
			std::string transformType = in.read_string_a();
			out.write_string_a(transformType);

			// Merge transform component data
			MergeComponentData(in, out, incomming_c, incomming_o);

			// Copy the component count
			size_t numComponents;
			in.read(&numComponents, sizeof(size_t));
			out.write(&numComponents, sizeof(size_t));

			// Copy the other component names
			for (size_t i = 0; i < numComponents; ++i)
			{
				std::string transformType = in.read_string_a();
				out.write_string_a(transformType);
			}

			// Merge the other component data
			for (size_t i = 0; i < numComponents; ++i)
			{
				MergeComponentData(in, out, incomming_c, incomming_o);
			}
		}

		void WriteComponent(CL_IODevice& out, IComponent* component)
		{
			FSN_ASSERT(component);

			bool conData;
			bool occData;

			RakNet::BitStream stream;
			component->SerialiseContinuous(stream);
			if (stream.GetWriteOffset() > 0)
			{
				out.write_uint16(stream.GetNumberOfBytesUsed());
				out.write(stream.GetData(), stream.GetNumberOfBytesUsed());
			}
			else
				out.write_uint16(0);
			stream.Reset();

			component->SerialiseOccasional(stream, IComponent::All);
			if (stream.GetWriteOffset() > 0)
			{
				out.write_uint16(stream.GetNumberOfBytesUsed());
				out.write(stream.GetData(), stream.GetNumberOfBytesUsed());
			}
			else
				out.write_uint16(0);
		}

		void ReadComponent(CL_IODevice& in, IComponent* component)
		{
			FSN_ASSERT(component);

			const auto conDataLen = in.read_uint16();
			std::vector<unsigned char> data(conDataLen);
			in.read(data.data(), conDataLen);

			const auto occDataLen = in.read_uint16();
			data.resize(conDataLen + occDataLen);
			in.read(data.data() + conDataLen, occDataLen);

			RakNet::BitStream stream(data.data(), data.size(), false);

			if (conDataLen)
				component->DeserialiseContinuous(stream);

			stream.AlignReadToByteBoundary();

			if (occDataLen)
				component->DeserialiseOccasional(stream, IComponent::All);

			//stream.AssertStreamEmpty();
			if (stream.GetNumberOfUnreadBits() >= 8)
				SendToConsole("Not all serialised data was used when reading a " + component->GetType());
		}

		//{
		//	RakNet::BitStream stream;
		//	entity->SerialiseReferencedEntitiesList(stream);

		//	out.write_uint32(stream.GetNumberOfBytesUsed());
		//	out.write(stream.GetData(), stream.GetNumberOfBytesUsed());
		//}

		void SaveEntity(CL_IODevice& out, EntityPtr entity, bool id_included)
		{
			if (id_included)
			{
				ObjectID id = entity->GetID();
				out.write(&id, sizeof(ObjectID));
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
				{
					WriteComponent(out, component.get());
				}
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

			//const auto dataLen = in.read_uint32();
			//std::vector<unsigned char> referencedEntitiesData(dataLen);
			//in.read(referencedEntitiesData.data(), referencedEntitiesData.size());

			ComponentPtr transform;
			{
				std::string transformType = in.read_string_a();
				transform = factory->InstanceComponent(transformType);

				ReadComponent(in, transform.get());
			}

			auto entity = std::make_shared<Entity>(manager, &manager->m_PropChangedQueue, transform);
			entity->SetID(id);

			transform->SynchronisePropertiesNow();

			//{
			//	RakNet::BitStream stream(referencedEntitiesData.data(), referencedEntitiesData.size(), false);
			//	entity->DeserialiseReferencedEntitiesList(stream, EntityDeserialiser(manager));
			//}

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
