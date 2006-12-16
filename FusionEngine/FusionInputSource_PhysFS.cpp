
#include "FusionInputSource_PhysFS.h"

InputSource_PhysFS::InputSource_PhysFS(const std::string &filename)
{
	m_Filename = filename;
	m_PhysFile = NULL;
	open();
}

InputSource_PhysFS::InputSource_PhysFS(const InputSource_PhysFS *source)
{
	m_Filename = source->m_Filename;
	m_PhysFile = NULL;

	open();
	PHYSFS_seek(m_PhysFile, PHYSFS_tell(source->m_PhysFile));
}

InputSource_PhysFS::~InputSource_PhysFS()
{
	close();
}

//std::string CL_InputSource_File::translate_path(const std::string &path)
//{
//	int len = 0;
//
//	// try to figure out if path is absolute.
//	if (path.length() > 0 && (path[0] == '\\' || path[0] == '/' || (path.length() > 1 && path[1] == ':')))
//	{
//		// path is absolute
//		return path;
//	}
//	else
//	{
//
//		//note, I moved BigZaphod's mac path hack to Application/MacOS/clanapp.cpp -mrfun may 18 2006
//		
//		char cwd[1026];
//		if (getcwd(cwd, 1024) == NULL) throw CL_Error("Working dir is more than 1024 characters!");
//		len = strlen(cwd);
//		if (cwd[len-1] != '/' && cwd[len-1] != '\\') strcat(cwd, "/");
//		
//		return std::string(cwd) + std::string(path);
//	}
//}

int InputSource_PhysFS::read(void *data, int size)
{
	return PHYSFS_read(m_PhysFile, data, sizeof(char), size);
}

void InputSource_PhysFS::open()
{
	if (m_PhysFile != NULL) return;

	//m_Filename = translate_path(m_Filename);

	m_PhysFile = PHYSFS_openRead(m_Filename.c_str());
	if (m_PhysFile == NULL)
	{
		throw CL_Error(PHYSFS_getLastError());
	}

	m_Filesize = PHYSFS_fileLength(m_PhysFile);

}

void InputSource_PhysFS::close()
{
	if (m_PhysFile == NULL) return;
	PHYSFS_close(m_PhysFile);

	m_PhysFile = NULL;
}

CL_InputSource *InputSource_PhysFS::clone() const
{
	return new InputSource_PhysFS(this);
}

int InputSource_PhysFS::tell() const
{
	return PHYSFS_tell(m_PhysFile);
}

void InputSource_PhysFS::seek(int pos, SeekEnum seek_type)
{
	// PHYSFS only supports seeking form the begining of the file, so
	//  we have to do a little bit of extra work here...
	switch (seek_type)
	{
		case seek_cur:
			// seek from current position
			PHYSFS_seek(m_PhysFile, PHYSFS_tell(m_PhysFile) + pos);
			break;

		case seek_set:
			PHYSFS_seek(m_PhysFile, pos);
			break;

		case seek_end:
			// seek from end
			PHYSFS_seek(m_PhysFile, PHYSFS_fileLength(m_PhysFile) - pos);
			break;
	}
}

int InputSource_PhysFS::size() const
{
	return m_Filesize;
}

void InputSource_PhysFS::push_position()
{
	PHYSFS_sint64 a = PHYSFS_tell(m_PhysFile);

	m_Stack.push(a);
}

void InputSource_PhysFS::pop_position()
{
	PHYSFS_sint64 a = m_Stack.top();
	m_Stack.pop();

	PHYSFS_seek(m_PhysFile, a);
}
