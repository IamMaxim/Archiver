#pragma once

#include <vector>
#include <string>

#ifdef _WIN32
#include "dirent.h"
#else

#include <dirent.h>
#include <sys/stat.h>
#include <stdint-gcc.h>

#endif

std::vector<std::string> getFilesInDir(char *path) {
    DIR *dir;
    struct dirent *ent;
    std::vector<std::string> files;
    if ((dir = opendir(path)) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..")) {
                files.push_back(ent->d_name);
            }
        }
        closedir(dir);
    } else {
        return std::vector<std::string>();
    }

    return files;
}

bool isDir(char *path) {
    struct stat info;
    if (stat(path, &info) == 0) {
        return info.st_mode & S_IFDIR;
    } else return false;
}

bool isDir(std::string &path) {
    return isDir((char *) path.c_str());
}

void _mkdir(std::string path) {
    printf("Making dir '%s'\n", path.c_str());
#ifdef _WIN32
    mkdir(path.c_str());
#else
    mkdir(path.c_str(), 0777);
#endif
}

void mkDirs(std::string &path) {
    std::string currentDir = "";
    uint64_t index;

    if (path.at(path.length() - 1) != '/')
        path += '/';

    while ((index = path.find("/")) != -1) {
        _mkdir(currentDir + path.substr(0, index));

        currentDir += path.substr(0, index) + "/";
        path = path.substr(index + 1, path.length() - index - 1);
    }
}