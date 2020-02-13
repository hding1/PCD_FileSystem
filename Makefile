CFLAGS += -g -Wall -pedantic -Werror

files := disk.c db.c sb.c inode.c fs.c syscall.c pcd_fuse.c

all: pcd_fuse block_test test_syscall

pcd_fuse: disk.c db.c sb.c inode.c fs.c syscall.c pcd_fuse.c
	$(CC) $(CFLAGS) $^ `pkg-config fuse3 --cflags --libs` -o $@

block_test: block_test.c disk.c db.c sb.c
	$(CC) $(CFLAGS) $^ -o $@

# inode_test: inode.c db.c disk.c sb.c
# 	$(CC) $(CFLAGS) $^ -o $@

syscall_test: test_syscall.c disk.c db.c sb.c inode.c fs.c syscall.c
	$(CC) $(CFLAGS) $^ `pkg-config fuse3 --cflags --libs` -o $@

clean:
	$(RM) pcd_fuse block_test inode_test syscall_test
