#include <iostream>
#include "Archiver.h"
#include <vector>

enum Mode {
	UNDEFINED,
	PACK,
	UNPACK
};

int main(int argc, char **argv) {
	Archiver *archiver;
	Mode mode = UNDEFINED;
	std::vector<char*> args;

	// debug
	//archiver = new Archiver("arch.arch", true);
	//archiver->addFile("testdir", "");
	//archiver->writeArchive();
	//delete archiver;

	for (int i = 1; i < argc; i++) { //skip first argument, because it is executabke path
		args.push_back(argv[i]);
	}

	//parse arguments
	for (int i = 0; i < args.size(); i++) {
		if (strcmp(args[i], "-pack") == 0) {
			mode = PACK;
			if (i + 1 >= argc) {
				printf("Excepted output archive name after -pack\n");
				return -1;
			}
			archiver = new Archiver(args[i + 1], true);
			args.erase(args.begin() + i, args.begin() + i + 2);
			i--; //repeat this index again
		} else if (strcmp(args[i], "-unpack") == 0) {
			mode = UNPACK;
			args.erase(args.begin() + i);
		}
	}

	if (mode == UNDEFINED) {
		printf("Usage: archiver [-pack output_file] [-unpack] filepaths...\n");
		return -1;
	}

	if (args.size() == 0) {
		printf("Expected at least one element\n");
		return -1;
	}

	if (mode == PACK) {
		for (int i = 0; i < args.size(); i++) {
			archiver->addFile(args[i], "");
		}
		archiver->writeArchive();
		return 0;
	}
	
	if (mode == UNPACK) {
		printf("Restoring archive...");

		for (int i = 0; i < args.size(); i++) {
			archiver = new Archiver(args[i], false);
			archiver->readArchive();

			for (int j = 0; j < archiver->files.size(); j++) {
				printf("\33[2K\r> Restoring file '%s'", archiver->files[j].filename->c_str());
				archiver->restoreFile(j);
			}
		}

		printf("\33[2K\rAll files restored     \n");
	}

	return 0;
}