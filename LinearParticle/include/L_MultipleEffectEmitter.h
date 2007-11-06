//===============================================================================
//
// LinearParticle Copyright (c) 2006 Wong Chin Foo
// LinearParticle Extended Copyright 2007 Elliot Hayward
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software in a
// product, an acknowledgment in the product documentation would be
// appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
//===============================================================================


#ifndef LEXT_MULTIPLEEFFECTEMITTER_H
#define LEXT_MULTIPLEEFFECTEMITTER_H

#include "L_Extended.h"
#include "L_EffectManager.h"

LPEXTENDED_NAMESPACE_BEGIN

//! Effect emitter can be used to emit multiple effects easily.
class MultipleEffectEmitter : public L_EffectManager
{
public:
	typedef std::vector<L_ParticleEffect*> ParticleEffectList;

protected:
	ParticleEffectList effect_type_list;

public:
	MultipleEffectEmitter();
	//! Constructs an emitter using the given list of effects
	MultipleEffectEmitter(ParticleEffectList effect_list);
	//! Creates new instances of all effects in the list
	void emit( L_REAL x_pos, L_REAL y_pos );
	//! Adds the given effect type to the effect list
	/*!
	 *  A new instance (copy) of this effect will be emited when emit is called
	 */
	void add_type(L_ParticleEffect* effect);

};

LPEXTENDED_NAMESPACE_END

#endif
