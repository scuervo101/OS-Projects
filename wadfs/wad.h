#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <cstring>

unsigned long fetchint(unsigned char *p);

struct elements
{
	//unsigned char* stuff;
	unsigned long offset;
	unsigned long length;
	char name[9];
};

struct node
{
	node *parent;
	//vector<node *> children;
	elements content;
	bool isContent;
	bool isDir;
};

struct header
{
	char type;
	unsigned long num;
	unsigned long offset;
};

class Wad
{
public:
	Wad(const char * c);
	~Wad();

	FILE* wad;

	char* magic;

	const char * c;

	std::vector<node> sorted;
	header head;

	static Wad* loadWad(const std::string &path);

	char* getMagic();

	bool isContent(const std::string &path);

	bool isDirectory(const std::string &path);

	int getSize(const std::string &path);

	int getContents(const std::string &path, char *buffer, int length, int offset = 0);

	int getDirectory(const std::string &path, std::vector<std::string> *directory);

};

std::vector<std::string> pathing(std::string paths);

void wad_header_init(struct header * h, unsigned char *lump);

void wad_dir_init(struct elements * w, unsigned char * lump);

