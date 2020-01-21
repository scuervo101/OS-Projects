#define FUSE_USE_VERSION 26

#include <fuse.h>
#include <iostream>
#include <string>
#include <cstring>
#include "wad.h"

using namespace std;

Wad* wad;

fuse_operations fuse_example_operations;

static int getattr_callback(const char *path, struct stat *stbuf) 
{
	memset(stbuf, 0, sizeof(struct stat));
	if (wad->isDirectory(path)) {
		stbuf->st_mode = S_IFDIR | 0555;
		stbuf->st_nlink = 2;
		return 0;
	}
	if (wad->isContent(path)) {
		stbuf->st_mode = S_IFREG | 0444;
		stbuf->st_nlink = 1;
		stbuf->st_size = wad->getSize(path);
		stbuf->st_blocks = wad->getSize(path);
		return 0;
	}
	return -ENOENT;
}


static int readdir_callback(const char *path, void *buf, fuse_fill_dir_t filler,
	off_t offset, struct fuse_file_info *fi) 
{
	(void)offset;
	(void)fi;

	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	vector<string> dirList;

	int dirSize = wad->getDirectory(path, &dirList);
	for (int x = 0; x < dirSize; x++) {
		filler(buf, dirList[x].c_str(), NULL, 0);
	}

	return 0;
}

// Open files
static int open_callback(const char *path, struct fuse_file_info *fi) 
{
	return 0;
}

// Read files
static int read_callback(const char *path, char *buf, size_t size, off_t offset,
	struct fuse_file_info *fi) 
{
	if (wad->isContent(path)) 
		return wad->getContents(path, buf, size, offset);

	return -ENOENT;
}


int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("The format is WAD file and mount directory \n");
		return 0;
	}
	// sets fuse operations
	fuse_example_operations.getattr = getattr_callback;
	fuse_example_operations.open = open_callback;
	fuse_example_operations.read = read_callback;
	fuse_example_operations.readdir = readdir_callback;

	// Loads WAD
	wad = Wad::loadWad(argv[argc - 2]);

	// Remove WAD from args
	argv[1] = argv[2];
	argv[2] = NULL;
	argc--;

	int fuse_stat = fuse_main(argc, argv, &fuse_example_operations, NULL);

	// Deletes WAD
	delete wad;
	return fuse_stat;
}
