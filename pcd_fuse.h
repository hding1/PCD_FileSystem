#ifndef PCD_FILESYSTEM_PCD_FUSE_H_
#define PCD_FILESYSTEM_PCD_FUSE_H_

#define FUSE_USE_VERSION 31

#include <fuse.h>

int pcd_mkroot();

#endif  //PCD_FILESYSTEM_PCD_FUSE_H_