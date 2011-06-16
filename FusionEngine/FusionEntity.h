/*
*  Copyright (c) 2007-2011 Fusion Project Team
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

#ifndef H_FusionEntity
#define H_FusionEntity

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionRefCounted.h"

#include "FusionCommon.h"
#include "FusionEntityComponent.h"
#include "FusionCommandQueue.h"
#include "FusionEntityDeserialiser.h"
#include "FusionPhysicsCallback.h"
#include "FusionPlayerInput.h"
#include "FusionRenderable.h"
#include "FusionResourcePointer.h"
#include "FusionSerialisedData.h"
#include "FusionTagFlagDictionary.h"
#include "FusionVector2.h"

#include <boost/mpl/for_each.hpp>
#include <boost/preprocessor.hpp>

namespace FusionEngine
{
	//! Number of 'domains' - used to group entities
	static const size_t s_EntityDomainCount = 8;

	enum EntityDomains { SYSTEM_DOMAIN, GAME_DOMAIN, SYSTEM_LOCAL_DOMAIN, USER_DOMAIN };

	typedef std::vector<EntityPtr> EntityArray;

	// TODO: there could be a ComponentEntity that implements Entity (the version below)...

	/*!
	 * \brief
	 * In game object base class
	 */
	class Entity : public GarbageCollected<Entity>, noncopyable
	{
	public:
		//! Constructor
		Entity();
		//! Constructor. Names the Entity.
		Entity(const std::string& name);
		//! Destructor
		virtual ~Entity();

	public:
		//! Array of StreamedResourceUser objects
		typedef std::vector<StreamedResourceUser*> StreamedResourceArray;
		typedef std::tr1::unordered_set<std::string> TagSet;

		//! Sets the search-name of this Entity
		void _setName(const std::string &name);
		//! Gets the search-name of this Entity
		const std::string &GetName() const;

		//! Informs this entity that it has been given a default name
		void _notifyDefaultName(const std::string &name);
		//! Returns true if the name currently assigned to this Entity is default
		/*!
		* If an entity is added to the manager without a search name, it will get a default
		* search name assigned to it. Such names should not be saved to save-game files,
		* so this property allows the saved-game creator to make an informed decision.
		*/
		bool HasDefaultName() const;

		//! Sets the sync ID of this Entity.
		/*!
		* Set to zero to make this a pseudo-entity, i.e. prevent this entity from sync.ing
		*/
		void SetID(ObjectID id);
		//! Returns the sync. ID of this Entity
		ObjectID GetID() const;

		//! Sets the owner id of this entity
		void SetOwnerID(PlayerID owner);
		//! Returns the owner ID of this entity
		PlayerID GetOwnerID() const;

		void SetAuthority(PlayerID authority);
		PlayerID GetAuthority() const;

		unsigned int m_SkippedPackets;
		void PacketSkipped() { ++m_SkippedPackets; }
		void AddedToPacket() { m_SkippedPackets = 0; }
		unsigned int GetSkippedPacketsCount() const { return m_SkippedPackets; }

		//! Returns true if this Entity is a synchronised entity
		/*!
		* Synchronised entities have a unique entity-ID, so this returns
		* true when GetID() > 0
		*/
		bool IsSyncedEntity() const;
		//! Returns true if this Entity is a pseudo-entity - an entity which doesn't sync.
		bool IsPseudoEntity() const;

		//! Default implementation
		virtual void EnumReferences(asIScriptEngine *engine)
		{}
		//! Default implementation
		virtual void ReleaseAllReferences(asIScriptEngine *engine)
		{}

		//! Returns the typename of this entity
		virtual std::string GetType() const =0;

		//! Gets position
		virtual const Vector2 &GetPosition() =0;
		//! Gets angle (rotation) value
		virtual float GetAngle() const =0;

		//! Gets position
		virtual void SetPosition(const Vector2 &position) =0;
		//! Gets angle (rotation) value
		virtual void SetAngle(float angle) =0;

		// The interface type passed here determines the interface that other components must use to access the added component
		template <class Com>
		void AddComponent(std::shared_ptr<Com>& component)
		{
			static_assert(std::is_base_of<IComponent, Com>::value, "Com must be derrived from IComponent");

			ComponentInfo entry;
			entry.component = component;

			boost::mpl::for_each<Com::Interfaces>(addComInterface(m_ComponentInterfaces, component, entry.interfaces));

			for (auto it = m_Components.begin(), end = m_Components.end(); it != end; ++it)
			{
				it->component->OnSiblingAdded(entry.interfaces, entry.component);
				component->OnSiblingAdded(it->interfaces, it->component);
			}
			
			m_Components.push_back(std::move(entry));
		}

	private:
		struct ComponentInfo
		{
			std::set<std::string> interfaces;
			std::shared_ptr<IComponent> component;
		};
		std::vector<ComponentInfo> m_Components;

		typedef std::map<std::string, std::vector<std::pair<std::shared_ptr<IComponent>, std::unique_ptr<ICallQueue>>>> ComInterfaceMap;
		ComInterfaceMap m_ComponentInterfaces;

		template <class C>
		struct addComInterface
		{
			addComInterface(ComInterfaceMap& map_, std::shared_ptr<C>& component, std::set<std::string>& interface_names_)
				: map(_map),
				com(component),
				interface_names(interface_names_)
			{}

			template <class I>
			void operator() (I)
			{
				interface_names.insert(I::GetTypeName());
				map[I::GetTypeName()].push_back( std::make_pair(com, std::unique_ptr<ICallQueue>(new CallQueue<I>(com))) );
			}

			ComInterfaceMap& map;
			std::shared_ptr<C>& com;
			std::set<std::string>& interface_names;
		};

	public:

		template <class Interface>
		void InvokeOnComponent(std::function<void (std::shared_ptr<Interface>)> function)
		{
			auto _where = m_ComponentInterfaces.find(Interface::GetTypeName());
			if (_where != m_ComponentInterfaces.end())
			{
				CallQueue<Interface> *actualQueue = dynamic_cast<CallQueue<Interface>*>(_where->second);
				if (actualQueue) actualQueue->Enqueue(function);
			}
		}

		template <class Interface>
		std::shared_ptr<Interface> GetComponent()
		{
			static_assert(Interface::IsThreadSafe(), "Use InvokeOnComponent to access non-threadsafe interfaces");

			auto _where = m_ComponentInterfaces.find(Interface::GetTypeName());
			if (_where != m_ComponentInterfaces.end())
			{
				return _where->first;
			}
		}

		//! Returns renderables
		virtual RenderableArray &GetRenderables();
		//! Adds a renderable
		virtual void AddRenderable(RenderablePtr renderable);
		//! Removes the given renderable object from the Entity
		virtual void RemoveRenderable(RenderablePtr renderable);
		//! Removes renderables with the given tag
		virtual void RemoveRenderablesWithTag(const std::string &tag);

		//! Returns the cumulative AABB of this Entity's renderables
		CL_Rectf CalculateOnScreenAABB() const;

		//! Adds an object that uses a resource that should be loaded when the Entity is streamed in
		void AddStreamedResource(StreamedResourceUser * const user);
		//! Removes the given SRU
		void RemoveStreamedResource(StreamedResourceUser * const user);
		//! Sets resources that should be loaded when the Entity is streamed in
		//void SetStreamedResources(const StreamedResourceArray &resources);
		//! Returns streamed resources
		//const StreamedResourceArray &GetStreamedResources() const;

		//! Valid types for property vars.
		enum PropertyType
		{
			pt_none = 0,
			pt_bool = 1,
			pt_int8 = 2,
			pt_int16 = 3,
			pt_int32 = 4,
			pt_int64 = 5,
			pt_uint8 = 6,
			pt_uint16 = 7,
			pt_uint32 = 8,
			pt_uint64 = 9,
			pt_float = 10,
			pt_double = 11,
			// These types can be pointers (with pointer flag)
			pt_string = 13,
			pt_vector = 14,
			pt_colour = 15,
			// Type flags (define extra characteristics)
			pt_reserved_flag1 = 32,
			pt_reserved_flag2 = 64,
			pt_array_flag = 128, // Any of the above types can be stored in an array, access using GetAddressOfProperty(index, arrayIndex)
			pt_pointer_flag = 256,
			// Entity is always a pointer so there is no seperate pointer-flag version of this type ID
			pt_entity = 12 | pt_pointer_flag,
		};

		//! Returns the number of editable properties
		virtual unsigned int GetPropertyCount() const { return 0; }

		//! Returns the name of the given property
		virtual std::string GetPropertyName(unsigned int index) const { return ""; }
		//! Returns the property with the given index
		virtual const boost::any& GetPropertyValue(unsigned int index) const;
		//! Sets the given property
		virtual void SetPropertyValue(unsigned int index, const boost::any &value);
		//! Template typed GetPropertyValue
		template <typename T>
		T GetPropertyValue(unsigned int index) const
		{
			const boost::any& value = GetPropertyValue(index);
			try
			{
				return boost::any_cast<T>( value );
			}
			catch (const boost::bad_any_cast &)
			{
				throw FSN_EXCEPT(ExCode::InvalidArgument,
					"Property #" + boost::lexical_cast<std::string>( index ) + " isn't of type " + typeid(T).name());
			}
		}
		//! Returns the given property as an EntityPtr
		virtual EntityPtr GetPropertyEntity(unsigned int index, unsigned int array_index) const;
		//! Sets the given entity property
		virtual void SetPropertyEntity(unsigned int index, unsigned int array_index, const EntityPtr &entity);

		//! Returns the size of the given property if it is an array, 0 otherwise
		/*!
		* \see Entity#PropertyIsArray()
		*/
		virtual unsigned int GetPropertyArraySize(unsigned int index) const =0;
		//! Returns true if the given property is an array
		bool PropertyIsArray(unsigned int index) const { return GetPropertyArraySize(index) > 0; };
		//! Returns the type of the given property
		virtual int GetPropertyType(unsigned int index) const =0;
		//! Returns the address of the given property's data
		virtual void* GetAddressOfProperty(unsigned int index, unsigned int array_index) const =0;
		//! Returns the address of the given property's data (default for non-array types)
		void* GetAddressOfProperty(unsigned int index) const
		{
			return GetAddressOfProperty(index, 0);
		}

		//! Sets the dictionary used by the EntityManager
		/*!
		 * EntityManager objects use this to pass the current
		 * dictionary to Entities when they are added to them.
		 */
		void SetTagDictionary(TagFlagDictionaryPtr dictionary);

		//! Adds a Tag
		void AddTag(const std::string &tag);
		//! Removes a Tag
		void RemoveTag(const std::string &tag);
		//! Removes all tags
		void ClearTags();
		//! Checks whether this Entity is tagged with the given Tag
		bool CheckTag(const std::string &tag) const;

		//! Returns this entity's tags
		StringVector GetTags() const;

		void _notifyPausedTag(const std::string &tag);
		void _notifyResumedTag(const std::string &tag);
		void _notifyHiddenTag(const std::string &tag);
		void _notifyShownTag(const std::string &tag);

		const StringSet &GetPausedTags() const;
		const StringSet &GetHiddenTags() const;

		bool IsPausedByTag() const;
		bool IsHiddenByTag() const;

		//! Sets the tag-flags for this entity
		/*!
		 * This is intended for internal use.
		 * Use Entity#AddTag() unless you know you
		 * should be using this.
		 */
		void SetTagFlags(unsigned int flag);
		//! Activates the given tag-flag for this entity
		/*!
		 * This is intended for internal use.
		 * Use AddTag() unless you know you
		 * should be using this.
		 */
		void AddTagFlag(unsigned int flag);
		//! Deactivates the given tag-flag for this entity
		/*!
		 * This is intended for internal use.
		 * Use RemoveTag() unless you know you
		 * should be using this.
		 */
		void RemoveTagFlag(unsigned int flag);
		//! Returns the tag-flags for this entity
		/*!
		 * This is intended for internal use.
		 * Use CheckTag() unless you know you
		 * should be using this.
		 */
		unsigned int GetTagFlags() const;

		//! Sets the update domain for this entity
		void SetDomain(EntityDomain domain_index);
		//! Gets the update domain for this entity
		EntityDomain GetDomain() const;

		void SetLayer(size_t layer);
		size_t GetLayer() const;

		//! Returns true if this Entity has been streamed in
		bool IsStreamedIn() const;

		void SetStreamingCellIndex(unsigned int index);
		unsigned int GetStreamingCellIndex() const;

		void SetPaused(bool is_paused);
		bool IsPaused() const;

		void SetHidden(bool is_hidden);
		bool IsHidden() const;

		void SetDepth(int depth);
		int GetDepth() const;

		//! Stops the Entity from being updated for a number of steps
		void SetWait(unsigned int steps);
		//! Returns true when the waiting period is over if called once per step
		bool Wait();

		//! Marks this Entity as one that will be deleted during the next update (effectively constant time)
		void MarkToRemove();
		//! Returns true if this Entity has been marked to delete.
		bool IsMarkedToRemove() const;

		//! Used to remove the entity from the active entity list in effectively constant time
		void MarkToDeactivate();
		//! Undoes MarkToDeactivate()
		void RemoveDeactivateMark();
		//! Returns true if the Entity is marked to deactivate
		bool IsMarkedToDeactivate() const;

		//! Called when a new local player is added
		virtual void OnPlayerAdded(unsigned int local_index, PlayerID net_id) {};

		//! Returns true if the Entity has been spawned
		bool IsSpawned() const;
		//! Makes IsSpawned() return true, without calling OnSpawn()
		void SetSpawned(bool spawned);

		//! Calls OnSpawn, sets the spawned flag to true
		void Spawn();

		//! Called after the Entity is spawned
		virtual void OnSpawn() =0;
		//! Updates
		virtual void Update(float split) =0;
		//! Draws
		virtual void Draw() {}

		//virtual void UpdateRenderables();

		//! Streams in resources
		void StreamIn();
		//! Streams out resources
		void StreamOut();

		//! Called after an Entity is streamed in
		virtual void OnStreamIn() =0;
		//! Called after an Entity is steamed out
		virtual void OnStreamOut() =0;

		void _setPlayerInput(const PlayerInputPtr &player_input);

		bool InputIsActive(const std::string &input);
		float GetInputPosition(const std::string &input);

		//! Writes entity ownership info
		void SerialiseIdentity(CL_IODevice& device);
		//! Writes basic properties
		void SerialiseBasicProperties(CL_IODevice& device);

		//! Save state to buffer
		/*!
		* \param[in] local
		* Whether the state should be serialized in 'local' mode - i.e. for
		* saving game rather than network-sync.
		*/
		virtual void SerialiseState(SerialisedData &state, bool local) const =0;
		//! Read state from buffer
		/*!
		* \param[in] state
		* State data to read
		*
		* \param[in] local
		* Whether the given state is supposed to have been serialized in local mode.
		* see the local param in SerialiseState().
		*
		* \param[in] entity_deserialiser
		* Used to deserialise ObjectIDs to EntityPtrs
		*
		* \returns
		* Amount of data deserialised - useful for Entity types derived from
		* other Entities, so they know where to start reading.
		*/
		virtual size_t DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser) =0;

		//! Returns a human-readable string
		virtual std::string ToString() const;

		virtual void OnInstanceRequestFulfilled(const EntityPtr &entity) {}

		//! Implementation of ICollisionHandler#CanCollideWith()
		//virtual bool CanCollideWith(PhysicsBodyPtr other);

		//! Implementation of ICollisionHandler#BeginContact()
		virtual void ContactBegin(const Contact &contact) {}
		//! Implementation of ICollisionHandler#ContactPersist()
		virtual void ContactPersist(const Contact &contact) {}
		//! Implementation of ICollisionHandler#ContactEnd()
		virtual void ContactEnd(const Contact &contact) {}

		//! Registers the script Entity type
		static void Register(asIScriptEngine *engine);

	protected:
		std::string m_Name;
		ObjectID m_Id;
		bool m_HasDefaultName;

		// The player who owns this entity, 0 for default ownership
		//  (which falls to the arbitrator, if ownership is needed)
		//  Most entities have no owner (i.e. this will be set to 0)
		//  Where there is no owner, the authority (below) is used.
		PlayerID m_OwnerID;
		// The player who currently has authority over this entity.
		//  This is only used when OwnerID is zero - otherwise the
		//  authority is always the owner.
		PlayerID m_Authority;

		PlayerInputPtr m_PlayerInput;

		TagFlagDictionaryPtr m_TagFlagDictionary;

		StringSet m_PausedTags;
		StringSet m_HiddenTags;

		TagSet m_Tags;
		// Markers (flags) for this entity
		// If any of the true bits correspond to true bits
		// in the entity manager, this entity isn't drawn
		// or updated
		unsigned int m_Flags;

		unsigned int m_CellIndex;

		bool m_Spawned;
		bool m_StreamedIn;
		bool m_Paused;
		bool m_Hidden;
		unsigned int m_WaitStepsRemaining;
		bool m_MarkedToRemove;
		bool m_MarkedToDeactivate;

		int m_Depth;

		// EntityManager domain (1-8)
		EntityDomain m_Domain;
		// Renderer layer
		size_t m_Layer;

		RenderableArray m_Renderables;
		StreamedResourceArray m_StreamedResources;

		inline void SetStreamedIn(bool is_streamed_in);
	};

}

#endif
