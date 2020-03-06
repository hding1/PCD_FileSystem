
#define FUSE_USE_VERSION 29

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "fs.h"
#include "syscall.h"

int test_filler(void *buf, const char *name,
				const struct stat *stbuf, off_t off);

int test_filler(void *buf, const char *name,
				const struct stat *stbuf, off_t off){
	printf("name: %s, ", name);
	printf("mode: %d, ", stbuf->st_mode);
	printf("inum: %lu\n", stbuf->st_ino);
	return 0;
}

int test_mkdir(void){
	//making an dir1 under root
	printf("Testing mkdir\n");
	printf("Creating dir1 in /root/dir1\n");
	const char* path1 = "/dir1";
	mode_t dir1 = S_IFDIR;
	int test1 = pcd_mkdir(path1, dir1);
	if(test1<0){
		printf("Mkdir Test1 failed");
		return -1;
	}
	printf("Creating dir2 in /root/dir1/dir2\n");
	//making an dir2 under root/dir1
	const char* path2 = "/dir1/dir2";
	mode_t dir2 = S_IFDIR;
	int test2 = pcd_mkdir(path2, dir2);
	if(test2<0){
		printf("Mkdir Test2 failed");
		return -1;
	}
	printf("Creating dir3 in /root/dir3\n");
	//making an dir3 under root/
	const char* path3 = "/dir3";
	mode_t dir3 = S_IFDIR;
	int test3 = pcd_mkdir(path3, dir3);
	if(test3<0){
		printf("Mkdir Test3 failed");
		return -1;
	}
	printf("Creating dir4 in /root/dir1/dir2/dir4\n");
	//making an dir4 under root/dir1/dir2
	const char* path4 = "/dir1/dir2/dir4";
	mode_t dir4 = S_IFDIR;
	int test4 = pcd_mkdir(path4, dir4);
	if(test4<0){
		printf("Mkdir Test4 failed");
		return -1;
	}
	printf("Four Directories Should be Created\n");
	printf("\troot\ndir3\t\tdir1\n\t\t\tdir2\n\t\t\t\tdir4\n");
	printf("End of Testing mkdir\n\n");
	return 0;
}

int test_readdir(const char* path){
	printf("Reading Directory: %s\n",path);
	char* buf = (char *)malloc(1000);
	if(pcd_readdir(path,buf,test_filler,0,0)<0){
		printf("Read Directory Failed");
		return -1;
	}
	free(buf);
	return 0;
}

int test_unlink(void){
	const char* path = "/dir1/dir2/dir4";
	printf("Unlinking Directory: %s\n",path);
	if(pcd_unlink(path)<0){
		printf("Unlink Test1 Failed");
		return -1;
	}
	const char* dir2Path = "/dir1/dir2";
	test_readdir(dir2Path);
	printf("End of Testing unlink\n\n");
	return 0;
}

int test_mknod(void){
	printf("Testing mknod\n");
	const char* path1 = "/dir1/dir2/test.txt";
	printf("Creating File %s\n",path1);
	pcd_mknod(path1,S_IFREG,0);
	const char* path2 = "/dir1/dir2";
	printf("Read Directory %s\n",path2);
	char* buf = (char *)malloc(1000);
	if(pcd_readdir(path2,buf,test_filler,0,0)<0){
		printf("Read Directory Failed");
		return -1;
	}
	free(buf);
	printf("End of Testing mknod\n\n");
	return 0;
}

int test_open(void){
	return 0;
}

int test_read(void){
	printf("Testing read\n");
	const char* path1 = "/dir1/dir2/test.txt";
	printf("Reading from path '%s'\n",path1);
	char* buf1 = (char *)malloc(21);
	if(pcd_read(path1, buf1, 20, 0, 0)){
		printf("Read Failed");
		return -1;
	}
	buf1[20] = '\0';
	printf("Message Read: '%s'\n",buf1);
	const char* path2 = "/dir1/dir2";
	printf("Directory Read: %s\n",path2);
	char* buf2 = (char *)malloc(1000);
	if(pcd_readdir(path2, buf2, test_filler, 0, 0) < 0){
		printf("Read Directory Failed");
		return -1;
	}

	free(buf1);
	free(buf2);
	printf("End of Testing read\n\n");
	return 0;
}

int test_write(void){
	printf("Testing write\n");
	const char* path1 = "/dir1/dir2/test.txt";
	char* buf1 = "hello world";
	printf("Writing '%s'",buf1);
	printf(" to path %s\n",path1);
	if(pcd_write(path1,buf1,strlen(buf1),0,0)<0){
		printf("Write Failed");
		return -1;
	}
	printf("End of Testing write\n\n");
	return 0;
}

int read_dir(void){
	printf("Reading Directory from 3 paths\n");
	const char* rootPath = "/";
	const char* dir1Path = "/dir1";
	const char* dir2Path = "/dir1/dir2";
	test_readdir(rootPath);
	test_readdir(dir1Path);
	test_readdir(dir2Path);
	printf("End of Reading Directory\n\n");
	return 0;
}
int main(){
	// init file system
	initialize("./disk");
	mkfs();
	// allocate space
	test_mkdir();
	read_dir();
	test_mknod();
	test_write();
	test_read();
	test_unlink();
	freefs();
}
