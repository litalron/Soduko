/*
 * Parser.c
 *
 *  Created on: 23 בספט 2018
 *      Author: ליטל
 */

#include "Parser.h"
#include "Solver.h"

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gurobi.h"
#include "gurobi_c.h"
#include <assert.h>
#include "SPBufferset.h"
#include "Game.h"
#define solve 0
#define edit 1
#define init 2
#define backtrack 3
#define forwardtrack 7


/*
 * the program prints invalid command
 * when there is an invalid execution
 */
void invalid_command(){
	printf("ERROR: invalid command\r\n");
}

/*
 * sets the "mark_errors" to x.
 * x can only be 1/0.
 */
void mark_errors(int x){
	 if (mode != solve){
		 invalid_command();
		 return;
	 }
	 if (x!=0 && x!=1){
		 printf("Error: the value should be 0 or 1\r\n");
	 }
	 else{
	 mark = x;
	 }
}

int gameOn(){
int i,j;
char str[256];
char * token;
int array2 [2]; /* array2 stands for the arguments in the command hint X Y. for example: if the user writes 'hint 4 5' then array2 - {4,5} */
int array3 [3]; /* array3 stands for the arguments in the command set X Y Z. for example: if the user writes 'set 4 5 5' then array3 - {4,5,5} */
int valid = 1; /* valid indicates if the input in commands 'hint', 'set' is valid */
char c;
/*  int solved = 0; solved indicates if the board is solved */
/* probably not relevant */
SP_BUFF_SET() /* ; */
printf("Sudoku\r\n------\r\n");
mode=init;
while(1){
	printf("Enter your command:\r\n");
	valid = 1;
	if(!fgets(str,256,stdin)){ /* we've reached EOF */
		exiting();
		return 0;
	}
	token = strtok(str, " \t\r\r\n");
	if (token == NULL){
		continue;
	}
	if ( !strcmp(token,"hint")){ /* the command hint is relevant only when the puzzle is not solved */
		for (i=0; i<2; i++){
			token = strtok(NULL," \t\r\r\n");
			if(token == NULL){
				invalid_command();
				valid = 0;
				break;
			}
			c=0;
			for(j=0; j<strlen(token); j++){
				c = c * 10 + ( token[j] - '0' );
			}
			array2[i]=c;
		}
		if (valid){
			hint(array2[0]-1,array2[1]-1);
		}
	}
	else if ( !strcmp(token,"set")){ /* the command set is relevant only when the puzzle is not solved */
		for (i=0; i<3; i++){
					token = strtok(NULL," \t\r\r\n");
					if(token == NULL){
						invalid_command();
						valid = 0;
						break;
					}
					else{
						c=0;
						for(j=0; j<strlen(token); j++){
							c = c * 10 + ( token[j] - '0' );
						}
						array3[i]=c;
					}
				}
				if (valid){
							if(set(array3[0]-1,array3[1]-1, array3[2])){
								printboard();
								if (v==0 && mode ==solve){
									if (has_error()){
										printf("Puzzle solution erroneous\r\n");
									}
									else{
										printf("Puzzle solved successfully\r\n");
										free_list();
										free_matrixes();
										gameOn();

									}
								}
							}
						}
			}
	else if (!strcmp(token,"validate")){ /* the command validate is relevant only when the puzzle is not solved */
		validate(1);
	}
	else if (!strcmp(token,"reset")){
		reset();
	}
	else if (!strcmp(token,"exit")){

		exiting();
		break;
	}
	else if (!strcmp(token,"solve")){
		token = strtok(NULL," \t\r\r\n");
		solving(token);
	}

	else if (!strcmp(token,"edit")){
		token = strtok(NULL," \t\r\r\n");
		editing(token);
	}

	else if (!strcmp(token,"mark_errors")){
			token = strtok(NULL," \t\r\r\n");
			if(token == NULL){
				invalid_command();
				valid = 0;
			}
			else{
				c = *token-'0';
			}
			if (valid){
				mark_errors(c);
			}
		}
	else if (!strcmp(token,"print_board")){
		printboard();
	}
	else if (!strcmp(token,"generate")){
		for (i=0; i<2; i++){
			token = strtok(NULL," \t\r\r\n");
			if(token == NULL){
				invalid_command();
				valid = 0;
				break;
			}
			c=0;
			for(j=0; j<strlen(token); j++){
				c = c * 10 + ( token[j] - '0' );
				}
			array2[i]=c;
		}
		if (valid){
			generating(array2[0],array2[1]);
		}
	}
	else if (!strcmp(token,"undo")){
		new_undo(1);
		}
	else if (!strcmp(token,"redo")){
		new_redo();
		}
	else if (!strcmp(token,"save")){
				token = strtok(NULL," \t\r\r\n");
				if(token == NULL){
					invalid_command();
					valid = 0;
				}
				if (valid){
					save(token); /* not sure*/
				}
	}
	else if (!strcmp(token,"num_solutions")){
		num_solutions();
	}
	else if (!strcmp(token,"autofill")){
			autofill();
		}
	else{
		invalid_command();
	}
}
return 1;

}
