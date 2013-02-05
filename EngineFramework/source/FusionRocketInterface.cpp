/*
*  Copyright (c) 2009-2012 Fusion Project Team
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

#include "FusionRocketInterface.h"

#include <Rocket/Core.h>

#include "physfs.h"
#include "FusionPhysFS.h"
#include "FusionVirtualFileSource_PhysFS.h"
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
		m_RocketLog->SetThreshold(LOG_INFO);
#endif
	}

	float RocketSystem::GetElapsedTime()
	{
		return (float)clan::System::get_time() / 1000.f;
	}

	bool RocketSystem::LogMessage(Rocket::Core::Log::Type type, const Rocket::Core::String& message)
	{
		if (m_RocketLog)
		{
			LogSeverity logLevel = LOG_NORMAL;
			if (type == Rocket::Core::Log::LT_ERROR || type == Rocket::Core::Log::LT_ASSERT)
				logLevel = LOG_CRITICAL;
			else if (type == Rocket::Core::Log::LT_INFO)
				logLevel = LOG_INFO;
			m_RocketLog->AddEntry(std::string(message.CString(), (std::string::size_type)message.Length()), logLevel);
		}

		Rocket::Core::SystemInterface::LogMessage(type, message);

		return true;
	}

	void RocketSystem::Release()
	{
		delete this;
	}

	/////////////////
	// RocketRenderer
	struct GeometryVertex
	{
		clan::Vec2f position;
		clan::Vec4f color;
		clan::Vec2f tex_coord;
	};

	struct RocketCLTexture
	{
		RocketCLTexture(clan::Texture2D tex) : texture(tex) {}
		clan::Texture2D texture;
	};

	struct GeometryData
	{
		int num_verticies;
		//clan::VertexArrayVector<GeometryVertex> vertex_buffer;
		clan::VertexArrayVector<clan::Vec4f> verticies;
		clan::VertexArrayVector<clan::Vec4f> colours;
		clan::VertexArrayVector<clan::Vec2f> texCoords;
		RocketCLTexture* texture;
	};


	RocketRenderer::RocketRenderer(const clan::Canvas& canvas)
		: m_Canvas(canvas),
		m_ClipEnabled(false)
	{
	}

	void RocketRenderer::RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const Rocket::Core::Vector2f& translation)
	{
		//auto compiledGeometry = CompileGeometry(vertices, num_vertices, indices, num_indices, texture);

		//RenderCompiledGeometry(compiledGeometry, translation);

		//ReleaseCompiledGeometry(compiledGeometry);

		std::vector<clan::Vec2f> verts(num_indices);
		std::vector<clan::Colorf> colours(num_indices);
		std::vector<clan::Vec2f> texPos(num_indices);
		for (int i = 0; i < num_indices; i++)
		{
			int vertex_index = indices[i];
			verts[i].x = vertices[vertex_index].position.x;
			verts[i].y = vertices[vertex_index].position.y;

			colours[i] = clan::Colorf(
				vertices[vertex_index].colour.red,
				vertices[vertex_index].colour.green,
				vertices[vertex_index].colour.blue,
				vertices[vertex_index].colour.alpha);
			//data[i].color.r = vertices[vertex_index].colour.red / 255.f;
			//data[i].color.g = vertices[vertex_index].colour.green / 255.f;
			//data[i].color.b = vertices[vertex_index].colour.blue / 255.f;
			//data[i].color.a = vertices[vertex_index].colour.alpha / 255.f;

			texPos[i].x = vertices[vertex_index].tex_coord.x;
			texPos[i].y = vertices[vertex_index].tex_coord.y;
		}

		m_Canvas.push_modelview();

		m_Canvas.mult_translate(translation.x, translation.y);

		if (m_ClipEnabled)
			m_Canvas.push_cliprect(clan::Rect(m_Scissor_left, m_Scissor_top, m_Scissor_right, m_Scissor_bottom));

		if (texture != NULL)
		{
			auto textureHolder = reinterpret_cast<RocketCLTexture*>(texture);
			auto textureObj = textureHolder->texture;
			m_Canvas.fill_triangles(&verts[0], &texPos[0], num_indices, textureObj, &colours[0]);
		}
		else
		{
			m_Canvas.fill_triangles(&verts[0], &colours[0], num_indices);
		}

		if (m_ClipEnabled)
			m_Canvas.pop_cliprect();

		m_Canvas.pop_modelview();
	}

	Rocket::Core::CompiledGeometryHandle RocketRenderer::CompileGeometry(Rocket::Core::Vertex *vertices, int num_vertices, int *indices, int num_indices, Rocket::Core::TextureHandle texture)
	{
		using namespace Rocket;

		return Rocket::Core::CompiledGeometryHandle(NULL);

		//std::vector<GeometryVertex> data(num_indices);
		//for (int i = 0; i < num_indices; i++)
		//{
		//	int vertex_index = indices[i];
		//	data[i].position.x = vertices[vertex_index].position.x;
		//	data[i].position.y = vertices[vertex_index].position.y;

		//	data[i].color = clan::Colorf(
		//		vertices[vertex_index].colour.red,
		//		vertices[vertex_index].colour.green,
		//		vertices[vertex_index].colour.blue,
		//		vertices[vertex_index].colour.alpha);

		//	data[i].tex_coord.x = vertices[vertex_index].tex_coord.x;
		//	data[i].tex_coord.y = vertices[vertex_index].tex_coord.y;
		//}
		std::vector<clan::Vec4f> verts(num_indices);
		std::vector<clan::Vec4f> colours(num_indices);
		std::vector<clan::Vec2f> texCoords(num_indices);
		for (int i = 0; i < num_indices; i++)
		{
			int vertex_index = indices[i];
			verts[i].x = vertices[vertex_index].position.x;
			verts[i].y = vertices[vertex_index].position.y;

			colours[i] = clan::Colorf(
				vertices[vertex_index].colour.red,
				vertices[vertex_index].colour.green,
				vertices[vertex_index].colour.blue,
				vertices[vertex_index].colour.alpha);

			texCoords[i].x = vertices[vertex_index].tex_coord.x;
			texCoords[i].y = vertices[vertex_index].tex_coord.y;
		}

		auto gcVerts = clan::VertexArrayVector<clan::Vec4f>(m_Canvas.get_gc(), verts);
		auto gcColours = clan::VertexArrayVector<clan::Vec4f>(m_Canvas.get_gc(), colours);
		auto gcTexCoords = clan::VertexArrayVector<clan::Vec2f>(m_Canvas.get_gc(), texCoords);
		//clan::VertexArrayBuffer buffer(m_Canvas.get_gc(), data.data(), sizeof(GeometryVertex) * num_indices);

		GeometryData* compiledGeometry = new GeometryData;
		compiledGeometry->num_verticies = num_indices;
		compiledGeometry->verticies = gcVerts;
		compiledGeometry->colours = gcColours;
		compiledGeometry->texCoords = gcTexCoords;
		compiledGeometry->texture = reinterpret_cast<RocketCLTexture*>(texture);

		//m_Geometry.push_back(buffer);
		return reinterpret_cast<Core::CompiledGeometryHandle>(compiledGeometry);
	}

	void RocketRenderer::RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const Rocket::Core::Vector2f& translation)
	{
		using namespace Rocket;

		GeometryData* data = (GeometryData*)geometry;
		//auto& vertex_buffer = data->vertex_buffer;

		//m_Canvas.push_modelview();

		//m_Canvas.set_map_mode(clan::map_2d_upper_left);

		//m_Canvas.mult_translate(translation.x, translation.y);
		//m_Canvas.set_translate(translation.x, translation.y);

		//m_Canvas.set_blend_mode(m_BlendMode);

		clan::PrimitivesArray prim_array(m_Canvas.get_gc());
		if (data->texture != nullptr)
		{
			//prim_array.set_attributes(clan::attrib_position, vertex_buffer, cl_offsetof(GeometryVertex, position));
			//prim_array.set_attributes(clan::attrib_color, vertex_buffer, cl_offsetof(GeometryVertex, color));
			//prim_array.set_attributes(clan::attrib_texture_position, vertex_buffer, cl_offsetof(GeometryVertex, tex_coord));

			prim_array.set_attributes(0, data->verticies);
			prim_array.set_attributes(1, data->colours);
			prim_array.set_attributes(2, data->texCoords);

			m_Canvas.get_gc().set_program_object(clan::program_single_texture);

			m_Canvas.get_gc().set_texture(0, data->texture->texture);
		}
		else
		{
			prim_array.set_attributes(0, data->verticies);
			prim_array.set_attributes(1, data->colours);

			m_Canvas.get_gc().set_program_object(clan::program_color_only);
		}

		//if (data->texture)
		//	m_Canvas.set_texture(0, data->texture->texture);

		//if (m_ClipEnabled)
		//	m_Canvas.get_gc().set_scissor(clan::Rect(m_Scissor_left, m_Scissor_top, m_Scissor_right, m_Scissor_bottom), clan::y_axis_top_down);

		//m_Canvas.draw_triangles(&(prim_array[clan::attrib_position]), &prim_array[clan::attrib_texture_position], data->num_verticies, data->texture->texture);
		m_Canvas.get_gc().draw_primitives(clan::type_triangles, data->num_verticies, prim_array);

		//if (m_ClipEnabled)
		//	m_Canvas.pop_cliprect();

		if (data->texture)
			m_Canvas.get_gc().reset_texture(0);
		m_Canvas.get_gc().reset_program_object();

		//m_Canvas.reset_blend_mode();
		//m_Canvas.reset_buffer_control();

		//m_Canvas.pop_modelview();
	}

	void RocketRenderer::ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry)
	{
		delete (GeometryData*)geometry;
	}

	// Called by Rocket when it wants to enable or disable scissoring to clip content.
	void RocketRenderer::EnableScissorRegion(bool enable)
	{
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
	}

	bool RocketRenderer::LoadTexture(Rocket::Core::TextureHandle& texture_handle, Rocket::Core::Vector2i& texture_dimensions, const Rocket::Core::String& source)
	{
		using namespace Rocket::Core;

		clan::VirtualDirectory physfsDir(clan::VirtualFileSystem(new VirtualFileSource_PhysFS()), "");
		try
		{
			auto filename = std::string(source.CString(), source.Length());

			clan::Texture2D texture(m_Canvas.get_gc(), physfsDir.open_file_read(filename), clan::PathHelp::get_extension(filename));
			//texture.set_min_filter(clan::filter_linear);
			//texture.set_mag_filter(clan::filter_linear);

			if (texture.is_null())
				return false;

			texture_dimensions.x = texture.get_width();
			texture_dimensions.y = texture.get_height();

			texture_handle = reinterpret_cast<TextureHandle>(new RocketCLTexture(texture));
			return true;
		}
		catch (clan::Exception& ex)
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
			clan::PixelBuffer rgbaImage = clan::PixelBuffer(source_dimensions.x, source_dimensions.y, clan::tf_rgba8, reinterpret_cast<const void*>(source));

			clan::Texture2D texture(m_Canvas.get_gc(), source_dimensions.x, source_dimensions.y);
			texture.set_image(m_Canvas.get_gc(), rgbaImage);
			texture.set_min_filter(clan::filter_linear);
			texture.set_mag_filter(clan::filter_linear);

			if (texture.is_null())
				return false;

			texture_handle = reinterpret_cast<Rocket::Core::TextureHandle>(new RocketCLTexture(texture));
			return true;
		}
		catch (clan::Exception& ex)
		{
			Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, "CLRocketRenderer failed to generate a texture");
			Rocket::Core::Log::Message(Rocket::Core::Log::LT_ERROR, ex.what());
			return false;
		}
	}

	// Called by Rocket when a loaded texture is no longer required.
	void RocketRenderer::ReleaseTexture(Rocket::Core::TextureHandle texture)
	{
		delete ((RocketCLTexture*)texture);
	}

	void RocketRenderer::Release()
	{
		delete this;
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
	Rocket::Core::FileHandle RocketFileSystem::Open(const Rocket::Core::String& path)
	{
		// Attempt to open the file using the search path (PhysFS expects this.)
		PHYSFS_File* fileHandle = PHYSFS_openRead(path.CString());
		return (Rocket::Core::FileHandle)fileHandle;
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

		switch (origin)
		{
		case SEEK_SET:
			absolute_pos = (PHYSFS_uint64)offset;
			break;

		case SEEK_CUR:
			{
				PHYSFS_sint64 curPos = PHYSFS_tell((PHYSFS_File*)file);
				if (curPos == -1)
				{
					Rocket::Core::Log::Message(Rocket::Core::Log::LT_WARNING, "RocketFileSystem couldn't Seek: couldn't get the current position");
					return false;
				}
				absolute_pos = (PHYSFS_uint64)curPos + offset;
			}
			break;

		case SEEK_END:
			{
				if (offset > 0)
					return false;
				PHYSFS_sint64 length = PHYSFS_fileLength((PHYSFS_File*)file);
				if (length == -1)
				{
					Rocket::Core::Log::Message(Rocket::Core::Log::LT_WARNING, "RocketFileSystem couldn't Seek: couldn't get file length");
					return false;
				}
				else
					absolute_pos = (PHYSFS_uint64)length + offset;
			}
			break;
		}

		return PHYSFS_seek((PHYSFS_File*)file, absolute_pos) != 0;
	}

	// Returns the current position of the file pointer.
	size_t RocketFileSystem::Tell(Rocket::Core::FileHandle file)
	{
		return (size_t)PHYSFS_tell((PHYSFS_File*)file);
	}

	size_t RocketFileSystem::Length(Rocket::Core::FileHandle file)
	{
		return (size_t)PHYSFS_fileLength((PHYSFS_File*)file);
	}

	void RocketFileSystem::Release()
	{
		delete this;
	}

}