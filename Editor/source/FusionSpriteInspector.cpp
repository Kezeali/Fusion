/*
*  Copyright (c) 2012 Fusion Project Team
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

#include "FusionSpriteInspector.h"

namespace FusionEngine { namespace Inspectors
{

	void SpriteInspector::InitUI()
	{
		{
		StringInputCallback_t setter = [](std::string path, ComponentIPtr<ISprite> component) { component->ImagePath.Set(path); };
		StringPropertyCallback_t getter = [](ComponentIPtr<ISprite> component)->std::string { return component->ImagePath.Get(); };
		AddTextInput("Image",
			setter,
			getter);
		}

		{
		StringInputCallback_t setter = [](std::string path, ComponentIPtr<ISprite> component) { component->AnimationPath.Set(path); };
		StringPropertyCallback_t getter = [](ComponentIPtr<ISprite> component)->std::string { return component->AnimationPath.Get(); };
		AddTextInput("Animation",
			setter,
			getter);
		}

		{
		FloatInputCallback_t setter = [](float angle, ComponentIPtr<ISprite> component) { component->BaseAngle.Set(angle); };
		FloatPropertyCallback_t getter = [](ComponentIPtr<ISprite> component) { return component->BaseAngle.Get(); };
		AddTextInput("BaseAngle",
			setter,
			getter);
		}

		{
		FloatInputCallback_t setter = [](float x, ComponentIPtr<ISprite> component) { component->Scale.Set(Vector2(x, component->Scale.Get().y)); };
		FloatPropertyCallback_t getter = [](ComponentIPtr<ISprite> component) { return component->Scale.Get().x; };
		AddTextInput("Scale-X",
			setter,
			getter);
		}
		{
		FloatInputCallback_t setter = [](float y, ComponentIPtr<ISprite> component) { component->Scale.Set(Vector2(component->Scale.Get().x, y)); };
		FloatPropertyCallback_t getter = [](ComponentIPtr<ISprite> component) { return component->Scale.Get().y; };
		AddTextInput("Scale-Y",
			setter,
			getter);
		}
	}

} }
