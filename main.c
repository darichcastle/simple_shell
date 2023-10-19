#include "main.h"

/**
 * main - the main function
 *
 * Return: (Success) 0 always
 * ------- (Fail) we drop out the looser :)
 */
int main(void)
{
	sh_t data;
	int parseLineResult;

	_memset((void *)&data, 0, sizeof(data));
	signal(SIGINT, signal_handler);

	while (1)
	{	index_cmd(&data);

		if (read_line(&data) < 0)
		{
			if (isatty(STDIN_FILENO))
				PRINT("\n");
			break;
		}
		if (split_line(&data) < 0)
		{	free_data(&data);
			continue;
		}
		parseLineResult = parse_line(&data);

		if (parseLineResult == 0)
		{
			free_data(&data);
			continue;
		}

		if (parseLineResult < 0)
		{
			print_error(&data);
			continue;
		}
		if (process_cmd(&data) < 0)
		{	print_error(&data);
			break;
		}
		free_data(&data);
	}
	free_data(&data);
	exit(EXIT_SUCCESS);
}

/**
 * read_line - read a line from the standard input
 * @data: a pointer to the struct of data
 *
 * Return: (Success) a positive number
 * ------- (Fail) a negative number
 */
int read_line(sh_t *data)
{
	char *currentPtr, *endPtr, character;
	size_t size = BUFSIZE, readStatus, length, newSize;

	data->line = malloc(size * sizeof(char));
	if (data->line == NULL)
		return (FAIL);

	if (isatty(STDIN_FILENO))
		PRINT(PROMPT);

	for (currentPtr = data->line, endPtr = data->line + size;;)
	{
		readStatus = read(STDIN_FILENO, &character, 1);
		if (readStatus == 0)
			return (FAIL);
		*currentPtr++ = character;

		if (character == '\n')
		{
			*currentPtr = '\0';
			return (SUCCESS);
		}

		if (currentPtr + 2 >= endPtr)
		{
			newSize = size * 2;
			length = currentPtr - data->line;
			data->line =
			_realloc(data->line, size * sizeof(char), newSize * sizeof(char));

			if (data->line == NULL)
				return (FAIL);

			size = newSize;
			endPtr = data->line + size;
			currentPtr = data->line + length;
		}
	}
}

/**
 * split_line - splits line to tokens
 * @data: a pointer to the struct of data
 *
 * Return: (Success) a positive number
 * ------- (Fail) a negative number
 */
int split_line(sh_t *data)
{
	char *token;
	size_t size = TOKENSIZE, newSize, i = 0;

	if (_strcmp(data->line, "\n") == 0)
		return (FAIL);

	data->args = malloc(size * sizeof(char *));
	if (data->args == NULL)
		return (FAIL);

	token = strtok(data->line, " \n\t\r\a\v");

	if (token == NULL)
		return (FAIL);

	while (token)
	{
		data->args[i++] =  token;

		if (i + 2 >= size)
		{
			newSize = size * 2;
			data->args =
			_realloc(data->args, size * sizeof(char *), newSize * sizeof(char *));

			if (data->args == NULL)
				return (FAIL);

			size = newSize;
		}
		token = strtok(NULL, " \n\t\r\a\v");
	}
	data->args[i] = NULL;
	return (SUCCESS);
}

/**
 * parse_line - parses arguments to valid command
 * @data: a pointer to the struct of data
 *
 * Return: (Success) a positive number
 * ------- (Fail) a negative number
 */
int parse_line(sh_t *data)
{
	if (is_path_form(data) > 0)
		return (SUCCESS);

	if (is_builtin(data) > 0)
	{
		if (handle_builtin(data) < 0)
			return (FAIL);
		return (NEUTRAL);
	}

	is_short_form(data);
	return (SUCCESS);
}

/**
 * process_cmd - process command and execute process
 * @data: a pointer to the struct of data
 *
 * Return: (Success) a positive number
 * ------- (Fail) a negative number
 */
int process_cmd(sh_t *data)
{
	pid_t pid;
	int status;

	pid = fork();

	if (pid == 0)
	{
		signal(SIGINT, SIG_DFL);

		if (execve(data->cmd, data->args, environ) < 0)
		{
			data->error_msg = _strdup("not found\n");
			return (FAIL);
		}
	}
	else
	{
		waitpid(pid, &status, WUNTRACED);
	}

	return (SUCCESS);
}
