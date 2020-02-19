CFLAGS += -g -Wall -pedantic -Werror

files := disk.c db.c sb.c inode.c fs.c syscall.c pcd_fuse.c

all: pcd_fuse test_layer1 syscall_test

pcd_fuse: disk.c db.c sb.c inode.c fs.c syscall.c pcd_fuse.c
	$(CC) $(CFLAGS) $^ `pkg-config fuse3 --cflags --libs` -o $@

test_layer1: test_layer1.c disk.c db.c sb.c inode.c
	$(CC) $(CFLAGS) $^ -o $@

# inode_test: test_inode.c inode.c db.c disk.c sb.c
# 	$(CC) $(CFLAGS) $^ -o $@

syscall_test: test_syscall.c disk.c db.c sb.c inode.c fs.c syscall.c
	$(CC) $(CFLAGS) $^ `pkg-config fuse3 --cflags --libs` -o $@

clean:
	$(RM) pcd_fuse test_layer1 inode_test syscall_test
