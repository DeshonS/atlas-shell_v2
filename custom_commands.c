#include "main.h"

/**
 * customCmd - check if the given input is a custom command. If so, executes it
 *
 * @tokens: tokenized user-input
 * @interactive: if the shell is running in interactive mode (isAtty)
 * @f1: variable to be freed if the command exits. (i.e. input)
 * @f2: variable to be freed if the command exits. (i.e. cmd)
 * @f3: variable to be freed if the command exits. (i.e. cmd_token)
 * @argv: array of user inputs
 *
 * Return: 1 if it was a custom command and it was successfully executed,
 * 0 if it's not a custom command,
 * -1 on error
 */
int customCmd(char **tokens, int interactive)
{
	int ifRtn;
	/* ------------------ custom command "env" ------------------ */
	if (ifCmdEnv(tokens))
		return (1); /* might convert all returns to function return values */

	/* ----------------- custom command "exit" ----------------- */

	if (ifCmdExit(tokens, interactive) == 1) /* exits with or without exit code*/
	{
		return (1); /* now it returns 1, if it exits */
	}

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
	/* ----------------- custom command "echo" ----------------- */
	if (ifCmdEcho(tokens))
		return (1);

	return (0); /* indicate that the input is not a custom command */
}
/*
 * note: customCmd() was variadic, but we undid that because of
 * problems calling resetAll() using the args variable.
 */

/**
 * ifCmdSelfDestruct - self destruct oscar mike golf
 * @tokens: tokenized array of user-inputs
 * @f1: variable to be freed if the command exits. (i.e. input)
 * @f2: variable to be freed if the command exits. (i.e. cmd)
 * @f3: variable to be freed if the command exits. (i.e. cmd_token)
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
 */
int ifCmdExit(char **tokens, int interactive)
{
	int exit_code = EXIT_SUCCESS; // Default exit code

	if (tokens != NULL && tokens[0] != NULL &&
		(_strcmp(tokens[0], "exit") == 0 || _strcmp(tokens[0], "quit") == 0))
	{

		if (tokens[1] != NULL)
		{ // Check for an exit code argument
			if (isNumber(tokens[1]))
			{
				exit_code = _atoi_safe(tokens[1]);
				if (exit_code == 0)
				{
					exit_code = 2; // invalid number
				}
			}
			else
			{
				// Handle non-numeric argument (error)
				if (interactive)
				{
					selfDestruct(5); /* or another **appropriate** action */
				}
				else
				{ /* not interactive, print to standard error. */
					fprintf(stderr, "exit: Illegal number: %s\n", tokens[1]);
				}
				safeExit(2); /* exit with error if not number */
			}
		}

		if (interactive)
		{
			printf("%s\nThe %sGates Of Shell%s have closed. Goodbye.\n%s",
				   CLR_YELLOW_BOLD, CLR_RED_BOLD, CLR_YELLOW_BOLD, CLR_DEFAULT);
		}

		safeExit(exit_code); /* Exit with the determined code */
		return 1;			 /* Should never reach here, but good practice */
	}

	return 0; // Not an exit/quit command
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
		{
			freeIfCmdCd(previous_cwd, home, pwd);
			return (3); /* custom error */
		}
		else if (tokens[1] != NULL)
		{

			if (_strcmp(tokens[1], "-") == 0) /* previous path */
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
			else if ((_strncmp(tokens[1], "/root", 5) == 0) && (access(tokens[1], X_OK) != 0))
				error_msg = 4;

			else if (tokens[1][0] == '/')  /* absolute path */
			{
				chdir_rtn = chdir(tokens[1]);
				if (chdir_rtn == -1)
					error_msg = 1;
			}
			else if (_strcmp(tokens[1], "~") == 0)  /* home */

				if (home)
				{
					chdir_rtn = chdir(home);
					free(home);
					home = NULL;
				}
				else
					error_msg = 0;
			else /* relative path */
			{
				_build_path(cwd_buf, tokens[1], abs_path);
				chdir_rtn = chdir(abs_path);
			}
		}
		else if (home) /* default go $HOME */
		{
			chdir_rtn = chdir(home);
			free(home);
			home = NULL;
		}
		else
			error_msg = 0;

		if ((chdir_rtn == -1) || (error_msg > 0)) /* chdir failed or custom error */
		{
			if (error_msg == 1)
				printf("%s\n", cwd_buf);

			freeIfCmdCd(previous_cwd, home, pwd);
			if (chdir_rtn == -1)
				return (-1);
			if (error_msg == 2)
				return (2);
			if (error_msg == 3)
				return(3);  /* custom error */
			else if (error_msg == 4)
				return (-1);
			return (0); /* consider return errno */
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

int ifCmdEcho(char **tokens)
{
	if (tokens[0] != NULL && (_strcmp(tokens[0], "echo") == 0))
	{
		if (tokens[2] != NULL && (_strcmp(tokens[2], ">") == 0))
		{
			echor(tokens[1], tokens[3]);
		}
		else if (tokens[2] != NULL && (_strcmp(tokens[2], ">>") == 0))
		{
			echodr(tokens[1], tokens[3]);
		}
		/* else if (tokens[2] != NULL && (_strcmp(tokens[2], "<") == 0))
		{
			echol(tokens[1], tokens[3]);
		} */
		return (1);  /* signals it was the echo command */
	}
	return (0);  /* signals it wasn't the echo command */
}
