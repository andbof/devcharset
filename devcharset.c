/*
 * charset device using the CUSE character device in userspace framework
 * Copyright (C) 2013 Andreas Bofjall <andreas@gazonk.org>
 *
 * This program can be distributed under the terms of the GNU GPLv2 or later.
 * See the file COPYING for licensing details.
 * */

#define FUSE_USE_VERSION 30
#include <assert.h>
#include <cuse_lowlevel.h>
#include <errno.h>
#include <fuse_opt.h>
#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <config.h>
#include "parse.h"

#define DEFAULT_CHARSET "a-z"
#define DEFAULT_DEV_NAME "charset"
#define DEFAULT_RANDOM_DEV "/dev/urandom"

struct data {
	char *valid_chars;
	size_t valid_chars_size;
	int rand_fd;
};

enum fuse_opt_keys {
	KEY_DEBUG,
	KEY_HELP,
	KEY_VERSION,
};

static int debug;
static char *valid_chars;
static size_t valid_chars_size;
static const char *random_dev;

struct params {
	unsigned major;
	unsigned minor;
	char *charset;
	char *devname;
	char *random_dev;
};

#define CHARSET_OPT(t, p) { t, offsetof(struct params, p), 1 }
static const struct fuse_opt charset_opts[] = {
	CHARSET_OPT("-M %u", major),
	CHARSET_OPT("--maj=%u", major),
	CHARSET_OPT("-m %u", minor),
	CHARSET_OPT("--min=%u", minor),
	CHARSET_OPT("-n %s", devname),
	CHARSET_OPT("--name=%s", devname),
	CHARSET_OPT("-c %s", charset),
	CHARSET_OPT("--charset %s", charset),
	CHARSET_OPT("-r %s", random_dev),
	CHARSET_OPT("--random %s", random_dev),
	FUSE_OPT_KEY("-d", KEY_DEBUG),
	FUSE_OPT_KEY("--debug", KEY_DEBUG),
	FUSE_OPT_KEY("-h", KEY_HELP),
	FUSE_OPT_KEY("--help", KEY_HELP),
	FUSE_OPT_KEY("-V", KEY_VERSION),
	FUSE_OPT_KEY("--version", KEY_VERSION),
	FUSE_OPT_END
};

static int open_dev(const char *dev)
{
	int rand_fd;

	rand_fd = open(dev, O_RDONLY);
	if (rand_fd < 0)
		return -1;

	return rand_fd;
}

static int validate_dev(const char *dev)
{
	int fd;

	fd = open_dev(dev);
	if (fd < 0) {
		fprintf(stderr, "unable to open random device %s: %s\n",
				dev, strerror(errno));
		return 1;
	}
	close(fd);

	return 0;
}

static int fill_buffer_from_fd(unsigned char *buf, const size_t size,
		const int fd)
{
	ssize_t r;
	size_t idx = 0;

	do {
		r = read(fd, buf, size - idx);
		if (r < 0)
			return 1;

		idx += r;
	} while (idx < size);

	return 0;
}

static int strprepend(char **dest, const char *src)
{
	const size_t len = strlen(*dest) + strlen(src);
	char *new;

	new = malloc(len + 1);
	if (!new)
		return 1;

	strcpy(new, src);
	strcat(new, *dest);

	free(*dest);
	*dest = new;

	return 0;
}

static int process_arg(void *data, const char *arg, int key,
		struct fuse_args *outargs)
{
	/*
	 * help and version printouts go on stderr because that's where fuse
	 * prints them
	 */

	switch (key) {
	case KEY_HELP:
		/*
		 * -d should not be listed here because we're piggybacking on
		 *  the FUSE debug option. -V should be the same way, but it
		 *  isn't listed in the FUSE help (2.9.2)
		 */
		fprintf(stderr,
				"usage: " PACKAGE_NAME " [options]\n"
				"\n"
				"charset options:\n"
				"    --charset=SET|-c SET   character set (default is " DEFAULT_CHARSET ")\n"
				"    --help|-h              print this help message\n"
				"    --maj=MAJ|-M MAJ       device major number (default auto)\n"
				"    --min=MIN|-m MIN       device minor number (default auto)\n"
				"    --name=DEV|-n DEV      device name (default is " DEFAULT_DEV_NAME ")\n"
				"    --random=DEV|-r DEV    random device (default is " DEFAULT_RANDOM_DEV ")\n"
				"    --version|-V           display version information\n"
				"\n");
		return fuse_opt_add_arg(outargs, "-ho");
	case KEY_DEBUG:
		/* We're consuming the switch here and must re-add it so FUSE gets it */
		return fuse_opt_add_arg(outargs, "-d");
	case KEY_VERSION:
		fprintf(stderr, PACKAGE_NAME " version " PACKAGE_VERSION "\n");
		/* We're consuming the switch here and must re-add it so FUSE gets it */
		return fuse_opt_add_arg(outargs, "--version");
	default:
		return 1;
	}

	return 0;
}

static int parse_opts(struct cuse_info *ci, struct params *param,
		struct fuse_args *args)
{
	if (fuse_opt_parse(args, param, charset_opts, process_arg)) {
		fprintf(stderr, "failed to parse option\n");
		return 1;
	}

	if (!param->charset)
		param->charset = strdup(DEFAULT_CHARSET);
	if (!param->devname)
		param->devname = strdup(DEFAULT_DEV_NAME);
	if (strprepend(&param->devname, "DEVNAME="))
		goto err_mem;
	if (!param->random_dev)
		param->random_dev = strdup(DEFAULT_RANDOM_DEV);

	ci->dev_info_argv = malloc(sizeof(void*));
	if (!ci->dev_info_argv)
		goto err_mem;

	ci->dev_major = param->major;
	ci->dev_minor = param->minor;
	ci->dev_info_argc = 1;
	ci->dev_info_argv[0] = param->devname;
	ci->flags = 0;

	return 0;

err_mem:
	fprintf(stderr, "out of memory\n");
	return 1;
}

static void charset_open(fuse_req_t req, struct fuse_file_info *fi)
{
	struct data *data = malloc(sizeof(*data));
	if (!data) {
		fuse_reply_err(req, ENOMEM);
		return;
	}

	data->rand_fd = open_dev(random_dev);
	if (data->rand_fd < 0) {
		assert(errno);
		fuse_reply_err(req, errno);
		return;
	}

	data->valid_chars_size = valid_chars_size;
	data->valid_chars = malloc(valid_chars_size);
	if (!data->valid_chars) {
		fuse_reply_err(req, ENOMEM);
		return;
	}
	memcpy(data->valid_chars, valid_chars, valid_chars_size);

	fi->fh = (uintptr_t)data;

	fuse_reply_open(req, fi);
}

static void charset_release(fuse_req_t req, struct fuse_file_info *fi)
{
	struct data *data = (struct data*)(uintptr_t)fi->fh;

	free(data->valid_chars);
	free(data);

	fuse_reply_err(req, 0);
}

static void charset_read(fuse_req_t req, size_t size, off_t off,
		struct fuse_file_info *fi)
{
	struct data *data = (struct data*)(uintptr_t)fi->fh;

	unsigned char *buf = malloc(size);
	if (!buf) {
		fuse_reply_err(req, ENOMEM);
		return;
	}

	if (fill_buffer_from_fd(buf, size, data->rand_fd)) {
		assert(errno);
		fuse_reply_err(req, errno);
		return;
	}

	for (size_t i = 0; i < size; i++)
		buf[i] = valid_chars[buf[i] % valid_chars_size];

	fuse_reply_buf(req, (char*)buf, size);

	free(buf);
}

static void charset_write(fuse_req_t req, const char *buf, size_t size,
		off_t off, struct fuse_file_info *fi)
{
	/*
	 * To behave in a similar fashion to /dev/zero and /dev/null, writes
	 * are silently swallowed and ignored.
	 */
	fuse_reply_write(req, size);
}

static const struct cuse_lowlevel_ops charset_ops = {
	.open = charset_open,
	.release = charset_release,
	.read = charset_read,
	.write = charset_write,
};

int main(int argc, char **argv)
{
	struct cuse_info ci;
	struct params param;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);

	memset(&ci, 0, sizeof(ci));
	memset(&param, 0, sizeof(param));

	if (parse_opts(&ci, &param, &args))
		return 1;

	if (validate_dev(param.random_dev))
		return 1;

	if (set_charset(&valid_chars, &valid_chars_size, param.charset))
		return 1;

	if (debug) {
		printf("Reading random bytes from %s\n", param.random_dev);
		printf("Using charset %s which I'm interpreting as:\n", param.charset);
		for (size_t i = 0; i < valid_chars_size; i++)
			printf("0x%hhx ", valid_chars[i]);
		printf("\n");
	}

	random_dev = param.random_dev;

	return cuse_lowlevel_main(args.argc, args.argv, &ci, &charset_ops,
			NULL);
}
