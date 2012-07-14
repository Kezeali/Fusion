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

	inline CL_Origin StringToOrigin(const std::string str)
	{
		if (str == "Top-Left")
			return origin_top_left;
		else if (str == "Top-Center")
			return origin_top_center;
		else if (str == "Top-Right")
			return origin_top_right;

		else if (str == "Center-Left")
			return origin_center_left;
		else if (str == "Center")
			return origin_center;
		else if (str == "Center-Right")
			return origin_center_right;

		else if (str == "Bottom-Left")
			return origin_bottom_left;
		else if (str == "Bottom-Center")
			return origin_bottom_center;
		else if (str == "Bottom-Right")
			return origin_bottom_right;

		else
			return origin_center;
	}

	inline std::string OriginToString(const CL_Origin origin)
	{
		switch (origin)
		{
		case origin_top_left: return "Top-Left";
		case origin_top_center: return "Top-Center";
		case origin_top_right: return "Top-Right";

		case origin_center_left: return "Center-Left";
		case origin_center: return "Center";
		case origin_center_right: return "Center-Right";

		case origin_bottom_left: return "Bottom-Left";
		case origin_bottom_center: return "Bottom-Center";
		case origin_bottom_right: return "Bottom-Right";
		default: return "Center";
		};
	}

	void SpriteInspector::InitUI()
	{
		AddTextInput("Image",
			StringSetter_t([](std::string path, ComponentIPtr<ISprite> component) { component->ImagePath.Set(path); }),
			StringGetter_t([](ComponentIPtr<ISprite> component)->std::string { return component->ImagePath.Get(); }),
			35
			);

		AddProperty("AnimationPath", AddTextInput("Animation",
			StringSetter_t([](std::string path, ComponentIPtr<ISprite> component) { component->AnimationPath.Set(path); }),
			StringGetter_t([](ComponentIPtr<ISprite> component)->std::string { return component->AnimationPath.Get(); }),
			35
			));

		// Alignment
		std::vector<std::string> originOptions;
		for (int i = 0; i < 9; ++i)
			originOptions.push_back(OriginToString((CL_Origin)i));
		AddSelectInput("AlignmentOrigin",
			originOptions,
			StringSetter_t([](std::string str, ComponentIPtr<ISprite> component) { component->AlignmentOrigin.Set(StringToOrigin(str)); }),
			StringGetter_t([](ComponentIPtr<ISprite> component)->std::string { return OriginToString(component->AlignmentOrigin.Get()); }));

		AddTextInput("AlignmentOffset",
			IntSetter_t([](int x, ComponentIPtr<ISprite> component) { component->AlignmentOffset.Set(Vector2i(x, component->AlignmentOffset.Get().y)); }),
			IntGetter_t([](ComponentIPtr<ISprite> component) { return component->AlignmentOffset.Get().x; })
			);
		AddTextInput("",
			IntSetter_t([](int y, ComponentIPtr<ISprite> component) { component->AlignmentOffset.Set(Vector2i(component->AlignmentOffset.Get().x, y)); }),
			IntGetter_t([](ComponentIPtr<ISprite> component) { return component->AlignmentOffset.Get().y; })
			);

		// Rotation Hotspot
		AddSelectInput("RotationOrigin",
			originOptions,
			StringSetter_t([](std::string str, ComponentIPtr<ISprite> component) { component->RotationOrigin.Set(StringToOrigin(str)); }),
			StringGetter_t([](ComponentIPtr<ISprite> component)->std::string { return OriginToString(component->RotationOrigin.Get()); })
			);

		AddTextInput("RotationOffset",
			IntSetter_t([](int x, ComponentIPtr<ISprite> component) { component->RotationOffset.Set(Vector2i(x, component->RotationOffset.Get().y)); }),
			IntGetter_t([](ComponentIPtr<ISprite> component) { return component->RotationOffset.Get().x; })
			);
		AddTextInput("",
			IntSetter_t([](int y, ComponentIPtr<ISprite> component) { component->RotationOffset.Set(Vector2i(component->RotationOffset.Get().x, y)); }),
			IntGetter_t([](ComponentIPtr<ISprite> component) { return component->RotationOffset.Get().y; })
			);

		// Base-Angle
		AddTextInput("BaseAngle",
			FloatSetter_t([](float angle, ComponentIPtr<ISprite> component) { component->BaseAngle.Set(angle); }),
			FloatGetter_t([](ComponentIPtr<ISprite> component) { return component->BaseAngle.Get(); })
			);

		// Scale
		AddTextInput("Scale",
			FloatSetter_t([](float x, ComponentIPtr<ISprite> component) { component->Scale.Set(Vector2(x, component->Scale.Get().y)); }),
			FloatGetter_t([](ComponentIPtr<ISprite> component) { return component->Scale.Get().x; })
			);
		AddTextInput("",
			FloatSetter_t([](float y, ComponentIPtr<ISprite> component) { component->Scale.Set(Vector2(component->Scale.Get().x, y)); }),
			FloatGetter_t([](ComponentIPtr<ISprite> component) { return component->Scale.Get().y; })
			);
		
		// Animation frame
		AddTextInput("AnimationFrame",
			IntSetter_t([](int frame, ComponentIPtr<ISprite> component) { component->AnimationFrame.Set(frame); }),
			IntGetter_t([](ComponentIPtr<ISprite> component) { return component->AnimationFrame.Get(); })
			);
	}

} }
