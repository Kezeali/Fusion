/*
  Copyright (c) 2009-2011 Fusion Project Team

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

#include "FusionStableHeaders.h"

#include "FusionRocketInterface.h"

#include <Rocket/Core.h>

#include "PhysFS.h"
#include "FusionPhysFS.h"
#include "FusionLogger.h"
#include "FusionConsole.h"

namespace FusionEngine
{

	///////////////
	// RocketSystem
	RocketSystem::RocketSystem()
	{
		m_RocketLog = Logger::getSingleton().OpenLog("rocket");
#ifdef _DEBUG
		m_RocketLog->SetThreshold(LOG_TRIVIAL);
		Logger::getSingleton().SetLogingToConsole("rocket", true);
#endif
	}

	float RocketSystem::GetElapsedTime()
	{
		return (float)CL_System::get_time() / 1000.f;
	}

	bool RocketSystem::LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message)
	{
		LogSeverity logLevel = LOG_NORMAL;
		if (type == Rocket::Core::Log::LT_ERROR || type == Rocket::Core::Log::LT_ASSERT)
			logLevel = LOG_CRITICAL;
		else if (type == Rocket::Core::Log::LT_INFO)
			logLevel = LOG_TRIVIAL;
		Logger *logger = Logger::getSingletonPtr();
		if (logger != NULL)
			m_RocketLog->AddEntry(std::string(message.CString(), (std::string::size_type)message.Length()), logLevel);

		Rocket::Core::SystemInterface::LogMessage(type, message);

		return true;
	}

	//void RocketSystem::Release()
	//{
	//	delete this;
	//}

	/////////////////
	// RocketRenderer
	struct GeometryVertex
	{
		CL_Vec2f position;
		CL_Vec4f color;
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


	RocketRenderer::RocketRenderer(const CL_GraphicContext &gc)
		: m_gc(gc),
		m_ClipEnabled(false)
	{
	}

	void RocketRenderer::RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation)
	{
		CL_Vec2f *polygon = new CL_Vec2f[num_indices];
		CL_Vec4f *vert_colour = new CL_Vec4f[num_indices];
		CL_Vec2f *tex_cords = new CL_Vec2f[num_indices];
		for (int i = 0; i < num_indices; i++)
		{
			int vertex_index = indices[i];
			polygon[i].x = vertices[vertex_index].position.x;
			polygon[i].y = vertices[vertex_index].position.y;

			//vert_colour[i] = CL_Colorf(
			//	vertices[vertex_index].colour.red,
			//	vertices[vertex_index].colour.green,
			//	vertices[vertex_index].colour.blue,
			//	vertices[vertex_index].colour.alpha);
			vert_colour[i].r = vertices[vertex_index].colour.red / 255.f;
			vert_colour[i].g = vertices[vertex_index].colour.green / 255.f;
			vert_colour[i].b = vertices[vertex_index].colour.blue / 255.f;
			vert_colour[i].a = vertices[vertex_index].colour.alpha / 255.f;

			tex_cords[i].x = vertices[vertex_index].tex_coord.x;
			tex_cords[i].y = vertices[vertex_index].tex_coord.y;
		}

		m_gc.push_translate(translation.x, translation.y);

		m_gc.set_map_mode(cl_map_2d_upper_left);
		if (texture != NULL)
			m_gc.set_texture(0, reinterpret_cast<RocketCL_Texture*>(texture)->texture);

		CL_PrimitivesArray prim_array(m_gc);
		if (texture != NULL)
		{
			prim_array.set_attributes(0, polygon);
			prim_array.set_attributes(1, vert_colour);
			prim_array.set_attributes(2, tex_cords);

			m_gc.set_program_object(cl_program_single_texture);
		}
		else
		{
			prim_array.set_attributes(0, polygon);
			prim_array.set_attributes(1, vert_colour);

			m_gc.set_program_object(cl_program_color_only);
		}

		//if (m_ClipEnabled)
		//	m_gc.push_cliprect(CL_Rect(m_Scissor_left, m_Scissor_top, m_Scissor_right, m_Scissor_bottom));

		m_gc.draw_primitives(cl_triangles, num_indices, prim_array);

		//if (m_ClipEnabled)
		//	m_gc.pop_cliprect();

		delete[] polygon;
		delete[] vert_colour;
		delete[] tex_cords;

		m_gc.reset_program_object();
		if (texture != NULL)
			m_gc.reset_texture(0);

		//m_gc.reset_blend_mode();
		//m_gc.reset_buffer_control();

		m_gc.pop_modelview();
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

			//buffer_data[i].color = CL_Colorf(
			//	vertices[vertex_index].colour.red,
			//	vertices[vertex_index].colour.green,
			//	vertices[vertex_index].colour.blue,
			//	vertices[vertex_index].colour.alpha);
			buffer_data[i].color.r = vertices[vertex_index].colour.red / 255.f;
			buffer_data[i].color.g = vertices[vertex_index].colour.green / 255.f;
			buffer_data[i].color.b = vertices[vertex_index].colour.blue / 255.f;
			buffer_data[i].color.a = vertices[vertex_index].colour.alpha / 255.f;

			buffer_data[i].tex_coord.x = vertices[vertex_index].tex_coord.x;
			buffer_data[i].tex_coord.y = vertices[vertex_index].tex_coord.y;
		}

		buffer.unlock();

		GeometryData* data = new GeometryData;
		data->num_verticies = num_indices;
		data->vertex_buffer = buffer;
		data->texture = (RocketCL_Texture*)texture;

		//m_Geometry.push_back(buffer);
		return reinterpret_cast<Core::CompiledGeometryHandle>(data);
	}

	void RocketRenderer::RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation)
	{
		using namespace Rocket;

		GeometryData* data = (GeometryData*)geometry;
		CL_VertexArrayBuffer vertex_buffer = data->vertex_buffer;


		m_gc.push_modelview();

		//m_gc.set_map_mode(cl_map_2d_upper_left);

		m_gc.mult_translate(translation.x, translation.y);
		//m_gc.set_translate(translation.x, translation.y);

		if (data->texture)
			m_gc.set_texture(0, data->texture->texture);

		//m_gc.set_blend_mode(m_BlendMode);

		CL_PrimitivesArray prim_array(m_gc);
		if (data->texture != NULL)
		{
			prim_array.set_attributes(0, vertex_buffer, 2, cl_type_float, &static_cast<GeometryVertex*>(0)->position, sizeof(GeometryVertex));
			prim_array.set_attributes(1, vertex_buffer, 4, cl_type_float, &static_cast<GeometryVertex*>(0)->color, sizeof(GeometryVertex));
			prim_array.set_attributes(2, vertex_buffer, 2, cl_type_float, &static_cast<GeometryVertex*>(0)->tex_coord, sizeof(GeometryVertex));

			m_gc.set_program_object(cl_program_single_texture);
		}
		else
		{
			prim_array.set_attributes(0, vertex_buffer, 2, cl_type_float, &static_cast<GeometryVertex*>(0)->position, sizeof(GeometryVertex));
			prim_array.set_attributes(1, vertex_buffer, 4, cl_type_float, &static_cast<GeometryVertex*>(0)->color, sizeof(GeometryVertex));

			m_gc.set_program_object(cl_program_color_only);
		}

		if (m_ClipEnabled)
			m_gc.push_cliprect(CL_Rect(m_Scissor_left, m_Scissor_top, m_Scissor_right, m_Scissor_bottom));

		m_gc.draw_primitives(cl_triangles, data->num_verticies, prim_array);

		if (m_ClipEnabled)
			m_gc.pop_cliprect();

		m_gc.reset_program_object();
		if (data->texture)
			m_gc.reset_texture(0);

		//m_gc.reset_blend_mode();
		//m_gc.reset_buffer_control();

		m_gc.pop_modelview();

	}

	void RocketRenderer::ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry)
	{
		delete (GeometryData*)geometry;
	}

	// Called by Rocket when it wants to enable or disable scissoring to clip content.
	void RocketRenderer::EnableScissorRegion(bool enable)
	{
		//if (!enable)
		//	m_gc.reset_cliprect();
		//else
		//	m_gc.set_cliprect(CL_Rect(m_Scissor_left, m_Scissor_top, m_Scissor_right, m_Scissor_bottom));

		m_ClipEnabled = enable;
	}

	// Called by Rocket when it wants to change the scissor region.
	void RocketRenderer::SetScissorRegion(int x, int y, int width, int height)
	{
		m_Scissor_left = x;
		m_Scissor_top = y;
		m_Scissor_right = x + width;
		m_Scissor_bottom = y + height;

		EnableScissorRegion(m_ClipEnabled);
		//m_gc.set_cliprect(CL_Rect(m_Scissor_left, m_Scissor_top, m_Scissor_right, m_Scissor_bottom));
	}

	bool RocketRenderer::LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions, const Rocket::Core::String& source)
	{
		using namespace Rocket::Core;

		CL_VirtualDirectory physfsDir(CL_VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		try
		{
			CL_PixelBuffer image = CL_ImageProviderFactory::load( CL_String(source.CString()), physfsDir );
			if (image.is_null())
				return false;

			CL_Texture texture(m_gc, cl_texture_2d);
			texture.set_image(image);
			texture.set_min_filter(cl_filter_linear);
			texture.set_mag_filter(cl_filter_linear);

			if (texture.is_null())
				return false;

			texture_dimensions.x = texture.get_width();
			texture_dimensions.y = texture.get_height();

			texture_handle = reinterpret_cast<TextureHandle>(new RocketCL_Texture(texture));
			return true;
		}
		catch (CL_Exception& ex)
		{
			Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, ("CLRocketRenderer couldn't load a texture from \"" + source + "\"").CString());
			Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, ex.what());
			return false;
		}
	}

	// Called by Rocket when a texture is required to be built from an internally-generated sequence of pixels.
	bool RocketRenderer::GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const Rocket::Core::byte* source, const Rocket::Core::Vector2i& source_dimensions)
	{
		try
		{
			CL_PixelBuffer rgbaImage = CL_PixelBuffer(source_dimensions.x, source_dimensions.y, cl_abgr8, (const void*)source).to_format(cl_rgba8);

			CL_Texture texture(m_gc, cl_texture_2d);
			texture.set_image(rgbaImage);
			texture.set_min_filter(cl_filter_linear);
			texture.set_mag_filter(cl_filter_linear);

			if (texture.is_null())
				return false;

			texture_handle = reinterpret_cast<Rocket::Core::TextureHandle>(new RocketCL_Texture(texture));
			return true;
		}
		catch (CL_Exception& ex)
		{
			Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "CLRocketRenderer failed to generate a texture");
			Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, ex.what());
			return false;
		}
	}

	// Called by Rocket when a loaded texture is no longer required.
	void RocketRenderer::ReleaseTexture(Rocket::Core::TextureHandle texture)
	{
		delete ((RocketCL_Texture*)texture);
	}

	//void RocketRenderer::Release()
	//{
	//	delete this;
	//}

	//////
	// RFS
	RocketFileSystem::RocketFileSystem()
	{
	}

	RocketFileSystem::~RocketFileSystem()
	{
	}

	// Opens a file.
	Rocket::Core::FileHandle RocketFileSystem::Open(const Rocket::Core::String& path)
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
		return (size_t)PHYSFS_read((PHYSFS_File*)file, buffer, 1, size);
	}

	// Seeks to a point in a previously opened file.
	bool RocketFileSystem::Seek(Rocket::Core::FileHandle file, long offset, int origin)
	{
		PHYSFS_uint64 absolute_pos = 0;
		// Define vars used within certain cases
		PHYSFS_sint64 curPos;
		PHYSFS_sint64 length;

		switch (origin)
		{
		case SEEK_SET:
			absolute_pos = offset;
			break;

		case SEEK_CUR:
			curPos = PHYSFS_tell((PHYSFS_File*)file);
			if (curPos == -1)
			{
				Rocket::Core::Log::Message(Rocket::Core::Log::LT_WARNING, "RocketFileSystem couldn't Seek: couldn't get the current position");
				return false;
			}
			absolute_pos = (PHYSFS_uint64)curPos + offset;
			break;

		case SEEK_END:
			if (offset > 0)
				return false;
			length = PHYSFS_fileLength((PHYSFS_File*)file);
			if (length == -1)
			{
				Rocket::Core::Log::Message(Rocket::Core::Log::LT_WARNING, "RocketFileSystem couldn't Seek: couldn't get file length");
				return false;
			}
			else
				absolute_pos = (PHYSFS_uint64)length + offset;
			break;
		}

		return PHYSFS_seek((PHYSFS_File*)file, absolute_pos) != 0;
	}

	// Returns the current position of the file pointer.
	size_t RocketFileSystem::Tell(Rocket::Core::FileHandle file)
	{
		return (size_t)PHYSFS_tell((PHYSFS_File*)file);
	}

	//void RocketFileSystem::Release()
	//{
	//	delete this;
	//}

}