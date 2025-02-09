#include "main.h"
#include "colors.h"

/**
 * selfDestruct - a fun way of slowly exiting the shell with a
 * self-destruct sequence. Prints a fake segmentation fault,
 * counts down from the given number, and exits.
 *
 * @countdown: the number of seconds to count down from before exiting.
 */
void selfDestruct(int countdown)
{
	char *paths[1] = {NULL}; /* environment for execve */
	char **sleep1args = malloc(sizeof(char *) * 3); /* sleep args for execve */
	char **sleep2args = malloc(sizeof(char *) * 3); /* sleep args for execve */

	/* execve sleep command arguments setup */
	sleep1args[0] = "sleep"; /* command to pass to execve */
	sleep2args[0] = "sleep"; /* command to pass to execve */
	sleep1args[1] = "1"; /* specifies how many seconds to sleep (1) */
	sleep2args[1] = "2"; /* specifies how many seconds to sleep (2) */
	sleep1args[2] = NULL; /* Null terminate */
	sleep2args[2] = NULL; /* Null terminate */

	printf("Segmentation fault\n"); /* fake seg fault */
	runCommand("/usr/bin/sleep", sleep1args, paths); /* 1 second delay */
	printf(CLR_RED_BOLD); /* sets the text color to red */
	printf("Shellf destruct mode activated.\n\n");

	if (countdown > 3)
		printf(CLR_DEFAULT); /* reset color so it's no still red bold */

	runCommand("/usr/bin/sleep", sleep2args, paths); /* 2 second delay */

	while (countdown) /* prints countdown */
	{
		if (countdown == 3)
			printf(CLR_RED); /* sets the text color to red for last 3 seconds */

		printf("%d\n", countdown);
		countdown--;
		runCommand("/usr/bin/sleep", sleep1args, paths); /* 1 second delay */
	}

	printf("%s\nThe %sGates Of Shell%s have closed. Goodbye.\n%s",
		   CLR_YELLOW_BOLD, CLR_RED_BOLD, CLR_YELLOW_BOLD, CLR_DEFAULT);

	/* free memory */
	free(sleep1args);
	free(sleep2args);

	safeExit(EXIT_SUCCESS);
}

/**
 * initCmd - initialize cmd to the command to pass to execve
 * @cmd: variable to be initialized
 * @tokens: tokens
 *
 * Return: command
 */
void initCmd(char **cmd, char *const *tokens)
{
	if (tokens[0][0] != '/' && tokens[0][0] != '.') /* if input isn't a path */
		*cmd = findPath(tokens[0]);
	else /* if user's input is a path */
		*cmd = strdup(tokens[0]); /* initialize cmd to the input path */
}

/**
 * isNumber - check for non-number characters in a string.
 * If any of the supplied numbers have non-number characters in them,
 * return 0.
 *
 * @number: string to be checked if it's all numbers
 *
 * Return: 1 if it's all numbers; 0 if not.
 */
int isNumber(char *number)
{
	unsigned int i;

	for (i = 0; i < strlen(number); i++)
	{
		if (number[i] > '9' || number[i] < '0')
			return (0);
	}

	return (1);
}

/**
 * _atoi - returns the integer version of a string
 * @s: string
 * Return: string as an int
 */
int _atoi(const char *s)
{
	int i = 0;
	int numberStarted = 0;
	int numberEnded = 0;
	int neg = 1;
	int size;

	for (size = 0; s[size] != '\0'; size++)
	{
		if (!numberEnded)
		{
			if (s[size] == '-' && !numberStarted)
				neg *= -1;
			else if (s[size] >= 48 && s[size] <= 57 && !numberEnded)
			{
				if (!numberStarted)
					numberStarted = 1;
				i *= 10;
				if (neg == -1)
				{
					i *= -1;
					i -= s[size] - 48;
					neg = 0;
				}
				else if (!neg)
					i -= s[size] - 48;
				else
					i += s[size] - 48;
			}
			if (numberStarted && !numberEnded && (s[size] < 48 || s[size] > 57))
				numberEnded = 1;
		}
	}

	return (i);
}

void echor(const char *input, const char *file) 
{
	int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1)
	{
		perror("Error opening file");
		return;
	}
	if (write(fd, input, strlen(input)) < 0)
	{
		perror("Error writing to file");
		close(fd);
		return;
	}
	if (write(fd, "\n", 1) < 0)
    {
        perror("Error writing newline");
        close(fd);
        return;
    }
	close(fd);
}

char* cat(const char *file)
{
    int fd = open(file, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return NULL;
    }

    char *buffer = malloc(1024);
    if (!buffer) {
        perror("Memory allocation failed");
        close(fd);
        return NULL;
    }

    ssize_t bytesRead = read(fd, buffer, 1023);
    if (bytesRead == -1) {
        perror("Error reading file");
        free(buffer);
        close(fd);
        return NULL;
    }

    buffer[bytesRead] = '\0';
    close(fd);
    return buffer;
}

void echodr(const char *input, const char *file) 
{
	int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
	if (fd == -1)
	{
		perror("Error opening file");
		return;
	}
	if (write(fd, input, strlen(input)) < 0)
	{
		perror("Error writing to file");
		close(fd);
		return;
	}
	if (write(fd, "\n", 1) < 0)
    {
        perror("Error writing newline");
        close(fd);
        return;
    }
	close(fd);
}

void rev(char *str, ssize_t len)
{
    ssize_t i, j;
    char temp;
    for (i = 0, j = len - 1; i < j; i++, j--)
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
    }
}

/* void echol(const char *file)
{
    int fd = open(file, O_RDONLY);
    if (fd == -1)
    {
        perror("Error opening file");
        return;
    }

    char buffer[1024];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    
    if (bytes_read < 0)
    {
        perror("Error reading file");
        close(fd);
        return;
    }

    buffer[bytes_read] = '\0';

    rev(buffer, bytes_read);
    printf("%s", buffer);

    close(fd);
} */