/*
*  Copyright (c) 2011-2012 Fusion Project Team
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

#include "PrecompiledHeaders.h"

#include "FusionEntitySerialisationUtils.h"

#include "FusionArchetypalEntityManager.h"
#include "FusionArchetypeFactory.h"
#include "FusionEntity.h"
#include "FusionEntityComponent.h"
#include "FusionComponentFactory.h"
#include "FusionEntityManager.h"
#include "FusionEntityInstantiator.h"
#include "FusionResourceManager.h"

#include "FusionBinaryStream.h"

#include <BitStream.h>
#include <StringCompressor.h>

#include <boost/crc.hpp>

#include <functional>

using namespace FusionEngine::IO::Streams;

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
				RakNet::BitStream::ReverseBytes(reinterpret_cast<unsigned char*>(&value), output, sizeof(IntegerT));
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
		if (in.ReadBit())
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
			outValue = 0;

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

		void StdSerialisePosition(RakNet::BitStream& stream, const Vector2& pos, const Vector2& origin, float radius)
		{
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

		bool SerialiseEntity(RakNet::BitStream& out, const EntityPtr& entity, SerialiseMode mode)
		{
			bool dataWritten = false;

			//entity->SerialiseReferencedEntitiesList(out);

			auto compressor = RakNet::StringCompressor::Instance();

			auto& components = entity->GetComponents();
			size_t numComponents = components.size() - 1; // minus transform

			auto transform = dynamic_cast<EntityComponent*>(entity->GetTransform().get());
			{
				std::string type = transform->GetType();
				compressor->EncodeString(type.c_str(), type.length() + 1, &out);
				
				RakNet::BitStream tempStream;
				transform->SerialiseContinuous(tempStream);
				bool conData = tempStream.GetNumberOfBitsUsed() > 0;
				//FSN_ASSERT(conData || tempStream.GetNumberOfBitsUsed() == 0);
				if (!conData)
					tempStream.Reset();

				const auto bitsUsedBeforeWriting = tempStream.GetNumberOfBitsUsed();
				transform->SerialiseOccasional(tempStream);
				bool occData = tempStream.GetNumberOfBitsUsed() > bitsUsedBeforeWriting;
				//FSN_ASSERT(occData || tempStream.GetNumberOfBitsUsed() == bitsUsedBeforeWriting);

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
					component->SerialiseContinuous(tempStream);
					bool conData = tempStream.GetNumberOfBitsUsed() > 0;
					FSN_ASSERT(conData || tempStream.GetNumberOfBitsUsed() == 0);
					if (!conData)
						tempStream.Reset();

					const auto bitsUsedBeforeWriting = tempStream.GetNumberOfBitsUsed();
					component->SerialiseOccasional(tempStream);
					bool occData = tempStream.GetNumberOfBitsUsed() > bitsUsedBeforeWriting;

					out.Write(conData);
					out.Write(occData);
					out.Write(tempStream);

					dataWritten |= (conData || occData);
				}
			}

			return dataWritten;
		}

		EntityPtr DeserialiseEntity(RakNet::BitStream& in, ComponentFactory* factory, EntityManager* manager)
		{
			EntityPtr entity;

			auto mode = SerialiseMode::All;

			//entity->DeserialiseReferencedEntitiesList(in, EntityDeserialiser(manager));

			auto stringCompressor = RakNet::StringCompressor::Instance();

			ComponentPtr transform;
			{
				char typeNameBuf[256u];
				stringCompressor->DecodeString(&typeNameBuf[0], 256, &in);
				std::string typeName(typeNameBuf);

				//FSN_ASSERT(entity->GetTransform()->GetType() == typeName);
				//transform = entity->GetTransform();

				transform = factory->InstantiateComponent(typeName);

				bool conData = in.ReadBit();
				bool occData = in.ReadBit();
				if (conData)
					transform->DeserialiseContinuous(in);
				if (occData)
					transform->DeserialiseOccasional(in);
			}

			transform->SynchronisePropertiesNow();

			entity = std::make_shared<Entity>(manager, transform);

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
					component = factory->InstantiateComponent(typeName);
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
						component->DeserialiseOccasional(in);
				}
			}

			return entity;
		}

		bool WriteStateWithLength(RakNet::BitStream& out, RakNet::BitStream& state)
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
					FSN_EXCEPT(SerialisationError, "Serialised state for is too large");
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

		bool SerialiseContinuous(RakNet::BitStream& out, const EntityPtr& entity, SerialiseMode mode)
		{
			bool dataWritten = false;

			//entity->SerialiseReferencedEntitiesList(out);

			auto compressor = RakNet::StringCompressor::Instance();

			auto lock = entity->GetComponentLock(false);

			auto& components = entity->GetComponents();
			size_t numComponents = components.size() - 1; // minus transform

			auto transformComponent = entity->GetTransform();
			{
				Vector2 origin; float radius = 0.f;
				auto transform = dynamic_cast<ITransform*>(entity->GetTransform().get());
				if (transform->HasContinuousPosition())
				{
					out.Write1();
					StdSerialisePosition(out, transform->GetPosition(), origin, radius);
				}
				else
					out.Write0();

				RakNet::BitStream tempStream;
				transformComponent->SerialiseContinuous(tempStream);
				dataWritten = WriteStateWithLength(out, tempStream);
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

					dataWritten |= WriteStateWithLength(out, tempStream);
				}
			}

			return dataWritten;
		}

		void DeserialiseContinuous(RakNet::BitStream& in, const EntityPtr& entity, SerialiseMode mode)
		{
			//entity->DeserialiseReferencedEntitiesList(in, EntityDeserialiser(manager));

			auto stringCompressor = RakNet::StringCompressor::Instance();

			auto lock = entity->GetComponentLock(false);

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

					if ((in.GetReadOffset() - startPos) != stateLength)
						FSN_EXCEPT(Exception, "Component read too much/little data");
				}
			}

			auto& existingComponents = entity->GetComponents();

			size_t numComponents;
			in.Read(numComponents);
			// TEMP - just ignore states for incomplete entities (TODO: synchronise entities properly)
			if (numComponents != existingComponents.size() - 1) { SendToConsole("Warning: received partial entity"); return; }
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
					{
						const auto startPos = in.GetReadOffset();

						component->DeserialiseContinuous(in);

						if ((in.GetReadOffset() - startPos) != stateLength)
							FSN_EXCEPT(Exception, "Component read too much/little data");
					}
				}
			}
		}

		bool SerialiseComponentOccasional(RakNet::BitStream& out, uint32_t& storedChecksum, EntityComponent* component, SerialiseMode mode)
		{
			RakNet::BitStream tempStream;
			component->SerialiseOccasional(tempStream);

			bool conData = tempStream.GetNumberOfBitsUsed() > 0;

			if (conData)
			{
				boost::crc_32_type crc;
				crc.process_bytes(tempStream.GetData(), tempStream.GetNumberOfBytesUsed());
				uint32_t checksum = crc.checksum();

				if (mode == SerialiseMode::Changes)
				{
					conData = checksum != storedChecksum; // Ignore unchanged states
				}
				storedChecksum = checksum;
			}

			WriteStateWithLength(out, tempStream);

			return conData;
		}

		bool SerialiseOccasional(RakNet::BitStream& out, std::vector<uint32_t>& checksums, const EntityPtr& entity, SerialiseMode mode)
		{
			bool dataWritten = false;

			//entity->SerialiseReferencedEntitiesList(out);

			auto compressor = RakNet::StringCompressor::Instance();

			auto lock = entity->GetComponentLock(false);

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
				if (!transform->HasContinuousPosition())
				{
					out.Write1();
					StdSerialisePosition(out, transform->GetPosition(), origin, radius);
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

		void DeserialiseComponentOccasional(RakNet::BitStream& in, uint32_t& checksum, EntityComponent* component)
		{
			//const auto stateLength = ReadStateLength(in);
			//if (stateLength)
			{
				auto start = in.GetReadOffset();
				component->DeserialiseOccasional(in);

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

		void DeserialiseOccasional(RakNet::BitStream& in, std::vector<uint32_t>& checksums, const EntityPtr& entity, SerialiseMode mode)
		{
			//entity->DeserialiseReferencedEntitiesList(in, EntityDeserialiser(manager));

			auto stringCompressor = RakNet::StringCompressor::Instance();

			auto lock = entity->GetComponentLock(false);

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

		static void MergeComponentData(ICellStream& instr, OCellStream& outstr, RakNet::BitStream& continuous, RakNet::BitStream& occasional)
		{
			CellStreamReader in(&instr);
			CellStreamWriter out(&outstr);

			auto existingConDataBytes = in.ReadValue<uint16_t>();
			auto existingOccDataBytes = in.ReadValue<uint16_t>();

			auto conDataBits = ReadStateLength(continuous);
			RakNet::BitStream tempStream;
			if (conDataBits > 0)
			{
				continuous.Read(tempStream, conDataBits);

				out.WriteAs<uint16_t>(tempStream.GetNumberOfBytesUsed());
				outstr.write(reinterpret_cast<char*>(tempStream.GetData()), tempStream.GetNumberOfBytesUsed());
				
				tempStream.Reset();
			}
			else
			{
				std::vector<char> tempData(existingConDataBytes);
				instr.read(tempData.data(), existingConDataBytes);

				out.WriteAs<uint16_t>(existingConDataBytes);
				outstr.write(tempData.data(), existingConDataBytes);
			}

			auto occDataBits = ReadStateLength(occasional);
			if (occDataBits > 0)
			{
				occasional.Read(tempStream, occDataBits);

				out.WriteAs<uint16_t>(tempStream.GetNumberOfBytesUsed());
				outstr.write(reinterpret_cast<char*>(tempStream.GetData()), tempStream.GetNumberOfBytesUsed());
			}
			else
			{
				std::vector<char> tempData(existingOccDataBytes);
				instr.read(tempData.data(), existingOccDataBytes);

				out.WriteAs<uint16_t>(existingOccDataBytes);
				outstr.write(reinterpret_cast<char*>(tempData.data()), existingOccDataBytes);
			}
		}

		static bool WritePositionDataIfAvailable(OCellStream& outstr, RakNet::BitStream& incomming)
		{
			CellStreamWriter out(&outstr);

			auto result = DeserialisePosition(incomming, Vector2(), 0);
			if (result.first)
			{
				RakNet::BitStream tempStream;
				StdSerialisePosition(tempStream, result.second, Vector2(), 0);

				out.Write(tempStream.GetNumberOfBytesUsed());
				outstr.write(reinterpret_cast<char*>(tempStream.GetData()), tempStream.GetNumberOfBytesUsed());
			}
			
			return result.first;
		}

		std::streamsize MergeEntityData(ICellStream& instr, OCellStream& outstr, RakNet::BitStream& incomming_c, RakNet::BitStream& incomming_o)
		{
			CellStreamReader in(&instr);
			CellStreamWriter out(&outstr);

			auto owner = in.ReadValue<PlayerID>();
			out.Write(owner);

			auto name = in.ReadString();
			out.WriteString(name);

			// Copy transform component type name
			std::string transformType = in.ReadString();
			out.WriteString(transformType);

			// Write position (transform component)
			if (!WritePositionDataIfAvailable(outstr, incomming_c) && !WritePositionDataIfAvailable(outstr, incomming_o))
			{
				// No new position to insert: copy the old data
				auto len = in.ReadValue<RakNet::BitSize_t>();
				out.Write(len);
				std::vector<char> buffer(len);
				instr.read(buffer.data(), buffer.size());
				outstr.write(buffer.data(), buffer.size());
			}

			// Merge transform component data
			MergeComponentData(instr, outstr, incomming_c, incomming_o);

			// Copy the component count
			size_t numComponents;
			in.Read(numComponents);
			out.Write(numComponents);

			// Copy the other component names
			for (size_t i = 0; i < numComponents; ++i)
			{
				std::string transformType = in.ReadString();
				out.WriteString(transformType);
			}

			// Merge the other component data
			for (size_t i = 0; i < numComponents; ++i)
			{
				MergeComponentData(instr, outstr, incomming_c, incomming_o);
			}

			return 0;//fileWrapper.totalbytes();
		}

		void CopyEntityData(ICellStream& instr, OCellStream& outstr)
		{
			FSN_EXCEPT(NotImplementedException, "Decided not to use this, so haven't implmented it (this is basically just the code from MergeEntityData)");

			RakNet::BitStream incomming_c;
			RakNet::BitStream incomming_o;

			CellStreamReader in(&instr);
			CellStreamWriter out(&outstr);

			// Copy transform component type name
			std::string transformType = in.ReadString();
			out.WriteString(transformType);

			// Merge transform component data
			MergeComponentData(instr, outstr, incomming_c, incomming_o);

			// Copy the component count
			size_t numComponents;
			in.Read(numComponents);
			out.Write(numComponents);

			// Copy the other (non-transform) component names
			for (size_t i = 0; i < numComponents; ++i)
			{
				std::string transformType = in.ReadString();
				out.WriteString(transformType);
			}

			// Merge the other component data
			for (size_t i = 0; i < numComponents; ++i)
			{
				MergeComponentData(instr, outstr, incomming_c, incomming_o);
			}
		}

		void WriteComponent(OCellStream& outstr, EntityComponent* component, SerialisedDataStyle data_style)
		{
			FSN_ASSERT(component);

			CellStreamWriter out(&outstr);

			if (data_style == FastBinary)
			{
				RakNet::BitStream stream;
				component->SerialiseContinuous(stream);
				if (stream.GetWriteOffset() > 0)
				{
					out.Write(stream.GetNumberOfBytesUsed());
					outstr.write(reinterpret_cast<const char*>(stream.GetData()), stream.GetNumberOfBytesUsed());
				}
				else
					out.WriteAs<RakNet::BitSize_t>(0);
				stream.Reset();

				component->SerialiseOccasional(stream);
				if (stream.GetWriteOffset() > 0)
				{
					out.Write(stream.GetNumberOfBytesUsed());
					outstr.write(reinterpret_cast<const char*>(stream.GetData()), stream.GetNumberOfBytesUsed());
				}
				else
					out.WriteAs<RakNet::BitSize_t>(0);
			}
			else
			{
				RakNet::BitStream stream;
				component->SerialiseEditable(stream);
				if (stream.GetWriteOffset() > 0)
				{
					out.Write(stream.GetNumberOfBytesUsed());
					outstr.write(reinterpret_cast<const char*>(stream.GetData()), stream.GetNumberOfBytesUsed());
				}
				else
					out.WriteAs<RakNet::BitSize_t>(0);
			}
		}

		void ReadComponent(ICellStream& instr, EntityComponent* component, SerialisedDataStyle data_style)
		{
			FSN_ASSERT(component);

			CellStreamReader in(&instr);

			if (data_style == FastBinary)
			{
				const auto conDataLen = in.ReadValue<RakNet::BitSize_t>();
				std::vector<char> data(conDataLen);
				if (conDataLen > 0)
				{
					instr.read(data.data(), conDataLen);

					RakNet::BitStream stream(reinterpret_cast<unsigned char*>(data.data()), data.size(), false);

					component->DeserialiseContinuous(stream);

					if (stream.GetNumberOfUnreadBits() >= 8)
						SendToConsole("Not all serialised data was used when reading a " + component->GetType());
				}

				const auto occDataLen = in.ReadValue<RakNet::BitSize_t>();
				if (occDataLen > 0)
				{
					data.resize(occDataLen);
					instr.read(data.data(), occDataLen);

					RakNet::BitStream stream(reinterpret_cast<unsigned char*>(data.data()), data.size(), false);

					component->DeserialiseOccasional(stream);

					if (stream.GetNumberOfUnreadBits() >= 8)
						SendToConsole("Not all serialised data was used when reading a " + component->GetType());
				}
			}
			else
			{
				const auto dataLen = in.ReadValue<RakNet::BitSize_t>();
				std::vector<char> data(dataLen);
				if (dataLen > 0)
				{
					instr.read(data.data(), dataLen);

					RakNet::BitStream stream(reinterpret_cast<unsigned char*>(data.data()), data.size(), false);

					component->DeserialiseEditable(stream);

					if (stream.GetNumberOfUnreadBits() >= 8)
						SendToConsole("Not all serialised data was used when reading a " + component->GetType() + " (editable mode)");
				}
			}

			component->SynchronisePropertiesNow();
		}

		//{
		//	RakNet::BitStream stream;
		//	entity->SerialiseReferencedEntitiesList(stream);

		//	out.write_uint32(stream.GetNumberOfBytesUsed());
		//	out.write(stream.GetData(), stream.GetNumberOfBytesUsed());
		//}

		void SaveEntity(OCellStream& outstr, EntityPtr entity, bool id_included, SerialisedDataStyle data_style)
		{
			CellStreamWriter out(&outstr);

			if (id_included)
			{
				ObjectID id = entity->GetID();
				out.Write(id);
			}

			out.Write(entity->GetOwnerID());

			if (!entity->HasDefaultName())
				out.WriteString(entity->GetName());
			else
				out.WriteString("");

			out.Write(entity->IsTerrain());

			const bool archetypal = entity->IsArchetypal();
			if (archetypal)
			{
				// Write the archetype identifier
				out.WriteString(entity->GetArchetype());
			}
			else
				out.WriteString("");

			auto lock = entity->GetComponentLock(false);

			auto& components = entity->GetComponents();
			size_t numComponents = components.size() - 1; // - transform

			// Synchronise all the properties of each component to make sure the most recent state is
			//  saved (important when, for example, an entity is loaded and immediately saved without
			//  ever being activated)
			for (auto it = components.begin(); it != components.end(); ++it)
				(*it)->SynchronisePropertiesNow();

			// Write the entity position (whether or not it is archetypal)
			auto tfComponent = entity->GetTransform().get();
			auto transform = dynamic_cast<ITransform*>(tfComponent);
			if (!archetypal)
				out.WriteString(tfComponent->GetType());
			// Write position
			{
				RakNet::BitStream tempStream;
				StdSerialisePosition(tempStream, transform->GetPosition(), Vector2(), 0);
				out.Write(tempStream.GetNumberOfBytesUsed());
				outstr.write(reinterpret_cast<char*>(tempStream.GetData()), tempStream.GetNumberOfBytesUsed());
			}

			if (archetypal)
			{
				// Write the agent data
				auto agent = entity->GetArchetypeAgent();
				RakNet::BitStream tempStream;
				agent->Serialise(tempStream);
				out.Write(tempStream.GetNumberOfBytesUsed());
				outstr.write(reinterpret_cast<char*>(tempStream.GetData()), tempStream.GetNumberOfBytesUsed());
			}
			// Write a unique entity
			else
			{
				WriteComponent(outstr, tfComponent, data_style);
			
				out.Write(numComponents);
				for (auto it = components.begin(), end = components.end(); it != end; ++it)
				{
					auto& component = *it;
					if (component.get() != tfComponent)
					{
						out.WriteString(component->GetType());
						out.WriteString(component->GetIdentifier());
					}
				}
				for (auto it = components.begin(), end = components.end(); it != end; ++it)
				{
					auto& component = *it;
					if (component.get() != tfComponent)
					{
						WriteComponent(outstr, component.get(), data_style);
					}
				}
			}
		}

		class DoNothingEntityFuture : public EntityFuture
		{
		public:
			DoNothingEntityFuture(std::shared_ptr<ICellStream>&& file, const EntityPtr& entity) : m_Stream(file), m_Entity(entity) {}

			EntityPtr get_entity() { return std::move(m_Entity); }

			std::shared_ptr<ICellStream> get_file() { return std::move(m_Stream); }

			std::pair<EntityPtr, std::shared_ptr<ICellStream>> get() { return std::make_pair(std::move(m_Entity), std::move(m_Stream)); }

			bool is_ready() const { return true; }

		private:
			std::shared_ptr<ICellStream> m_Stream;
			EntityPtr m_Entity;
		};

		class ArchetypalEntityFuture : public EntityFuture
		{
		public:
			ArchetypalEntityFuture(std::shared_ptr<ICellStream>&& cell_data_stream, const std::string& archetype_id, const std::function<EntityPtr (const std::shared_ptr<ICellStream>&, const ResourceDataPtr&)>& finalise);

			void OnArchetypeLoaded(ResourceDataPtr resource);

			EntityPtr get_entity();

			std::shared_ptr<ICellStream> get_file();

			std::pair<EntityPtr, std::shared_ptr<ICellStream>> get();

			bool is_ready() const;

		private:
			ResourceDataPtr m_Resource;
			std::shared_ptr<ICellStream> m_Stream;
			std::function<EntityPtr (const std::shared_ptr<ICellStream>&, const ResourceDataPtr&)> m_FinaliseFn;

			std::shared_ptr<boost::signals2::scoped_connection> m_ResourceLoaderCnx;

			CL_Event m_GotResult;
		};

		ArchetypalEntityFuture::ArchetypalEntityFuture(std::shared_ptr<ICellStream>&& cell_data_stream, const std::string& archetype_id, const std::function<EntityPtr (const std::shared_ptr<ICellStream>&, const ResourceDataPtr&)>& finalise)
			: m_Stream(std::move(cell_data_stream)),
			m_FinaliseFn(finalise),
			m_GotResult(true, false)
		{
			m_ResourceLoaderCnx = std::make_shared<boost::signals2::scoped_connection>(
				ResourceManager::getSingleton().GetResource("ArchetypeFactory", archetype_id, std::bind(&ArchetypalEntityFuture::OnArchetypeLoaded, this, std::placeholders::_1)));
		}

		void ArchetypalEntityFuture::OnArchetypeLoaded(ResourceDataPtr resource)
		{
			m_Resource = resource;
			m_GotResult.set();
		}

		EntityPtr ArchetypalEntityFuture::get_entity()
		{
			CL_Event::wait(m_GotResult);

			if (m_Resource)
				return m_FinaliseFn(m_Stream, m_Resource);
			else
				return EntityPtr();
		}

		std::shared_ptr<ICellStream> ArchetypalEntityFuture::get_file()
		{
			CL_Event::wait(m_GotResult);

			if (m_Resource)
				return std::move(m_Stream);
			else
				return std::move(m_Stream);
		}

		std::pair<EntityPtr, std::shared_ptr<ICellStream>> ArchetypalEntityFuture::get()
		{
			CL_Event::wait(m_GotResult);

			if (m_Resource)
			{
				auto entity = m_FinaliseFn(m_Stream, m_Resource);
				return std::make_pair(std::move(entity), std::move(m_Stream));
			}
			else
				return std::make_pair(EntityPtr(), std::move(m_Stream));
		}

		bool ArchetypalEntityFuture::is_ready() const
		{
			return (bool)m_Resource;
		}

		std::pair<EntityPtr, std::shared_ptr<ICellStream>> LoadEntityImmeadiate(std::shared_ptr<ICellStream>&& in, bool id_included, ObjectID override_id, SerialisedDataStyle data_style, ComponentFactory* factory, EntityManager* manager, EntityInstantiator* synchroniser)
		{
			return LoadEntity(std::move(in), id_included, override_id, data_style, factory, manager, synchroniser)->get();
		}

		std::shared_ptr<EntityFuture> LoadEntity(std::shared_ptr<ICellStream>&& instr, bool id_included, ObjectID override_id, SerialisedDataStyle data_style, ComponentFactory* factory, EntityManager* manager, EntityInstantiator* instantiator)
		{
			FSN_ASSERT(factory);
			FSN_ASSERT(manager);
			FSN_ASSERT(instantiator);

			CellStreamReader in(instr.get());

			// Load entity info
			ObjectID id = 0;
			if (id_included)
			{
				in.Read(id);
				instantiator->TakeID(id);
			}
			else if (override_id != 0)
			{
				id = override_id;
				instantiator->TakeID(id);
			}

			PlayerID owner = 0;
			in.Read(owner);

			auto name = in.ReadString();

			bool terrain = false;
			in.Read(terrain);

			// Check for an archetype
			std::string archetypeId = in.ReadString();
			const bool isArchetypal = !archetypeId.empty();

			if (isArchetypal)
			{
				return std::make_shared<ArchetypalEntityFuture>(std::move(instr), archetypeId,
					[id, owner, name, terrain, data_style, factory, manager, instantiator](const std::shared_ptr<ICellStream>& stream, const ResourceDataPtr& resource)->EntityPtr
				{
					auto archetypeFactory = static_cast<ArchetypeFactory*>(resource->GetDataPtr());
					return LoadArchetypalEntity(*stream, archetypeFactory, id, owner, name, terrain, data_style, factory, manager, instantiator);
				});
			}
			else
			{
				return std::make_shared<DoNothingEntityFuture>(std::move(instr), LoadUniqueEntity(*instr, id, owner, name, terrain, data_style, factory, manager, instantiator));
			}
		}

		EntityPtr LoadUniqueEntity(ICellStream& instr, ObjectID id, PlayerID owner, const std::string& name, bool terrain, SerialisedDataStyle data_style, ComponentFactory* factory, EntityManager* manager, EntityInstantiator* instantiator)
		{
			FSN_ASSERT(factory);
			FSN_ASSERT(manager);

			CellStreamReader in(&instr);

			ComponentPtr transform;
			//const auto dataLen = in.read_uint32();
			//std::vector<unsigned char> referencedEntitiesData(dataLen);
			//in.read(referencedEntitiesData.data(), referencedEntitiesData.size());
			{
				std::string transformType = in.ReadString();
				transform = factory->InstantiateComponent(transformType);

				if (transformType != "StaticTransform")
				{
					auto i = transformType.length();
				}

				auto tf = dynamic_cast<ITransform*>(transform.get()); FSN_ASSERT(tf);
				auto len = in.ReadValue<RakNet::BitSize_t>();
				if (len > 0)
				{
					std::vector<char> buffer(len);
					instr.read(buffer.data(), len);
					RakNet::BitStream stream(reinterpret_cast<unsigned char*>(buffer.data()), buffer.size(), false);
					auto result = StdDeserialisePosition(stream, Vector2(), 0);
					tf->SetPosition(result);
				}
			}

			auto entity = std::make_shared<Entity>(manager, transform);

			entity->SetID(id);
			entity->SetOwnerID(owner);

			if (!name.empty())
				entity->SetName(name);

			entity->SetTerrain(terrain);

			// Read the rest of the transform data, now that the component has been initialised (by adding it to the entity)
			ReadComponent(instr, transform.get(), data_style);

			//transform->SynchronisePropertiesNow();

			//{
			//	RakNet::BitStream stream(referencedEntitiesData.data(), referencedEntitiesData.size(), false);
			//	entity->DeserialiseReferencedEntitiesList(stream, EntityDeserialiser(manager));
			//}

			size_t numComponents;
			in.Read(numComponents);
			for (size_t i = 0; i < numComponents; ++i)
			{
				std::string type = in.ReadString();
				std::string ident = in.ReadString();
				auto component = factory->InstantiateComponent(type);
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

					ReadComponent(instr, component.get(), data_style);
				}
			}

			return entity;
		}

		EntityPtr LoadArchetypalEntity(ICellStream& instr, ArchetypeFactory* archetype_factory, ObjectID id, PlayerID owner, const std::string& name, bool terrain, SerialisedDataStyle data_style, ComponentFactory* factory, EntityManager* manager, EntityInstantiator* instantiator)
		{
			FSN_ASSERT(archetype_factory);
			FSN_ASSERT(factory);
			FSN_ASSERT(manager);

			CellStreamReader in(&instr);

			Vector2 position;
			const float angle = 0.0f;

			auto len = in.ReadValue<RakNet::BitSize_t>();
			if (len > 0)
			{
				std::vector<char> buffer(len);
				instr.read(buffer.data(), len);
				RakNet::BitStream stream(reinterpret_cast<unsigned char*>(buffer.data()), buffer.size(), false);

				position = StdDeserialisePosition(stream, Vector2(), 0);
			}
			else
			{
				FSN_EXCEPT(FileSystemException, "Failed to load archetype: missing position data");
			}

			// HACK: kind of, in that this assumes FastBinary style means this is being loaded in play mode, and EditableBinary / HumanReadable mean that this entity will be used by the editor
			//  Link/unlinked mode should be a setting in ArchetypeFactoryManager, so that the archetype factories can be created with that parameter and then any instances
			//  they make can be correctly set up.
			EntityPtr entity =
				data_style != FastBinary ? archetype_factory->MakeInstance(factory, position, angle) : archetype_factory->MakeUnlinkedInstance(factory, position, angle);

			// Deserialise the archetype agent
			{
				auto agent = entity->GetArchetypeAgent();

				// Read the property overrides
				auto len = in.ReadValue<RakNet::BitSize_t>();
				if (len > 0)
				{
					std::vector<char> buffer(len);
					instr.read(buffer.data(), len);
					RakNet::BitStream stream(reinterpret_cast<unsigned char*>(buffer.data()), buffer.size(), false);

					agent->Deserialise(stream);
				}
			}

			return entity;
		}

	}
}
