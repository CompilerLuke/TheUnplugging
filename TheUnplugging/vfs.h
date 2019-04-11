#pragma once
#include <string>
#include <fstream>

struct Level {
	std::string asset_folder_path;
};

std::string asset_path(const std::string& filename);
std::string to_asset_path(const std::string& filename);

struct File {
	std::ifstream fstream;

	File(const std::string&);
	std::string read();
};

#include <time.h>

long time_modified(const std::string& filename);

void set_level(const std::string& asset_folder_path);