/*
  Copyright (c) 2006-2009 Fusion Project Team

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

#include "Common.h"

#include "FusionClanLibRocketInterface.h"

#include "PhysFS.h"

namespace FusionEngine
{

	///////////////
	// RocketSystem
	float RocketSystem::GetElapsedTime()
	{
		return (float)CL_System::get_time() / 1000.f;
	}

	bool RocketSystem::LogMessage(EMP::Core::Log::Type type, const EMP::Core::String& message)
	{
		LogSeverity logLevel = LOG_NORMAL;
		Console::MessageType mtype = Console::MTWARNING;
		if (type == EMP::Core::Log::LT_ERROR || type == EMP::Core::Log::LT_ASSERT)
		{
			logLevel = LOG_CRITICAL;
			mtype = Console::MTERROR;
		}
		else if (type == EMP::Core::Log::LT_INFO)
		{
			logLevel = LOG_TRIVIAL;
			mtype = Console::MTNORMAL;
		}
		Logger::getSingleton().Add(std::string(message.CString()), "rocket_log", logLevel);
		SendToConsole(std::string(message.CString()), mtype);

		return true;
	}

	/////////////////
	// RocketRenderer
	struct GeometryVertex
	{
		CL_Vec2f position;
		CL_Vec2f tex_coord;
	};

	struct RocketCL_Texture
	{
		RocketCL_Texture(CL_Texture tex) : texture(tex) {}
		CL_Texture texture;
	};

	struct GeometryData
	{
		int num_verticies;
		CL_VertexArrayBuffer vertex_buffer;
		RocketCL_Texture* texture;
	};


	RocketRenderer::RocketRenderer(CL_GraphicContext gc)
		: m_gc(gc)
	{
	}

	void RocketRenderer::RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const EMP::Core::Vector2f& translation)
	{
		SendToConsole(L"Rocket requested uncompiled render; ignored", Console::MTWARNING);
	}

	Rocket::Core::CompiledGeometryHandle RocketRenderer::CompileGeometry(Rocket::Core::Vertex *vertices, int num_vertices, int *indices, int num_indices, Rocket::Core::TextureHandle texture)
	{
		using namespace Rocket;

		CL_VertexArrayBuffer buffer(m_gc, num_indices * sizeof(GeometryVertex));

		buffer.lock(cl_access_write_only);
		GeometryVertex* buffer_data = (GeometryVertex*) buffer.get_data();

		for (int i = 0; i < num_indices; i++)
		{
			int vertex_index = indices[i];
			buffer_data[i].position.x = vertices[vertex_index].position.x;
			buffer_data[i].position.y = vertices[vertex_index].position.y;

			buffer_data[i].tex_coord.x = vertices[vertex_index].tex_coord.x;
			buffer_data[i].tex_coord.y = vertices[vertex_index].tex_coord.y;
		}

		buffer.unlock();

		GeometryData* data = new GeometryData;
		data->num_verticies = num_indices;
		data->vertex_buffer = buffer;
		data->texture = (RocketCL_Texture*)texture;

		//m_Geometry.push_back(buffer);
		return (Core::CompiledGeometryHandle*)data;
	}

	void RocketRenderer::RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const EMP::Core::Vector2f& translation)
	{
		using namespace Rocket;

		GeometryData* data = (GeometryData*)geometry;
		CL_VertexArrayBuffer vertex_buffer = data->vertex_buffer;

		m_gc.set_map_mode(cl_map_2d_upper_left);
		m_gc.set_texture(0, data->texture->texture);

		CL_PrimitivesArray prim_array(m_gc);
		prim_array.set_positions(vertex_buffer, data->num_verticies, cl_type_float, &static_cast<GeometryVertex*>(0)->position, sizeof(GeometryVertex));
		prim_array.set_tex_coords(0, vertex_buffer, data->num_verticies, cl_type_float, &static_cast<GeometryVertex*>(0)->tex_coord, sizeof(GeometryVertex));

		m_gc.draw_primitives(cl_triangles, data->num_verticies, prim_array);
	}

	void RocketRenderer::ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry)
	{
		delete (GeometryData*)geometry;
	}

	// Called by Rocket when it wants to enable or disable scissoring to clip content.
	void RocketRenderer::EnableScissorRegion(bool enable)
	{
		if (!enable)
			m_gc.set_cliprect(CL_Rect(0, 0, m_gc.get_width(), m_gc.get_height()));
		else
			m_gc.set_cliprect(CL_Rect(m_Scissor_left, m_Scissor_top, m_Scissor_right, m_Scissor_bottom));
	}

	// Called by Rocket when it wants to change the scissor region.
	void RocketRenderer::SetScissorRegion(int x, int y, int width, int height)
	{
		m_Scissor_left = x;
		m_Scissor_top = y;
		m_Scissor_right = x + width;
		m_Scissor_bottom = y + height;

		m_gc.set_cliprect(CL_Rect(m_Scissor_left, m_Scissor_top, m_Scissor_right, m_Scissor_bottom));
	}

	bool RocketRenderer::LoadTexture(Rocket::Core::TextureHandle& texture_handle, EMP::Core::Vector2i& texture_dimensions, const EMP::Core::String& source, const EMP::Core::String& source_path)
	{
		CL_PixelBuffer image = CL_ImageProviderFactory::load( CL_String((source_path + source).CString()) );
		if (image.is_null())
			return false;

		CL_Texture texture(m_gc, cl_texture_2d);
		texture.set_image(image);

		if (texture.is_null())
			return false;

		texture_dimensions.x = texture.get_width();
		texture_dimensions.y = texture.get_height();

		texture_handle = new RocketCL_Texture(texture);
		return true;
	}

	// Called by Rocket when a texture is required to be built from an internally-generated sequence of pixels.
	bool RocketRenderer::GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const EMP::Core::byte* source, const EMP::Core::Vector2i& source_dimensions)
	{
		static int texture_id = 1;

		int pitch = source_dimensions.x * 4;
		CL_PixelBuffer image(source_dimensions.x, source_dimensions.y, pitch, CL_PixelFormat::abgr8888, (void*)source);

		CL_Texture texture(m_gc, cl_texture_2d);
		texture.set_image(image);

		if (texture.is_null())
			return false;

		texture_handle = new RocketCL_Texture(texture);
		return true;
	}

	// Called by Rocket when a loaded texture is no longer required.
	void RocketRenderer::ReleaseTexture(Rocket::Core::TextureHandle texture)
	{
		delete ((RocketCL_Texture*)texture);
	}

	//////
	// RFS
	RocketFileSystem::RocketFileSystem()
	{
	}

	RocketFileSystem::~RocketFileSystem()
	{
	}

	// Opens a file.
	Rocket::Core::FileHandle RocketFileSystem::Open(const EMP::Core::String& path)
	{
		// Attempt to open the file using the search path (PhysFS expects this.)
		PHYSFS_File* fileHandle = PHYSFS_openRead(path.CString());
		return (Rocket::Core::FileHandle)fileHandle;

		// Attempt to open the file relative to the application's root
		//  (might work if using the search path above failed, but this
		//  isn't normal useage for PhysFS.)
		//fileHandle = PHYSFS_openRead((root + path).CString());
		//return (Rocket::Core::FileHandle)fileHandle;
	}

	// Closes a previously opened file.
	void RocketFileSystem::Close(Rocket::Core::FileHandle file)
	{
		PHYSFS_close((PHYSFS_File*)file);
	}

	// Reads data from a previously opened file.
	size_t RocketFileSystem::Read(void* buffer, size_t size, Rocket::Core::FileHandle file)
	{
		return PHYSFS_read((PHYSFS_File*)file, buffer, 1, size);
	}

	// Seeks to a point in a previously opened file.
	bool RocketFileSystem::Seek(Rocket::Core::FileHandle file, long offset, int origin)
	{
		cl_int64 absolute_pos = 0;
		cl_int64 curPos = (cl_int64)PHYSFS_tell((PHYSFS_File*)file);
		switch (mode)
		{
		case SEEK_SET:
			absolute_pos = seek_pos;
			break;

		case SEEK_CUR:
			if (curPos == -1)
				return false;
			absolute_pos = curPos + seek_pos;
			break;

		case SEEK_END:
			if (seek_pos > 0)
				return false;
			absolute_pos = PHYSFS_fileLength((PHYSFS_File*)file) + seek_pos;
			break;
		}

		return PHYSFS_seek((PHYSFS_File*)file, absolute_pos) != 0;
	}

	// Returns the current position of the file pointer.
	size_t RocketFileSystem::Tell(Rocket::Core::FileHandle file)
	{
		return PHYSFS_tell((PHYSFS_File*)file);
	}

}