#define _POSIX_C_SOURCE 199309L

#define ROOT_UID 0

#define ANSI_COLOR_BRIGHT_RED "\x1b[1;31m"
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_BRIGHT_GREEN "\x1b[1;32m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_BRIGHT_YELLOW "\x1b[1;33m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_BRIGHT_BLUE "\x1b[1;34m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_BRIGHT_MAGENTA "\x1b[1;35m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_BRIGHT_CYAN "\x1b[1;36m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

#include <fnmatch.h>
#include <glob.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "config.h"
#include "util/util.h"


typedef struct {
	char mon_binary[1024];
	char dir[1024];
	char log_dir[1024];
	char apps[1024];
	unsigned int sleep;
	unsigned int attempts;
} config_t;

typedef struct {
	char name[1024];
	char command[1024];
	char out_file[1024];
	char err_file[1024];
	unsigned int sleep;
	unsigned int attempts;
} app_t;

static config_t config;

void initialize()
{
	memset(&config, 0, sizeof(config));
	strncpy(config.mon_binary, MON_BINARY, sizeof(config.mon_binary));
	strncpy(config.dir, MONCTL_DIR, sizeof(config.dir));
	strncpy(config.log_dir, MONCTL_LOG_DIR, sizeof(config.log_dir));
	strncpy(config.apps, MONCTL_APPS, sizeof(config.apps));
	config.sleep = 1;
	config.attempts = 0;
}

int pid_alive(const int pid)
{
	return (kill(pid, 0) == 0);
}


int read_mon_pid_file(const char *fname)
{

	if (access(fname, F_OK) == -1)
		return 0;

	FILE *fh = fopen(fname, "r");

	if (ferror(fh))
		error("read config error");


	char buf[1024];

	while (fgets(buf, sizeof(buf), fh)) {
		char *line = trim(buf);

		if (!strcmp("", line)) {
			break;
		}
	}
	int rv = strtol(trim(buf), NULL, 10);

	fclose(fh);

	return rv;
}


void read_config()
{

	if (access(MONCTL_CONFIG, F_OK) == -1)
		error("config file not exists");


	FILE *cfg_fh = fopen(MONCTL_CONFIG, "r");

	if (ferror(cfg_fh))
		error("read config error");

	char *line;
	char buf[10240];

	char key[1024] = {'\0'};
	char value[1024] = {'\0'};

	while (fgets(buf, sizeof(buf), cfg_fh)) {
		line = trim(buf);
		if (line == NULL || !strcmp("", line))
			continue;

		sscanf(line, "%[^= ] = %[^\n]", key, value);

		if (!strcmp("mon_binary", key)) {
			memset(config.mon_binary, '\0', sizeof(config.mon_binary));
			strcat(config.mon_binary, trim(value));
			continue;
		}
		if (!strcmp("dir", key)) {
			memset(config.dir, '\0', sizeof(config.dir));
			strcat(config.dir, trim(value));
			continue;
		}
		if (!strcmp("log_dir", key)) {
			memset(config.log_dir, '\0', sizeof(config.log_dir));
			strcat(config.log_dir, trim(value));
			continue;
		}
		if (!strcmp("apps", key)) {
			memset(config.apps, '\0', sizeof(config.apps));
			strcat(config.apps, trim(value));
			continue;
		}
		if (!strcmp("sleep", key)) {
			unsigned int sleep = 0;
			sscanf(value, "%u", &sleep);
			config.sleep = sleep;
			continue;
		}
		if (!strcmp("attempts", key)) {
			unsigned int attempts = 0;
			sscanf(value, "%u", &attempts);
			config.attempts = attempts;
			continue;
		}
	}

	fclose(cfg_fh);
}


void pre_check()
{
	if (access(config.mon_binary, F_OK) == -1)
		error("cannot find mon_binary");

	if (access(config.mon_binary, X_OK) == -1)
		error("cannot execute mon_binary");
}


app_t read_app_config(const char * fname)
{

	app_t rv = {
		.name = "",
		.command = "",
		.out_file = "",
		.err_file = "",
		.sleep = 0,
		.attempts = 0
	};

	if (fname == NULL || !strcmp("", fname)) {
		error("read_app_config: no fname");
	}

	if (access(fname, F_OK) == -1)
		error("read_app_config: file not exists");

	FILE *cfg_fh = fopen(fname, "r");

	if (ferror(cfg_fh)) {

		char err_msg[2048] = {'\0'};
		sprintf(err_msg, "read %s error", fname);
		error(err_msg);
	}

	char *line;
	char buf[10240];
	while (fgets(buf, sizeof(buf), cfg_fh)) {
		line = trim(buf);

		if (line == NULL || !strcmp("", line))
			continue;

		char key[1024] = {'\0'};
		char value[1024] = {'\0'};

		sscanf(line, "%[^= ] = %[^\n]", key, value);

		if (!strcmp("name", key)) {
			memset(rv.name, '\0', sizeof(rv.name));
			strcat(rv.name, trim(value));
			continue;
		}
		if (!strcmp("command", key)) {
			memset(rv.command, '\0', sizeof(rv.command));
			strcat(rv.command, trim(value));
			continue;
		}
		if (!strcmp("stdout", key)) {
			memset(rv.out_file, '\0', sizeof(rv.out_file));
			strcat(rv.out_file, trim(value));
			continue;
		}
		if (!strcmp("stderr", key)) {
			memset(rv.err_file, '\0', sizeof(rv.err_file));
			strcat(rv.err_file, trim(value));
			continue;
		}
		if (!strcmp("sleep", key)) {
			unsigned int sleep = 0;
			sscanf(value, "%u", &sleep);
			rv.sleep = sleep;
			continue;
		}
		if (!strcmp("attempts", key)) {
			unsigned int attempts = 0;
			sscanf(value, "%u", &attempts);
			rv.attempts = attempts;
			continue;
		}
	}

	fclose(cfg_fh);

	return rv;
}


void reap()
{
}


void get_pid_fname(const char *name, char *needle)
{
	memset(needle, '\0', 1);
	strcat(needle, config.dir);
	strcat(needle, "/");
	strcat(needle, name);
	strcat(needle, ".pid");
}


void get_mon_pid_fname(const char *name, char *needle)
{
	memset(needle, '\0', 1);
	strcat(needle, config.dir);
	strcat(needle, "/");
	strcat(needle, name);
	strcat(needle, ".mon.pid");
}


void get_stdout_fname(const char *name, char *needle)
{
	memset(needle, '\0', 1);
	strcat(needle, config.log_dir);
	strcat(needle, "/");
	strcat(needle, name);
	strcat(needle, ".out");
}


void get_stderr_fname(const char *name, char *needle)
{
	memset(needle, '\0', 1);
	strcat(needle, config.log_dir);
	strcat(needle, "/");
	strcat(needle, name);
	strcat(needle, ".err");
}


void start(const app_t app)
{

	size_t s = 0;

	char _mon_pid_fname[2048] = {'\0'};
	get_mon_pid_fname(app.name, _mon_pid_fname);
	char *mon_pid_fname = trim(_mon_pid_fname);

	int mon_pid = read_mon_pid_file(mon_pid_fname);

	if (mon_pid && pid_alive(mon_pid)) {
		printf("    "
		       ANSI_COLOR_BRIGHT_GREEN
		       "started"
		       ANSI_COLOR_RESET
		       " : %s\n",
		       app.name);
		return;
	}

	char _p_mon_pid[strlen(mon_pid_fname) + 128];
	memset(_p_mon_pid, '\0', sizeof(_p_mon_pid));

	sprintf(_p_mon_pid, " -m %s", mon_pid_fname);
	char *p_mon_pid = trim(_p_mon_pid);

	int _sleep = app.sleep ? app.sleep : config.sleep;
	char sleep[32] = {'\0'};
	sprintf(sleep, " -s %d", _sleep);

	char *p_sleep = trim(sleep);

	int _attempts = app.attempts ? app.attempts : config.attempts;
	char *p_attempts = "";
	char attempt[32] = {'\0'};
	if (_attempts > 0) {
		sprintf(attempt, " -a %d", _attempts);
		p_attempts = trim(attempt);
	}

	char _pid[2048] = {'\0'};
	get_pid_fname(app.name, _pid);
	char _p_pid[2048] = {'\0'};
	sprintf(_p_pid, " -p %s", _pid);
	char *p_pid = trim(_p_pid);


	char _stdout[2048] = {'\0'};
	strcpy(_stdout, app.out_file);
	if (app.out_file == NULL || strlen(app.out_file) == 0) {
		get_stdout_fname(app.name, _stdout);
	}
	char _p_stdout[2048] = {'\0'};
	sprintf(_p_stdout, " -l %s", _stdout);
	char *p_stdout = trim(_p_stdout);


	char _stderr[2048] = {'\0'};
	strcpy(_stderr, app.err_file);
	if (app.err_file == NULL || strlen(app.err_file) == 0) {
		get_stderr_fname(app.name, _stderr);
	}
	char _p_stderr[2048] = {'\0'};
	sprintf(_p_stderr, " -e %s", _stderr);
	char *p_stderr = trim(_p_stderr);


	s = strlen(config.mon_binary) +
		strlen(p_sleep) +
		strlen(p_attempts) +
		strlen(p_pid) +
		strlen(p_mon_pid) +
		strlen(p_stdout) +
		strlen(p_stderr) +
		strlen(app.command) + 128;


	char _cmd[s];
	memset(_cmd, '\0', s);

	strcat(_cmd, config.mon_binary);
	strcat(_cmd, " -d ");
	strcat(_cmd, p_sleep);
	strcat(_cmd, " ");
	strcat(_cmd, p_attempts);
	strcat(_cmd, " ");
	strcat(_cmd, p_pid);
	strcat(_cmd, " ");
	strcat(_cmd, p_mon_pid);
	strcat(_cmd, " ");
	strcat(_cmd, p_stdout);
	strcat(_cmd, " ");
	strcat(_cmd, p_stderr);
	strcat(_cmd, " ");
	strcat(_cmd, "\"");
	strcat(_cmd, app.command);
	strcat(_cmd, "\"");

	char *cmd = trim(_cmd);

	int status;
	char cmd_stdout[1024] = {'\0'};
	char cmd_stderr[1024] = {'\0'};

	subprocess(cmd, &status,
		   1024, cmd_stdout,
		   1024, cmd_stderr);

	usleep(500000);

	printf("    "
	       ANSI_COLOR_BRIGHT_GREEN
	       "started"
	       ANSI_COLOR_RESET
	       " : %s\n",
	       app.name);
}


void stop(const app_t app)
{
	char _mon_pid_fname[2048];
	get_mon_pid_fname(app.name, _mon_pid_fname);
	char *mon_pid_fname = trim(_mon_pid_fname);

	int mon_pid = read_mon_pid_file(mon_pid_fname);

	if (mon_pid == 0 || !pid_alive(mon_pid)) {
		goto exit;
	}

	kill(mon_pid, SIGTERM);
	usleep(500000);

exit:
	printf("    "
	       ANSI_COLOR_BRIGHT_RED
	       "stopped"
	       ANSI_COLOR_RESET
	       " : %s\n",
	       app.name);
}


void restart(const app_t app)
{
	stop(app);
	usleep(500000);
	start(app);
}

void tail(const app_t app)
{
	char _fname[2048] = {'\0'};
	strcpy(_fname, app.out_file);
	if (app.out_file == NULL || strlen(app.out_file) == 0) {
		get_stdout_fname(app.name, _fname);
	}
	char *fname = trim(_fname);

	char _cmd[strlen(fname) + 128];
	memset(_cmd, '\0', sizeof(_cmd));

	strcat(_cmd, "tail -n 30 -F ");
	strcat(_cmd, fname);
	char *cmd = trim(_cmd);

	fprintf(stdout, cmd);
	fprintf(stdout, "\n");
	fflush(stdout);

	execl("/bin/sh", "sh", "-c", cmd, NULL);
}


void taile(const app_t app)
{
	char _fname[2048] = {'\0'};
	strcpy(_fname, app.err_file);
	if (app.err_file == NULL || strlen(app.err_file) == 0) {
		get_stderr_fname(app.name, _fname);
	}
	char *fname = trim(_fname);

	char _cmd[strlen(fname) + 128];
	memset(_cmd, '\0', sizeof(_cmd));

	strcat(_cmd, "tail -n 30 -F ");
	strcat(_cmd, fname);
	char *cmd = trim(_cmd);

	fprintf(stderr, cmd);
	fprintf(stderr, "\n");
	fflush(stderr);

	execl("/bin/sh", "sh", "-c", cmd, NULL);
}

void help()
{
	printf("\n");
	printf("  monctl usage: \n");
	printf("\n");
	printf("    monctl [status]\n");
	printf("    monctl status [app_name]\n");
	printf("    monctl start [app_name]\n");
	printf("    monctl stop [app_name]\n");
	printf("    monctl restart [app_name]\n");
	printf("    monctl tail [app_name]\n");
	printf("    monctl taile [app_name]\n");
	printf("    monctl help\n");
	printf("\n");
}


void status(const app_t app)
{
	char _pid_fname[2048];
	get_pid_fname(app.name, _pid_fname);
	char *p_pid = trim(_pid_fname);

	int status = 0;

	size_t s = strlen(config.mon_binary) + strlen(p_pid) + 128;

	char _cmd[s];
	memset(_cmd, '\0', sizeof(_cmd));

	strcat(_cmd, config.mon_binary);
	strcat(_cmd, " --pidfile ");
	strcat(_cmd, p_pid);
	strcat(_cmd, " --status");
	char *cmd = trim(_cmd);

	char cmd_stdout[1024] = {'\0'};
	char cmd_stderr[1024] = {'\0'};

	subprocess(cmd, &status,
		   1024, cmd_stdout,
		   1024, cmd_stderr);

	if (strlen(cmd_stdout)) {
		printf("    %s : %s\n", app.name, trim(cmd_stdout));
	}
	else {
		printf("    %s : "
		       ANSI_COLOR_BRIGHT_RED
		       "stopped"
		       ANSI_COLOR_RESET
		       "\n", app.name);
	}
}


int main(int argc, char** argv)
{

	if (getuid() != ROOT_UID) {
		/* TODO: maybe not check */
		error("monctl should be run by root");
	}

	initialize();

	void (*func)(const app_t) = NULL;

	char *name;

	if (argc == 1)
		func = &status;

	if (argc >= 2) {

		if (!strcmp("status", argv[1])) {
			func = &status;
		}
		if (!strcmp("start", argv[1])) {
			func = &start;
		}
		if (!strcmp("stop", argv[1])) {
			func = &stop;
		}
		if (!strcmp("restart", argv[1])) {
			func = &restart;
		}
		if (!strcmp("tail", argv[1])) {
			if (argc <= 2) {
				help();
				return -1;
			}
			func = &tail;
		}
		if (!strcmp("taile", argv[1])) {
			if (argc <= 2) {
				help();
				return -1;
			}
			func = &taile;
		}

		if (!strcmp("help", argv[1]) ||
		    !strcmp("-h", argv[1]) ||
		    !strcmp("--help", argv[1])
			) {
			help();
			return -1;
		}

		if (!strcmp("version", argv[1]) ||
		    !strcmp("-V", argv[1]) ||
		    !strcmp("--version", argv[1])
			) {
			printf("\n  monctl version: " VERSION "\n\n");
			return -1;
		}

		if (func == NULL){
			char err_msg[1024];
			sprintf(err_msg, "unknown action %s", argv[1]);
			trim(err_msg);
			error(err_msg);
		}
	}

	if (argc >= 3) {
		name = argv[2];
	}
	else {
		name = "*";
	}

	read_config();
	pre_check();

	if (config.apps == NULL || (strcmp("", config.apps))) {

		glob_t result;

		glob(config.apps, 0, NULL, &result);
		printf("\n");

		for(unsigned int i = 0; i < result.gl_pathc; i++) {
			const app_t app =
				read_app_config(result.gl_pathv[i]);
			if (strlen(app.name) > 0 && fnmatch(name, app.name, 0) == 0) {
				(*func)(app);

			}
		}

		globfree(&result);
		printf("\n");

	}

	reap();

	return 0;
}
