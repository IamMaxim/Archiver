#pragma once

#include <vector>
#include <string>
#include "dirent.h"

std::vector<std::string> getFilesInDir(char* path) {
	DIR *dir;
	struct dirent *ent;
	std::vector<std::string> files;
	if ((dir = opendir(path)) != NULL) {
		while ((ent = readdir(dir)) != NULL) {
			//if (ent->d_type == DT_DIR)
			//	strcat(ent->d_name, "/");
			files.push_back(ent->d_name);
		}
		closedir(dir);
	} else {
		return std::vector<std::string>();
	}

	//remove ./ and ../
	files.erase(files.begin(), files.begin() + 2);
	return files;
}

bool isDir(char* path) {
	struct stat info;
	if (stat(path, &info) == 0) {
		return info.st_mode & S_IFDIR;
	} else return false;
}

bool isDir(std::string &path) {
	return isDir((char*) path.c_str());
}

void mkDirs(std::string &path) {
	std::string currentDir = "";
	int index;

	if (path.at(path.length() - 1) != '/')
		path += '/';

	while ((index = path.find("/")) != -1) {
		mkdir((currentDir + "/" + path.substr(0, index)).c_str());

		currentDir += path.substr(0, index);
		path = path.substr(index + 1, path.length() - index - 1);
	}
}