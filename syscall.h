#ifndef PCD_FILESYSTEM_SYSCALL_H_
#define PCD_FILESYSTEM_SYSCALL_H_

#define FUSE_USE_VERSION 29

#include <fuse.h>
#include <limits.h>
#include <sys/stat.h>

// mkdir: makes a directory
// mknod: makes a file
// readdir: reads a directory
// unlink: removes a file or directory
// open/close: opens/closes a file
// read/write: reads/writes a file

int pcd_mkroot();
int pcd_mkdir(const char *path, mode_t mode);
int pcd_unlink(const char *path);
int pcd_rmdir(const char *path);
int pcd_mknod(const char *path, mode_t mode, dev_t rdev);
int pcd_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
		       off_t offset, struct fuse_file_info *fi);
int pcd_read(const char *path, char *buf, size_t size, off_t offset,
		    struct fuse_file_info *fi);
int pcd_write(const char *path, const char *buf, size_t size,
		     off_t offset, struct fuse_file_info *fi);
int pcd_open(const char *path, struct fuse_file_info *fi);
int pcd_chmod(const char *path, mode_t mode);
int pcd_chown(const char *path, uid_t uid, gid_t gid);
int pcd_utimens(const char *, const struct timespec tv[2]);
int pcd_rename(const char *oldpath, const char *newpath);
int pcd_link(const char *oldpath, const char *newpath);
int pcd_truncate(const char *, off_t size);
int pcd_symlink(const char *linkname, const char *path);
int pcd_readlink(const char *path, char *buf, size_t len);
int find_inode(const char *path);

#endif //PCD_FILESYSTEM_SYSCALL_H_