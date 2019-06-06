#include <sys/queue.h>
#include <stdio.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * Enum contains all errors aliases.
 */ 
enum error {
	error_NONE,
	error_INVALID_ADDRESS,
	error_UNEXPECTED_ADDRESS,
	error_UNKNOWN_COMMAND,
	error_INVALID_COMMAND_SUFFIX
};

typedef struct _Line {
	TAILQ_ENTRY(_Line) pointers;
	char content[1024];
} Line;

TAILQ_HEAD(lines_q, _Line) lines;


/**
 * Function reads file line by line and stores each line as element of returned
 * array.
 * The lines array resize automaticly if it is filled while reading file.
 * Also it stores numbers of lines into n_lines, numbers of characters into
 * n_chars.
 */ 
static void
read_file (const char *arg, int *p_n_lines, int *p_n_chars)
{
	TAILQ_INIT(&lines);
	FILE *fp;
	fp = fopen(arg, "r");
	
	if (fp == NULL) {
		perror(arg);
		return;
	}
	
	char line[1024];
	
	while (fgets ( line, sizeof(line), fp ) != NULL) {
		Line * nl = malloc(sizeof(Line));
		strcpy(nl->content, line);
		TAILQ_INSERT_TAIL(&lines, nl, pointers);
		*p_n_chars += strlen(line);
		++*p_n_lines;
	}
	
	fclose(fp);
	return;
}


/**
 * Function prints last error according to seted value.
 */
static void
print_last_error(enum error last_error)
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

static void
print_error_message()
{
	printf("?\n");
}


/**
 * Function processes address properly and execute provided command.
 * Also it manipulates with n_lins, actual_line, lines, last_error, options
 * varialbles if command requires.
 */
static void
exec_command(
			 char			*command,
			 char			*address,
			 int			*p_n_lines,
			 struct _Line 	**p_actual_line,
			 int			*p_no_act_line,
			 enum error		*p_last_error)
{
	int 	address_start =		*p_no_act_line;
	int 	address_end = 		*p_no_act_line;
	char	address_provided = 	0;
	
	if (strlen(address) != 0) {
		char 	first_part[256] = 	"";
		char 	second_part[256] = 	"";
		size_t	i = 				0;
		char	adress_valid = 		1;
		
		address_provided = 1;
		
		for (i = 0; i < strlen(address); ++i)
			if (address[i] == ',')
				break;
		
		/**
		 * If missing one field of address
		 */
		if (i == strlen(address) - 1 || i == 0) {
			*p_last_error = error_INVALID_ADDRESS;
			print_error_message();
			return;
		}
		
		strncpy(first_part, address, i);
		strcpy(second_part, address + i + 1);
		
		/*
		 * Both address fields specified
		 */
		if (strlen(first_part) != 0 && strlen(second_part) != 0) {
			if (strlen(first_part) == 1 && first_part[0] == '.')
				sprintf(first_part, "%d", *p_no_act_line);
			
			if (strlen(first_part) == 1 && first_part[0] == '$')
				sprintf(first_part, "%d", *p_n_lines);
			
			if (strlen(second_part) == 1 && second_part[0] == '.')
				sprintf(second_part, "%d", *p_no_act_line);
			
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
			
			if (address_start < 0 && address_end < 0) {
				address_start +=	*p_no_act_line;
				address_end +=		*p_no_act_line;
			}
		}
		
		/*
		 * Only one address field specified
		 */
		else if (strlen(first_part) != 0) {
			if (strlen(first_part) == 1 && first_part[0] == '.')
				sprintf(first_part, "%d", *p_no_act_line);
			
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
				address_start += *p_no_act_line;
			
			address_end = address_start;
		}
		else
			adress_valid = 0;
		
		if (address_end > *p_n_lines ||	address_start > *p_n_lines ||
			address_start <= 0 || address_end <= 0)
			adress_valid = 0;
		
		if (!adress_valid) {
			*p_last_error = error_INVALID_ADDRESS;
			print_error_message();
			return;
		}
	}
	
	char command_suffix = 0;
	
	if (*p_no_act_line > address_start) {
		for (int i = 0; i < *p_no_act_line - address_start; ++i) {
			*p_actual_line = TAILQ_PREV((*p_actual_line), lines_q, pointers);
		}
	}
	else if (*p_no_act_line < address_start) {
		for (int i = 0; i < address_start - *p_no_act_line; ++i) {
			*p_actual_line = TAILQ_NEXT((*p_actual_line), pointers);
		}
	}
	
	switch (command[0]) {
		case 'p':
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			if (*p_no_act_line == 0)
			{
				*p_last_error = error_INVALID_ADDRESS;
				print_error_message();
				return;
			}
	
			for (; address_start < address_end; ++address_start) {
				printf("%s", (*p_actual_line)->content);
				*p_actual_line = TAILQ_NEXT((*p_actual_line), pointers);
			}
			printf("%s", (*p_actual_line)->content);
			++address_start;
			
			break;
			
			
		case '\n':
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			if (++address_start < *p_n_lines) {
				*p_actual_line = TAILQ_NEXT((*p_actual_line), pointers);
				printf("%s", (*p_actual_line)->content);
				++(address_end);
			}
			else
			{
				*p_last_error = error_INVALID_ADDRESS;
				print_error_message();
				return;
			}
			break;
			
			
		case 'H':
			if (address_provided) {
				*p_last_error = error_UNEXPECTED_ADDRESS;
				print_error_message();
				return;
			}
			
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			print_last_error(*p_last_error);
			break;
			
			
		case 'h':
			if (address_provided) {
				*p_last_error = error_UNEXPECTED_ADDRESS;
				print_error_message();
				return;
			}
			
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			print_last_error(*p_last_error);
			break;
			
			
		case 'q':
			if (address_provided) {
				*p_last_error = error_UNEXPECTED_ADDRESS;
				print_error_message();
				return;
			}
			
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			exit(1);
			break;
			
			
		case 'n':
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			if (*p_no_act_line == 0)
			{
				*p_last_error = error_INVALID_ADDRESS;
				print_error_message();
				return;
			}
			
			for (; address_start < address_end; ++address_start) {
				printf("%d\t%s", address_start, (*p_actual_line)->content);
				*p_actual_line = TAILQ_NEXT((*p_actual_line), pointers);
			}
			printf("%s", (*p_actual_line)->content);
			++address_start;
			
			break;
			
			
		default:
			*p_last_error = error_UNKNOWN_COMMAND;
			print_error_message();
			return;
			break;
	}
	
	*p_no_act_line = address_end;
	
	if (command_suffix) {
		*p_last_error = error_INVALID_COMMAND_SUFFIX;
		print_error_message();
	}
}

int
main (int argc, char ** argv)
{
	int 	n_lines =		0;
	int 	n_chars =		0;
	char 	input[256] = 	"";
	
	read_file(argv[1], &n_lines, &n_chars);
	
	enum error 		last_error = 	error_NONE;
	struct _Line *	actual_line =	TAILQ_LAST(&lines, lines_q);
	int				no_act_line =	n_lines;
	
	if (!TAILQ_EMPTY(&lines))
		printf("%d\n", n_chars);
	
	while (fgets(input, sizeof(input), stdin)) {
		strtok(input, "\n");
		size_t i;
		
		for (i = 0; i < strlen(input); ++i)
			if ((input[i] > 'a' && input[i] < 'z') ||
				(input[i] > 'A' && input[i] < 'Z') || input[i] == '\n')
				break;
				
		char command[256] = 	"";
		char address[256] = 	"";
		
		strcpy(command, input + i);
		strncpy(address, input, i);
		
		if (strlen(command) == 0)
			command[0] = 'p';
		
		exec_command(command, address, &n_lines, &actual_line, &no_act_line, &last_error);
	}
	if (last_error == error_NONE) {
		return 0;
	}
	return 1;
}
