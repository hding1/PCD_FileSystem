#ifndef PCD_FILESYSTEM_DIR_H_
#define PCD_FILESYSTEM_DIR_H_


#define MAX_FILE_NAME 256
#define DIRENT_SIZE sizeof(dirent)

typedef struct{
	int inum;
	char file_type;
	char name[MAX_FILE_NAME];
}dirent;

#endif //PCD_FILESYSTEM_DIR_H_