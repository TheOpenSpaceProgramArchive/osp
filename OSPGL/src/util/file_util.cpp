#include "file_util.h"

bool FileUtil::write_file(std::string path, std::string data)
{
	std::ofstream file(path);
	if (file.is_open())
	{
		file << data;
		return true;
	}
	else
	{
		return false;
	}
}

std::string FileUtil::load_file(std::string path)
{
	std::ifstream t(path);
	std::string str((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());

	return str;
}

std::vector<std::string> FileUtil::get_all_files(std::string path, bool recursive)
{
	std::vector<std::string> files;

	using namespace std::experimental::filesystem;

	for (recursive_directory_iterator i(path), end; i != end; ++i)
	{
		if (!is_directory(i->path()))
		{
			files.push_back(i->path().string());
		}
	}

	return files;
}
