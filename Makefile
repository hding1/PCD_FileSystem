CFLAGS += -g -Wall -pedantic -Werror -D_FILE_OFFSET_BITS=64

files := disk.c db.c sb.c inode.c fs.c syscall.c pcd_fuse.c

help:
	#please use "make fuse" or "make fuse3"

fuse: FUSE_LIB := `pkg-config fuse --cflags --libs`
fuse: pcd_fuse test_layer1 syscall_test

fuse3: FUSE_LIB := `pkg-config fuse3 --cflags --libs`
fuse3: pcd_fuse test_layer1 syscall_test

pcd_fuse: disk.c db.c sb.c inode.c fs.c syscall.c pcd_fuse.c
	$(CC) $(CFLAGS) $^ $(FUSE_LIB) -o $@

layer1_test: test_layer1.c disk.c db.c sb.c inode.c
	$(CC) $(CFLAGS) $^ -o $@

# inode_test: test_inode.c inode.c db.c disk.c sb.c
# 	$(CC) $(CFLAGS) $^ -o $@

syscall_test: test_syscall.c disk.c db.c sb.c inode.c fs.c syscall.c
	$(CC) $(CFLAGS) $^ $(FUSE_LIB) -o $@

clean:
	$(RM) pcd_fuse test_layer1 inode_test syscall_test
