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
	error_INVALID_COMMAND_SUFFIX,
	error_BUFFER_MODIFIED
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
			
		case error_BUFFER_MODIFIED:
			printf("Warning: buffer modified\n");
			break;
			
		case error_NONE:
			break;
			
		default:
			break;
	}
}

static void
print_error_message(int *Hcommand, enum error *last_error)
{
	if (*Hcommand) {
		printf("?\n");
		print_last_error(*last_error);
	}
	else {
		printf("?\n");
	}
}

static void
get_p_act_line(int *p_no_act_line, int *address_start, struct _Line **p_actual_line)
{
	if (*p_no_act_line > *address_start) {
		for (int i = 0; i < *p_no_act_line - *address_start; ++i) {
			*p_actual_line = TAILQ_PREV((*p_actual_line), lines_q, pointers);
		}
	}
	else if (*p_no_act_line < *address_start) {
		for (int i = 0; i < *address_start - *p_no_act_line; ++i) {
			*p_actual_line = TAILQ_NEXT((*p_actual_line), pointers);
		}
	}
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
			 enum error		*p_last_error,
			 int			*p_Hcommand,
			 int 			*p_input_mode,
			 char			*file_name,
			 int			*p_modified)
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
			print_error_message(p_Hcommand, p_last_error);
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
			print_error_message(p_Hcommand, p_last_error);
			return;
		}
	}
	
	char file[256] = 		"";
	char command_suffix = 	0;
	strcpy(file, file_name);
	
	switch (command[0]) {
			
		case 'w':
			if (*p_modified == 2)
				*p_modified = 1;
			
			if (address_provided) {
				*p_last_error = error_UNEXPECTED_ADDRESS;
				print_error_message(p_Hcommand, p_last_error);
				return;
			}
			
			if (strlen(command) > 1) {
				if (command[1] != ' ') {
					command_suffix = 1;
					break;
				}
				else
				{
					memcpy(file, command + 2, strlen(command) - 2);
				}
			}
			
			FILE *fp;
			fp = fopen(file, "w");
			
			if (fp == NULL) {
				fprintf(stderr, "No current filename\n");
				return;
			}
			
			int no_char = 0;
			Line * nl = malloc(sizeof(Line));
			TAILQ_FOREACH(nl, &lines, pointers) {
				no_char += strlen(nl->content);
				fprintf(fp, "%s", nl->content);
			}
			
			fclose(fp);
			printf("%d\n", no_char);
			*p_modified = 0;
			return;
			
			
		case 'd':
			if (*p_modified == 2)
				*p_modified = 1;
			
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			if (*p_no_act_line == 0) {
				*p_last_error = error_INVALID_ADDRESS;
				print_error_message(p_Hcommand, p_last_error);
				return;
			}
			
			get_p_act_line(p_no_act_line, &address_start, p_actual_line);
			*p_no_act_line = address_start;
			
			for (; address_start <= address_end; ++address_start) {
				*p_modified = 1;
				--(*p_n_lines);
				struct _Line *tmp_act_line = TAILQ_NEXT(*p_actual_line, pointers);
				
				if (tmp_act_line == NULL)
					tmp_act_line = TAILQ_PREV(*p_actual_line, lines_q, pointers);
				
				TAILQ_REMOVE(&lines, *p_actual_line, pointers);
				*p_actual_line = tmp_act_line;
			}
			
			if (*p_n_lines < *p_no_act_line)
				--(*p_no_act_line);
			
			break;
			
			
		case 'i':
			if (*p_modified == 2)
				*p_modified = 1;
			
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			if (address_start != address_end) {
				*p_last_error = error_INVALID_ADDRESS;
				print_error_message(p_Hcommand, p_last_error);
				return;
			}
			
			*p_input_mode = 1;
			
			break;
			
			
		case 'p':
			if (*p_modified == 2)
				*p_modified = 1;
			
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			if (*p_no_act_line == 0) {
				*p_last_error = error_INVALID_ADDRESS;
				print_error_message(p_Hcommand, p_last_error);
				return;
			}
	
			get_p_act_line(p_no_act_line, &address_start, p_actual_line);
			
			for (; address_start < address_end; ++address_start) {
				printf("%s", (*p_actual_line)->content);
				*p_actual_line = TAILQ_NEXT((*p_actual_line), pointers);
			}
			printf("%s", (*p_actual_line)->content);
			++address_start;
			*p_no_act_line = address_end;
			
			break;
			
			
		case '\n':
			if (*p_modified == 2)
				*p_modified = 1;
			
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			if (++address_start < *p_n_lines) {
				*p_actual_line = TAILQ_NEXT((*p_actual_line), pointers);
				printf("%s", (*p_actual_line)->content);
				++(address_end);
				*p_no_act_line = address_end;
			}
			else
			{
				*p_last_error = error_INVALID_ADDRESS;
				print_error_message(p_Hcommand, p_last_error);
				return;
			}
			break;
			
			
		case 'H':
			if (*p_modified == 2)
				*p_modified = 1;
			
			if (address_provided) {
				*p_last_error = error_UNEXPECTED_ADDRESS;
				print_error_message(p_Hcommand, p_last_error);
				return;
			}
			
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			if (!*p_Hcommand) {
				*p_Hcommand = 1;
				print_last_error(*p_last_error);
			}
			else
			{
				*p_Hcommand = 0;
			}
			
			break;
			
			
		case 'h':
			if (*p_modified == 2)
				*p_modified = 1;
			
			if (address_provided) {
				*p_last_error = error_UNEXPECTED_ADDRESS;
				print_error_message(p_Hcommand, p_last_error);
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
				print_error_message(p_Hcommand, p_last_error);
				return;
			}
			
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			if (*p_modified == 1) {
				*p_modified = 2;
				*p_last_error = error_BUFFER_MODIFIED;
				print_error_message(p_Hcommand, p_last_error);
				return;
			}
			
			if (*p_last_error == error_NONE)
				exit(0);
			else
				exit(1);
			
			break;
			
			
		case 'n':
			if (*p_modified == 2)
				*p_modified = 1;
			
			if (strlen(command) > 1) {
				command_suffix = 1;
				break;
			}
			
			if (*p_no_act_line == 0)
			{
				*p_last_error = error_INVALID_ADDRESS;
				print_error_message(p_Hcommand, p_last_error);
				return;
			}
			
			get_p_act_line(p_no_act_line, &address_start, p_actual_line);
			
			for (; address_start < address_end; ++address_start) {
				printf("%d\t%s", address_start, (*p_actual_line)->content);
				*p_actual_line = TAILQ_NEXT((*p_actual_line), pointers);
			}
			printf("%d\t%s", address_start, (*p_actual_line)->content);
			++address_start;
			*p_no_act_line = address_end;
			
			break;
			
			
		default:
			*p_last_error = error_UNKNOWN_COMMAND;
			print_error_message(p_Hcommand, p_last_error);
			return;
			break;
	}
	
	if (command_suffix) {
		*p_last_error = error_INVALID_COMMAND_SUFFIX;
		print_error_message(p_Hcommand, p_last_error);
	}
}

int
main (int argc, char ** argv)
{
	int		modified = 		0;
	int 	n_lines =		0;
	int 	n_chars =		0;
	char 	input[1024] = 	"";
	int		Hcommand = 		0;
	int		input_mode = 	0;
	char	file_name[254] = 	"";
	TAILQ_INIT(&lines);
	if (argc == 2) {
		read_file(argv[1], &n_lines, &n_chars);
		strcpy(file_name, argv[1]);
	}
	
	enum error 		last_error = 	error_NONE;
	struct _Line *	actual_line =	TAILQ_LAST(&lines, lines_q);
	int				no_act_line =	n_lines;
	
	if (!TAILQ_EMPTY(&lines))
		printf("%d\n", n_chars);
	
	while (fgets(input, sizeof(input), stdin)) {
		strtok(input, "\n");
		if (input_mode == 0) {
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
			
			if (input[0] == 'w')
				strcpy(command, input);
			
			exec_command(command, address, &n_lines, &actual_line, &no_act_line,
						 &last_error, &Hcommand, &input_mode, file_name, &modified);
			
		}
		else
		{
			input_mode = 0;
		}
		
		if (input_mode) {
			while (fgets(input, sizeof(input), stdin)) {
				modified = 1;
				Line * nl = malloc(sizeof(Line));
				strcpy(nl->content, input);
				if (actual_line == NULL)
					TAILQ_INSERT_TAIL(&lines, nl, pointers);
				else
					TAILQ_INSERT_BEFORE(actual_line, nl, pointers);
				++n_lines;
				++no_act_line;
			}
			input_mode = 2;
			clearerr(stdin);
		}
	}

	
	if (last_error == error_NONE) {
		return 0;
	}
	
	return 1;
}

// udelat w
