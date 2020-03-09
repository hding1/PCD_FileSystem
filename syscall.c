#define FUSE_USE_VERSION 29

#include <fuse.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include "dir.h"
#include "inode.h"

static const int debug = 1;

int pcd_mkroot(){
	if(debug) fprintf(stderr, "pcd_mkroot()\n");
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

	//set inode fields
	inode_write_mode(myInum, S_IFDIR | 0755);
	inode_write_UID(myInum, getuid());
	inode_write_GID(myInum, getgid());
	inode_write_last_accessed(myInum, time(NULL));
	inode_write_last_modified(myInum, time(NULL));
	return 0;
}

// get parent name, filename and parent path from path
int get_parent(const char *path, char * parent, char * filename, char ** parentPath){
	if(debug) fprintf(stderr, "get_parent(%s, parent, filename, parentPath)\n", path);
	char* val = strrchr(path, '/');
	if(val == NULL || strcmp(val,"/")==0){
		return -1;
	}
	//obtain file name
	strcpy( filename, val + 1);

    //obtain parent name
    int length = strlen(path)-strlen(val);
	*parentPath = (char*)malloc(length+2);
    if(length==0){
    	strcpy(*parentPath,"/");
    	strcpy( parent, "/");
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
	char * copy = strdup(path);
	char * tok1 = strtok(copy, "/");
	char * out = strdup(tok1);
	free(copy);
	return out;
}

// read in an inum parent directory and target
// return inode number if found, if not return -1
int find_inode_index(int inum, char * target){
	// start number = 2, since 0 and 1 are for . and ..
	unsigned int start = 2;
	unsigned int offset = start*DIRENT_SIZE;
	char tempbuf[DIRENT_SIZE];
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
			int num = mydirent->inum;
			return num;
		}
		start++;
		offset = start*DIRENT_SIZE;
	}
	// no target found
	return -1;
}

// Given an path to find the inode of the last element
// return -1 if not found, inumber if found 
int find_inode(const char *path){	
	// make a copy of path
	char* pathCopyStart = (char*)malloc(strlen(path)+1);
	char* pathCopy = pathCopyStart;
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
			free(dir);
			return -1;
		}
		if(strlen(pathCopy)==strlen(dir)){
			free(dir);
			free(pathCopyStart);
			return myInum;
		}
		pathCopy = pathCopy+strlen(dir)+1;
		free(dir);
	}
	free(pathCopyStart);
	return myInum;
}

// Create a directory
int pcd_mkdir(const char *path, mode_t mode)
{
	if(debug) fprintf(stderr, "pcd_mkdir(%s, mode)\n", path);
	int myInum = inode_allocate();
	if(myInum == -1 ){
		perror("Error Inode Allocation Failed");
		return -1;
	}

	struct fuse_context* context = fuse_get_context();

	//set inode fields
	inode_write_mode(myInum, S_IFDIR | mode);
	inode_write_UID(myInum, context->uid);
	inode_write_GID(myInum, context->gid);
	inode_write_last_accessed(myInum, time(NULL));
	inode_write_last_modified(myInum, time(NULL));

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
int delete_dirent(int inum, char * target, dirent * dirent_out){
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
			//if found, copy it to the output buffer
			if(dirent_out != NULL){
				memcpy(dirent_out, mydirent, DIRENT_SIZE);
			}
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
	if(debug) fprintf(stderr, "pcd_unlink(%s)\n", path);
	int myInum = find_inode(path);
	if(myInum==-1){
		perror("Error Cannot Find Inode");
		return -ENOENT;
	}

	if(!is_dir(myInum) || (is_dir(myInum) && is_empty_dir(myInum))){
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
		delete_dirent(parentInum, fileName, NULL);
		// reduce link count and delete inode if necessary
		inode_reduce_link_count(myInum);
	}
	else{
		perror("cannot delete non-empty directory");
		return -ENOTEMPTY;
	}
	return 0;
}

int pcd_rmdir(const char *path)
{
	if(debug) fprintf(stderr, "pcd_rmdir(%s)\n", path);
	int myInum = find_inode(path);
	if(myInum==-1){
		perror("Error Cannot Find Inode");
		return -ENOENT;
	}

	if((is_dir(myInum) && is_empty_dir(myInum))){
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
		delete_dirent(parentInum, fileName, NULL);
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
	if(debug) fprintf(stderr, "pcd_mknod(%s, mode, rdev)\n", path);
	int myInum = inode_allocate();
	if(myInum == -1 ){
		perror("Error Inode Allocation Failed");
		return -1;
	}

	struct fuse_context* context = fuse_get_context();

	//set inode fields
	inode_write_mode(myInum, mode);
	inode_write_UID(myInum, context->uid);
	inode_write_GID(myInum, context->gid);
	inode_write_last_accessed(myInum, time(NULL));
	inode_write_last_modified(myInum, time(NULL));

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
	if(debug) fprintf(stderr, "pcd_readdir(%s, ...)\n", path);
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
	if(debug) fprintf(stderr, "pcd_read(%s, ...)\n", path);
	//path get inode and inum
	int myInum = find_inode(path);
	if(myInum==-1){
		perror("Error Cannot Find Inode");
		return -1;
	}
	//read to buf
	memset(buf, 0, size);
	int numRead = read_file(myInum, buf, size, offset);
	if(numRead == -1){
		perror("Error Reading File");
		return -1;
	}

	inode_write_last_accessed(myInum, time(NULL));
	return numRead;
}

//Write data to the file. Should write all data.
int pcd_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi)
{
	if(debug) fprintf(stderr, "pcd_write(%s, ...)\n", path);
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

	inode_write_last_modified(myInum, time(NULL));
	return size;
}
//Open path. mode is as for the system call open. 
//(mode & O_ACCMODE) is one of O_RDONLY, O_WRONLY and O_RDWR. The mode can also contain other flags, most notably O_APPEND.
int pcd_open(const char *path, struct fuse_file_info *fi)
{
	if(debug) fprintf(stderr, "pcd_open(%s, fuse_file_info)\n", path);
	int res;

	res = find_inode(path);
	if (res == -1)
		return -1;

	if(fi != NULL){
		fi->fh = res;
	}

	return 0;
}


int pcd_chmod(const char *path, mode_t mode){
	if(debug) fprintf(stderr, "chmod(%s, mode)\n", path);
	int myInum = find_inode(path);
	if(myInum==-1){
		fprintf(stderr, "Error: Cannot Find Inode for path \"%s\"\n", path);
		return -ENOENT;
	}

	int status = 0;
	mode_t oldMode;
	status = inode_read_mode(myInum, &oldMode);
	if(status < 0){
		return status;
	}
	mode_t newFilePermission = mode & (!S_IFMT);
	mode_t oldFileType = oldMode & (S_IFMT);
	status = inode_write_mode(myInum, newFilePermission | oldFileType);
	if(status < 0){
		return status;
	}
	return 0;
}

int pcd_chown(const char *path, uid_t uid, gid_t gid){
	if(debug) fprintf(stderr, "chown(%s, uid, gid)\n", path);
	int myInum = find_inode(path);
	if(myInum==-1){
		fprintf(stderr, "Error: Cannot Find Inode for path \"%s\"\n", path);
		return -ENOENT;
	}
	
	int status = 0;
	if(uid!=UINT_MAX){
		status = inode_write_UID(myInum, uid);
		if(status < 0){
			return status;
		}
	}
	if(gid!=UINT_MAX){
		status = inode_write_GID(myInum, gid);
		if(status < 0){
			return status;
		}
	}
	return 0;
}

int pcd_utimens(const char *path, const struct timespec tv[2]){
	if(debug) fprintf(stderr, "pcd_utimens(%s, struct timespec tv[2])\n", path);
	int myInum = find_inode(path);
	if(myInum==-1){
		fprintf(stderr, "Error: Cannot Find Inode for path \"%s\"\n", path);
		return -ENOENT;
	}

	int status = 0;
	status = inode_write_last_accessed(myInum, tv[0].tv_sec);
	if(status < 0){
		return status;
	}
	status = inode_write_last_modified(myInum, tv[1].tv_sec);
	if(status < 0){
		return status;
	}
	return 0;
}

int get_parent_inum_and_filename(const char *path, int *inum, char* fileName){
	char parentName[MAX_FILE_NAME];
	char * parentPath;
	get_parent(path, parentName, fileName, &parentPath);

	// find inode of the parent
	*inum = find_inode(parentPath);
	free(parentPath);

	if(*inum==-1){
		fprintf(stderr, "Error: Cannot Find Parent Inode for path \"%s\"\n", path);
		return -ENOENT;
	}

	return 0;
}

int pcd_rename(const char *oldpath, const char *newpath){
	if(debug) fprintf(stderr, "pcd_rename(%s, %s)\n", oldpath, newpath);
	int myInum = find_inode(oldpath);
	if(myInum < 0){
		fprintf(stderr, "Error: Cannot Find Inode for path \"%s\"\n", oldpath);
		return -ENOENT;
	}

	int oldParentInum = 0;
	int status = 0;
	char oldFileName[MAX_FILE_NAME] = {0};
	status = get_parent_inum_and_filename(oldpath, &oldParentInum, oldFileName);
	if(status < 0){return status;}

	dirent entry = {0};
	// rename from parent
	status = delete_dirent(oldParentInum, oldFileName, &entry);
	if(status < 0){return status;}

	int newParentInum = 0;
	status = get_parent_inum_and_filename(newpath, &newParentInum, entry.name);
	if(status < 0){return status;}

	unsigned long parentInodeSize = 0;
	status = inode_read_size(newParentInum, &parentInodeSize);
	if(status < 0){return status;}

	status = write_file(newParentInum, (void*) &entry, DIRENT_SIZE, parentInodeSize);
	if(status < 0){
		fprintf(stderr, "Error Writing to File\n");
		return status;
	}

	return 0;
}

int pcd_link(const char *oldpath, const char *newpath){
	if(debug) fprintf(stderr, "pcd_link(%s, %s)\n", oldpath, newpath);
	int myInum = find_inode(oldpath);
	if(myInum < 0){
		fprintf(stderr, "Error: Cannot Find Inode for path \"%s\"\n", oldpath);
		return -ENOENT;
	}

	dirent entry = {0};

	int status = 0;
	int newParentInum = 0;
	status = get_parent_inum_and_filename(newpath, &newParentInum, entry.name);
	if(status < 0){return status;}

	mode_t mode;
	status = inode_read_mode(myInum, &mode);
	if(status < 0){return status;}
	entry.inum = myInum;
	entry.file_type = FileType(mode);

	unsigned long parentInodeSize = 0;
	status = inode_read_size(newParentInum, &parentInodeSize);
	if(status < 0){return status;}

	status = write_file(newParentInum, (void*) &entry, DIRENT_SIZE, parentInodeSize);
	if(status < 0){
		fprintf(stderr, "Error Writing to File\n");
		return status;
	}

	status = inode_increase_link_count(myInum);
	if(status < 0){return status;}

	return 0;
}

int pcd_truncate(const char *path, off_t size){
	if(debug) fprintf(stderr, "pcd_truncate(%s, %ld)\n", path, size);
	int myInum = find_inode(path);
	if(myInum < 0){
		fprintf(stderr, "Error: Cannot Find Inode for path \"%s\"\n", path);
		return -ENOENT;
	}

	truncate_file(myInum, size);
	int status = inode_write_last_modified(myInum, time(NULL));
	if(status < 0){return status;}

	return 0;
}

int pcd_symlink(const char *targetPath, const char *linkname){
	if(debug) fprintf(stderr, "symlink(%s, %s)\n", targetPath, linkname);

	pcd_mknod(linkname, S_IFLNK| 777, 0);
	pcd_write(linkname, targetPath, strlen(targetPath)+1, 0, NULL);

	return 0;
}

int pcd_readlink(const char *path, char *buf, size_t len){
	if(debug) fprintf(stderr, "readlink(%s, buf, %lud)\n", path, len);

	pcd_read(path, buf, len, 0, NULL);

	return 0;
}
