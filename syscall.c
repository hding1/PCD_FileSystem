#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>

#include "dir.h"
#include "inode.h"

int pcd_mkroot(){
	int myInum = inode_allocate();
	//write . and .. to the inode
	dirent dir1 = {myInum,'d',"."};
	if(write_file(myInum, (char*)&dir1, DIRENT_SIZE, 0)==-1){
		perror("Error Writing to File");
		return -EIO;
	}

	dirent dir2 = {myInum,'d',".."};
	if(write_file(myInum, (char*)&dir2, DIRENT_SIZE, DIRENT_SIZE)==-1){
		perror("Error Writing to File");
		return -EIO;
	}
	return 0;
}

// get parent name, filename and parent path from path
int get_parent(const char *path, char * parent, char * filename, char ** parentPath){
	char* val = strrchr(path, '/');
	if(val == NULL || strcmp(val,"/")==0){
		return -1;
	}
	//obtain file name
	strcpy( filename, val + 1);

    //obtain parent name
    int length = strlen(path)-strlen(val);
	*parentPath = (char*)malloc(length);
    if(length==0){
    	strcpy(*parentPath,"/");
    	strcpy( parent, "root");
    	return 0;
    }
    strncpy(*parentPath,path,length);
    (*parentPath)[length] = '\0';
    val = strrchr(*parentPath, '/');
    strcpy( parent, val + 1);
	return 0;
}

// get first dir of a path
// e.g. /User/peter, return User
char * get_dir(char *path){
	char * copy = malloc(strlen(path) + 1); 
	strcpy(copy, path);
	copy = strtok(copy,"/");
	return copy;
}

// read in an inum parent directory and target
// return inode number if found, if not return -1
int find_inode_index(int inum, char * target){
	// start number = 2, since 0 and 1 are for . and ..
	unsigned int start = 2;
	unsigned int offset = start*DIRENT_SIZE;
	char * tempbuf = (char*)malloc(DIRENT_SIZE);
	unsigned long inodeSize = 0;
	inode_read_size(inum, &inodeSize);
	while(offset < inodeSize){
		// read one entry at an time
		if(read_file(inum, tempbuf, DIRENT_SIZE, offset)==-1){
			perror("Error Unable to Read");
			return -1;
		}
		// cast the buffer to dirent
		dirent * mydirent = (dirent *) tempbuf;
		// compare the dirent with target
		if(strcmp(mydirent->name,target)==0){
			return mydirent->inum;
		}
		start++;
		offset = start*DIRENT_SIZE;
	}
	// no target found
	free(tempbuf);
	return -1;
}

// Given an path to find the inode of the last element
// return -1 if not found, inumber if found 
int find_inode(const char *path){	
	// make a copy of path
	char* pathCopy = (char*)malloc(strlen(path));
	char* pathCopyStart = pathCopy;
	strcpy(pathCopy, path);
	// check if the first directory is root
	if(pathCopy[0] != '/'){
		printf("%s\n",path);
		perror("Error Incorrect Path (No Root)");
		free(pathCopyStart);
		return -1;
	}
	// get root
	int myInum = get_root_inum();
	pathCopy = pathCopy+1;
	// loop throught the path
	while(strlen(pathCopy)!=0){
		char* dir = get_dir(pathCopy);
		myInum = find_inode_index(myInum,dir);
		if(myInum == -1){
			printf("%s\n",path);
			perror("Error Cannot Find Directory");
			free(pathCopyStart);
			return -1;
		}
		if(strlen(pathCopy)==strlen(dir)){
			return myInum;
		}
		pathCopy = pathCopy+strlen(dir)+1;
	}
	free(pathCopyStart);
	return myInum;
}

// Create a directory
int pcd_mkdir(const char *path, mode_t mode)
{
	int myInum = inode_allocate();
	if(myInum == -1 ){
		perror("Error Inode Allocation Failed");
		return -1;
	}
	//set inode type
	inode_write_mode(myInum, mode);

	//write current node id into the parent
	char parentName[MAX_FILE_NAME];
	char fileName[MAX_FILE_NAME];
	char * parentPath;
	get_parent(path,parentName,fileName,&parentPath);
	// find inode of the parent
	
	int parentInum = find_inode(parentPath);
	free(parentPath);
	if(parentInum==-1){
		perror("Error Cannot Find Directory Inode");
		return -1;
	}
	dirent dir;
	dir.inum = myInum;
	dir.file_type = 'd';
	strcpy(dir.name, fileName);
	unsigned long parentInodeSize = 0;
	inode_read_size(parentInum, &parentInodeSize);
	if(write_file(parentInum, (char*)&dir, DIRENT_SIZE, parentInodeSize)==-1){
		perror("Error Writing to File");
		return -1;
	}
	//write . and .. to the inode
	dirent dir1 = {myInum,'d',"."};
	if(write_file(myInum, (char*)&dir1, DIRENT_SIZE, 0)==-1){
		perror("Error Writing to File");
		return -1;
	}
	dirent dir2 = {parentInum,'d',".."};
	if(write_file(myInum, (char*)&dir2, DIRENT_SIZE, DIRENT_SIZE)==-1){
		perror("Error Writing to File");
		return -1;
	}
	return 0;
}


// read in an inum parernt directory and target
// delete the dirent under parent
// return 0 if success, if not return -1
int delete_dirent(int inum, char * target){
	// start number = 2, since 0 and 1 are for . and ..
	unsigned int start = 2;
	unsigned int offset = start*DIRENT_SIZE;
	char * tempbuf = (char*)malloc(DIRENT_SIZE);
	
	unsigned long parentInodeSize = 0;
	inode_read_size(inum, &parentInodeSize);
	while(offset < parentInodeSize){
		// read one entry at an time
		if(read_file(inum, tempbuf, DIRENT_SIZE, offset)==-1){
			perror("Error Unable to Read");
			free(tempbuf);
			return -1;
		}
		// cast the buffer to dirent
		dirent * mydirent = (dirent *) tempbuf;
		// compare the dirent with target
		if(strcmp(mydirent->name,target)==0){
			//if found, rename the entry to empty ""
			strcpy(mydirent->name, "");
			mydirent->inum = -1;
			char* buf = (char*)malloc(DIRENT_SIZE);
			memcpy(buf,mydirent,DIRENT_SIZE);
			if(write_file(inum, buf, DIRENT_SIZE,offset)==-1){
				perror("Error Writing to File");
				free(tempbuf);
				free(buf);
				return -1;
			}
			free(tempbuf);
			free(buf);
			return 0;
		}
		start++;
		offset = start*DIRENT_SIZE;
	}
	free(tempbuf);
	// no target found
	return -1;
}

int is_dir(int inum){
	mode_t mode = 0;
	inode_read_mode(inum, &mode);
	return (mode & S_IFDIR) ? 1: 0;
}

int is_empty_dir(int inum){
	dirent* direntbuf = (dirent*)malloc(DIRENT_SIZE);

	unsigned long inodeSize = 0;
	inode_read_size(inum, &inodeSize);
	for(unsigned int offset_idx = 2*DIRENT_SIZE; offset_idx < inodeSize; offset_idx += DIRENT_SIZE){
		// read one entry at an time
		if(read_file(inum, (char*)direntbuf, DIRENT_SIZE, offset_idx)==-1){
			perror("Error Unable to Read");
			free(direntbuf);
			return -1;
		}

		if(direntbuf->name[0] != '\0'){
			free(direntbuf);
			return 0;
		}
	}
	free(direntbuf);
	return 1;
}

// Remove a file
int pcd_unlink(const char *path)
{
	int myInum = find_inode(path);
	if(myInum==-1){
		perror("Error Cannot Find Inode");
		return -ENOENT;
	}


	if(is_dir(myInum) && is_empty_dir(myInum)){
		char parentName[MAX_FILE_NAME];
		char fileName[MAX_FILE_NAME];
		char * parentPath;
		get_parent(path,parentName,fileName,&parentPath);

		// find inode of the parent
		int parentInum = find_inode(parentPath);
		free(parentPath);

		if(parentInum==-1){
			perror("Error Cannot Find Directory Inode");
			return -ENOENT;
		}

		// remove from parent
		delete_dirent(parentInum, fileName);
		// reduce link count and delete inode if necessary
		inode_reduce_link_count(myInum);
	}
	else{
		perror("cannot delete non-empty directory");
		return -ENOTEMPTY;
	}
	return 0;
}

char FileType (mode_t m) {
    switch (m & S_IFMT) {   //bitwise AND to determine file type
        case S_IFSOCK: return 's';     //socket
        case S_IFLNK: return 'l';     //symbolic link
        case S_IFREG: return '-';     //regular file
        case S_IFBLK: return 'b';     //block device
        case S_IFDIR: return 'd';     //directory
        case S_IFCHR: return 'c';     //char device
        case S_IFIFO: return 'p';     //pipe
        default: return '?';            //unknown
    }
}

mode_t FileTypeToModeT (char m) {
    switch (m) {   //bitwise AND to determine file type
        case 's': return S_IFSOCK;     //socket
        case 'l': return S_IFLNK;     //symbolic link
        case '-': return S_IFREG;     //regular file
        case 'b': return S_IFBLK;     //block device
        case 'd': return S_IFDIR;     //directory
        case 'c': return S_IFCHR;     //char device
        case 'p': return S_IFIFO;     //pipe
        default: return (mode_t)0;    //unknown
    }
}

// Create a node (file, device special, or named pipe)
int pcd_mknod(const char *path, mode_t mode, dev_t rdev)
{
	int myInum = inode_allocate();
	if(myInum == -1 ){
		perror("Error Inode Allocation Failed");
		return -1;
	}
	//set inode type
	inode_write_mode(myInum, mode);
	char fileType = FileType(mode);
	//write current node id into the parent directory
	char parentName[MAX_FILE_NAME];
	char fileName[MAX_FILE_NAME];
	char * parentPath;
	get_parent(path,parentName,fileName,&parentPath);
	int parentInum = find_inode(parentPath);
	free(parentPath);
	if(parentInum==-1){
		perror("Error Cannot Find Directory Inode");
		return -1;
	}
	dirent dir;
	dir.inum = myInum;
	dir.file_type = fileType;
	strcpy(dir.name, fileName);
	char* buf = (char*)malloc(DIRENT_SIZE);
	memcpy(buf,&dir,DIRENT_SIZE);

	unsigned long parentInodeSize = 0;
	inode_read_size(parentInum, &parentInodeSize);
	if(write_file(parentInum, buf, DIRENT_SIZE, parentInodeSize)==-1){
		perror("Error Writing to File");
		free(buf);
		return -1;
	}
	free(buf);
	return 0;
}

// Read directory

// The filesystem may choose between two modes of operation:

// 1) The readdir implementation ignores the offset parameter, and passes zero to the filler function's offset. 
// The filler function will not return '1' (unless an error happens), so the whole directory is read in a single readdir operation.

// 2) The readdir implementation keeps track of the offsets of the directory entries. 
// It uses the offset parameter and always passes non-zero offset to the filler function. 
// When the buffer is full (or an error happens) the filler function will return '1'.
int pcd_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi)
{
	(void) offset;
	(void) fi;

	int inum = find_inode(path);
	if (inum == -1)
		return -ENOENT;

	dirent* direntbuf = (dirent*)malloc(DIRENT_SIZE);
	unsigned long inodeSize = 0;
	inode_read_size(inum, &inodeSize);
	for(unsigned int offset_idx = 0; offset_idx < inodeSize; offset_idx += DIRENT_SIZE){
		// read one entry at an time
		if(read_file(inum, (char*)direntbuf, DIRENT_SIZE, offset_idx)==-1){
			perror("Error Unable to Read");
			free(direntbuf);
			return -1;
		}

		if(direntbuf->name[0] == '\0'){
			continue;
		}
		
		struct stat st = {0};
		st.st_ino = (ino_t)direntbuf->inum;
		st.st_mode = FileTypeToModeT(direntbuf->file_type);
		
		if(filler(buf, direntbuf->name, &st, 0)){
			//error
			break;
		}
	}

	free(direntbuf);
	return 0;
}

//Read data from an open file
//Read should return exactly the number of bytes requested except on EOF or error,
//otherwise the rest of the data will be substituted with zeroes. 
//An exception to this is when the 'direct_io' mount option is specified, in which case the return value of the read system call will reflect the return value of this operation.
int pcd_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi)
{
	//path get inode and inum
	int myInum = find_inode(path);
	if(myInum==-1){
		perror("Error Cannot Find Inode");
		return -1;
	}
	//read to buf
	memset(buf, 0, size);
	if(read_file(myInum, buf, size, offset)==-1){
		perror("Error Reading File");
		return -1;
	}
	return 0;
}

//Write data to the file. Should write all data.
int pcd_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	//path get inode and inum
	int myInum = find_inode(path);
	if(myInum==-1){
		perror("Error Cannot Find Inode");
		return -1;
	}
	//read to buf
	if(write_file(myInum, buf, size, offset)==-1){
		perror("Error Writing File");
		return -1;
	}
	return 0;
}
//Open path. mode is as for the system call open. 
//(mode & O_ACCMODE) is one of O_RDONLY, O_WRONLY and O_RDWR. The mode can also contain other flags, most notably O_APPEND.
int pcd_open(const char *path, struct fuse_file_info *fi)
{
	int res;

	res = find_inode(path);
	if (res == -1)
		return -1;

	fi->fh = res;

	return 0;
}
