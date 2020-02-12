#include "fs.h"
#include "syscall.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int test_filler(void *buf, const char *name,
				const struct stat *stbuf, off_t off,
				enum fuse_fill_dir_flags flags);

int test_filler(void *buf, const char *name,
				const struct stat *stbuf, off_t off,
				enum fuse_fill_dir_flags flags){
	printf("name: %s, ", name);
	printf("mode: %d, ", stbuf->st_mode);
	printf("inum: %d", stbuf->st_ino);
	return 0;
}

int test_mkdir(void){
	//making an dir1 under root
	const char* path1 = "/root/dir1";
	mode_t dir1 = S_IFDIR;
	int test1 = pcd_mkdir(path,dir1);
	if(test1<0){
		printf("Mkdir Test1 failed");
		return -1;
	}
	//making an dir2 under root/dir1
	const char* path2 = "/root/dir1/dir2";
	mode_t dir2 = S_IFDIR;
	int test2 = pcd_mkdir(path,dir2);
	if(test2<0){
		printf("Mkdir Test2 failed");
		return -1;
	}
	//making an dir3 under root/
	const char* path3 = "/root/dir3";
	mode_t dir3 = S_IFDIR;
	int test3 = pcd_mkdir(path,dir3);
	if(test3<0){
		printf("Mkdir Test3 failed");
		return -1;
	}
	//making an dir4 under root/dir1/dir2
	const char* path4 = "/root/dir1/dir2/dir4";
	mode_t dir4 = S_IFDIR;
	int test4 = pcd_mkdir(path,dir4);
	if(test4<0){
		printf("Mkdir Test4 failed");
		return -1;
	}
	printf("Four Directories Should be Created\n");
	printf("\troot\ndir3\t\tdir1\n\t\t\tdir2\n\t\t\t\tdir4");
}

int test_readdir(){
	const char* path1 = "/root";
	char* buf = (char *)malloc(1000);
	if(pcd_readir(path1,buf,test_filler,0,0,0)<0){
		printf("Read Directory Failed");
		return -1;
	}
	free(buf);
	return 0;
}

int test_unlink(void){
	const char* path1 = "/root/dir1/dir2/dir4";
	if(pcd_unlink(path1)<0){
		printf("Unlink Test1 Failed");
		return -1;
	}
	return 0;
}

int test_mknod(void){
	const char* path1 = "/root/dir1/dir2/test.txt";
	pcd_mknod(path1,S_IFREG,0);
	const char* path2 = "/root/dir1/dir2";
	char* buf = (char *)malloc(1000);
	if(pcd_readir(path2,buf,test_filler,0,0,0)<0){
		printf("Read Directory Failed");
		return -1;
	}
	free(buf);
	return 0;
}

int test_open(void){

}

int test_read(void){
	const char* path1 = "/root/dir1/dir2/test.txt";
	char* buf = (char *)malloc(20);
	if(pcd_read(path1,buf,strlen(buf),0,0)){
		printf("Read Failed");
		return -1;
	}
	const char* path2 = "/root/dir1/dir2";
	char* buf = (char *)malloc(1000);
	if(pcd_readir(path2,buf,test_filler,0,0,0)<0){
		printf("Read Directory Failed");
		return -1;
	}
	print("%s\n",buf);
	free(buf);
	return 0;
}

int test_write(void){
	const char* path1 = "/root/dir1/dir2/test.txt";
	char* buf = "hello world";
	if(pcd_write(path1,buf,strlen(buf),0,0)<0){
		printf("Write Failed");
		return -1;
	}
	const char* path2 = "/root/dir1/dir2";
	char* buf = (char *)malloc(1000);
	if(pcd_readir(path2,buf,test_filler,0,0,0)<0){
		printf("Read Directory Failed");
		return -1;
	}
	free(buf);
	return 0;
}

int main(){
	// init file system
	mkfs();
	// allocate space
	test_mkdir();
	test_readdir();
	test_mknod();
	test_write();
	test_read();
	freefs();
}