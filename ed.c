//
//  ed.c
//  ed
//
//  Created by Filip Čižmář on 12/05/2019.
//  Copyright © 2019 Filip Čižmář. All rights reserved.
//

#include <stdio.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 *	Enum contains all options aliases.
 *	If more then 8 enlarge size of options variable.
 */
enum options {
	options_TRADITIONAL,
	options_LOOSE_EXIT_STATUS,
	options_PROMPT,
	options_RESTRICTED,
	options_QUIET,
	options_VERBOSE
};

/**
 *	Enum contains all errors aliases.
 */ 
enum error {
	error_NONE,
	error_INVALID_ADDRESS,
	error_UNEXPECTED_ADDRESS,
	error_UNKNOWN_COMMAND,
	error_INVALID_COMMAND_SUFFIX
};

/**
 *	Function reads file line by line and stores each line as element of returned array.
 * 	The lines array resize automaticly if it is filled while reading file.
 *	Also it stores numbers of lines into n_lines, numbers of characters into n_chars.
 */ 
char **
read_file (const char *arg, int *p_n_lines, int *p_n_chars, char *p_options)
{
	
	if(*p_options & (1U << options_RESTRICTED))
		if(strstr(arg, "/"))
		{
			fprintf(stderr, "%s: No such file or directory\n", arg);
			return NULL;
		}
	
	FILE *fp;
	fp = fopen(arg, "r"); // read mode
	
	if (fp == NULL)
	{
		perror(arg);
		return NULL;
	}
	
	char **lines = malloc(sizeof(char *));
	int size_of_lines = 1;
	char line[1024];
	
	while (fgets ( line, sizeof line, fp ) != NULL)
	{
		if (*p_n_lines == size_of_lines)
		{
			char **plines = malloc(sizeof(char *) * size_of_lines * 2);
			memcpy(plines, lines, sizeof(char *) * *p_n_lines);
			free(lines);
			lines = plines;
			size_of_lines *= 2;
		}
		
		*p_n_chars += strlen(line);
		lines[*p_n_lines] = malloc(strlen(line) + 1);
		strcpy(lines[*p_n_lines], line);
		++*p_n_lines;
	}
	
	fclose(fp);
	return lines;
}

/**
 *	Print help.
 */
void
opt_help()
{
	printf("Usage: ed [options] [file]\n"
		   "Options:\n"
		   "-h, --help                 display this help and exit\n"
		   "-V, --version              output version information and exitn\n"
		   "-G, --traditional          run in compatibility mode\n"
		   "-l, --loose-exit-status    exit with 0 status even if a command fails\n"
		   "-p, --prompt=STRING        use STRING as an interactive prompt\n"
		   "-r, --restricted           run in restricted mode\n"
		   "-s, --quiet, --silent      suppress diagnostics, byte counts and '!' prompt\n"
		   "-v, --verbose              be verbose; equivalent to the 'H' command\n\n"
		   "Commands:\n"
		   "(.,.)                      Print the addressed line(s), and sets the current address to the last line printed.\n"
		   "H                          Toggle the printing of error explanations.\n"
		   "h                          Print an explanation of the last error.\n"
		   "(.,.)n                     Print the addressed lines along with their line numbers.\n"
		   "(.,.)p                     Print the addressed lines. The current address is set to the last line printed.\n"
		   "q                          Quit ed.\n"
		   "Start edit by reading in 'file' if given.\n");
	exit(0);
}

/**
 *	Print version.
 */
void
opt_version()
{
	printf("ed 1.00.0\n"
		   "Copyright (C) 2019 Filip Cizmar.\n"
		   "This is free software: you are free to change and redistribute it.\n"
		   "There is NO WARRANTY, to the extent permitted by law.\n");
	exit(0);
}


void opt_traditional(char *p_options)
{
	printf("Compatibility mode has no effect in current version.\n");
	*p_options |= 1U << options_TRADITIONAL;
}


/**
 *	Function sets appropriate bits of options variable in accordance with the specified command line options.
 *	Also it fills prompt variable if necessary.
 *	Function returns position of file name argument.
 */
int
load_options (char *p_options, char *prompt, char ** argv, const int argc)
{
	int i;
	for (i = 1 ; i < argc; ++i)
		if (argv[i][0] == '-' && argv[i][1] == '-')
			if (!strcmp(argv[i], "--help"))
				opt_help();
			
			else if (!strcmp(argv[i], "--version"))
				opt_version();
			
			else if (!strcmp(argv[i], "--traditional"))
				opt_traditional(p_options);
			
			else if (!strcmp(argv[i], "--loose-exit-status"))
				*p_options |= 1U << options_LOOSE_EXIT_STATUS;
			
			else if (!strncmp(argv[i], "--prompt=", strlen("--prompt=")))
			{
				*p_options |= 1U << options_PROMPT;
				strcpy(prompt, argv[i] + strlen("--prompt="));
			}
			
			else if (!strcmp(argv[i], "--restricted"))
				*p_options |= 1U << options_RESTRICTED;
			
			else if (!strcmp(argv[i], "--quiet") || !strcmp(argv[i], "--silent"))
				*p_options |= 1U << options_QUIET;
			
			else if (!strcmp(argv[i], "--verbose"))
				*p_options |= 1U << options_VERBOSE;
			
			else
			{
				fprintf(stderr, "ed: illegal option -- %s\n", argv[i] + 2);
				fprintf(stderr, "usage: ed file\n");
				exit(1);
			}
		else if (argv[i][0] == '-')
			for (size_t j = 1; j < strlen(argv[i]); ++j)
				if (argv[i][j] == 'h')
					opt_help();
	
				else if (argv[i][j] == 'V')
					opt_version();
	
				else if (argv[i][j] == 'G')
					opt_traditional(p_options);
	
				else if (argv[i][j] == 'l')
					*p_options |= 1U << options_LOOSE_EXIT_STATUS;
	
				else if (argv[i][j] == 'p')
				{
					*p_options |= 1U << options_PROMPT;
					if (++i == argc)
					{
						fprintf(stderr, "ed: illegal option -- p, trailing STRING needed\n");
						fprintf(stderr, "usage: ed file\n");
						exit(1);
					}
					strcpy(prompt, argv[i]);
					break;
				}
	
				else if (argv[i][j] == 'r')
					*p_options |= 1U << options_RESTRICTED;
	
				else if (argv[i][j] == 's')
					*p_options |= 1U << options_QUIET;
	
				else if (argv[i][j] == 'v')
					*p_options |= 1U << options_VERBOSE;
	
				else
				{
					fprintf(stderr, "ed: illegal option -- %s\n", argv[i] + 1);
					fprintf(stderr, "usage: ed file\n");
					exit(1);
				}
		else
			break;
	
	if (i == argc)
	{
		fprintf(stderr, "usage: ed file\n");
		exit(1);
	}
	
	return i;
}

/**
 *	Function prints last error according to seted value.
 */
void print_last_error(enum error last_error)
{
	switch (last_error) {
			
		case error_INVALID_ADDRESS:
			printf("Invalid address\n");
			break;
			
		case error_UNEXPECTED_ADDRESS:
			printf("Unexpected address\n");
			break;
			
		case error_UNKNOWN_COMMAND:
			printf("Unknown command\n");
			break;
			
		case error_INVALID_COMMAND_SUFFIX:
			printf("Invalid command suffix\n");
			break;
			
		case error_NONE:
			break;
			
		default:
			break;
	}
}

void print_error_message(char *options, enum error *last_error)
{
	if (*options & (1U << options_VERBOSE))
	{
		printf("?\n");
		print_last_error(*last_error);
	}
	else
		printf("?\n");
}

/**
 * 	Function processes address properly and execute provided command.
 * 	Also it manipulates with n_lins, actual_line, lines, last_error, options varialbles if command requires.
 */
void
exec_command(char *command, char *address, int *p_n_lines, int *p_actual_line, char **lines, enum error *p_last_error, char *options)
{
	int 	address_start =		*p_actual_line;
	int 	address_end = 		*p_actual_line;
	char	address_provided = 	0;
	
	if (strlen(address) != 0)
	{
		char 	first_part[256] = 	"";
		char 	second_part[256] = 	"";
		size_t	i = 				0;
		char	adress_valid = 		1;
		
		address_provided = 1;
		
		for (i = 0; i < strlen(address); ++i)
			if (address[i] == ',')
				break;
		
		if (i == strlen(address) - 1 || i == 0) // carka bez jedne adresy
		{
			*p_last_error = error_INVALID_ADDRESS;
			print_error_message(options, p_last_error);
			return;
		}
		
		strncpy(first_part, address, i);
		strcpy(second_part, address + i + 1);
		
		if (strlen(first_part) != 0 && strlen(second_part) != 0)
		{
			if (strlen(first_part) == 1 && first_part[0] == '.')
				sprintf(first_part, "%d", *p_actual_line);
			
			if (strlen(first_part) == 1 && first_part[0] == '$')
				sprintf(first_part, "%d", *p_n_lines);
			
			if (strlen(second_part) == 1 && second_part[0] == '.')
				sprintf(second_part, "%d", *p_actual_line);
			
			if (strlen(second_part) == 1 && second_part[0] == '$')
				sprintf(second_part, "%d", *p_n_lines);
			
			i = 0;
			
			if (first_part[0] == '-')
				i = 1;
			
			for (; i < strlen(first_part); ++i)
				if (!isdigit(first_part[i]))
					adress_valid = 0;
			
			i = 0;
			
			if (second_part[0] == '-')
				i = 1;
			
			for (; i < strlen(second_part); ++i)
				if (!isdigit(second_part[i]))
					adress_valid = 0;
			
			address_start = atoi(first_part);
			address_end = atoi(second_part);
			
			if (address_end == 0 || address_start == 0)
				adress_valid = 0;
			
			if (address_start > address_end || address_start * address_end < 0)
				adress_valid = 0;
			
			if (address_start < 0 && address_end < 0)
			{
				address_start +=	*p_n_lines;
				address_end +=		*p_n_lines;
			}
		}
		else if (strlen(first_part) != 0)
		{
			if (strlen(first_part) == 1 && first_part[0] == '.')
				sprintf(first_part, "%d", *p_actual_line);
			
			if (strlen(first_part) == 1 && first_part[0] == '$')
				sprintf(first_part, "%d", *p_n_lines);
			
			i = 0;
			
			if (first_part[0] == '-')
				i = 1;
			
			for (; i < strlen(first_part); ++i)
				if (!isdigit(first_part[i]))
					adress_valid = 0;
			
			address_start = atoi(first_part);
			
			if (address_start == 0)
				adress_valid = 0;
			
			if (address_start < 0)
				address_start += *p_n_lines;
			
			address_end = address_start;
		}
		else
			adress_valid = 0;
		
		if (address_end > *p_n_lines || address_start > *p_n_lines)
			adress_valid = 0;
		
		if (!adress_valid)
		{
			*p_last_error = error_INVALID_ADDRESS;
			print_error_message(options, p_last_error);
			return;
		}
	}
	
	*p_actual_line = address_end;
	char command_suffix = 0;
	
	switch (command[0]) {
			
		case 'p':
			if (strlen(command) > 1)
			{
				command_suffix = 1;
				break;
			}
	
			for (; address_start <= address_end; ++address_start)
				printf("%s", lines[address_start - 1]);
			
			break;
			
			
		case '\n':
			if (strlen(command) > 1)
			{
				command_suffix = 1;
				break;
			}
			
			if (++address_start <= *p_n_lines)
			{
				printf("%s", lines[address_start - 1]);
				++(*p_actual_line);
			}
			else
			{
				*p_last_error = error_INVALID_ADDRESS;
				print_error_message(options, p_last_error);
				return;
			}
			break;
			
			
		case 'H':
			if (address_provided)
			{
				*p_last_error = error_UNEXPECTED_ADDRESS;
				print_error_message(options, p_last_error);
				return;
			}
			
			if (strlen(command) > 1)
			{
				command_suffix = 1;
				break;
			}
			
			*options ^= 1U << options_VERBOSE;
			print_last_error(*p_last_error);
			break;
			
			
		case 'h':
			if (address_provided)
			{
				*p_last_error = error_UNEXPECTED_ADDRESS;
				print_error_message(options, p_last_error);
				return;
			}
			if(strlen(command) > 1)
			{
				command_suffix = 1;
				break;
			}
			
			print_last_error(*p_last_error);
			break;
			
			
		case 'q':
			if (address_provided)
			{
				*p_last_error = error_UNEXPECTED_ADDRESS;
				print_error_message(options, p_last_error);
				return;
			}
			
			if (strlen(command) > 1)
			{
				command_suffix = 1;
				break;
			}
			
			if (*options & (1U << options_LOOSE_EXIT_STATUS) || *p_last_error == error_NONE)
				exit(0);
			
			exit(1);
			break;
			
			
		case 'n':
			if (strlen(command) > 1)
			{
				command_suffix = 1;
				break;
			}
			
			for (; address_start <= address_end; ++address_start)
				printf("%d\t%s", address_start, lines[address_start - 1]);
			
			break;
			
			
		default:
			*p_last_error = error_UNKNOWN_COMMAND;
			print_error_message(options, p_last_error);
			return;
			break;
	}
	
	if (command_suffix)
	{
		*p_last_error = error_INVALID_COMMAND_SUFFIX;
		print_error_message(options, p_last_error);
	}
}

int
main (int argc, char ** argv)
{
	char		options =			0;
	char 		prompt[256] =		"";
	int 		n_lines =			0;
	int 		n_chars =			0;
	char 		**lines =			read_file(argv[ load_options(&options, prompt, argv, argc) ], &n_lines, &n_chars, &options);
	int 		actual_line =		n_lines;
	char 		input[256] = 		"";
	enum error 	last_error = 		error_NONE;
	
	if(lines && !(options & (1U << options_QUIET)))
		printf("%d\n", n_chars);
	
	if(options & (1U << options_PROMPT))
	   printf("%s", prompt);
	
	while (fgets(input, sizeof(input), stdin))
	{
		strtok(input, "\n");
		size_t i;
		
		for (i = 0; i < strlen(input); ++i)
			if ((input[i] > 'a' && input[i] < 'z') || (input[i] > 'A' && input[i] < 'Z') || input[i] == '\n')
				break;
				
		char command[256] = 	"";
		char address[256] = 	"";
		
		strcpy(command, input + i);
		strncpy(address, input, i);
		
		if (strlen(command) == 0)
			command[0] = 'p';
		
		exec_command(command, address, &n_lines, &actual_line, lines, &last_error, &options);
		
		if (options & (1U << options_PROMPT))
			printf("%s", prompt);
	}
	
	if (options & (1U << options_LOOSE_EXIT_STATUS) || last_error == error_NONE)
		return 0;
	
	return 1;
}
