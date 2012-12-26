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

#include "FusionPolygonLoader.h"

#include "FusionCLStream.h"
#include "FusionResourceLoaderUtils.h"

#include <Box2D/Box2D.h>
#include "FusionPhysFSIOStream.h"

namespace FusionEngine
{

	void operator >> (const YAML::Node& node, b2Vec2& vec)
	{
		node[0] >> vec.x;
		node[1] >> vec.y;
	}

	YAML::Emitter& operator << (YAML::Emitter& out, const b2Vec2& vec)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << vec.x << vec.y << YAML::EndSeq;
		return out;
	}

	std::unique_ptr<b2PolygonShape> PolygonResource::Load(std::istream& stream)
	{
		try
		{
			YAML::Parser p(stream);
			YAML::Node doc;
			if (p.GetNextDocument(doc))
			{
				if (doc.begin() == doc.end())
					return std::unique_ptr<b2PolygonShape>(new b2PolygonShape());

				//const auto& vertsNode = doc["verts"];
				const auto& vertsNode = doc;

				if (vertsNode.size() > b2_maxPolygonVertices)
				{
					FSN_EXCEPT(FileTypeException, "The polygon has too many verticies");
				}

				std::vector<b2Vec2> verts;
				verts.reserve(vertsNode.size());
				for (auto it = vertsNode.begin(); it != vertsNode.end(); ++it)
				{
					auto& vertNode = *it;

					b2Vec2 vert;
					vertNode >> vert;
					verts.push_back(vert);
				}

				auto shape = std::unique_ptr<b2PolygonShape>(new b2PolygonShape());
				shape->Set(verts.data(), verts.size());
				if (shape->Validate())
					return std::move(shape);
				else
				{
					FSN_EXCEPT(FileTypeException, "The polygon is invalid (probably non-convex)");
				}
			}
			else
			{
				//FSN_EXCEPT(FileTypeException, "The given polygon file contains no entries");
				return std::unique_ptr<b2PolygonShape>(new b2PolygonShape());
			}
		}
		catch (YAML::Exception& ex)
		{
			FSN_EXCEPT(FileTypeException, std::string("Invalid polygon file: ") + ex.what());
		}
	}

	void PolygonResource::Save(std::ostream& stream, const b2PolygonShape& shape)
	{
		try
		{
			YAML::Emitter emitter;
			
			emitter << YAML::BeginSeq;
			for (int i = 0; i < shape.GetVertexCount(); ++i)
			{
				emitter << shape.GetVertex(i);
			}
			emitter << YAML::EndSeq;
			stream.write(emitter.c_str(), emitter.size());
		}
		catch (YAML::Exception& ex)
		{
			FSN_EXCEPT(FileTypeException, std::string("Failed to write polygon file: ") + ex.what());
		}
	}

	void PolygonResource::CreateEmpty(std::ostream& stream)
	{
		try
		{
			YAML::Emitter emitter;

			emitter << YAML::BeginSeq;
			emitter << YAML::EndSeq;

			stream.write(emitter.c_str(), emitter.size());
		}
		catch (YAML::Exception& ex)
		{
			FSN_EXCEPT(FileTypeException, std::string("Failed to write polygon file: ") + ex.what());
		}
	}

	void LoadPolygonResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data)
	{
		FSN_ASSERT(!resource->IsLoaded());

		resource->setLoaded(false);

		std::unique_ptr<b2PolygonShape> data;
		try
		{
			//auto dev = vdir.open_file(resource->GetPath(), CL_File::open_existing, CL_File::access_read);
			IO::PhysFSStream stream(resource->GetPath(), IO::Read);
			data = PolygonResource::Load(stream);

			resource->SetMetadata(CreateFileMetadata(resource->GetPath(), stream));
		}
		catch (CL_Exception& ex)
		{
			FSN_EXCEPT(FileSystemException, "'" + resource->GetPath() + "' could not be loaded: " + std::string(ex.what()));
		}
		catch (Exception&)
		{
			throw;
		}

		resource->SetDataPtr(data.release());

		resource->setLoaded(true);
	}

	void UnloadPolygonResource(ResourceContainer* resource, CL_VirtualDirectory vdir, boost::any user_data)
	{
		if (resource->IsLoaded())
		{
			resource->setLoaded(false);
			delete static_cast<b2PolygonShape*>(resource->GetDataPtr());
		}
		resource->SetDataPtr(nullptr);
	}

}
