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

namespace FusionEngine
{

	typedef std::tr1::shared_ptr<Entity> EntityPtr;

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

	/*!
	 * \brief
	 * In game object base class
	 */
	class Entity : public ICollisionHandler, public RefCounted
	{
	public:
		//! Constructor
		Entity();
		//! Constructor. Names the Entity.
		Entity(const std::string& name);
		//! Destructor
		~Entity();

	public:
		typedef std::tr1::unordered_set<std::string> TagSet;

		void _setName(const std::string &name);
		const std::string &GetName() const;

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
		//! Checks whether this Entity is tagged with the given Tag
		bool CheckTag(const std::string &tag) const;

		//! Returns this entity's tags
		StringVector GetTags() const;


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

		//! Marks this Entity to be deleted when the current update completes - USELESS BECAUSE NEEDS TO HAPPEN AFTER UPDATE ANYWAY (so just add to list)
		void MarkToRemove();
		//! Returns true if this Entity has been marked to delete.
		bool IsMarkedToRemove() const;

		//! Updates
		virtual void Update() =0;
		//! Draws
		virtual void Draw() =0;

		virtual std::string SerializeState() const =0;
		virtual void DeserializeState(const std::string& state) =0;

		//! Returns a human-readable string
		virtual std::string ToString() const;

		//! Implementation of ICollisionHandler#CanCollideWith()
		//virtual bool CanCollideWith(PhysicsBodyPtr other);

		//! Implementation of ICollisionHandler#BeginContact()
		virtual void ContactBegin(const Contact& contact);

	protected:
		std::string m_Name;

		TagFlagDictionaryPtr m_TagFlagDictionary;

		TagSet m_Tags;
		// Markers (flags) for this entity
		// If any of the true bits correspond to true bits
		// in the entity manager, this entity isn't drawn
		// or updated
		unsigned int m_Flags;

		bool m_MarkedToRemove;

	};

}

#endif
