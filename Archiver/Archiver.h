#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
//#include <direct.h>
#include <regex>
#include "FS.h"

#define min(a, b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

class Archiver {
public:
    struct File {
        std::string *filename;
        uint64_t startPos;
        uint64_t length;

        File(std::string *filename) {
            this->filename = filename;
        }

        File(std::string *filename, uint64_t startPos, uint64_t length) {
            this->filename = filename;
            this->startPos = startPos;
            this->length = length;
        }

        File() {}
    };

    std::vector<File> files;

private:
    static const uint64_t BUFFER_SIZE = 1024;

    bool write;
    char *filepath;
    std::ofstream *ofs;
    std::ifstream *ifs;
    uint64_t headerLength;

    void preWriteHeader() {
        uint64_t blank = 0;
        headerLength = 8; //64bit file count int
        uint64_t filesSize = files.size();
        ofs->write((char *) &filesSize, 8);

        /*for each (File f in files) {
            headerLength += f.filename->length() + 18; // add filename length and 16 bytes for startPos, length and filepathLength

            uint16_t filepathLength = f.filename->length();

            ofs->write((char*)&filepathLength, 2);
            ofs->write(f.filename->c_str(), f.filename->length());
            ofs->write((char*)&blank, 8);
            ofs->write((char*)&blank, 8);
        }*/

        for (File f : files) {
            headerLength += f.filename->length() +
                            18; // add filename length and 16 bytes for startPos, length and filepathLength

            uint16_t filepathLength = f.filename->length();

            ofs->write((char *) &filepathLength, 2);
            ofs->write(f.filename->c_str(), f.filename->length());
            ofs->write((char *) &blank, 8);
            ofs->write((char *) &blank, 8);
        }
    }

    void postWriteHeaderFile(int64_t fileIndex, uint64_t startPos, uint64_t length) {
        uint64_t pos = 8;
        for (int64_t i = 0; i < fileIndex; i++) {
            pos += files[i].filename->length() +
                   18; // add filename length and 18 bytes for startPos, length and filepathLength
        }

        pos += files[fileIndex].filename->length() + 2;

        //save current position
        uint64_t curPos = (uint64_t) ofs->tellp();

        ofs->seekp(pos);
        ofs->write((char *) &startPos, 8);
        ofs->write((char *) &length, 8);

        //restore current position
        ofs->seekp(curPos);
    }

    void writeFiles() {
        uint64_t pos = headerLength;
        uint64_t fileIndex = 0;

        printf("Writing archive...");

/*		for each(File f in files) {
			std::ifstream ifs(*f.filename, std::ios::in | std::ios::binary);

			uint64_t startPos = ofs->tellp();
			uint64_t length = 0;

			//check if file exists and ready
			if (!ifs || !ifs.good()) {
				throw new std::exception(("Can't add file '" + *f.filename + "'\n").c_str());
			}

			printf("\r> Writing file '%s'", f.filename->c_str());

			char buffer[BUFFER_SIZE]; //byte buffer
			while (true) {
				ifs.read(buffer, BUFFER_SIZE);
				uint64_t read = ifs.gcount(); //amount of data read

				if (read == 0)
					break;

				ofs->write(buffer, read);
				length += read;
			}

			postWriteHeaderFile(fileIndex, startPos, length);

			ifs.close();
			fileIndex++;
		}*/

        for (File f : files) {
            std::ifstream ifs(*f.filename, std::ios::in | std::ios::binary);

            int64_t startPos = ofs->tellp();
            uint64_t length = 0;

            //check if file exists and ready
            if (!ifs || !ifs.good()) {
                printf("Can't add file '%s'\n", f.filename->c_str());
                throw new std::exception();
            }

            printf("\33[2K\r> Writing file '%s'     ", f.filename->c_str());

            char buffer[BUFFER_SIZE]; //byte buffer
            while (true) {
                ifs.read(buffer, BUFFER_SIZE);
                int64_t read = ifs.gcount(); //amount of data read

                if (read == 0)
                    break;

                ofs->write(buffer, read);
                length += read;
            }

            postWriteHeaderFile(fileIndex, startPos, length);

            ifs.close();
            fileIndex++;
        }

        printf("\33[2K\rAll files written     \n");
    }

    void readHeader() {
        uint64_t count;
        ifs->read((char *) &count, 8);

        for (int i = 0; i < count; i++) {
            uint16_t filepathLength;
            uint64_t startPos, length;
            ifs->read((char *) &filepathLength, 2);

            char filepathBuffer[65536];
            ifs->read(filepathBuffer, filepathLength);

            std::string *filepath = new std::string(filepathBuffer, filepathLength);
            ifs->read((char *) &startPos, 8);
            ifs->read((char *) &length, 8);

            files.push_back(File(filepath, startPos, length));
        }
    }

    inline bool fileExists(const std::string &name) {
        if (FILE *file = fopen(name.c_str(), "r")) {
            fclose(file);
            return true;
        } else {
            return false;
        }
    }

public:
    Archiver(char *filename, bool write) {
        this->filepath = filename;
        this->write = write;
        if (write)
            ofs = new std::ofstream(filename, std::ios::out | std::ios::binary);
        else
            ifs = new std::ifstream(filename, std::ios::in | std::ios::binary);
    }

    ~Archiver() {
        printf("Destroying archiver '%s'\n", filepath);
        if (write)
            ofs->close();
        else
            ifs->close();
    }

    void addFile(char *filename, char *dir) {
        std::string filepath = std::string(dir);
        std::replace(filepath.begin(), filepath.end(), '\\', '/');
        if (!filepath.empty() && filepath.at(filepath.size() - 1) != '/')
            filepath += "/";
        filepath += filename;

        if (isDir(filepath)) {
            std::vector<std::string> files = getFilesInDir((char *) filepath.c_str());
//			for each (std::string f in files)
            for (std::string f : files)
                addFile((char *) f.c_str(), (char *) filepath.c_str());
        } else
            files.push_back(File(new std::string(filepath)));
    }

    void writeArchive() {
        if (!write) {
            printf("Archive opened in read mode! Can't write");
            return;
        }

        preWriteHeader();
        writeFiles();
    }

    void readArchive() {
        if (write) {
            printf("Archive opened in write mode! Can't read");
            return;
        }

        readHeader();
    }

    void restoreFile(int index) {
        File f = files[index];
        std::string path = *f.filename;
        std::replace(path.begin(), path.end(), '\\', '/');
        if (path.find("/") != -1) {
            std::string dir = path.substr(0, path.find_last_of("/"));
            if (!isDir(dir))
                mkDirs(dir);
        }

        std::ofstream *ofs = new std::ofstream(path, std::ios::out | std::ios::binary);

        ifs->seekg(f.startPos);
        uint64_t read = 0;
        char buffer[BUFFER_SIZE]; //byte buffer
        while (true) {
            int toRead = min(f.length - read, BUFFER_SIZE);

            if (toRead <= 0)
                break;

            ifs->read(buffer, toRead);
            ofs->write(buffer, toRead);
            read += toRead;
        }

        ofs->close();
    }
};