/*
 *	create a list:		C
 *	insert an item:		I<nnn>
 *	remove an item:		R
 *	print a list:		P
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

/*
 *	List element structure
 */
struct item{
	int            n;
	struct item    *next;
};


/*
 *	Insert element to end of the list.
 *	Change the main scope variable tail.
 */
void
insert (struct item **tail, const int n)
{
	struct item *p = malloc(sizeof(struct item));

	if (*tail != NULL)
	{
		(*tail)->next =		p;
		*tail =			(*tail)->next;
	} else {
		*tail =			p;
	}

	(*tail)->n =			n;
	(*tail)->next =			NULL;
	return;
}


/*
 *	Remove item from the begging of the list.
 *	Change the main scope variable head.
 */
void
remove_item (struct item **head)
{
	struct item *p =		*head;
	*head =				(*head)->next;
	free(p);
	return;
}


/*
 *	Print the list from begging to end.
 */
void
print_list (struct item *head)
{
	printf("List:");

	if (head == NULL)
	{
		printf(" EMPTY\n");
		return;
	}

	while (head != NULL)
	{
		printf(" %d", head->n);
		head =			head->next;
	}
	printf("\n");
}


int
main (int argc, char ** argv)
{
	struct item *head =		NULL;
	struct item *tail =		NULL;
	char initialized = 0;

	for (int i = 1; i < argc; ++i)
	{
		switch(argv[i][0])
		{
			/*
			 *	Create list command.
			 */
			case 'C':
				if (strlen(argv[i]) != 1)
					goto wrong_command;

				if (initialized)
					err(1, "List already created, exiting");

				tail =		head;
				initialized = 1;
				break;

			/*
			 *	Insert to list command.
			 */
			case 'I':
				if (!initialized)
					err(1, "List not created yet.");

				if (strlen(argv[i]) == 1)
					err(1, "Nothing to insert: %s", argv[i]);

				int n =		atoi(argv[i] + 1);
				insert(&tail, n);
				if (head == NULL)
					head =	tail;
				break;

			/*
			 *	Remove element command.
			 */
			case 'R':
				if (strlen(argv[i]) != 1)
					goto wrong_command;

				if (!initialized)
					err(1, "List not created yet.");

				if (head == NULL)
					err(1, "Cannot remove an item from an empty list.");

				remove_item(&head);
				if (head == NULL)
					tail =	NULL;
				break;

			/*
			 *	Print list command.
			 */
			case 'P':
				if (strlen(argv[i]) != 1)
					goto wrong_command;
				if (!initialized)
					err(1, "List not created yet.");

				print_list(head);
				break;


			default:
				goto wrong_command;
		}

		continue;

		wrong_command:
			err(1, "Wrong command: %s", argv[i]);
	}
}
