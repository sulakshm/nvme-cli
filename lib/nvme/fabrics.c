/*
 * lib/parser.c - simple parser for mount, etc. options.
 *
 * This source code is licensed under the GNU General Public License,
 * Version 2.  See the file COPYING for more details.
 */

#define _GNU_SOURCE
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>

#include "fabrics.h"

#define PATH_NVME_FABRICS	"/dev/nvme-fabrics"

struct match_token {
	int token;
	const char *pattern;
};

typedef struct match_token match_table_t[];

/* Maximum number of arguments that match_token will find in a pattern */
enum {MAX_OPT_ARGS = 3};

/* Describe the location within a string of a substring */
typedef struct {
	char *from;
	char *to;
} substring_t;

/**
 * match_one: - Determines if a string matches a simple pattern
 * @s: the string to examine for presence of the pattern
 * @p: the string containing the pattern
 * @args: array of %MAX_OPT_ARGS &substring_t elements. Used to return match
 * locations.
 *
 * Description: Determines if the pattern @p is present in string @s. Can only
 * match extremely simple token=arg style patterns. If the pattern is found,
 * the location(s) of the arguments will be returned in the @args array.
 */
static int match_one(char *s, const char *p, substring_t args[])
{
	char *meta;
	int argc = 0;

	if (!p)
		return 1;

	while(1) {
		int len = -1;
		meta = strchr(p, '%');
		if (!meta)
			return strcmp(p, s) == 0;

		if (strncmp(p, s, meta-p))
			return 0;

		s += meta - p;
		p = meta + 1;

		if (isdigit(*p))
			len = strtoul(p, (char **) &p, 10);
		else if (*p == '%') {
			if (*s++ != '%')
				return 0;
			p++;
			continue;
		}

		if (argc >= MAX_OPT_ARGS)
			return 0;

		args[argc].from = s;
		switch (*p++) {
		case 's': {
			size_t str_len = strlen(s);

			if (str_len == 0)
				return 0;
			if (len == -1 || len > str_len)
				len = str_len;
			args[argc].to = s + len;
			break;
		}
		case 'd':
			strtol(s, &args[argc].to, 0);
			goto num;
		case 'u':
			strtoul(s, &args[argc].to, 0);
			goto num;
		case 'o':
			strtoul(s, &args[argc].to, 8);
			goto num;
		case 'x':
			strtoul(s, &args[argc].to, 16);
		num:
			if (args[argc].to == args[argc].from)
				return 0;
			break;
		default:
			return 0;
		}
		s = args[argc].to;
		argc++;
	}
}

/**
 * match_token: - Find a token (and optional args) in a string
 * @s: the string to examine for token/argument pairs
 * @table: match_table_t describing the set of allowed option tokens and the
 * arguments that may be associated with them. Must be terminated with a
 * &struct match_token whose pattern is set to the NULL pointer.
 * @args: array of %MAX_OPT_ARGS &substring_t elements. Used to return match
 * locations.
 *
 * Description: Detects which if any of a set of token strings has been passed
 * to it. Tokens can include up to MAX_OPT_ARGS instances of basic c-style
 * format identifiers which will be taken into account when matching the
 * tokens, and whose locations will be returned in the @args array.
 */
static int match_token(char *s, const match_table_t table, substring_t args[])
{
	const struct match_token *p;

	for (p = table; !match_one(s, p->pattern, args) ; p++)
		;

	return p->token;
}

/**
 * match_number: scan a number in the given base from a substring_t
 * @s: substring to be scanned
 * @result: resulting integer on success
 * @base: base to use when converting string
 *
 * Description: Given a &substring_t and a base, attempts to parse the substring
 * as a number in that base. On success, sets @result to the integer represented
 * by the string and returns 0. Returns -ENOMEM, -EINVAL, or -ERANGE on failure.
 */
static int match_number(substring_t *s, int *result, int base)
{
	char *endp;
	char *buf;
	int ret;
	long val;
	size_t len = s->to - s->from;

	buf = malloc(len + 1);
	if (!buf)
		return -ENOMEM;
	memcpy(buf, s->from, len);
	buf[len] = '\0';

	ret = 0;
	val = strtol(buf, &endp, base);
	if (endp == buf)
		ret = -EINVAL;
	else if (val < (long)INT_MIN || val > (long)INT_MAX)
		ret = -ERANGE;
	else
		*result = (int) val;
	free(buf);
	return ret;
}

/**
 * match_int: - scan a decimal representation of an integer from a substring_t
 * @s: substring_t to be scanned
 * @result: resulting integer on success
 *
 * Description: Attempts to parse the &substring_t @s as a decimal integer. On
 * success, sets @result to the integer represented by the string and returns 0.
 * Returns -ENOMEM, -EINVAL, or -ERANGE on failure.
 */
static int match_int(substring_t *s, int *result)
{
	return match_number(s, result, 0);
}

enum {
	OPT_INSTANCE,
	OPT_CNTLID,
	OPT_ERR
};

static const match_table_t opt_tokens = {
	{ OPT_INSTANCE,		"instance=%d"	},
	{ OPT_CNTLID,		"cntlid=%d"	},
	{ OPT_ERR,		NULL		},
};

static int add_bool_argument(char **argstr, char *tok, bool arg)
{
	char *nstr;

	if (arg) {
		if (asprintf(&nstr, "%s,%s", *argstr, tok) < 0)
			return -ENOMEM;
		free(*argstr);
		*argstr = nstr;
	}
	return 0;
}

static int add_int_argument(char **argstr, char *tok, int arg,
		 bool allow_zero)
{
	char *nstr;

	if ((arg && !allow_zero) || (arg != -1 && allow_zero)) {
		if (asprintf(&nstr, "%s,%s=%d", *argstr, tok, arg) < 0)
			return -ENOMEM;
		free(*argstr);
		*argstr = nstr;
	}
	return 0;
}

static int add_argument(char **argstr, char *tok, char *arg)
{
	char *nstr;

	if (arg && strcmp(arg, "none")) {
		if (asprintf(&nstr, "%s,%s=%s", *argstr, tok, arg) < 0)
			return -ENOMEM;
		free(*argstr);
		*argstr = nstr;
	}
	return 0;
}

int build_options(char **argstr, struct fabrics_config *cfg, bool discover)
{
	if (!cfg->transport) {
		fprintf(stderr, "transport not specified\n");
		return -EINVAL;
	}

	if (strncmp(cfg->transport, "loop", 4)) {
		if (!cfg->traddr) {
			fprintf(stderr, "transport address not specified\n");
			return -EINVAL;
		}
	}

	/* always specify nqn as first arg - this will init the string */
	if (asprintf(argstr, "nqn=%s", cfg->nqn) < 0)
		return -ENOMEM;

	if (add_argument(argstr, "transport", cfg->transport) ||
	    add_argument(argstr, "traddr", cfg->traddr) ||
	    add_argument(argstr, "host_traddr", cfg->host_traddr) ||
	    add_argument(argstr, "trsvcid", cfg->trsvcid) ||
	    add_argument(argstr, "hostnqn", cfg->hostnqn) ||
	    add_argument(argstr, "hostid", cfg->hostid) ||
	    add_int_argument(argstr, "nr_write_queues", cfg->nr_write_queues, false) ||
	    add_int_argument(argstr, "nr_poll_queues", cfg->nr_poll_queues, false) ||
	    add_int_argument(argstr, "reconnect_delay", cfg->reconnect_delay, false) ||
	    add_int_argument(argstr, "ctrl_loss_tmo", cfg->ctrl_loss_tmo, false) ||
	    add_int_argument(argstr, "tos", cfg->tos, true) ||
	    add_bool_argument(argstr, "duplicate_connect", cfg->duplicate_connect) ||
	    add_bool_argument(argstr, "disable_sqflow", cfg->disable_sqflow) ||
	    add_bool_argument(argstr, "hdr_digest", cfg->hdr_digest) ||
	    add_bool_argument(argstr, "data_digest", cfg->data_digest) ||
	    (!discover && (
	      add_int_argument(argstr, "queue_size", cfg->queue_size, false) ||
	      add_int_argument(argstr, "keep_alive_tmo", cfg->keep_alive_tmo, false) ||
	      add_int_argument(argstr, "nr_io_queues", cfg->nr_io_queues, false)))) {
		free(argstr);
		return -ENOMEM;
	}
	return 0;
}

int add_ctrl(const char *argstr)
{
	substring_t args[MAX_OPT_ARGS];
	char buf[BUF_SIZE], *options, *p;
	int token, ret, fd, len = strlen(argstr);

	fd = open(PATH_NVME_FABRICS, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s: %s\n",
			 PATH_NVME_FABRICS, strerror(errno));
		ret = -errno;
		goto out;
	}

	ret = write(fd, argstr, len);
	if (ret != len) {
		if (errno != EALREADY)
			fprintf(stderr, "Failed to write to %s: %s\n",
				 PATH_NVME_FABRICS, strerror(errno));
		ret = -errno;
		goto out_close;
	}

	len = read(fd, buf, BUF_SIZE);
	if (len < 0) {
		fprintf(stderr, "Failed to read from %s: %s\n",
			 PATH_NVME_FABRICS, strerror(errno));
		ret = -errno;
		goto out_close;
	}

	buf[len] = '\0';
	options = buf;
	while ((p = strsep(&options, ",\n")) != NULL) {
		if (!*p)
			continue;

		token = match_token(p, opt_tokens, args);
		switch (token) {
		case OPT_INSTANCE:
			if (match_int(args, &token))
				goto out_fail;
			ret = token;
			goto out_close;
		default:
			/* ignore */
			break;
		}
	}

out_fail:
	fprintf(stderr, "Failed to parse ctrl info for \"%s\"\n", argstr);
	ret = -EINVAL;
out_close:
	close(fd);
out:
	return ret;
}
