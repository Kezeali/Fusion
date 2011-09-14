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

namespace FusionEngine
{

	namespace EntitySerialisationUtils
	{

		bool SerialiseEntity(RakNet::BitStream& out, const EntityPtr& entity, IComponent::SerialiseMode mode)
		{
			bool dataWritten = false;

			entity->SerialiseReferencedEntitiesList(out);

			auto compressor = RakNet::StringCompressor::Instance();

			auto& components = entity->GetComponents();
			size_t numComponents = components.size() - 1; // minus transform

			auto transform = dynamic_cast<IComponent*>(entity->GetTransform().get());
			{
				std::string type = transform->GetType();
				compressor->EncodeString(type.c_str(), type.length(), &out);
				
				RakNet::BitStream tempStream;
				bool conData = transform->SerialiseContinuous(tempStream);
				bool occData = transform->SerialiseOccasional(tempStream, mode);
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

					std::string type = component->GetType();
					compressor->EncodeString(type.c_str(), type.length(), &out);
					// TODO: give IDs from an IDStack to components and use those to identify them
					//  (the interface identifier are really ment as a scripting convinience thing - i.e. for #uses <Interface> <identifier> directives)
					compressor->EncodeString(component->GetIdentifier().c_str(), component->GetIdentifier().length(), &out);
				}
				for (auto it = begin; it != end; ++it)
				{
					auto& component = *it;
					FSN_ASSERT(component.get() != transform);

					RakNet::BitStream tempStream;
					bool conData = component->SerialiseContinuous(tempStream);
					bool occData = component->SerialiseOccasional(tempStream, mode);
					out.Write(conData);
					out.Write(occData);
					out.Write(tempStream);

					dataWritten |= conData || occData;
				}
			}

			return dataWritten;
		}

		void DeserialiseEntity(RakNet::BitStream& in, const EntityPtr& entity, IComponent::SerialiseMode mode, EntityFactory* factory, EntityManager* manager)
		{
			entity->DeserialiseReferencedEntitiesList(in, EntityDeserialiser(manager));

			auto stringCompressor = RakNet::StringCompressor::Instance();

			ComponentPtr transform;
			{
				RakNet::RakString string;
				stringCompressor->DecodeString(&string, 256, &in);

				FSN_ASSERT(entity->GetTransform()->GetType() == string.C_String());
				transform = entity->GetTransform();

				//transform = factory->InstanceComponent(string.C_String());

				bool conData = in.ReadBit();
				bool occData = in.ReadBit();
				if (conData)
					transform->DeserialiseContinuous(in);
				if (occData)
					transform->DeserialiseOccasional(in, mode);
			}

			//transform->SynchronisePropertiesNow();

			std::vector<ComponentPtr> localComponents;

			size_t numComponents;
			in.Read(numComponents);
			localComponents.reserve(numComponents);
			for (size_t i = 0; i < numComponents; ++i)
			{
				std::string typeName(256u, '\n');
				stringCompressor->DecodeString(&typeName[0], 256, &in);
				std::string identifier(256u, '\n');
				stringCompressor->DecodeString(&identifier[0], 256, &in);

				auto component = entity->GetComponent(typeName, identifier);
				if (!component)
				{
					component = factory->InstanceComponent(typeName);
					if (component)
						entity->AddComponent(component, identifier);
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
				auto it = components.begin(), end = components.end();
				for (++it; it != end; ++it)
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
