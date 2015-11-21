#define _POSIX_C_SOURCE 199309L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "error.h"


void subprocess(const char *command, int *status,
		const size_t stdout_size, char *stdout,
		const size_t stderr_size, char *stderr){

	int stdout_fd[2];
	int stderr_fd[2];
	pid_t pid;

	/* clear out output pointers */
	*stdout = '\0';
	*stderr = '\0';

	int _ = pipe(stdout_fd);
	if (_ != 0) {
		error("pipe error");
	}
	_ = pipe(stderr_fd);
	if (_ != 0) {
		error("pipe error");
	}

	pid = fork();

	switch(pid) {
	case -1:
		perror("fork error");
		exit(-1);

	case 0:
		/* child */
		dup2(stdout_fd[1], 1);
		dup2(stderr_fd[1], 2);

		close(stdout_fd[0]);
		close(stdout_fd[1]);
		close(stderr_fd[0]);
		close(stderr_fd[1]);

		execl("/bin/bash", "bash", "-c", command, (char *) 0);
		perror("exec error");
		exit(-1);

	default:
		/* parent */
		close(stdout_fd[1]);
		close(stderr_fd[1]);

		usleep(100);

		int p_status = 0;

		waitpid(0, &p_status, 0);

		if (WIFEXITED(p_status))
			*status = WEXITSTATUS(p_status);
		else
			*status = -1;

		size_t nbytes = 0;
		size_t total_bytes = 0;

		char buf[1024] = {0};

		// set stdout non-blocking
		int flags = fcntl(stdout_fd[0], F_GETFL, 0);
		fcntl(stdout_fd[0], F_SETFL, flags | O_NONBLOCK);

		while ((nbytes =
			read(stdout_fd[0], buf, sizeof(buf))) > 0) {
			if ((total_bytes + nbytes) > stdout_size) {
				strncat(stdout, buf,
					stdout_size - total_bytes);
				break;
			}
			else {
				strncat(stdout, buf, nbytes);
				total_bytes += nbytes;
				memset(buf, '\0', sizeof(buf));
			}
		}

		close(stdout_fd[0]);


		memset(buf, '\0', sizeof(buf));
		nbytes = 0;
		total_bytes = 0;

		// set stderr non-blocking
		flags = fcntl(stderr_fd[0], F_GETFL, 0);
		fcntl(stderr_fd[0], F_SETFL, flags | O_NONBLOCK);

		while ((nbytes =
			read(stderr_fd[0], buf, sizeof(buf))) > 0) {
			if ((total_bytes + nbytes > stderr_size)) {
				strncat(stderr, buf,
					stderr_size - total_bytes);
			}
			else {
				strncat(stderr, buf, nbytes);
				total_bytes += nbytes;
				memset(buf, '\0', sizeof(buf));
			}
		}

		close(stderr_fd[0]);

	}
}
