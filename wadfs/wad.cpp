#include "wad.h"

using namespace std;

unsigned long fetchint(unsigned char *p)
{
	return (unsigned long)p[0] | (unsigned long)p[1] << 8 | (unsigned long)p[2] << 16 | (unsigned long)p[3] << 24;
}

Wad::Wad(const char * cWu)
{
	c = cWu;
	wad;
	head;
	unsigned char hbuf[12];

	struct elements *waddir, *current;
	struct node *parent, *active;

	if (NULL == (wad = fopen(c, "rb")))		//Here we are checking if we can access the file in question
	{
		printf("\n ERROR: cannot access %s\n", c);
		exit(EXIT_FAILURE);
	}

	if (1 != fread(hbuf, 12, 1, wad))		//Here we are loading the header into our hbuf char
	{
		printf("\n ERROR: could not read the header from %s\n", c);
		exit(EXIT_FAILURE);
	}
	wad_header_init(&head, hbuf);			//Create our head struct

	if (0 != fseek(wad, head.offset, SEEK_SET)) 
	{		//Shift our stream with the offset of the header
		printf("\n Could not seek to by offset %lu \n", head.offset);
		exit(EXIT_FAILURE);
	}


	//Allocatin the memory for our elements
	waddir = (struct elements *)malloc((unsigned long)sizeof(struct elements)*head.num);
	if (NULL == waddir) 
	{
		printf("\n ERROR: could not allocate %lu bytes\n", ((unsigned long)sizeof(struct elements)*head.num));
		exit(EXIT_FAILURE);
	}
	current = waddir;
	
	sorted;

	//The Generate wad stuff function
	//*******************************************************************************************************************************************************
	parent = nullptr;
	active = new node();
	for (unsigned long x = 0; x < head.num; x++)
	{
		unsigned char dbuf[16];

		if (1 != fread(dbuf, 16, 1, wad)) {
			printf("ERROR cannot read DIR \n");
			exit(EXIT_FAILURE);
		}

		wad_dir_init(current, dbuf);

		active->parent = parent;
		active->content = *current;
		active->isContent = false;
		active->isDir = false;

		if ((current->name[2] == (unsigned) '_' && current->name[3] == (unsigned) 'S' &&
			current->name[4] == (unsigned) 'T' && current->name[5] == (unsigned) 'A' &&
			current->name[6] == (unsigned) 'R' && current->name[7] == (unsigned) 'T' &&
			current->name[8] == (unsigned) '\0') || (
			current->name[1] == (unsigned) '_' && current->name[2] == (unsigned) 'S' &&
			current->name[3] == (unsigned) 'T' && current->name[4] == (unsigned) 'A' &&
			current->name[5] == (unsigned) 'R' && current->name[6] == (unsigned) 'T' &&
			current->name[7] == (unsigned) '\0'))
		{

			if (current->name[2] == (unsigned) '_')
			{
				active->content.name[2] = '\0';
			}
			if (current->name[1] == (unsigned) '_')
			{
				active->content.name[2] = '\0';
			}

			parent = new node();
			parent->parent = active->parent;
			parent->content = active->content;

			active->isDir = true;
			sorted.push_back(*active);

			//printf("Start trigger \n");
		}

		//If statement to check if _END tail exists
		else if ((current->name[2] == (unsigned) '_' && current->name[3] == (unsigned) 'E' &&
			current->name[4] == (unsigned) 'N' && current->name[5] == (unsigned) 'D' &&
			current->name[6] == '\0') || (
			current->name[1] == (unsigned) '_' && current->name[2] == (unsigned) 'E' &&
			current->name[3] == (unsigned) 'N' && current->name[4] == (unsigned) 'D' &&
			current->name[5] == '\0'))
		{
			if (parent->parent == nullptr)
			{
				parent = nullptr;
			}
			else
			{
				node* temp = new node();
				temp->parent = parent->parent->parent;
				temp->content = parent->parent->content;
				parent = new node();
				parent->parent = temp->parent;
				parent->content = temp->content;
			}


			//printf("End trigger \n");
		}

		//if statement to check if e*m* is declared
		else if (current->name[0] == (unsigned) 'E' && current->name[2] == (unsigned) 'M' &&
			current->name[4] == '\0')
		{

			parent = new node();
			parent->parent = active->parent;
			parent->content = active->content;

			active->isDir = true;
			sorted.push_back(*active);
			for (int y = 0; y < 10; y++)
			{
				active = new node();

				if (1 != fread(dbuf, 16, 1, wad)) {
					printf("ERROR cannot read DIR \n");
					exit(EXIT_FAILURE);
				}

				wad_dir_init(current, dbuf);

				active->parent = parent;
				active->content = *current;
				active->isContent = true;
				sorted.push_back(*active);
			}
			x = x + (unsigned long)10;

			if (parent->parent == nullptr)
			{
				parent = nullptr;
			}
			else
			{
				node* temp = new node();
				temp->parent = parent->parent->parent;
				temp->content = parent->parent->content;
				parent = new node();
				parent->parent = temp->parent;
				parent->content = temp->content;
			}

			//printf("E*M* trigger \n");
		}
		else
		{
			active->parent = parent;
			active->isContent = true;
			sorted.push_back(*active);

			//printf("Content trigger \n");
		}

	}

	//*******************************************************************************************************************************************************

	/*for (vector<node>::iterator it = sorted.begin(); it != sorted.end(); ++it)
	{
		printf("%8s \n", it->content.name);
	}*/

	fclose(wad);
}

Wad::~Wad()
{
}

Wad* Wad::loadWad(const string &path)
{
	const char* cWu = path.c_str();

	Wad *wad = new Wad(cWu);

	wad->magic = new char[5];
	wad->magic[0] = wad->head.type;
	wad->magic[1] = 'W';
	wad->magic[2] = 'A';
	wad->magic[3] = 'D';
	wad->magic[4] = '\0';

	return wad;
}
char* Wad::getMagic()
{
	return this->magic;
}

bool Wad::isContent(const string &path)
{
	vector<string> pathTokens = pathing(path);

	if (pathTokens.size() == 0)
		return false;

	int it;
	int pNum = 0;
	node pathNode;
	for (it = 0; it < sorted.size(); it++)
	{
		string str = sorted[it].content.name;


		if (pathTokens[pNum] == str)
		{
			if (sorted[it].parent == nullptr && pNum == 0)
			{
				pathNode = sorted[it];
			}
			else if ((string)sorted[it].parent->content.name == pathTokens[pNum-1])
			{
				pathNode = sorted[it];
			}
			else
			{
				//printf("Invalid path \n");
				return false;
			}
			pNum++;
		}

		if (pNum >= pathTokens.size())
			break;
	}
	if (pNum < pathTokens.size())
		return false;

	if (pathNode.isContent == true)
		return true;
	return false;
}

bool Wad::isDirectory(const string &path)
{
	vector<string> pathTokens = pathing(path);

	if (pathTokens.size() == 0)
		return true;

	int it;
	int pNum = 0;
	node pathNode;
	for (it = 0; it < sorted.size(); it++)
	{
		string str = sorted[it].content.name;


		if (pathTokens[pNum] == str)
		{
			if (sorted[it].parent == nullptr && pNum == 0)
			{
				pathNode = sorted[it];
			}
			else if ((string)sorted[it].parent->content.name == pathTokens[pNum - 1])
			{
				pathNode = sorted[it];
			}
			else
			{
				//printf("Invalid path \n");
				return false;
			}
			pNum++;
		}

		if (pNum >= pathTokens.size())
			break;
	}
	if (pNum < pathTokens.size())
		return false;

	if (pathNode.isDir == true)
		return true;
	return false;
}

int Wad::getSize(const string &path)
{
	if (!(this->isContent(path)))
		return -1;

	vector<string> pathTokens = pathing(path);

	if (pathTokens.size() == 0)
		return -1;

	int it;
	int pNum = 0;
	node pathNode;
	for (it = 0; it < sorted.size(); it++)
	{
		string str = sorted[it].content.name;


		if (pathTokens[pNum] == str)
		{
			if (sorted[it].parent == nullptr && pNum == 0)
			{
				pathNode = sorted[it];
			}
			else if ((string)sorted[it].parent->content.name == pathTokens[pNum - 1])
			{
				pathNode = sorted[it];
			}
			else
			{
				//printf("Invalid path \n");
				return -1;
			}
			pNum++;
		}

		if (pNum >= pathTokens.size())
			break;
	}

	return (int) pathNode.content.length;

}

int Wad::getContents(const string &path, char *buffer, int length, int offset)
{
	if (!(this->isContent(path)))
		return -1;

	vector<string> pathTokens = pathing(path);

	if (pathTokens.size() == 0)
		return -1;

	int it;
	int pNum = 0;
	node pathNode;
	for (it = 0; it < sorted.size(); it++)
	{
		string str = sorted[it].content.name;


		if (pathTokens[pNum] == str)
		{
			if (sorted[it].parent == nullptr && pNum == 0)
			{
				pathNode = sorted[it];
			}
			else if ((string)sorted[it].parent->content.name == pathTokens[pNum - 1])
			{
				pathNode = sorted[it];
			}
			else
			{
				//printf("Invalid path \n");
				return -1;
			}
			pNum++;
		}

		if (pNum >= pathTokens.size())
			break;
	}

	if (NULL == (wad = fopen(c, "rb")))		//Here we are checking if we can access the file in question
	{
		printf("\n ERROR: cannot access %s\n", c);
		exit(EXIT_FAILURE);
	}

	if (0 != fseek(wad, (pathNode.content.offset+offset), SEEK_SET))
	{		//Shift our stream with the offset of the header
		printf("\n Could not seek to by offset %lu \n", head.offset);
		exit(EXIT_FAILURE);

	}

	int size = pathNode.content.length - offset;
	int itMax = 0;
	if (length > size)
		itMax = size;
	if (length <= size)
		itMax = length;

	if (1 != fread(buffer, 16, 1, wad)) {
		printf("ERROR cannot read DIR \n");
		exit(EXIT_FAILURE);
	}
	return itMax;
}

int Wad::getDirectory(const string &path, vector<string> *directory)
{
	if (!(this->isDirectory(path)))
		return -1;

	vector<string> pathTokens = pathing(path);

	if (pathTokens.size() == 0)
	{
		int dirNum = 0;
		for (int x = 0; x < sorted.size(); x++)
		{
			if (sorted[x].parent == nullptr)
			{
				dirNum++;
				(*directory).push_back((string)sorted[x].content.name);
			}
		}
		return dirNum;
	}

	int it;
	int pNum = 0;
	node pathNode;
	for (it = 0; it < sorted.size(); it++)
	{
		string str = sorted[it].content.name;


		if (pathTokens[pNum] == str)
		{
			if (sorted[it].parent == nullptr && pNum == 0)
			{
				pathNode = sorted[it];
			}
			else if ((string)sorted[it].parent->content.name == pathTokens[pNum - 1])
			{
				pathNode = sorted[it];
			}
			else
			{
				//printf("Invalid path \n");
				return -1;
			}
			pNum++;
		}

		if (pNum >= pathTokens.size())
			break;
	}

	int dirNum = 0;
	for (it = 0; it < sorted.size(); it++)
	{
		if (sorted[it].parent != nullptr)
		{
			if ((string)sorted[it].parent->content.name == (string)pathNode.content.name)
			{
				dirNum++;
				(*directory).push_back((string)sorted[it].content.name);
			}
		}
	}
	return dirNum;
}

vector<string> pathing(string paths)
{
	vector<string> pathTokens;
	string piece;

	if (paths == "/")
		return pathTokens;

	if (paths.at(0) == '/')
		paths.erase(0, 1);

	stringstream check1(paths);
	while (getline(check1, piece, '/'))
	{
		pathTokens.push_back(piece);
	}

	return pathTokens;
}

void wad_header_init(struct header * h, unsigned char *lump) 
{
	h->type = lump[0];
	h->num = fetchint(lump + 4);
	h->offset = fetchint(lump + 8);
}

void wad_dir_init(struct elements * w, unsigned char * lump) 
{	
	unsigned char c;
	w->offset = fetchint(lump);
	w->length = fetchint(lump + 4);

	/* sanitize the name */
	strcpy((char *)w->name, (const char *) "--------");
	for (c = 8; c < 16; ++c) {
		if (isprint(lump[c])) w->name[c - 8] = lump[c];
		else if ('\0' == lump[c]) {
			w->name[c - 8] = '\0';
			break;
		}
	}
}


