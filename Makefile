CXXFLAGS += -g -Wall -pedantic

all: pcd_fuse

pcd_fuse:
	gcc -Wall pcd_fuse.c `pkg-config fuse3 --cflags --libs` -o pcd_fuse

clean:
	$(RM) pcd_fuse
