/*
  Copyright (c) 2006-2007 Fusion Project Team

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

#ifndef Header_FusionEngine_ScriptingFunctions
#define Header_FusionEngine_ScriptingFunctions

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

namespace FusionEngine
{

	//! Detonates the given object
	/*!
	 * Script function:
	 * \code
	 * Detonate(uint16)
	 * \endcode
	 */
	void SCR_DetonateProjectile(ObjectID index);
	//! Generic calling convention
	/*!
	 * \see SCR_DetonateProjectile
	 */
	void SCR_DetonateProjectileG(asIScriptGeneric *gen);

	//! Applys the default thrust to the given object
	/*!
	 * Script function:
	 * \code
	 * ApplyEngineForce(uint16)
	 * \endcode
	 *
	 * \todo Store engine force such that this method can be applied directly
	 * to the physical body (so we don't have to search for the given ID - we
	 * just get the physical body corresponding to that ID from a map).
	 *
	 * \param index
	 * The index of the object
	 */
	void SCR_ApplyEngineForce(ObjectID index);
	//! Generic calling convention
	/*!
	 * \see SCR_ApplyEngineForce
	 */
	void SCR_ApplyEngineForceG(asIScriptGeneric *gen);

	//! Applys the given force to the given object
	/*!
	 * Script function:
	 * \code
	 * ApplyForce(uint16, float, float)
	 * \endcode
	 *
	 *
	 * \param index
	 * The index of the object
	 *
	 * \param x
	 * The x parameter of the force to apply
	 * \param y
	 * The y parameter of the force to apply
	 */
	void SCR_ApplyForce(ObjectID index, float x, float y);
	//! Generic calling convention
	/*!
	 * \see SCR_ApplyForce
	 */
	void SCR_ApplyForceG(asIScriptGeneric *gen);

	//! Returns the distance to the nearest ship.
	/*!
	 * <p>
	 * Returns the distance to the ship nearest the object with the given index.
	 * </p>
	 * Script function:
	 * \code
	 * GetDistanceToNearestShip(uint16)
	 * \endcode
	 *
	 *
	 * \param index
	 * The index of the object to check
	 */
	float SCR_GetDistanceToNearestShip(ObjectID index);
	//! Generic calling convention
	/*!
	 * \see SCR_GetDistanceToNearestShip
	 */
	void SCR_GetDistanceToNearestShipG(asIScriptGeneric *gen);

	//! Creates the given effect.
	/*!
	 * Script function:
	 * \code
	 * CreateEffect(string, float, float)
	 * \endcode
	 *
	 *
	 * \param name
	 * The tag of the projectile to create
	 *
	 * \param x
	 * The x coord
	 * \param y
	 * The y coord
	 */
	void SCR_CreateEffect(std::string name, float x, float y);
	//! Generic calling convention
	/*!
	 * \see SCR_CreateEffect
	 */
	void SCR_CreateEffectG(asIScriptGeneric *gen);

	//! Creates a projectile.
	/*!
	 * Script function:
	 * \code
	 * CreateProjectile(uint16, string, float, float, float)
	 * \endcode
	 *
	 *
	 * \param ship_index
	 * The index of the ship which will own this projectile
	 *
	 * \param name
	 * The tag of the projectile to create
	 *
	 * \param x
	 * The x coord
	 * \param y
	 * The y coord
	 *
	 * \param a
	 * The angle
	 */
	void SCR_CreateProjectile(ObjectID ship_index, std::string name, float x, float y, float a);
	//! Generic calling convention
	/*!
	 * \see SCR_CreateProjectile
	 */
	void SCR_CreateProjectileG(asIScriptGeneric *gen);

	//! A specialisation of CreateProjectile.
	/*!
	 * <p>
	 * Creates a projectile infront of the given ship.
	 * </p>
	 * Script function:
	 * \code
	 * FireProjectile(uint16, string)
	 * \endcode
	 *
	 *
	 * \param ship_index
	 * The index of the ship which will own the projectile and in front of which
	 * the projectile will be created.
	 *
	 * \param name
	 * The tag of the projectile to fire.
	 */
	void SCR_FireProjectile(ObjectID ship_index, std::string name);
	//! Generic calling convention
	/*!
	 * \see SCR_FireProjectile
	 */
	void SCR_FireProjectileG(asIScriptGeneric *gen);

	//! Lists the current projectiles in the console
	void CON_ListProjectiles();

	//! Lists the current projectiles in the console
	/*!
	 * Generic calling convention
	 */
	void CON_ListProjectilesG(asIScriptGeneric *gen);

	//! Prints the given string to the console
	void CON_Print(std::string str);

	//! Prints the given string to the console
	/*!
	 * General calling convention
	 */
	void CON_PrintG(asIScriptGeneric* gen);

	//! Retreives the Text property of the given resource
	std::string ENT_GetResourceText(std::string name);

	//! Gets a ResourcePointer and adds it to the given entity
	/*!
	 * The resource will be accessable with the localKey given.
	 */
	void ENT_AddResource(std::string entName, std::string localKey, std::string name);

	//! Removes the given resource
	void ENT_RemoveResource(std::string entName, std::string key);

		//! Gets a ResourcePointer and adds it to the given entity
	/*!
	 * The resource will be accessable with the localKey given.
	 */
	void ENT_AttachChildNode(std::string entName, std::string localKey, std::string name);

	//! Removes the given resource
	void ENT_DetachChildNode(std::string entName, std::string key);
}

#endif