#include <string.h>
#include <stdio.h>
int
main (int argc, char ** argv)
{
	char input[256];
	char *fgets_ret = fgets(input, sizeof(input), stdin);
	while (fgets_ret != NULL) {
		printf("%s\n", input);
		fgets_ret = fgets(input, sizeof(input), stdin);
	}
}
