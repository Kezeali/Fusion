/*
  Copyright (c) 2007-2009 Fusion Project Team

  This software is provided 'as-is', without any express or implied warranty.
	In noevent will the authors be held liable for any damages arising from the
	use of this software.

  Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
		claim that you wrote the original software. If you use this software in a
		product, an acknowledgment in the product documentation would be
		appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not
		be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source distribution.


	File Author(s):

		Elliot Hayward

*/

#ifndef Header_FusionEngine_Entity
#define Header_FusionEngine_Entity

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionPrerequisites.h"

#include "FusionRefCounted.h"
#include "FusionStreamedResourceUser.h"

#include "FusionCommon.h"
#include "FusionEntityDeserialiser.h"
#include "FusionPhysicsCallback.h"
#include "FusionPlayerInput.h"
#include "FusionResourcePointer.h"
#include "FusionSerialisedData.h"
#include "FusionVector2.h"

#include <boost/any.hpp>

#include <ClanLib/Display/2D/sprite.h>


namespace FusionEngine
{

	//! Number of 'domains' (containers) which can be used to store entities
	static const size_t s_EntityDomainCount = 8;

	typedef unsigned char EntityDomain;

	enum EntityDomains { SYSTEM_DOMAIN, GAME_DOMAIN, TEMP_DOMAIN, USER_DOMAIN };

	typedef std::vector<EntityPtr> EntityArray;

	class TagFlagDictionary
	{
	public:
		//! Adds the given tag to the given entity
		/*!
		* \returns True if a flag was given to the tag
		*/
		bool AddTag(const std::string &tag, EntityPtr to_entity, bool add_flag);
		//! Removes the given tag from the given entity
		void RemoveTag(const std::string &tag, EntityPtr from_entity);

		//! Defines a flag for the given tag if one is available
		bool RequestFlagFor(const std::string &tag);
		//! Defines a flag for the given tag, taking one from another tag if necessary
		void ForceFlagFor(const std::string &tag);

		//! Returns the flag currently defined for the given tag
		unsigned int GetFlagFor(const std::string &tag);

	private:
		//! Updates the free flags state
		/*!
		* Removes the current m_MinFreeFlag from the m_FreeFlags mask and
		* sets m_MinFreeFlag to the next free flag.
		*/
		void takeMinFreeFlag();

		//! Called when a flag becomes free
		void flagFreed(unsigned int flag);

		struct Entry
		{
			std::string Tag;
			unsigned int Flag;
			std::tr1::unordered_set<std::string> References;
		};
		// Could use a boost::multi_index_container here - i.e. index by
		//  unique Tag AND Flag - but that's too much of a hassle and I
		//  doubt it'd be worth it.
		typedef std::tr1::unordered_map<std::string, Entry> TagFlagMap;
		TagFlagMap m_Entries;

		unsigned int m_MinFreeFlag;
		unsigned int m_FreeFlags;
	};

	typedef std::tr1::shared_ptr<TagFlagDictionary> TagFlagDictionaryPtr;

	typedef std::tr1::unordered_set<std::string> TagStringSet;
	
	//! A thing that can be drawn by the renderer
	class Renderable : public StreamedResourceUser
	{
	public:
		Renderable();
		Renderable(ResourceManager *res_man, const std::string &sprite_path, int priority);
		virtual ~Renderable();

		void SetTags(const TagStringSet &tags);
		void AddTag(const std::string &tag);
		void RemoveTag(const std::string &tag);
		bool HasTag(const std::string &tag) const;

		void SetAlpha(float _alpha);
		float GetAlpha() const;

		void SetColour(unsigned int r, unsigned int g, unsigned int b);

		void SetColour(const CL_Color & colour);
		const CL_Color &GetColour() const;

		void SetPosition(float x, float y);
		void SetPosition(const Vector2 &position);
		void SetPosition(const CL_Vec2f &_position);
		const Vector2 &GetPosition() const;

		void SetAngle(float angle);
		float GetAngle() const;

		void SetDepth(int depth);
		int GetDepth() const;

		void SetEnabled(bool enabled);
		bool IsEnabled() const;

		const CL_Rectf &GetAABB() const;

		void UpdateAABB();

		//void SetSpriteResource(ResourceManager *res_man, const std::string &path);
		ResourcePointer<CL_Sprite> &GetSpriteResource();

		void OnResourceLoad(ResourceDataPtr resource);
		//void OnStreamIn();
		void OnStreamOut();

		void Draw(CL_GraphicContext &gc);
		//! Draw the renderable at a position other than that of it's owning Entity
		void Draw(CL_GraphicContext &gc, const Vector2 &origin);

		void StartAnimation();
		void StopAnimaion();
		void PauseAnimation();

		bool IsPaused() const;

		static void Register(asIScriptEngine *engine);

	protected:
		bool m_Enabled;
		bool m_Paused;

		Vector2 m_Position;
		float m_Angle;
		float m_Alpha;
		CL_Color m_Colour;

		bool m_PositionChanged;

		Vector2 m_DerivedPosition;
		float m_DerivedAngle;

		CL_Rectf m_AABB;

		int m_Depth;

		ResourcePointer<CL_Sprite> m_Sprite;

		int m_PreviousWidth, m_PreviousHeight;
		CL_Angle m_PreviousAngle;

		TagStringSet m_Tags;
	};

	typedef boost::intrusive_ptr<Renderable> RenderablePtr;
	typedef std::vector<RenderablePtr> RenderableArray;


	struct InstancePrepDefinition
	{
		std::string Type;
		unsigned int Count;
		// Copy the owner ID from the origin entity 
		//  (the entity that creates the instance)
		bool CopyOwner;
	};

	typedef std::vector<InstancePrepDefinition> InstancesToPrepareArray;

	/*!
	 * \brief
	 * In game object base class
	 */
	class Entity : public /*RefCounted*/GarbageCollected<Entity>, noncopyable
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
		typedef std::vector<StreamedResourceUserPtr> StreamedResourceArray;
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
		void SetOwnerID(ObjectID owner);
		//! Returns the owner ID of this entity
		ObjectID GetOwnerID() const;

		void SetAuthority(ObjectID authority);
		ObjectID GetAuthority() const;

		unsigned int m_SkippedPackets;
		void PacketSkipped() { ++m_SkippedPackets; }
		void AddedToPacket() { m_SkippedPackets = 0; }
		unsigned int GetSkippedPacketsCount() const { return m_SkippedPackets; }

		//! Returns true if this Entity is a pseudo-entity - an entity which doesn't sync.
		/*!
		* Pseudo Entities have no Entity-ID, so this returns true when
		* <code>GetID() == 0</code>.
		*/
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
		//! Gets linear velocity
		virtual const Vector2 &GetVelocity() =0;
		//! Gets angular (rotational) velocity
		virtual float GetAngularVelocity() const =0;

		//! Gets position
		virtual void SetPosition(const Vector2 &position) =0;
		//! Gets angle (rotation) value
		virtual void SetAngle(float angle) =0;
		//! Gets linear velocity
		virtual void SetVelocity(const Vector2 &velocity) =0;
		//! Gets angular (rotational) velocity
		virtual void SetAngularVelocity(float angular_velocity) =0;

		//! Returns renderables
		virtual RenderableArray &GetRenderables();
		//! Adds a renderable
		virtual void AddRenderable(RenderablePtr renderable);
		//! Removes renderables
		virtual void RemoveRenderable(RenderablePtr renderable);

		//! Sets resources that should be loaded when the Entity is streamed in
		void SetStreamedResources(const StreamedResourceArray &resources);
		//! Adds a resource that should be loaded when the Entity is streamed in
		void AddStreamedResource(const StreamedResourceUserPtr &resource);
		//! Returns streamed resources
		const StreamedResourceArray &GetStreamedResources() const;

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
		virtual boost::any GetPropertyValue(unsigned int index) const;
		//! Sets the given property
		virtual void SetPropertyValue(unsigned int index, const boost::any &value);
		//! Template typed GetPropertyValue
		template <typename T>
		T GetPropertyValue(unsigned int index) const
		{
			boost::any value = GetPropertyValue(index);
			try
			{
				return boost::any_cast<T>( value );
			}
			catch (const boost::bad_any_cast &)
			{
				throw FSN_EXCEPT(ExCode::InvalidArgument, "Entity::GetPropertyValue<" + typeid(T).name() + ">",
					"Property #" + boost::lexical_cast<std::string>( index ) + " isn't of type " + typeid(T).name());
			}
		}
		//! Returns the given property as an EntityPtr
		virtual EntityPtr GetPropertyEntity(unsigned int index) const;
		//! Sets the given entity property
		virtual void SetPropertyEntity(unsigned int index, const EntityPtr &entity);

		//! Returns the size of the given property if it is an array
		virtual unsigned int GetPropertyArraySize(unsigned int index) const =0;
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

		void _setDomain(EntityDomain domain_index);
		EntityDomain GetDomain() const;

		void SetLayer(size_t layer);
		size_t GetLayer() const;

		void SetStreamedIn(bool is_streamed_in);
		bool IsStreamedOut() const;

		void SetPaused(bool is_paused);
		bool IsPaused() const;

		void SetHidden(bool is_hidden);
		bool IsHidden() const;

		void SetDepth(int depth);
		int GetDepth() const;

		void SetWait(unsigned int steps);
		bool Wait();

		//! Marks this Entity as one that will be deleted when the update completes
		void MarkToRemove();
		//! Returns true if this Entity has been marked to delete.
		bool IsMarkedToRemove() const;

		//! Spawns
		virtual void Spawn() =0;
		//! Updates
		virtual void Update(float split) =0;
		//! Draws
		virtual void Draw() {}

		//virtual void UpdateRenderables();

		void StreamIn();
		void StreamOut();

		//! Called after an Entity is streamed in
		virtual void OnStreamIn() =0;
		//! Called after an Entity is steamed out
		virtual void OnStreamOut() =0;

		void _setPlayerInput(const PlayerInputPtr &player_input);

		bool InputIsActive(const std::string &input);
		float GetInputPosition(const std::string &input);

		void DefineInstanceToPrepare(const std::string &type, unsigned int count, bool copy_owner);
		const InstancesToPrepareArray &GetInstancesToPrepare() const;

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
		ObjectID m_OwnerID;
		// The player who currently has authority over this entity.
		//  This is only used when OwnerID is zero - otherwise the
		//  authority is always the owner.
		ObjectID m_Authority;

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

		bool m_StreamedOut;
		bool m_Paused;
		bool m_Hidden;
		unsigned int m_WaitStepsRemaining;
		bool m_MarkedToRemove;

		int m_Depth;

		// EntityManager domain (1-8)
		EntityDomain m_Domain;
		// Renderer layer
		size_t m_Layer;

		RenderableArray m_Renderables;
		StreamedResourceArray m_StreamedResources;

		InstancesToPrepareArray m_InstancesToPrepare;

	};

}

#endif
