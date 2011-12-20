/*!
 * \file FusionPaths.h
 * Perhaps these should be loaded dynamically, but for now they're defined here
 */

namespace FusionEngine
{
	//! The relative path to packages, including trailing slash.
	const std::string s_PackagesPath = "Data/";
	//! The relative path to temporary data, including trailing slash.
	const std::string s_TempPath = "cache/";
	//! The relative path to the log folder, including trailing slash.
	const std::string s_LogfilePath = "Logs/";
	//! The path to the saves folder
	const std::string s_SavePath = "Saves/";
	//! The path to the editor saved-data folder
	const std::string s_EditorPath = "Editor/";
}
