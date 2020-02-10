#ifndef PCD_FILESYSTEM_SYSCALL_H_
#define PCD_FILESYSTEM_SYSCALL_H_

// mkdir: makes a directory
// mknod: makes a file
// readdir: reads a directory
// unlink: removes a file or directory
// open/close: opens/closes a file
// read/write: reads/writes a file

int pcd_mkdir(const char *path, mode_t mode);
int pcd_unlink(const char *path);
int pcd_mknod(const char *path, mode_t mode, dev_t rdev);
int pcd_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi,
		       enum fuse_readdir_flags flags);
int pcd_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi);
int pcd_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi);
int pcd_open(const char *path, struct fuse_file_info *fi);
int find_inode(const char *path, inode ** node);

#endif //PCD_FILESYSTEM_SYSCALL_H_