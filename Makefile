### variables ###

CFLAGS += -g -Wall -pedantic -Werror

files := disk.c db.c sb.c inode.c fs.c syscall.c pcd_fuse.c

# This specifies how fuse wants to be compiled. Equivalent to
# -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -lfuse -pthread
FUSE_LIB := `pkg-config fuse --cflags --libs`

targets := pcd_fuse layer1_test syscall_test



### targets ###

fuse: $(targets)

pcd_fuse: disk.c db.c sb.c inode.c fs.c syscall.c pcd_fuse.c
	$(CC) $(CFLAGS) $^ $(FUSE_LIB) -o $@

layer1_test: test_layer1.c disk.c db.c sb.c inode.c
	$(CC) $(CFLAGS) $^ -o $@

# inode_test: test_inode.c inode.c db.c disk.c sb.c
# 	$(CC) $(CFLAGS) $^ -o $@

syscall_test: test_syscall.c disk.c db.c sb.c inode.c fs.c syscall.c
	$(CC) $(CFLAGS) $^ $(FUSE_LIB) -o $@

clean:
	$(RM) $(targets)

.PHONY: clean fuse



### old stuff ###

# help:
# 	#please use "make fuse" or "make fuse3"

# fuse3: FUSE_LIB := `pkg-config fuse3 --cflags --libs`
# fuse3: pcd_fuse test_layer1 syscall_test