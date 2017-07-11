#include "cache.h"
#include "exec_cmd.h"
#include "attr.h"

extern char *program_invocation_name;
static const char *get_color(long int pid)
{
	switch(pid % 10)
	{
		case 1: return "\x1b[32m";
		case 2: return "\x1b[33m";
		case 3: return "\x1b[34m";
		case 4: return "\x1b[35m";
		case 5: return "\x1b[36m";
		case 6: return "\x1b[37m";
		case 7: return "\x1b[44m";
		case 8: return "\x1b[45m";
		case 9: return "\x1b[46m";
	}
	return "\x1b[41m";
}
static void my_debugger(const char *file, const int line, const char *function)
{
	FILE *fp = fopen ("/home/shikher/git/logger.logface", "a");
	if (fp != NULL) {
		fprintf(fp, "%s[%d][%ld][%s]\tHIT %s:%d\t%s\x1b[0m\n", get_color(getpid()), getpid(), time(NULL), program_invocation_name, file, line, function);
		fclose(fp);
	}
}

/*
 * Many parts of Git have subprograms communicate via pipe, expect the
 * upstream of a pipe to die with SIGPIPE when the downstream of a
 * pipe does not need to read all that is written.  Some third-party
 * programs that ignore or block SIGPIPE for their own reason forget
 * to restore SIGPIPE handling to the default before spawning Git and
 * break this carefully orchestrated machinery.
 *
 * Restore the way SIGPIPE is handled to default, which is what we
 * expect.
 */
static void restore_sigpipe_to_default(void)
{
	my_debugger(__FILE__,__LINE__,__PRETTY_FUNCTION__);
	sigset_t unblock;

	sigemptyset(&unblock);
	sigaddset(&unblock, SIGPIPE);
	sigprocmask(SIG_UNBLOCK, &unblock, NULL);
	signal(SIGPIPE, SIG_DFL);
}

int main(int argc, const char **argv)
{
	my_debugger(__FILE__,__LINE__,__PRETTY_FUNCTION__);
	/*
	 * Always open file descriptors 0/1/2 to avoid clobbering files
	 * in die().  It also avoids messing up when the pipes are dup'ed
	 * onto stdin/stdout/stderr in the child processes we spawn.
	 */
	sanitize_stdfds();

	git_setup_gettext();

	attr_start();

	git_extract_argv0_path(argv[0]);

	restore_sigpipe_to_default();

	return cmd_main(argc, argv);
}
