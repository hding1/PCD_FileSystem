CFLAGS += -g -Wall -pedantic

files := disk.c db.c sb.c inode.c fs.c syscall.c pcd_fuse.c

all: pcd_fuse block_test

pcd_fuse: disk.c db.c sb.c inode.c fs.c syscall.c pcd_fuse.c
	$(CC) $(CFLAGS) $^ `pkg-config fuse3 --cflags --libs` -o pcd_fuse

block_test: block_test.c disk.c db.c sb.c
	gcc block_test.c disk.c db.c sb.c -o block_test

clean:
	$(RM) pcd_fuse block_test

