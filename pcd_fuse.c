#include <pcd_fuse.h>


static void *pcd_init(struct fuse_conn_info *conn,
			struct fuse_config *cfg)
{
	// (void) conn;
	// cfg->kernel_cache = 1;
	// return NULL;
}

static int pcd_getattr(const char *path, struct stat *stbuf,
			 struct fuse_file_info *fi)
{
	(void) fi;
	int res = 0;

	// memset(stbuf, 0, sizeof(struct stat));
	// if (strcmp(path, "/") == 0) {
	// 	stbuf->st_mode = S_IFDIR | 0755;
	// 	stbuf->st_nlink = 2;
	// } else if (strcmp(path+1, options.filename) == 0) {
	// 	stbuf->st_mode = S_IFREG | 0444;
	// 	stbuf->st_nlink = 1;
	// 	stbuf->st_size = strlen(options.contents);
	// } else
	// 	res = -ENOENT;

	return res;
}

static int pcd_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
			 off_t offset, struct fuse_file_info *fi,
			 enum fuse_readdir_flags flags)
{
	(void) offset;
	(void) fi;
	(void) flags;

	// if (strcmp(path, "/") != 0)
	// 	return -ENOENT;

	// filler(buf, ".", NULL, 0, 0);
	// filler(buf, "..", NULL, 0, 0);
	// filler(buf, options.filename, NULL, 0, 0);

	return 0;
}

static int pcd_open(const char *path, struct fuse_file_info *fi)
{
	// if (strcmp(path+1, options.filename) != 0)
	// 	return -ENOENT;

	// if ((fi->flags & O_ACCMODE) != O_RDONLY)
	// 	return -EACCES;

	return 0;
}

static int pcd_read(const char *path, char *buf, size_t size, off_t offset,
		      struct fuse_file_info *fi)
{
	// size_t len;
	// (void) fi;
	// if(strcmp(path+1, options.filename) != 0)
	// 	return -ENOENT;

	// len = strlen(options.contents);
	// if (offset < len) {
	// 	if (offset + size > len)
	// 		size = len - offset;
	// 	memcpy(buf, options.contents + offset, size);
	// } else
	// 	size = 0;

	return size;
}

static struct fuse_operations pcd_oper = {
	.init           = pcd_init,
	.getattr	= pcd_getattr,
	.readdir	= pcd_readdir,
	.open		= pcd_open,
	.read		= pcd_read,
};


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