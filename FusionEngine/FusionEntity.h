/*
  Copyright (c) 2007 Fusion Project Team

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

#include "FusionCommon.h"

// Inherited
#include "FusionPhysicsCallback.h"
#include "FusionRefCounted.h"

// Fusion
#include "FusionSerialisedData.h"
#include "FusionEntityDeserialiser.h"
#include "FusionResourcePointer.h"

namespace FusionEngine
{

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

	class Renderable : public RefCounted
	{
		Vector2 position;
		float angle;

		ResourcePointer<CL_Sprite> sprite;

		void SetPosition(float x, float y)
		{
			position.x = x;
			position.y = y;
		}

		void SetPosition(const Vector2 &position)
		{
			this->position = position;
		}

		void SetPosition(const CL_Vec2f &_position)
		{
			this->position.x = _position.x;
			this->position.y = _position.y;
		}

		const Vector2 &GetPosition() const
		{
			return position;
		}

		void SetAngle(float angle)
		{
			this->angle = angle;
		}

		float GetAngle() const
		{
			return angle;
		}
	};

	typedef boost::intrusive_ptr<Renderable> RenderablePtr;
	typedef std::vector<RenderablePtr> RenderableArray;

	/*!
	 * \brief
	 * In game object base class
	 */
	class Entity : public RefCounted, public ICollisionHandler
	{
	public:
		//! Constructor
		Entity();
		//! Constructor. Names the Entity.
		Entity(const std::string& name);
		//! Destructor
		virtual ~Entity();

	public:
		typedef std::tr1::unordered_set<std::string> TagSet;

		//! Sets the search-name of this Entity post-hoc
		void _setName(const std::string &name);
		//! Gets the search-name of this Entity
		const std::string &GetName() const;

		//! Sets the sync ID of this Entity.
		/*!
		* Set to zero to make this a pseudo-entity, i.e. prevent this entity from sync.ing
		*/
		void SetID(ObjectID id);
		//! Returns the sync. ID of this Entity
		ObjectID GetID() const;

		//! Returns true if this Entity is a pseudo-entity - an entity which doesn't sync.
		/*!
		* Pseudo Entities have no Entity-ID, so this returns true when
		* <code>GetID() == 0</code>.
		*/
		bool IsPseudoEntity() const;

		//! Returns the typename of this entity
		virtual std::string GetType() const =0;

		//! Gets position
		virtual const Vector2 &GetPosition() =0;
		//! Gets rotation
		virtual float GetAngle() =0;

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
		void _notifyHiddenTag(const std::string &tag);

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

		void SetStreamedOut(bool is_streamed_out);
		bool IsStreamedOut() const;

		void SetPaused(bool is_paused);
		bool IsPaused() const;

		void SetHidden(bool is_hidden);
		bool IsHidden() const;

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
		virtual void Draw() =0;

		//typedef std::vector<ResourcePointer<CL_Sprite>> SpriteArray;

		//virtual const SpriteArray &GetSprites() const;

		//virtual const MeshArray &GetGeometry() const;

		virtual const RenderableArray &GetRenderables() const;
		virtual void AddRenderable(RenderablePtr renderable);
		virtual void RemoveRenderable(RenderablePtr renderable);

		//! Called after an Entity is streamed in
		virtual void OnStreamIn() =0;
		//! Called after an Entity is steamed out
		virtual void OnStreamOut() =0;

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
		*/
		virtual void DeserialiseState(const SerialisedData& state, bool local, const EntityDeserialiser &entity_deserialiser) =0;

		//! Returns a human-readable string
		virtual std::string ToString() const;

		//! Implementation of ICollisionHandler#CanCollideWith()
		//virtual bool CanCollideWith(PhysicsBodyPtr other);

		//! Implementation of ICollisionHandler#BeginContact()
		virtual void ContactBegin(const Contact& contact) {}
		//! Implementation of ICollisionHandler#ContactPersist()
		virtual void ContactPersist(const Contact &contact) {}
		//! Implementation of ICollisionHandler#ContactEnd()
		virtual void ContactEnd(const Contact &contact) {}

	protected:
		std::string m_Name;
		ObjectID m_Id;
		bool m_PseudoEntity;

		TagFlagDictionaryPtr m_TagFlagDictionary;

		TagSet m_PausedTags;
		TagSet m_HiddenTags;

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

		RenderableArray m_Renderables;

	};

}

#endif
