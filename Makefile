block_test: block_test.c disk.c db.c sb.c
	gcc block_test.c disk.c db.c sb.c -o block_test

inode_test: inode.c db.c disk.c sb.c
	gcc inode.c db.c disk.c sb.c -o inode_test

clean:
	rm inode_test
	rm block_test
