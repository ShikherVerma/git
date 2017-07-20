/*
 * Builtin "git verify-push-certs"
 *
 * Copyright (c) 2017 Shikher Verma <root@shikherverma.com>
 *
 * Based on git-verify-tag
 */
#include "cache.h"
#include "config.h"
#include "builtin.h"
#include "run-command.h"
#include <signal.h>
#include "parse-options.h"
#include "gpg-interface.h"

static const char * const verify_push_certs_usage[] = {
		N_("git verify-push-certs [-v | --verbose]"),
		NULL
};

static int run_gpg_verify(const char *buf, unsigned long size, unsigned flags)
{
	struct signature_check sigc;
	size_t payload_size;
	int ret;

	memset(&sigc, 0, sizeof(sigc));

	payload_size = parse_signature(buf, size);

	if (size == payload_size) {
		if (flags & GPG_VERIFY_VERBOSE)
			write_in_full(1, buf, payload_size);
		return error("no signature found");
	}

	ret = check_signature(buf, payload_size, buf + payload_size,
				size - payload_size, &sigc);

	if (!(flags & GPG_VERIFY_OMIT_STATUS))
		print_signature_buffer(&sigc, flags);

	signature_check_clear(&sigc);
	return ret;
}

static int verify_push_cert(const char *name, unsigned flags)
{
	enum object_type type;
	struct object_id oid;
	char *buf;
	unsigned long size;
	int ret;

	if (get_oid(name, &oid))
		return error("'%s' not found.", name);

	buf = read_sha1_file(oid.hash, &type, &size);
	if (!buf)
		return error("%s: unable to read file.", name);
	if (type != OBJ_BLOB)
		return error("%s: cannot verify a non-blob object of type %s.",
				name, typename(type));

	ret = run_gpg_verify(buf, size, flags);

	free(buf);
	return ret;
}

int cmd_verify_push_certs(int argc, const char **argv, const char *prefix)
{
	int verbose = 0;
	unsigned flags = 0;
	const struct option verify_push_certs_options[] = {
		OPT__VERBOSE(&verbose, N_("print push cert contents")),
		OPT_BIT(0, "raw", &flags, N_("print raw gpg status output"), GPG_VERIFY_RAW),
		OPT_END()
	};

	argc = parse_options(argc, argv, prefix, verify_push_certs_options,
			     verify_push_certs_usage, PARSE_OPT_KEEP_ARGV0);

	if (verbose)
		flags |= GPG_VERIFY_VERBOSE;

	/* sometimes the program was terminated because this signal
	 * was received in the process of writing the gpg input: */
	signal(SIGPIPE, SIG_IGN);
	unsigned char sha1[20];
	get_sha1(git_path_push_certs(), sha1);
	return verify_push_cert(sha1_to_hex(sha1), flags);
}
