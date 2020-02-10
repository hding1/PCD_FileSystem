#define MAX_FILE_NAME 256
#define DIRENT_SIZE sizeof(dirent)

typedef struct{
	int inum;
	char file_type;
	char name[MAX_FILE_NAME];
}dirent;

