#include "vfs.h"
#include <string_view>
#include <fstream>
#include <time.h>
#include <sys/stat.h>
#include <sstream>

void Level::set_level(const std::string& asset_folder_path) {
	this->asset_folder_path = asset_folder_path;
}

std::string Level::asset_path(const std::string& filename) {
	return asset_folder_path + filename;
}

std::string Level::to_asset_path(const std::string& filename) {
	std::string_view filename_view = filename;
	if (filename_view._Starts_with(asset_folder_path)) {
		return filename.substr(asset_folder_path.size(), filename.size());
	}

	throw "File not in asset path";
}

File::File(Level& level, const std::string& filename) {
	auto full_path = level.asset_path(filename);

	fstream.open(full_path);
	if (fstream.fail()) {
		throw std::string("Could not open file") + filename;
	}
}

std::string File::read() {
	std::stringstream strStream;
	strStream << fstream.rdbuf();
	return strStream.str();
}

long long Level::time_modified(const std::string& filename) {
	std::string f = asset_path(filename);
	
	struct _stat buffer;
	if (_stat(f.c_str(), &buffer) != 0) {
		throw std::string("Could not read file ") + f;
	}
	return buffer.st_mtime;
}