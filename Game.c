/*
 * Game.c
 *
 *  Created on: 23 בספט 2018
 *      Author: ליטל
 */

#include "Game.h"
#include "Parser.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gurobi.h"
#include "Solver.h"
#include "gurobi_c.h"
#include <assert.h>
#include "SPBufferset.h"
#define solve 0
#define edit 1
#define init 2
#define backtrack 3
#define forwardtrack 7

int a, ac, b, c_n,v;
int mark = 1; /* if the user in mode mark errors*/
int m ;
int n ;
int N; /* = m*n */
int mode; /*  0=mode solve;1=edit;2=init*/
list* lst; /* initialize*/
int** sudoku;
int** fixed;
int** error ;
FILE *f;
int *** validNumbers;
double ** solution;
int ** a_f;


/*
*check the error matrix
*and return 1 when there is at least one error in the puzzle.
*else return 0;
*/
int has_error(){
	 int i,j;
	 for(i=0;i<N; i++){
		 for(j=0; j< N; j++){
			 if (error[i][j] == 1){
				 return 1;
			 }
		 }
	 }
	 return 0;
}


/*
 * validate the current puzzle using ILP to ensure it is solvable.
 * the command is only available in Edit and Solve modes.
 * if the puzzle is validate the func returns 1, else 0.
 */
int validate(int flag){
	int i,j;
	 if( !(mode == edit || mode == solve)){
		 invalid_command();
		 return 0;
	 }
	 if (has_error()){
		printf("Error: board contains erroneous values\r\n");
		 return 0;
	 }
	for(i = 0; i< N; i++){
		for(j=0; j<N; j++){
			solution[i][j] = sudoku[i][j];
		}
	}
	i=ILP(solution);
	if(i){
		if (flag){
			printf("Validation passed: board is solvable\r\n");
			return 1;
		}
	}
	else{
		if (flag){
			printf("Validation failed: board is unsolvable\r\n");
			return 0;
		}
	}
	return i;
 }

/*
 * check if there is an error with cell<i,j>=Z, value-Z.
 * the func compares the value of the cell (Z) to the cells in column-i
 *  and the cells in row-i and in its block.
 *  if there is another cell with Z value the func returns 1.
 *  else return 0.
 */
int check_error(int i, int j, int**board){
	 int k, l;
	 int blockRow = (i/m), blockColumn = (j/n);
	 if (0== board[i][j]){
		 return 0;
	 }
	 for(k=0; k<N; k++){/*check error in row*/
	 	if (j!=k && board[i][k] == board[i][j]){
	 		return 1;
	 	}
	 }
	 for(k=0; k<N; k++){/*check error in column*/
	 	 if (i!=k && board[k][j] == board[i][j]){
	 	 	return 1;
	 	 }
	 }
	 	 for(k=(blockRow*m); k<(m+(blockRow*m)); k++){/*check error in block*/
	 		 for(l=blockColumn*n; l<n+(blockColumn*n); l++){
	 			 if (!((k==i) && (l==j)) && board[k][l] == board[i][j]){
	 				 return 1;
	 			 }
	 		 }
	 	 }
	 	 return 0;
}

/*
 * sets cells which has invalid value to 1 in the error matrix.
 */
void update_matrix_errors(){
    int num,i,j,k;
     for (i = 0; i < N; i++){
    	 for (j = 0; j < N; j++){
    		 if (check_error(i, j, sudoku))
    			 error[i][j]=1;
    		 else {
    			 error[i][j]=0;
    		 }
    	 }
     }
}


/*
 *  the procedure prints the current puzzle.
 *  the cells which values !=0 being printed as " ".
 *  if the cell is fixed it prints with "."
 *  if the cell has error and also the "mark_errors"==1 it being printed with "*"
 *  the command is only available in Edit and Solve modes.
 */
void printboard(){
     int num,i,j,k;
     if (!(mode==edit || mode == solve)){
    	 invalid_command();
    	 return;
     }
     for (i = 0; i < N; i++){
       if(i%m ==0){
    	  for (k = 0; k < 4*N+m+1; k++){
    		   printf("-");
    	   }
     	  printf("\r\n");
       }
       for (j = 0; j < N; j++){
     	 if(j%n == 0)
     		 printf("|");
     	 num = sudoku[i][j];
     	 printf(" ");
     	 if(num != 0){
     		  printf("%2d", num);
     	 }
     	 else{
     		 printf("  ");
     	 }
     	 if(fixed[i][j] && mode==solve){
     	      printf(".");
     	  }
     	 else if(error[i][j] &&(mode == edit || mark)){ /*mark errors*/
     		  printf("*");
     	 }
     	 else{
     		printf(" ");
     	 }
       }
       printf("|\r\n");
     }
     for (k = 0; k < 4*N+m+1; k++){
    	 printf("-");
     }
     printf("\r\n");
 }


/*
 * solving- loading a puzzle from the file with the path argument.
 * also update mode= solve.
 * after solving command any unsaved work is lost.
 */
void solving (char* path ){
	 if (path==NULL){
		 printf("Error: File doesn't exist or cannot be opened\r\n");
		 return;
	 }
	 f = fopen(path, "r");
	 if (f== NULL){
		 printf("Error: File doesn't exist or cannot be opened\r\n");
		 return;
	 }
	 if(mode!=init){/* free the list and matrixes*/
		 free_list();
		 free_matrixes();
	 }
	 mode = solve;
	 read(path);
	 printboard();
	 update_matrix_errors();
}


/*
 *  editing- loading a puzzle from the file with the path argument.
  * if no parameter is supply the program has an empty board (n=3,m=3).
  * update the mode=edit and mark_errors=1.
  * after editing command any unsaved work is lost.
  */
 void editing(char* path ){
	 if (path == NULL){
		 if (mode !=init){
			 free_list();
			 free_matrixes();
		 }
		 N=2, n=1, m=2;
		 initialize();
		 mode = edit;
		 mark = 1;
		 printboard();
		 return;
	 }
	f = fopen(path, "r");
	if (f== NULL){
			 printf("Error: File cannot be opened\r\n");
			 return;
		 }
	if (mode !=init){
		free_list();
		free_matrixes();
	}
	mode = edit;
	read(path);
	update_matrix_errors();
	mark = 1;
	printboard();
 }


 /*
  *  set z to cell <x,y> .
  * Clear any move beyond the current move from the undo/redo list
  * and add the new move to the list.
  * if its the last cell to fill and mode ==solve, check validation.
  * the command is only available in Edit and Solve modes.
 */
 int set (int y, int x, int z){
 	 node* n;
 	 int i,j;
 	 v=0;
 	 if (mode != edit && mode!=solve){
 		 invalid_command();
 		 return 0;
 	 }
 	 if ((x>N) || (x<0)|| (y>N) || (y<0)){ /*indexes */
 		 printf("Error: value not in range 0-%d\r\n",N);
 		 return 0;
 	 }
 	 if((z>N) || (z<0)){
 		 printf("Error: value not in range 0-%d\r\n",N);
 		 return 0;
 	 }
 	 if (fixed[x][y] == 1 && mode==solve){
 		 printf("Error: cell is fixed\r\n");
 		 		 return 0;
 	 }
 	if (sudoku[x][y]==z){
 		 return 1;
 	 }
 	 n = (node*)malloc(sizeof(node));
 	 assert(n!= NULL);
 	 n->n_a=1;
 	 if (a==1){
 		 n->n_a=c_n;
 	 }
 	 n->i = x;
 	 n->j = y;
 	 n->old_val = sudoku[x][y];
 	 n->val = z;
 	 n->next = NULL;
 	 n->prev = lst->pointer;
 	 if (lst->pointer != lst->first){
 		 if(lst->pointer->next != NULL)
 			 free_list_rec(lst->pointer->next);
 		 lst->pointer->next = n;
 		 lst->pointer = n;
 		 lst->pointer->next = NULL;
 		 lst->last = n;
 	 }
 	 else{
 		 lst->pointer->next=n;
 		 lst->last = n;
 		 lst->pointer = n;
 		 lst->pointer->next = NULL;
 	 }
 	 sudoku[x][y] = z;
 	 update_matrix_errors();
 	 if(mode==edit && fixed[x][y]==1){
 		 fixed[x][y]=0;
 	 }
 	 if (mode==solve){
 		 for(i=0;i<N;i++){
 			 for (j=0;j<N;j++){
 				 if (sudoku[i][j]==0){
 					 v++;
 				 }
 			 }
 		 }
 	 }
 	 return 1;
  }


 /*
  * Give a hint to the user by showing the solution of a single cell X,Y.
  */
 void hint (int y, int x){
	 if (mode != solve){
		 invalid_command();
		 return;
	 }
 	 if ((x>N) || (x<0)|| (y>N) || (y<0)){
 		 printf("Error: value not in range 1-%d\r\n",N);
 		 return;
 	 }
 	if (has_error()){
 		printf( "Error: board contains erroneous values\r\n");
 		return;
 	}
 	if (fixed[x][y] == 1 ){
 		printf("Error: cell is fixed\r\n");
 		return;
 	}
 	if (sudoku[x][y] != 0){
 	 		printf("Error: cell already contains a value\r\n");
 	 		return;
 	 }
 	 if(!ILP(solution)){
 		 printf("Error: board is unsolvable\r\n");
 	 }
 	 else{
 		 printf("Hint: set cell to %d\r\n", (int)solution[x][y]);
 	 }
 }


 /*
  * Saves the current game board to the specified file.
  * returns 1 if the board saved successfully, else returns 0.
  */
  int save_to_file(char*path){
  	int i,j;
  	f = fopen(path, "w");
  	if (f == NULL){
  	    return 0;
  	}
  	if (mode==edit){
  		set_all_cells_to_fixed();
  	}
  	fprintf(f, "%d ", m);
  	fprintf(f, "%d\r\n", n);
  	for(i=0; i< N; i++){
  		for(j=0;j<N;j++){
  			fprintf(f,"%d", sudoku[i][j]);
  			if (fixed[i][j] == 1){
  				fprintf(f, ".");
  			}
  			fprintf(f," ");
  		}
  		fprintf(f, "\r\n");
  	}
  	fclose(f);
  	return 1;
  }


  /*
   * Saves the current game board to the specified file, where X includes a full or
   * relative path to the file.
   * This command is only available in Edit and Solve modes.
   */
  void save(char* path){
 	 int succeeded = 0;
 	 if (mode != solve && mode!= edit){
 		 invalid_command();
 		 return;
 	 }
 	 if (mode == edit){
 		 if (has_error()){
 			 printf( "Error: board contains erroneous values\r\n");
 			 return;
 		 }
 		 else{
 			 /*validate before saving*/
 			 if (validate(0)){
 				 succeeded = save_to_file(path); /*might be different*/
 				 /*if(succeeded){
 					 set_all_cells_to_fixed();
 				 }*/
 			 }
 			 else{
 				 printf("Error: board validation failed\r\n");
 				 return;
 			 }
 		 }
 	 }
 	 if (mode == solve){
 		 succeeded = save_to_file(path);
 	 }
 	 if (!succeeded){
 		 printf("Error: File cannot be created or modified\r\n");
 		 return;
 	 }
 	 printf("Saved to: %s\r\n", path);
 	 return;
  }
  /*
   * prints the undo step.
   */
  void print_undo(){
 		int x,y, z;
 		if (lst->pointer != lst->first){
 			lst->pointer = lst->pointer->prev;
 			x = lst->pointer->next->i;
 			y = lst->pointer->next->j;
 			z = lst->pointer->next->old_val;
 			printf("Undo %d,%d: from ",y+1,x+1);
 			if (lst->pointer->next->val==0)
 				printf("_");
 			else
 				printf("%d",lst->pointer->next->val);
 			if (z==0)
 				printf(" to _\r\n");
 			else
 				printf(" to %d\r\n",z);
 		}
 	 }


  /*
   * Set the current move pointer to the previous move and update the board
   * accordingly.
   */

   void undo(){
  	int x,y, z, b = lst->pointer != lst->first;
  	if (b){
  		lst->pointer = (lst->pointer)->prev;
  		x = lst->pointer->next->i;
  		y = lst->pointer->next->j;
  		z = lst->pointer->next->old_val;
  		sudoku[x][y] = z;

  	}

   }


   /*
    * Undo previous moves done by the user.
    * Set the current move pointer to the previous move and update the board
    * accordingly. This does not add or remove any item to/from the list.
    * This command is only available in Edit and Solve modes.
    * print the board and the undo moves if flag ==1 .
    */

    void new_undo(int flag){
   	 int i,j, cnt=0;
   	 if (mode != 0 && mode!=1){
   		 invalid_command();
   		 return;
   	 }
   	 if(lst->pointer== lst->first){
   		 printf("Error: no moves to undo\r\n");
   		 return;
   	 }
   	 cnt = lst->pointer->n_a;
   	 for(i=0;i<cnt;i++){
   		 undo();
   	 }
   	 update_matrix_errors();
   	 if (flag){
   		 printboard();
   		 for(i=0; i<cnt;i++){
   			 lst->pointer=lst->pointer->next;
   		 }
   		 for(i=0;i<cnt ;i++){
   			 print_undo(lst->pointer);
   		 }
   	 }
    }


    /*
     * Redo a move previously undone by the user.
     * Set the current move pointer to the next move and update the board
     * accordingly. This does not add or remove any item from the list.
     * This command is only available in Edit and Solve modes.
     */
     void new_redo(){
    	 int cnt =0,i=0, b=0;
    	 	if (mode != 0 && mode!=1){
    	 		invalid_command();
    	 		return;
    	 		}
    	 	if(lst->pointer->next==NULL){
    	 		printf("Error: no moves to redo\r\n");
    	 		return;
    	 	}
    	 	 cnt = lst->pointer->next->n_a;
    	 	 for(i=0;i<cnt;i++){
    	 		 redo();
    	 	 }
    	 	 update_matrix_errors();
    	 	 printboard();
    	 	 for(i=0; i<cnt;i++){
    	 		 lst->pointer=lst->pointer->prev;
    	 	 }
    	 	 for(i=0;i<cnt ;i++){
    	 		 redo_print(lst->pointer);
    	 	 }
     }
     /*
      * Set the current move pointer to the next move and update the board
      * accordingly.
      */
     int redo(){
     	 int x,y, z;
    		lst->pointer = (lst->pointer)->next;
    		x = lst->pointer->i;
    		y = lst->pointer->j;
    		z = lst->pointer->val;
    		sudoku[x][y] = z;
    		return 1;
    	}
     /*
      * prints the redo move.
      */
     void redo_print(){
     	 int x,y, z;
     	lst->pointer=lst->pointer->next;
    		x = lst->pointer->i;
    		y = lst->pointer->j;
    		z = lst->pointer->val;
    		printf("Redo %d,%d: from ",y+1,x+1);
    		if (lst->pointer->old_val ==0)
    			printf("_");
    		else
    			printf("%d",lst->pointer->old_val);
    		if (z==0)
    			printf(" to _\r\n");
    		else
    			printf(" to %d\r\n",z);

    		return;

     }

     /*
      * the func reads the puzzle in the file "P"
      * and saves it in the memory of the game.
      */
     void read(char * p){
     	int i,j, len, x, y,num = 0,fix, cnt=0;
     	char c;
     	if (f == NULL){
     	       printf("Error: File doesn’t exist or cannot be opened\r\n");
     	       return;
     	 }
     	if (fscanf(f,"%d", &m)){
     		 i =0;
     	 }
     	 if (fscanf(f,"%d", &n)){
     		 i =0;
     	 }
     	 N = m * n;
     	 initialize();
     	 for(i=0;i<N;i++){
     		for(j=0;j<N+1;j++){
     			if (j==N){
     				if(ftell(f)!=feof(f)){
     					c=fgetc(f);
     					if(c=='.'){
     						fixed[i][j-1]=1;
     				}
     					continue;
     			}
     		}
     			fixed[i][j]=0;
     			if(ftell(f)!=feof(f)){
     				c=fgetc(f);
     				if(c=='.'){
     					fixed[i][j-1]=1;
     				}else
     					fseek(f,-1,SEEK_CUR);
     			}
     			if(fixed[i][j]==1)
     				fseek(f,1,SEEK_CUR);
     			fscanf(f,"%d",&sudoku[i][j]);
     		}
     	}
     	fclose(f);
     }
     /*
      * Generates a puzzle by randomly filling X cells with random legal values,
      * running ILP to solve the resulting board, and then clearing all but Y random
      * cells.
      * This command is only available in Edit mode.
      */
      void generating(int x, int y){
     	 int i ,j,k,b=0;
     	 if (mode!=edit){
     		 invalid_command();
     		 return;
     	 }
     	 if (x>(N*N) || y>(N*N) ){
     		 printf("Error: value not in range 0-%d\r\n",(N*N));
     		 return;
     	 }
     	 if (!is_empty()){
     		 printf("Error: board is not empty\r\n");
     		 return;
     	 }
     	 for (i=0; i<N;i++){
     		 for(j=0;j<N;j++){
     			 solution[i][j]=0;
     		 }
     	 }
     	 for(k=0; k< 1000; k++){
     		 if (b==0){
     			 choose_random_cells(x);
     			 random_backtracking();
     			 if (ILP(solution)){
     				 b = 1;
     				 break;
     			 }
     			 else{
     				 for(i =0;i<N;i++){
     					 for(j=0;j<N;j++){
     			  	         fixed[i][j] =0;
     		 	   	        sudoku[i][j] = 0;
     		 	   	        solution[i][j]=0;
     			 	   	  }
     				  }
     			 }
     		 }
     	 }
     	 if(k==1000){
     		 printf("Error: puzzle generator failed\r\n");
     		 return;
     	 }
     	 for(i=0; i<N; i++){/* sets the cells to 0 - fixed matrix*/
     		 for(j=0;j<N; j++){
     			 fixed[i][j] = 0;
     			 sudoku[i][j]=0;
     		 }
     	 }
     	 choose_random_cells(y);
     	 a=1;
     	 c_n=y;
     	 for(i=0; i<N; i++){/* sets the not chosen cells to 0*/
     			 for(j=0;j<N; j++){
     				if(fixed[i][j]){
     					set(j,i,solution[i][j]);
     				}
     				else
     					sudoku[i][j] =0;
     				 fixed[i][j]=0;
     			 }
     		 }
     	 a=0;
     	 printboard();
     	 return;
      }


      /*
       * Print the number of solutions for the current board.
       * This command is only available in Edit and Solve modes.
       */
       int num_solutions(){
       	int cnt;
       	 if( !(mode == edit || mode == solve)){
       			 invalid_command();
       			 return 0;
       		}
       	 if (has_error()){
       	 		 printf("Error: board contains erroneous values\r\n");
       	 		 return 0;
       	 	 }
       	cnt = exhaustive_backtracking();
       	printf("Number of solutions: %d\r\n" ,cnt);
       	if (cnt == 1){
       		printf("This is a good board!\r\n");
       	}
       	else{
       		printf("The puzzle has more than 1 solution, try to edit it further\r\n");
       	}
       	return cnt;
       }


       /*
        * Automatically fill "obvious" values – cells which contain a single legal value.
        * Go over the board and check the legal values of each empty cell.
        * If a cell <X,Y> has a single legal value,it been filled with the value
        * the func go over the cells from left-to-right, then top-to-bottom.
        * This command is only available in Solve mode.
        */
       void autofill(){
       	int i,j,k,c=0, cnt=0;
       	c_n=0;
       	if (!(mode == solve)){
       		invalid_command();
       		return;
       	}
       	if (has_error()){
       		printf("Error: board contains erroneous values\r\n");
       		return ;
       	}
       	a=1;

       	for(i=0;i<N; i++){
       		for(j=0;j<N; j++){
       			a_f[i][j] =0;
       			if (!fixed[i][j] && validateCell(j,i)==1){
       					for(k=0; k<N; k++){
       						if (validNumbers[i][j][k]!=0){
       							if(sudoku[i][j]==0){
       								a_f[i][j]=k+1;
       								cnt++;
       								printf("Cell <%d,%d> set to %d\r\n", j+1,i+1,(k+1));
       								break;
       							}
       						}
       					}
       			}
       		}
       	}
       	c_n=cnt;
       		for(i=0;i<N; i++){
       			for(j=0;j<N; j++){
       				if(a_f[i][j]>0 && a_f[i][j] <= N){
       					set(j,i,a_f[i][j]);
       				}
       			}
       		}
       	a=0;
       	printboard();

       }
       /*
        * Undo all moves, reverting the board to its original loaded state.
        * The command goes over the entire undo/redo list and reverts all moves (no
        * output is provided).
        */
      void reset(){
      	int i,j;
      	if( !(mode == edit || mode == solve)){
      				 invalid_command();
      				 return ;
      	}
      	while(lst->pointer != lst->first){
      		new_undo(0);
      	}
      	free_list();
      	printf("Board reset\r\n");
      	initialize_list(); /* might be wrong */

      }

      /*
       * Terminates the program.
       * All memory resources are been freed.
       * which includes the list, all matrixes.
       * and also print exiting massage.
       * Any unsaved work is lost.
       */
      void exiting(){
      	if(mode==init){
      		printf("Exiting...\r\n");
      		return;
      	}
      	free_list();
      	free_matrixes();
      	printf("Exiting...\r\n");
      }











