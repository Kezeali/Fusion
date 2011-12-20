
// Credit goes mostly to Gilles Vollant for minizip code

#include "FusionArchive.h"

/// Fusion
#include "FusionPaths.h"

/// System
#ifdef unix
# include <unistd.h>
# include <utime.h>
#else
# include <direct.h>
# include <io.h>
#endif

/// Defines
#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)
#define MAXFILENAME (256)

#ifdef WIN32
#define USEWIN32IOAPI
#include "iowin32.h"
#endif

using namespace FusionEngine;

Archive::Archive()
{
}

Archive::Archive(const std::string &filename)
{
	Open(filename);
}

void Archive::Open(const std::string &filename)
{
	m_ZipFile = filename;
}

bool Archive::Decompress()
{
	//const char *dirname = NULL;
	const char *zipfilename = NULL;
	char filename_try[MAXFILENAME+16] = "";
	int opt_extractdir = 0;
	unzFile zfile = NULL;

	// The return value
	bool ex_success = false;

	// Extract to the temp dir (now I use CL_Directory, so I can use a std::string directly)
	//dirname = TempPath.c_str();

#ifdef USEWIN32IOAPI
	zlib_filefunc_def ffunc;
#endif

	strncpy(filename_try, zipfilename, MAXFILENAME-1);
	/* strncpy doesnt append the trailing NULL, of the string is too long. */
	filename_try[ MAXFILENAME ] = '\0';

#ifdef USEWIN32IOAPI
	fill_win32_filefunc(&ffunc);
	zfile = unzOpen2(zipfilename, &ffunc);
#else
	zfile = unzOpen(zipfilename);
#endif
	// Try appending .zip to the filename (if the file couldn't be opened)
	if (zfile == NULL)
	{
		strcat(filename_try, ".zip");
#ifdef USEWIN32IOAPI
		zfile = unzOpen2(filename_try, &ffunc);
#else
		zfile = unzOpen(filename_try);
#endif
	}

	if (zfile == NULL)
	{
		std::cout << "Cannot open " << zipfilename << " or " << zipfilename << ".zip\n";
		return false;
	}
	std::cout << filename_try << " opened\n";

	if (opt_extractdir && CL_Directory::change_to(TempPath)) 
	{
		std::cout << "Error changing into " << TempPath << " aborting\n";
	}

	// Finally try extracting
	int opt_withoutpath = 0;
	ex_success = (do_extract(zfile, opt_withoutpath, "") == 1) ? true : false;

	unzCloseCurrentFile(zfile);

	return ex_success;
}

const StringVector &Archive::GetFileList() const
{
	return m_FileList;
}

std::string Archive::GetFile(const std::string &file)
{
	StringVector::iterator it = m_FileList.begin();
	for (; it != m_FileList.end(); ++it)
	{
		if ((*it) == file)
		{
			std::string ret = TempPath + file;
			return ret;
		}
	}

	// File not found
	return std::string();
}

void Archive::DeleteTemps()
{
	// Doesn't do anything yet... sry
	StringVector::iterator it = m_FileList.begin();
	for (; it != m_FileList.end(); ++it)
	{
		std::cout << "Archive didn't delete: " << (*it) << std::endl;
	}
}

int Archive::do_extract_currentfile(
			unzFile uf,
			const int popt_extract_without_path,
			const char* password)
{
	char filename_inzip[256];
	char* filename_withoutpath;
	char* p;
	int err = UNZ_OK;
	FILE *fout = NULL;
	void* buf;
	uInt size_buf;

	unz_file_info file_info;
	uLong ratio = 0;
	err = unzGetCurrentFileInfo(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);

	if (err != UNZ_OK)
	{
		std::cout << "error " << err << " with zipfile in unzGetCurrentFileInfo\n";
		return err;
	}

	size_buf = WRITEBUFFERSIZE;
	buf = (void*)malloc(size_buf);
	if (buf == NULL)
	{
		std::cout << "Error allocating memory\n";
		return UNZ_INTERNALERROR;
	}

	p = filename_withoutpath = filename_inzip;
	while ((*p) != '\0')
	{
		if (((*p) == '/') || ((*p) == '\\'))
			filename_withoutpath = p+1;
		p++;
	}

	if ((*filename_withoutpath) == '\0')
	{
		if (popt_extract_without_path == 0)
		{
			std::cout << "creating directory: " << filename_inzip << "\n";
			CL_Directory::create(std::string(filename_inzip));
		}
	}
	else
	{
		const char* write_filename;
		int skip = 0;

		if (popt_extract_without_path == 0)
			write_filename = filename_inzip;
		else
			write_filename = filename_withoutpath;

		err = unzOpenCurrentFilePassword(uf, password);
		if (err != UNZ_OK)
		{
			std::cout << "error " << err << " with zipfile in unzOpenCurrentFilePassword\n";
		}

		if ((skip == 0) && (err == UNZ_OK))
		{
			fout = fopen(write_filename, "wb");

			// Some zipfile don't contain directory alone before file
			if ((fout == NULL) && (popt_extract_without_path == 0) &&
				(filename_withoutpath != (char*)filename_inzip))
			{
				char c = *(filename_withoutpath-1);
				*(filename_withoutpath-1) = '\0';
				CL_Directory::create(write_filename);
				*(filename_withoutpath-1) = c;
				fout = fopen(write_filename, "wb");
			}

			if (fout == NULL)
			{
				std::cout << "error opening " << write_filename << std::endl;
			}
		}

		if (fout != NULL)
		{
			std::cout << " extracting: " << write_filename << std::endl;

			do
			{
				err = unzReadCurrentFile(uf, buf, size_buf);
				if (err < 0)
				{
					printf("error %d with zipfile in unzReadCurrentFile\n",err);
					break;
				}
				if (err > 0)
					if (fwrite(buf, err, 1, fout) != 1)
					{
						std::cout << "error in writing extracted file\n";
						err = UNZ_ERRNO;
						break;
					}
			}
			while (err > 0);
			if (fout)
				fclose(fout);

			if (err == 0)
				change_file_date(write_filename, file_info.dosDate, file_info.tmu_date);
		}

		if (err == UNZ_OK)
		{
			err = unzCloseCurrentFile(uf);
			// Who knows why the file wouldn't close (memory error?)... but anyway:
			if (err != UNZ_OK)
			{
				std::cout << "error " << err << " with zipfile in unzCloseCurrentFile\n";
			}
			else
			{
				// Hopefully, if we got here the file was successfully decompressed
				m_FileList.push_back(std::string(write_filename));
			}
		}
		else
			unzCloseCurrentFile(uf); // don't lose the error
	}

	free(buf);
	return err;
}


int Archive::do_extract(
			unzFile uf,
			const int opt_extract_without_path,
			const char* password)
{
	uLong i;
	unz_global_info gi;
	int err;
	FILE* fout = NULL;

	err = unzGetGlobalInfo (uf, &gi);
	if (err != UNZ_OK)
		std::cout << "error " << err << " with zipfile in unzGetGlobalInfo \n";

	for (i=0; i<gi.number_entry; i++)
	{
		if (do_extract_currentfile(uf, opt_extract_without_path, password) != UNZ_OK)
			break;

		if ((i+1) < gi.number_entry)
		{
			err = unzGoToNextFile(uf);
			if (err != UNZ_OK)
			{
				std::cout << "error " << err << " with zipfile in unzGoToNextFile\n";
				break;
			}
		}
	}

	return 0;
}

void Archive::change_file_date(
    const char *filename,
    uLong dosdate,
    tm_unz tmu_date)
{
#ifdef WIN32
	HANDLE hFile;
	FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

	hFile = CreateFile(LPCWSTR(filename),GENERIC_READ | GENERIC_WRITE,
		0,NULL,OPEN_EXISTING,0,NULL);
	GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
	DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
	LocalFileTimeToFileTime(&ftLocal,&ftm);
	SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
	CloseHandle(hFile);
#else
#ifdef unix
	struct utimbuf ut;
	struct tm newdate;
	newdate.tm_sec = tmu_date.tm_sec;
	newdate.tm_min=tmu_date.tm_min;
	newdate.tm_hour=tmu_date.tm_hour;
	newdate.tm_mday=tmu_date.tm_mday;
	newdate.tm_mon=tmu_date.tm_mon;
	if (tmu_date.tm_year > 1900)
		newdate.tm_year=tmu_date.tm_year - 1900;
	else
		newdate.tm_year=tmu_date.tm_year ;
	newdate.tm_isdst=-1;

	ut.actime=ut.modtime=mktime(&newdate);
	utime(filename,&ut);
#endif
#endif
}
