/*
  Copyright (c) 2009 Fusion Project Team

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

#ifndef Header_FusionEngine_RocketInterface
#define Header_FusionEngine_RocketInterface

#if _MSC_VER > 1000
#pragma once
#endif

#include "FusionCommon.h"

#include <Rocket/Core.h>

namespace FusionEngine
{

	class RocketSystem : public Rocket::Core::SystemInterface
	{
	public:
		RocketSystem() {}
	public:
		virtual float GetElapsedTime();
		virtual bool LogMessage(EMP::Core::Log::Type type, const EMP::Core::String& message);
	};

	class RocketRenderer : public Rocket::Core::RenderInterface
	{
	public:
		RocketRenderer() {}
		RocketRenderer(CL_GraphicContext gc);

		typedef std::tr1::unordered_map<CL_String, CL_Texture> TextureMap;
		typedef std::list<CL_VertexArrayBuffer> GeometryMap;
	public:
		//! Called by Rocket when it wants to render geometry that it does not wish to optimise.
		virtual void RenderGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture, const EMP::Core::Vector2f& translation);

		//! Called by Rocket when it wants to compile geometry it believes will be static for the forseeable future.
		virtual Rocket::Core::CompiledGeometryHandle CompileGeometry(Rocket::Core::Vertex* vertices, int num_vertices, int* indices, int num_indices, Rocket::Core::TextureHandle texture);
		//! Called by Rocket when it wants to render application-compiled geometry.
		virtual void RenderCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry, const EMP::Core::Vector2f& translation);
		//! Called by Rocket when it wants to release application-compiled geometry.
		virtual void ReleaseCompiledGeometry(Rocket::Core::CompiledGeometryHandle geometry);

		//! Called by Rocket when it wants to enable or disable scissoring to clip content.
		virtual void EnableScissorRegion(bool enable);
		//! Called by Rocket when it wants to change the scissor region.
		virtual void SetScissorRegion(int x, int y, int width, int height);

		//! Called by Rocket when a texture is required by the library.
		virtual bool LoadTexture(Rocket::Core::TextureHandle& texture_handle, EMP::Core::Vector2i& texture_dimensions, const EMP::Core::String& source, const EMP::Core::String& source_path);
		//! Called by Rocket when a texture is required to be built from an internally-generated sequence of pixels.
		virtual bool GenerateTexture(Rocket::Core::TextureHandle& texture_handle, const EMP::Core::byte* source, const EMP::Core::Vector2i& source_dimensions);
		//! Called by Rocket when a loaded texture is no longer required.
		virtual void ReleaseTexture(Rocket::Core::TextureHandle texture);
	protected:
		CL_GraphicContext m_gc;

		int m_Scissor_left;
		int m_Scissor_top;
		int m_Scissor_right;
		int m_Scissor_bottom;
		//TextureMap m_Textures;
		//GeometryMap m_Geometry;;
	};

	class RocketFileSystem : public Rocket::Core::FileInterface
	{
	public:
		RocketFileSystem();
		virtual ~RocketFileSystem();

		/// Opens a file.		
		virtual Rocket::Core::FileHandle Open(const EMP::Core::String& path);

		/// Closes a previously opened file.		
		virtual void Close(Rocket::Core::FileHandle file);

		/// Reads data from a previously opened file.		
		virtual size_t Read(void* buffer, size_t size, Rocket::Core::FileHandle file);

		/// Seeks to a point in a previously opened file.		
		virtual bool Seek(Rocket::Core::FileHandle file, long offset, int origin);

		/// Returns the current position of the file pointer.		
		virtual size_t Tell(Rocket::Core::FileHandle file);

	private:
		EMP::Core::String m_Root;
	};

}

#endif