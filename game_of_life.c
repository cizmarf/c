#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define WIDTH 380
#define TICKS 500


void
print_life(char * life)
{
	printf("\n");	// change to \r
	for (int i = 0; i < WIDTH; ++i)
	{
		if(life[i] == 0)
			printf(" ");
		else
			printf("X");
	}
	fflush(stdout);
}

int
rule(char a, char b, char c)
{
	return a ^ (b | c);	// rule of 30
	//return a ^ c ; 	// rule of 90
	//return (a | b) ^ (a & b & c); 	// rule of 110
}

int
main(void)
{
	char life[WIDTH] = {0}, temp_life[WIDTH] = {0};
	life[WIDTH / 2] = 1;
	//life[0] = 1;
	print_life(life);
	
	for (int i = 0; i < TICKS; ++i)
	{
		temp_life[0] = rule(0, life[0], life[1]);
		
		for (int j = 1; j < WIDTH - 1; ++j)
			temp_life[j] = rule(life[j - 1], life[j], life[j + 1]);
		
		temp_life[WIDTH - 1] = rule(life[WIDTH - 2], life[WIDTH - 1], 0);
		memcpy(&life, &temp_life, WIDTH);
		print_life(life);
		//sleep(1);
	}
}
