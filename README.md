# PCD File System

This file system is a class project for CS270  
Author - Peter Ding, Karl Wang, Zihao Zhao, Fuheng Zhao

# Phase 1
layer 0&1 - Zihao Zhao, Fuheng Zhao  
layer 2&3 - Peter Ding, Karl Wang
## Layer 0 - Disk Layer

 - [x] Disk Read
 - [x] Disk Write

## Layer 1 - Data Structure Layer

 1. mkfs - creates the superblock, free inode list, and free block list on disk
	 - [x] Complete
 2. Inode - routines to get, free and access inodes on disk
 	 - [x] Data Stucture
 	 - [x] Read
 	 - [x] Write
 	 - [x] Allocate
 	 - [x] free
 3. Data Block - routines to get, free and access blocks on disk
 	 - [x] Data Stucture
 	 - [x] Read
 	 - [x] Write
 	 - [x] Allocate
 	 - [x] free

## Layer 2 - Abstraction Layer

 - [x] mkdir: makes a directory
 - [x] mknod: makes a file
 - [x] readdir: reads a directory
 - [x] unlink: removes a file or directory
 - [x] open/close: opens/closes a file
 - [x]  read/write: reads/writes a file

## Layer 3 - The Interface Layer

 - [x]  connect to fuse

## Testing
- Under Consturction

# Layer 2


