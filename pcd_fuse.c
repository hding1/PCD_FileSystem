
#define FUSE_USE_VERSION 29

#include <fuse.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <assert.h>

#include <sys/stat.h>

#include "pcd_fuse.h"
#include "fs.h"
#include "inode.h"
#include "syscall.h"
#include "dir.h"

static const int debug = 1;

static void *pcd_init(struct fuse_conn_info *conn)
{
	if(debug) fprintf(stderr, "pcd_init(fuse_conn_info)\n");
	//not using conn
	(void) conn;

	mkfs();
	//could also allocate here
	//return calloc(1, 16*1024*1024 /*allocate 16MB*/);
	return NULL;
}

static int pcd_getattr(const char *path, struct stat *stbuf)
{
	if(debug) fprintf(stderr, "pcd_getattr(%s, stat)\n", path);
	memset(stbuf, 0, sizeof(struct stat));

	int inum = find_inode(path);
	if(inum == -1){
		fprintf(stderr, "cannot find inode (got inum=%d)\n", inum);
		return -ENOENT;
	}

	inode_read_mode(inum, &(stbuf->st_mode));
	unsigned int linkCount = 0;
	inode_read_link_count(inum, &linkCount);

	stbuf->st_nlink = linkCount;
	inode_read_UID(inum, stbuf->st_uid);
	inode_read_GID(inum, stbuf->st_gid);
	inode_read_last_accessed(inum, &(stbuf->st_atime));
	inode_read_last_modified(inum, &(stbuf->st_mtime));
	if(stbuf->st_mode & S_IFREG){
		// on off_t size: //stackoverflow.com/questions/9073667/
		// for 64 bit:
		// stbuf->st_size = (node.i_dir_acl << 32) & node.i_size;
		unsigned long inodeSize = 0;
		inode_read_size(inum, &inodeSize);
		stbuf->st_size = inodeSize;
	}

	return 0;
}


static struct fuse_operations pcd_oper = {
	.init       = pcd_init,
	.getattr	= pcd_getattr,
	.mkdir      = pcd_mkdir,
	.unlink     = pcd_unlink,
	.mknod      = pcd_mknod,
	.readdir	= pcd_readdir,
	.read		= pcd_read,
	.write		= pcd_write,
	.open		= pcd_open,
	.rmdir		= pcd_rmdir
};

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options {
	const char *device;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("-d %s", device),
	OPTION("--device=%s", device),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --device=<s>          Path of the block device\n"
	       "\n");
}

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.device = strdup("undefined");

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help || strcmp(options.device, "undefined") == 0) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0][0] = '\0';
	}
	initialize(options.device);

	ret = fuse_main(args.argc, args.argv, &pcd_oper, NULL);
	fuse_opt_free_args(&args);

	printf("syncing...");
	sync();

	printf("freeing in memory stuff...");
	freefs();

	return ret;
}