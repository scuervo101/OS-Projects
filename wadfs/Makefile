
wadfs: wad.cpp
	g++ -c wad.cpp
	ar cr libwad.a wad.o
	g++ -D_FILE_OFFSET_BITS=64 -DFUSE_USE_VERSION=26 fuse.cpp wad.cpp -o wadfs -lfuse 
