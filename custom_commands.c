#include "main.h"

/**
 * customCmd - check if the given input is a custom command. If so, executes it
 *
 * @tokens: tokenized user-input
 * @interactive: if the shell is running in interactive mode (isAtty)
 *
 * Return: 1 if it was a custom command and it was successfully executed,
 * 0 if it's not a custom command,
 * -1 on error
 */
int customCmd(char **tokens, int interactive, char *input)
{
	int ifRtn;
	/* ------------------ custom command "env" ------------------ */
	if (ifCmdEnv(tokens))
		return (1); /* might convert all returns to function return values */

	/* ----------------- custom command "exit" ----------------- */

	if (ifCmdExit(tokens, interactive, input)) /* exits with or without exit code*/
		return (1);							   /* now it returns 1, if it exits */

	/* ----------------- custom command "setenv" ----------------- */
	if (ifCmdSetEnv(tokens))
		return (1);

	/* ----------------- custom command "unsetenv" ----------------- */
	if (ifCmdUnsetEnv(tokens))
		return (1);

	/* ------------- custom command "self-destruct" ------------- */
	if (ifCmdSelfDestruct(tokens) == -1)
		return (-1);

	/* ----------------- custom command "cd" ----------------- */
	ifRtn = ifCmdCd(tokens);
	if (ifRtn)
		return (ifRtn);

	return (0); /* indicate that the input is not a custom command */
}

/**
 * ifCmdSelfDestruct - self destruct oscar mike golf
 * @tokens: tokenized array of user-inputs
 * Return: 0 if successful, -1 otherwise
 */
int ifCmdSelfDestruct(char **tokens)
{
	if (tokens[0] != NULL && (_strcmp(tokens[0], "self-destruct") == 0 ||
							  _strcmp(tokens[0], "selfdestr") == 0))
	{
		int countdown = 5; /* number of seconds to countdown from */

		/* check if user gave any args and if it's a valid positive number */
		if (tokens[1] != NULL && isNumber(tokens[1]) && _atoi_safe(tokens[1]) > 0)
			countdown = _atoi_safe(tokens[1]); /* set countdown to given number */
		/*
		 * NOTE: I'd use abs() instead of checking if its positive, but
		 * abs() is not an allowed function and I don't want to code it.
		 */
		selfDestruct(countdown); /* runs exit() when done */
		return (-1);			 /* indicate error if selfDestruct never exits */
	}
	return (0);
}

/**
 * ifCmdExit: if user-input is "exit" or "quit"
 * @tokens: tokenized array of user-inputs
 * @interactive: isatty() return value. 1 if interactive, 0 otherwise
 *
 * Return: 0 if the command isn't "exit" or "quit"
 */
int ifCmdExit(char **tokens, int interactive, char *input)
{
	int exit_code = EXIT_SUCCESS; // Default exit code

	if ((tokens != NULL) && (tokens[0] != NULL) &&
		((_strcmp(tokens[0], "exit") == 0) || (_strcmp(tokens[0], "quit") == 0)))
	{
		if (tokens[1] != NULL)
		{ // Check for an exit code argument
			if (isNumber(tokens[1]))
			{
				exit_code = _atoi_safe(tokens[1]);
				if (exit_code <= 0)
				{
					fprintf(stderr, "./hsh: 1: exit: Illegal number: %d\n",
							exit_code);
					exit_code = 2; // invalid number
				}
			}
			else /* string */
			{
				fprintf(stderr, "./hsh: 1: exit: Illegal number: %s\n",
						tokens[1]);
				resetAll(tokens, input, NULL);
				safeExit(2); /* exit with error if not number */
			}
		}

		if (interactive)
		{
			printf("%s\nThe %sGates Of Shell%s have closed. Goodbye.\n%s",
				   CLR_YELLOW_BOLD, CLR_RED_BOLD, CLR_YELLOW_BOLD, CLR_DEFAULT);
		}
		free(input);
		free(tokens);
		safeExit(exit_code); /* Exit with the determined code */
		return 1;			 /* Should never reach here, but good practice */
	}

	return 0; /* Not an exit/quit command */
}

/**
 * ifCmdEnv - prints env if the command is env
 * @tokens: tokenized user-input
 *
 * Return: 1 if success, 0 if failure
 */
int ifCmdEnv(char **tokens)
{
	int i;

	if (tokens[0] != NULL && (_strcmp(tokens[0], "env") == 0))
	{
		if (!environ)
			return (1);

		for (i = 0; environ[i] != NULL; i++)
			printf("%s\n", environ[i]);
		return (1); /* indicate success */
	}
	return (0); /* indicate that input is not "env" */
}

/**
 * ifCmdSetEnv - sets an environment variable
 * @tokens: tokenized user-inputed commands
 *
 * Return: 1 if success, 0 if not applicable, -1 if malloc failed
 */
int ifCmdSetEnv(char **tokens)
{
	int rtn;

	if (tokens[0] != NULL && (_strcmp(tokens[0], "setenv") == 0))
	{
		rtn = _setenv(tokens[1], tokens[2], 1);
		if (rtn == -1) /* error occurred in _setenv */
		{
			fprintf(stderr, "error: ");
			perror(NULL);
			return (-1);
		}
		else if (rtn == 0) /* success */
			return (1);
	}
	return (0); /* not setenv */
}

/**
 * ifCmdCd - changes directory
 * @tokens: tokenized array of user-input
 *
 * Return: 1 if successful, 0 if not applicable, 3 too many arguments, otherwise error
 */
int ifCmdCd(char **tokens)
{
	char cwd_buf[PATH_MAX], abs_path[PATH_MAX + 2];
	char *previous_cwd = _getenv("OLDPWD"); /* track previous cwd for '-' handling */
	int chdir_rtn = 0, error_msg = 0;
	char *home = _getenv("HOME");
	char *pwd = _getenv("PWD");

	if (getcwd(cwd_buf, PATH_MAX) == NULL)
	{
		freeIfCmdCd(previous_cwd, home, pwd); /* frees malloc'd strings */
		perror("getcwd");
		return (-1);
	}

	if (!pwd) /* set PWD if not already set */
		_setenv("PWD", cwd_buf, 1);
	else
	{
		free(pwd);
		pwd = NULL;
	}

	if ((tokens[0] != NULL) && (_strcmp(tokens[0], "cd") == 0)) /* cd command found */
	{
		if (tokens[2] != NULL) /* too many arguments */
			error_msg = 3;
		else if (tokens[1] != NULL)
		{
			if (_strcmp(tokens[1], "~") == 0) /* is home */
			{
				if (home)
				{
					chdir_rtn = chdir(home);
					if (chdir_rtn == -1)
						error_msg = 1;
					free(home);
					home = NULL;
				}
				else
					error_msg = 0;
			}
			else if (_strcmp(tokens[1], "-") == 0) /* is previous path */
				if (previous_cwd)
				{
					chdir_rtn = chdir(previous_cwd);
					if (chdir_rtn == -1)
						printf("%s\n", _getenv("PWD"));
					else
						printf("%s\n", previous_cwd);
					free(previous_cwd);
					previous_cwd = NULL;
				}
				else
					printf("%s\n", cwd_buf);
			// else if ((_strncmp(tokens[1], "/root", 5) == 0) && (access(tokens[1], X_OK) != 0))
			else if (access(tokens[1], X_OK) != 0) /* not permission */
			{
				// printf("\nNOT PERMISSION\n\n");
				error_msg = 2;
			}
			else if (is_directory(tokens[1]) == 0) /* is not a directory */
			{
				// printf("\nNOT DIRECTORY\n\n");
				error_msg = 4;
			}
			else if (tokens[1][0] == '/') /* is absolute path */
			{
				chdir_rtn = chdir(tokens[1]);
				if (chdir_rtn == -1)
					error_msg = 1;
			}
			else /* relative path */
			{
				_build_path(cwd_buf, tokens[1], abs_path);
				chdir_rtn = chdir(abs_path);
				if (chdir_rtn == -1)
					error_msg = 1;
			}
		}
		else /* default go $HOME */
		{
			if (home)
			{
				chdir_rtn = chdir(home);
				free(home);
				home = NULL;
			}
			else
				error_msg = 0;
		}
		if ((chdir_rtn == -1) || (error_msg > 0)) /* chdir failed or custom error */
		{
			if ((error_msg == 1) || (error_msg == 4))
				printf("%s\n", cwd_buf);

			freeIfCmdCd(previous_cwd, home, pwd);
			if (chdir_rtn == -1)
				return (-1);
			if ((error_msg == 1) || (error_msg == 4))
				return (1);
			if (error_msg == 2)
				return (2);
			if (error_msg == 3)
				return (3); /* custom error */
			return (0);		/* consider return errno */
		}
		else /* on success set OLD PWD and PWD */
		{
			_setenv("OLDPWD", cwd_buf, 1);

			if (getcwd(cwd_buf, PATH_MAX) == NULL)
			{
				freeIfCmdCd(previous_cwd, home, pwd);
				perror("getcwd");
				return (-1);
			}
			_setenv("PWD", cwd_buf, 1);
		}
	}
	else
	{
		freeIfCmdCd(previous_cwd, home, pwd);
		return (0); /* cd not applicable */
	}

	freeIfCmdCd(previous_cwd, home, pwd);
	return (1); /* success */
}

int RightDirect(char *line)
{
	int fd, i = 0, j = 0, position = 0;
	char *filename;
	char *args[100];
	int bufsize = 64;
	char **tokens = malloc(bufsize * sizeof(char *));
	char *token;

	if (!tokens)
	{ /* Stanard Malloc Error Handling */
		fprintf(stderr, "hsh: allocation error\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, " \t\r\n\a"); /* Tokenize the input */
	while (token != NULL)
	{
		char *marker = _strchr(token, '>'); /* Find the first occurrence of > and set it to variable marker */
		if (marker)
		{					/* If > is found in the input */
			*marker = '\0'; /* Split the token at > */
			if (*token != '\0')
			{								/* If there is something before > */
				tokens[position++] = token; /* Add the token before > */
			}
			tokens[position++] = ">"; /* Add > to the tokens */
			if (*(marker + 1) != '\0')
			{									 /* If there is something after > */
				tokens[position++] = marker + 1; /* Get rid of the space */
			}
		}
		else
		{
			tokens[position++] = token; /* Add the token to the tokens */
		}
		if (position >= bufsize)
		{														/* If the buffer is full */
			bufsize += 64;										/* Increase the buffer size */
			tokens = realloc(tokens, bufsize * sizeof(char *)); /* Reallocate the buffer */
			if (!tokens)
			{
				fprintf(stderr, "hsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, " \t\r\n\a"); /* Get the next token */
	}
	tokens[position] = NULL; /* Add NULL to the end of the tokens */
	while (tokens[i] != NULL)
	{
		if (_strcmp(tokens[i], ">") == 0)
			break;			   /* If > is found in the tokens */
		args[j++] = tokens[i]; /* Add the token to the args */
		i++;
	}
	args[j] = NULL; /* Add NULL to the end of the args */
	if (tokens[i] == NULL || tokens[i + 1] == NULL)
	{ /* If there is no filename after > */
		fprintf(stderr, "Syntax error: Missing filename after '>'\n");
		free(tokens);
		return -1;
	}

	filename = tokens[i + 1];								 /* Get the filename */
	fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644); /* Open the file for writing */
	if (fd == -1)
	{
		perror("open");
		free(tokens);
		return -1;
	}
	pid_t pid = fork(); /* Fork a new process */
	if (pid == -1)
	{
		perror("fork");
		close(fd);
		free(tokens);
		return -1;
	}
	if (pid == 0)
	{
		if (dup2(fd, STDOUT_FILENO) == -1)
		{ /* Redirect stdout to the file */
			perror("dup2");
			close(fd);
			free(tokens);
			exit(1);
		}
		close(fd);
		if (execvp(args[0], args) == -1)
		{
			fprintf(stderr, "./hsh: %d: %s: not found\n", 1, args[0]);
			free(tokens);
			exit(1);
		}
		execvp(args[0], args);
		perror("execvp");
		free(tokens);
		exit(1);
	}
	else
	{
		close(fd);
		wait(NULL);
	}
	free(tokens);
	return 1;
}

int DoubleRightDirect(char *line)
{
	int fd, i = 0, j = 0, position = 0;
	char *filename;
	char *args[100];
	int bufsize = 64;
	char **tokens = malloc(bufsize * sizeof(char *));
	char *token;

	if (!tokens)
	{ /* Stanard Malloc Error Handling */
		fprintf(stderr, "hsh: allocation error\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, " \t\r\n\a"); /* Tokenize the input */
	while (token != NULL)
	{
		char *marker = _strstr(token, ">>"); /* Find the first occurrence of > and set it to variable marker */
		if (marker)
		{					/* If > is found in the input */
			*marker = '\0'; /* Split the token at > */
			if (*token != '\0')
			{								/* If there is something before > */
				tokens[position++] = token; /* Add the token before > */
			}
			tokens[position++] = ">>"; /* Add > to the tokens */
			if (*(marker + 2) != '\0')
			{									 /* If there is something after > */
				tokens[position++] = marker + 2; /* Get rid of the space */
			}
		}
		else
		{
			tokens[position++] = token; /* Add the token to the tokens */
		}
		if (position >= bufsize)
		{														/* If the buffer is full */
			bufsize += 64;										/* Increase the buffer size */
			tokens = realloc(tokens, bufsize * sizeof(char *)); /* Reallocate the buffer */
			if (!tokens)
			{
				fprintf(stderr, "hsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, " \t\r\n\a"); /* Get the next token */
	}
	tokens[position] = NULL; /* Add NULL to the end of the tokens */
	while (tokens[i] != NULL)
	{
		if (_strcmp(tokens[i], ">>") == 0)
			break;			   /* If > is found in the tokens */
		args[j++] = tokens[i]; /* Add the token to the args */
		i++;
	}
	args[j] = NULL; /* Add NULL to the end of the args */
	if (tokens[i] == NULL || tokens[i + 1] == NULL)
	{ /* If there is no filename after > */
		fprintf(stderr, "Syntax error: Missing filename after '>>'\n");
		free(tokens);
		return -1;
	}

	filename = tokens[i + 1];								  /* Get the filename */
	fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644); /* Open the file for writing */
	if (fd == -1)
	{
		perror("open");
		free(tokens);
		return -1;
	}
	pid_t pid = fork(); /* Fork a new process */
	if (pid == -1)
	{
		perror("fork");
		close(fd);
		free(tokens);
		return -1;
	}
	if (pid == 0)
	{
		if (dup2(fd, STDOUT_FILENO) == -1)
		{ /* Redirect stdout to the file */
			perror("dup2");
			close(fd);
			free(tokens);
			exit(1);
		}
		close(fd);
		if (execvp(args[0], args) == -1)
		{
			fprintf(stderr, "./hsh: %d: %s: not found\n", 1, args[0]);
			for (int k = 0; k < position; k++)
				free(tokens[k]);
			free(tokens);
			exit(127);
		}
		perror("execvp");
		free(tokens);
		exit(1);
	}
	else
	{
		close(fd);
		wait(NULL);
	}
	free(tokens);
	return 1;
}

int LeftDirect(char *line)
{
	int fd, i = 0, j = 0, position = 0;
	char *filename;
	char *args[100];
	int bufsize = 64;
	char **tokens = malloc(bufsize * sizeof(char *));
	char *token;

	if (!tokens)
	{
		fprintf(stderr, "hsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, " \t\r\n\a");
	while (token != NULL)
	{
		char *marker = _strchr(token, '<');
		if (marker)
		{
			*marker = '\0';
			if (*token != '\0')
			{
				tokens[position++] = token;
			}
			tokens[position++] = "<";
			if (*(marker + 1) != '\0')
			{
				tokens[position++] = marker + 1;
			}
		}
		else
		{
			tokens[position++] = token;
		}

		if (position >= bufsize)
		{
			bufsize += 64;
			tokens = realloc(tokens, bufsize * sizeof(char *));
			if (!tokens)
			{
				fprintf(stderr, "hsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, " \t\r\n\a");
	}
	tokens[position] = NULL;

	while (tokens[i] != NULL)
	{
		if (_strcmp(tokens[i], "<") == 0)
			break;
		args[j++] = tokens[i];
		i++;
	}
	args[j] = NULL;

	if (tokens[i] == NULL || tokens[i + 1] == NULL)
	{
		fprintf(stderr, "Syntax error: Missing filename after '<'\n");
		free(tokens);
		return -1;
	}

	filename = tokens[i + 1];
	fd = open(filename, O_RDONLY);
	if (fd == -1)
	{
		perror("open");
		free(tokens);
		return -1;
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		perror("fork");
		close(fd);
		free(tokens);
		return -1;
	}

	if (pid == 0)
	{
		if (dup2(fd, STDIN_FILENO) == -1)
		{
			perror("dup2");
			close(fd);
			free(tokens);
			exit(1);
		}
		close(fd);

		if (execvp(args[0], args) == -1)
		{
			fprintf(stderr, "./hsh: %d: %s: not found\n", 1, args[0]);
			free(tokens);
			exit(1);
		}
		execvp(args[0], args);
		perror("execvp");
		free(tokens);
		exit(1);
	}
	else
	{
		close(fd);
		wait(NULL);
	}

	free(tokens);
	return 1;
}

int DoubleLeftDirect(char *line)
{
	int fd, i = 0, j = 0, position = 0;
	char *delimiter;
	char *args[100];
	int bufsize = 64;
	char **tokens = malloc(bufsize * sizeof(char *));
	char *token;

	if (!tokens)
	{
		fprintf(stderr, "hsh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(line, " \t\r\n\a");
	while (token != NULL)
	{
		char *marker = _strchr(token, '<');
		if (marker && *(marker + 1) == '<')
		{
			*marker = '\0';
			if (*token != '\0')
			{
				tokens[position++] = token;
			}
			tokens[position++] = "<<";
			if (*(marker + 2) != '\0')
			{
				tokens[position++] = marker + 2;
			}
		}
		else
		{
			tokens[position++] = token;
		}

		if (position >= bufsize)
		{
			bufsize += 64;
			tokens = realloc(tokens, bufsize * sizeof(char *));
			if (!tokens)
			{
				fprintf(stderr, "hsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token = strtok(NULL, " \t\r\n\a");
	}
	tokens[position] = NULL;

	while (tokens[i] != NULL)
	{
		if (_strcmp(tokens[i], "<<") == 0)
			break;
		args[j++] = tokens[i];
		i++;
	}
	args[j] = NULL;

	if (tokens[i] == NULL || tokens[i + 1] == NULL)
	{
		fprintf(stderr, "Syntax error: Missing delimiter after '<<'\n");
		free(tokens);
		return -1;
	}

	delimiter = tokens[i + 1];

	char input[1024];
	char *input_buffer = input;
	printf("Enter input (end with %s):\n", delimiter);

	while (fgets(input_buffer, sizeof(input), stdin))
	{
		if (strncmp(input_buffer, delimiter, strlen(delimiter)) == 0)
		{
			break;
		}
		input_buffer += strlen(input_buffer);
	}

	pid_t pid = fork();
	if (pid == -1)
	{
		perror("fork");
		free(tokens);
		return -1;
	}

	if (pid == 0)
	{
		fd = open("/tmp/heredoc.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
		if (fd == -1)
		{
			perror("open");
			free(tokens);
			exit(1);
		}
		write(fd, input, strlen(input));
		close(fd);

		fd = open("/tmp/heredoc.txt", O_RDONLY);
		if (fd == -1)
		{
			perror("open");
			free(tokens);
			exit(1);
		}

		if (dup2(fd, STDIN_FILENO) == -1)
		{
			perror("dup2");
			close(fd);
			free(tokens);
			exit(1);
		}
		close(fd);

		if (execvp(args[0], args) == -1)
		{
			fprintf(stderr, "./hsh: %d: %s: not found\n", 1, args[0]);
			free(tokens);
			exit(1);
		}
		execvp(args[0], args);
		perror("execvp");
		free(tokens);
		exit(1);
	}
	else
	{
		wait(NULL);
	}

	free(tokens);
	return 1;
}
