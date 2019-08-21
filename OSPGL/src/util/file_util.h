#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <experimental/filesystem>


class FileUtil
{
public:

	// Returns true if operation was possible
	static bool write_file(std::string path, std::string data);

	static std::string load_file(std::string path);
	// Gets all files contained in the given path, including in subfolders (if recursive is set),
	// and returns their relative path to "path"
	static std::vector<std::string> get_all_files(std::string path, bool recursive);

};

