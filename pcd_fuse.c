
#define FUSE_USE_VERSION 31

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

static void *pcd_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	//not using conn
	(void) conn;
	mkfs();

	//could also allocate here
	//return calloc(1, 16*1024*1024 /*allocate 16MB*/);
	return NULL;
}

static int pcd_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;

	memset(stbuf, 0, sizeof(struct stat));

	inode* node;
	if(!find_inode(path, &node)){
		return -ENOENT;
	}

	stbuf->st_mode = node->mode;
	stbuf->st_nlink = node->links_count;
	if(node->mode & S_IFREG){
		// on off_t size: //stackoverflow.com/questions/9073667/
		// for 64 bit:
		// stbuf->st_size = (node.i_dir_acl << 32) & node.i_size;
		stbuf->st_size = node->size;
	}
	free(node);

	return 0;
}


static struct fuse_operations pcd_oper = {
	.init           = pcd_init,
	.getattr	= pcd_getattr,
	.readdir	= pcd_readdir,
	.open		= pcd_open,
	.read		= pcd_read,
};

/*
 * Command line options
 *
 * We can't set default values for the char* fields here because
 * fuse_opt_parse would attempt to free() them when the user specifies
 * different values on the command line.
 */
static struct options {
	const char *filename;
	const char *contents;
	int show_help;
} options;

#define OPTION(t, p)                           \
    { t, offsetof(struct options, p), 1 }
static const struct fuse_opt option_spec[] = {
	OPTION("--name=%s", filename),
	OPTION("--contents=%s", contents),
	OPTION("-h", show_help),
	OPTION("--help", show_help),
	FUSE_OPT_END
};

static void show_help(const char *progname)
{
	printf("usage: %s [options] <mountpoint>\n\n", progname);
	printf("File-system specific options:\n"
	       "    --name=<s>          Name of the \"hello\" file\n"
	       "                        (default: \"hello\")\n"
	       "    --contents=<s>      Contents \"hello\" file\n"
	       "                        (default \"Hello, World!\\n\")\n"
	       "\n");
}

int main(int argc, char *argv[])
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	/* Set defaults -- we have to use strdup so that
	   fuse_opt_parse can free the defaults if other
	   values are specified */
	options.filename = strdup("pcd");
	options.contents = strdup("pcd World!\n");

	/* Parse options */
	if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
		return 1;

	/* When --help is specified, first print our own file-system
	   specific help text, then signal fuse_main to show
	   additional help (by adding `--help` to the options again)
	   without usage: line (by setting argv[0] to the empty
	   string) */
	if (options.show_help) {
		show_help(argv[0]);
		assert(fuse_opt_add_arg(&args, "--help") == 0);
		args.argv[0][0] = '\0';
	}

	ret = fuse_main(args.argc, args.argv, &pcd_oper, NULL);
	fuse_opt_free_args(&args);
	return ret;
}