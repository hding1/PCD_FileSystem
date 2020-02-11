CXXFLAGS += -g -Wall -pedantic

files := disk.c db.c sb.c inode.c fs.c syscall.c pcd_fuse.c

all: pcd_fuse test

pcd_fuse:
	gcc -Wall $(files) `pkg-config fuse3 --cflags --libs` -o pcd_fuse


test: test.c disk.c db.c sb.c
	gcc test.c disk.c db.c sb.c -o test

clean:
	$(RM) pcd_fuse test
