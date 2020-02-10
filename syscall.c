#define FUSE_USE_VERSION 31

#include <fuse.h>
#include <syscall.h>
#include <inode.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dir.h>
#include <sys/stat.h>

// #include <sys/stat.h>

// get parent name, filename and parent path from path
int get_parent(const char *path, char * parent, char * filename, char ** parentPath){
	char* val = strrchr(path, '/');
	if(val == NULL || strcmp(val,"/")==0){
		return -1;
	}
	//obtain file name
	strcpy( filename, val + 1);
    printf("%s\n",filename);

    //obtain parent name
    int length = strlen(path)-strlen(val);
    *parentPath = (char*)malloc(length);
    strncpy(*parentPath,path,length);
    val = strrchr(*parentPath, '/');
    strcpy( parent, val + 1);
    printf("%s\n",parent);
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

// read in an inode parent directory and target
// return inode number if found, if not return -1
int find_inode_index(inode * parent, char * target){
	// start number = 2, since 0 and 1 are for . and ..
	unsigned int start = 2;
	unsigned int offset = start*DIRENT_SIZE;
	char * tempbuf = (char*)malloc(DIRENT_SIZE);
	while(offset <= parent->size){
		// read one entry at an time
		if(read_file(&parent, &tempbuf, DIRENT_SIZE, offset)==-1){
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
	return -1;
}

// Use an path to find the inode of the last element
// return -1 if not found, inumber if found 
int find_inode(const char *path, inode ** node){	
	// make a copy of path
	char* pathCopy = (char*)malloc(strlen(path) + 1);
	pathCopy = path;
	// check if the first directory is root
	char* dir;
	dir = get_dir(pathCopy);
	pathCopy = pathCopy+strlen(dir)+1;
	if(strcmp(dir,"root")!=0){
		printf("%s\n",path);
		perror("Error Incorrect Path (No Root)");
		return -1;
	}
	// get root
	int myInum = get_root_inum();
	inode * myInode = malloc(sizeof(*myInode));
	if(inode_read(&myInode, myInum)==-1){
		perror("Error Getting Root Inode");
		return -1;
	}
	// loop throught the path
	while(strlen(pathCopy)!=0){
		char* dir = get_dir(pathCopy);
		myInum = find_inode_index(myInode,dir);
		if(myInum == -1){
			printf("%s\n",path);
			perror("Error Cannot Find Directory");
			return -1;
		}
		if(inode_read(&myInode, myInum)==-1){
			perror("Error Getting Inode");
			return -1;
		}
		path = path+strlen(dir)+1;
	}
	*node = myInode;
	free(pathCopy);
	return myInum;
}

// Create a directory
int pcd_mkdir(const char *path, mode_t mode)
{
	inode* newNode;
	int myInum = inode_allocate(&newNode);

	if(myInum == -1 ){
		perror("Error Inode Allocation Failed");
		return -1;
	}
	//set inode type
	newNode->mode = mode;

	//write current node id into the parent
	inode* parentNode;
	char parentName[MAX_FILE_NAME];
	char fileName[MAX_FILE_NAME];
	char * parentPath;
	get_parent(path,parentName,fileName,&parentPath);
	int parentInum = find_inode(path, &parentNode);
	if(parentInum==-1){
		perror("Error Cannot Find Directory Inode");
		return -1;
	}
	dirent dir;
	dir.inum = myInum;
	dir.file_type = 'd';
	dir.name = parentName;
	char* buf = (char*)malloc(DIRENT_SIZE);
	memcpy(buf,&dir,DIRENT_SIZE);
	if(write_file(&parentNode, buf, DIRENT_SIZE, parentNode->size)==-1){
		perror("Error Writing to File");
		return -1;
	}
	if(inode_write(&parentNode,parentInum)==-1){
		perror("Error Writing Inode");
		return -1;
	}
	//write . and .. to the inode
	dir = {myInum,'d',"."};
	char* buf = (char*)malloc(DIRENT_SIZE);
	memcpy(buf,&dir,DIRENT_SIZE);
	if(write_file(&newNode, buf, DIRENT_SIZE, newNode->size)==-1){
		perror("Error Writing to File");
		return -1;
	}
	dir = {parentInum,'d',".."};
	char* buf = (char*)malloc(DIRENT_SIZE);
	memcpy(buf,&dir,DIRENT_SIZE);
	if(write_file(&newNode, buf, DIRENT_SIZE, newNode->size)==-1){
		perror("Error Writing to File");
		return -1;
	}
	if(inode_write(&newNode,myInum)==-1){
		perror("Error Writing Inode");
		return -1;
	}
	return 0;
}

// Remove a file
int pcd_unlink(const char *path)
{
	inode* myNode;
	int myInum = find_inode(path, &myNode);
	if(myInum==-1){
		perror("Error Cannot Find Inode");
		return -1;
	}
	// reduce link count
	myNode->link_count--;
	if(myNode->link_count==0){
		//remove file
		if(inode_free(myInum)==-1){
			perror("Error Freeing Inode");
			return -1;
		}
	}else{
		//write to inode
		if(inode_write(&myNode,myInum)==-1){
			perror("Error Writing Inode");
			return -1;
		}
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

// Create a node (file, device special, or named pipe)
int pcd_mknod(const char *path, mode_t mode, dev_t rdev)
{
	inode* newNode;
	int myInum = inode_allocate(&newNode);
	if(myInum == -1 ){
		perror("Error Inode Allocation Failed");
		return -1;
	}
	//set inode type
	newNode->mode = mode;
	char fileType = FileType(mode);
	//write current node id into the parent directory
	inode* parentNode;
	char parentName[MAX_FILE_NAME];
	char fileName[MAX_FILE_NAME];
	char * parentPath;
	get_parent(path,parentName,fileName,&parentPath);
	int parentInum = find_inode(path, &parentNode);
	if(parentInum==-1){
		perror("Error Cannot Find Directory Inode");
		return -1;
	}
	dirent dir;
	dir.inum = myInum;
	dir.file_type = fileType;
	dir.name = parentName;
	char* buf = (char*)malloc(DIRENT_SIZE);
	memcpy(buf,&dir,DIRENT_SIZE);
	if(write_file(&parentNode, buf, DIRENT_SIZE, parentNode->size)==-1){
		perror("Error Writing to File");
		return -1;
	}
	if(inode_write(&parentNode,parentInum)==-1){
		perror("Error Writing Inode");
		return -1;
	}
	//write itself to the inode
	if(inode_write(&newNode,myInum)==-1){
		perror("Error Writing Inode");
		return -1;
	}
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
		       off_t offset, struct fuse_file_info *fi,
		       enum fuse_readdir_flags flags)
{
	DIR *dp;
	struct dirent *de;

	(void) offset;
	(void) fi;
	(void) flags;

	dp = opendir(path);
	if (dp == NULL)
		return -errno;

	while ((de = readdir(dp)) != NULL) {
		struct stat st;
		memset(&st, 0, sizeof(st));
		st.st_ino = de->d_ino;
		st.st_mode = de->d_type << 12;
		if (filler(buf, de->d_name, &st, 0, 0))
			break;
	}

	closedir(dp);
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
	inode* myNode;
	int myInum = find_inode(path, &myNode);
	if(myInum==-1){
		perror("Error Cannot Find Inode");
		return -1;
	}
	//read to buf
	if(read_file(&myNode, &buf, size, offset)==-1){
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
	inode* myNode;
	int myInum = find_inode(path, &myNode);
	if(myInum==-1){
		perror("Error Cannot Find Inode");
		return -1;
	}
	//read to buf
	if(write_file(&myNode, buf, size, offset)==-1){
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

	inode* node;
	res = find_inode(path, node);
	if (res == -1)
		return -errno;

	fi->fh = res;
	return 0;
}