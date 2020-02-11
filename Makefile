CXXFLAGS += -g -Wall -pedantic

all: pcd_fuse test

pcd_fuse:
	gcc -Wall pcd_fuse.c `pkg-config fuse3 --cflags --libs` -o pcd_fuse


test: test.c disk.c db.c sb.c
	gcc test.c disk.c db.c sb.c -o test

clean:
	$(RM) pcd_fuse test
