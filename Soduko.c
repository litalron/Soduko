/*
 ============================================================================
 Name        : Soduko.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
/* each cell is 4 caracters:
 * ' '
 * two digits printf"%2d"
 * '.' for fixed. '*' for erroneous in edit mode or mark errors=1 otherwise = 0.
 */
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gurobi.h"
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
 * the program prints invalid command
 * when there is an invalid execution
 */
void invalid_command(){
	printf("ERROR: invalid command\r\n");
}

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
 * check if the puzzle is empty.
 * i.e all the values of the cell are 0.
 * if the puzzle is empty returns 1.
 * else return 0;
 */
int is_empty(){
	int i,j;
	for(i=0;i<N;i++){
		for(j=0; j<N; j++){
			if (sudoku[i][j] != 0){
				return 0;
			}
		}
	}
	return 1;
}

/*
 * free all the nodes in the list
 * from lst->pointer->next till the end of the list.
 */
void free_set_nodes(){
	 int cnt=0, i;
	 if (lst->pointer->next==NULL){
		 return;
	 }
	 while(lst->pointer->next!=NULL){
		 cnt++;
		 lst->pointer = lst->pointer->next;
	 }
	 for(i=0; i<cnt; i++){
		 lst->pointer=lst->pointer->prev;
		 free(lst->pointer->next);
	 }
 }

/*
 * free all matrixes which includes-
 * a_f, sudoku, error, fixed, solution, validNumbers.
 */
void free_matrixes(){
	int i=0, j=0,k=0;
	for(i=0; i< N; i++){
		free(a_f[i]);
		free(sudoku[i]);
		free(error[i]);
		free(fixed[i]);
		free(solution[i]);

	}
	free(a_f);
	free(sudoku);
	free(error);
	free(fixed);
	free(solution);
	for(i=0; i<N; i++){
		for(j=0; j<N;j++){
				free(validNumbers[i][j]);
			}
			free(validNumbers[i]);
		}
	free(validNumbers);

}

/*
 * allocate memory for the list and for the first node of the list.
 * the first node is also initialized. next=NULL ,prev=NULL, n_a=0.
 * the lst->pointer is sets to be the first node.
 */
 void initialize_list(){
	 lst = (list*)malloc(sizeof(list));
	 node* n = (node*)malloc(sizeof(node));
	 lst->first = n;
	 lst->last = n;
	 lst->pointer = n;
	 lst->first->n_a=0;
	 lst->first->prev = NULL;
	 lst->first->next = NULL;
 }


/*
 * initialize the matrixes which include a_f, sudoku, error, fixed, solution, validNumbers.
 *	initialize the list by calling the func initialize_list().
 *	setting a=0, c_n=0.
 */
void initialize(){
	int i,j;
	a=0;
	c_n=0;
	a_f = (int**) calloc(N,sizeof(int*));
	for(i=0; i< N; i++){
		a_f[i] = (int*)calloc(N, sizeof(int));
	}
	sudoku = (int**) calloc(N,sizeof(int*));
	for(i=0; i< N; i++){
		sudoku[i] = (int*)calloc(N, sizeof(int));
	}
	fixed = (int**) calloc(N,sizeof(int*));
	for(i=0; i< N; i++){
		fixed[i] = (int*)calloc(N, sizeof(int));
	}
	error = (int**) calloc(N,sizeof(int*));
	for(i=0; i< N; i++){
		error[i] = (int*)calloc(N, sizeof(int));
	}
	solution = (double**) calloc(N,sizeof(double*));
	for(i=0; i< N; i++){
		solution[i] = (double*)calloc(N, sizeof(double));
	}
	validNumbers = (int***) calloc(N, sizeof(int**));
	for(i=0; i<N; i++){
		validNumbers[i] = (int**) calloc(N, sizeof(int*));
		for(j=0; j<N;j++){
			validNumbers[i][j] = (int*) calloc(N+1, sizeof(int));
		}
	}
	for(i =0;i<N;i++){
   	     for(j=0;j<N;j++){
   	         fixed[i][j] =0;
   	         error[i][j] = 0;
   	         sudoku[i][j] = 0;
   	         solution[i][j] = 0;
   	         a_f[i][j] = 0;
   	         initializeValidNumbers(i,j);
   	     }
 	 }
	initialize_list();
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

/*
 * pop- remove the last stack_node in the stack and return it.
 */
stack_node* pop(stack *s){
	stack_node* s_n = s->last;
	if (s_n == NULL){
		return NULL;
	}
	s->last= s_n->prev;
	if (s_n->prev != NULL){
		s->last->next = NULL;
	}
	s->num_of_elements--;
	return (s_n);
}

/*
 * free the stack
 */
void freestack(stack * s){
	free(s);
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
 * insert a new stack_node to the stack.
 */

void insert(stack*s, stack_node* node){
	stack_node *s_n = s->last;
	if (s->last != NULL){
		s_n->next = node;
		node->prev = s_n;
	}
	else{
		node->prev = NULL;
	}
	node->next = NULL;
	s->last = node;
	s->num_of_elements++;
}

/*
 * search for the next cell in the sudoku which his value !=0.
 * the searching  go over the sudoku from the func go over the cells from left-to-right, then top-to-bottom.
 * the searching starts from the cell<i,j>.
 * the next free cell is being saved in the array location.
 */
int nextFreeCell(int ** copy_of_fixed, int i, int j, int location[2]){
 	int r,c;
 	location[0] = 0;
 	location[1] = 0;
 	if (i==-1 && j==-1){
 		if ( copy_of_fixed[0][0] == 0)
 			return 1;
 		else{
 			i = 0;
 			j = 0;
 		}
 	}
 	for (r=i;r<N;r++){
 		for (c=0;c<N;c++){
 			if (r==i && c==0){
 				if (j != N-1){
 					c = j+1;
 				}
 				else{
 					r++;
 				}
 			}
 			if(r< N && c<N && copy_of_fixed[r][c]==0){
 				location[0]=r;
 				location[1]=c;
 				return 1;
 			}
 		}
 	}
	return 0;
}

/*
 * reset the matrix copy_of_sudoku to the original sudoku from the cell<i,j> till the cell<N,N>.
 * it the func goes over the cells from left-to-right, then top-to-bottom.
 */
void initialize_copy_of_soduko(int **copy_of_sudoku, int **copy_of_fixed,  int i, int j){
	int x = i;
	int y = j;
	int start = 1;
	for(x=i;x<N;x++){
		for(y=0;y<N;y++){
			if(start && x==i && j==N-1){
				start = !start;
				break;
			}
			else if(start && x==i && j!=N-1){
				start = !start;
				y = j;
				continue;
			}
			if (!copy_of_fixed[x][y]){
				copy_of_sudoku[x][y] = 0;
			}
		}
	}
}

/*
 * increment the value of that cell<i,j> in the  copy_of_sudoku matrix.
 * if its value >N it backtrack to the last cell which its sudoku[x][y]!=0 in order to retrieve other potential
 * solutions.
 */
int find_new_val(int i,int j, int ** copy_of_sudoku, stack *s,stack_node *s_n){
	int k;
	for(k=copy_of_sudoku[i][j]+1; k<N+1; k++){
		copy_of_sudoku[i][j] = k;
		if (!check_error(i, j, copy_of_sudoku)){
			s_n = (stack_node*)malloc(sizeof(stack_node));
			s_n->i = i;
			s_n->j = j;
			s_n->k = k;
			insert(s, s_n);
			return forwardtrack;
		}
	}
	return backtrack;
}

/*
 * copies the sudoku puzzle to the copy_of_sudoku matrix.
 * and also copies the fixed matrix to the copy_of_fixed matrix.
 */
void initialize_copy_of(int **copy_of_sudoku, int **copy_of_fixed){
	int i,j;
	for(i=0; i< N; i++){
		copy_of_sudoku[i] = (int*)calloc(N, sizeof(int));
		copy_of_fixed[i] = (int*)calloc(N, sizeof(int));
	}
	for(i =0; i< N; i++){ /*copy*/
		for(j=0;j<N; j++){
			copy_of_sudoku[i][j] = sudoku[i][j];
			if (sudoku[i][j] != 0)
				copy_of_fixed[i][j] = 1;
			else
				copy_of_fixed[i][j] = 0;
		}
	}
}

/*
 *The exhaustive backtracking algorithm exhausts all options for the current board.
 */
int exhaustive_backtracking(){
	int i=0,j=0,k, b=forwardtrack, finished =0;
	int cnt = 0;
	int  l[2];
	stack_node *s_n;
	stack *s = (stack*)malloc(sizeof(stack));
	s->num_of_elements = 0;
	s->last=NULL;
	int ** copy_of_sudoku= (int**) calloc(N,sizeof(int*));
	int ** copy_of_fixed= (int**) calloc(N,sizeof(int*));
	initialize_copy_of(copy_of_sudoku, copy_of_fixed);
	nextFreeCell(copy_of_fixed,-1,-1,l);
	i=l[0];
	j=l[1];
	do{
		b =find_new_val(i,j, copy_of_sudoku, s, s_n);
		if(b==backtrack){
			s_n = pop(s);
			if (s_n != NULL){
					i=s_n->i;
					j=s_n->j;
					free(s_n);
					initialize_copy_of_soduko(copy_of_sudoku, copy_of_fixed,  i, j);
				}
			else {
					break;
				}
		}
		else{
			finished = !nextFreeCell(copy_of_fixed,i,j,l);
			if (finished){
				cnt++;
				s_n = pop(s);
				if (s_n != NULL){
					i=s_n->i;
					j=s_n->j;
					free(s_n);
					initialize_copy_of_soduko(copy_of_sudoku, copy_of_fixed,  i, j);
				}
				else{
					break;
				}
			}
			else{
				i=l[0];
				j=l[1];
			}
		}

	}	while(s->num_of_elements!= 0);
		for(i=0; i< N; i++){
			free(copy_of_sudoku[i]);
			free(copy_of_fixed[i]);
		}
	free(copy_of_sudoku);
	free(copy_of_fixed);
	freestack(s);
	return cnt;
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
  * sets all the cells <X,Y>c= Z!=0 to fixed in the fixed puzzle.
  */
 void set_all_cells_to_fixed(){
	 int i, j;
	 for(i=0;i<N; i++){
	 	 for(j=0; j< N; j++){
	 		if (sudoku[i][j] != 0){
	 			fixed[i][j] = 1;
	 		}
	 	}
	 }
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
 * one recursive step to free the list
 */
 void free_list_rec(node* n){
 	if (n != NULL){
 		free_list_rec(n->next);
 		free(n);
 	}
 }

 /*
  * free the list.
  * the func is recursive.
  */
void free_list(){
	lst->pointer = lst->first;
	free_list_rec(lst->pointer);
 	free(lst);
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
  * the procedure receives the current game board suduko.
  * and indexes of a cell: i,j
  * validNumbers is a 3 dimensional array. each cell has an array which holds
  * the valid numbers for that cell.
  * the function updates that array and stores 0 in the array if the number doesnt fit
  * i.e already exists in the row/column/block
  */
    int validateCell(int j, int i){
 	   int k, num, cnt =N, temp = sudoku[i][j];
 	   initializeValidNumbers(i,j);
 	   for (k =0; k< N; k++ ){
 	      		  num = validNumbers[i][j][k];
 	      		 if (num != 0){
 	      			 sudoku[i][j] = num;
 	      		 	 if (check_error(i, j, sudoku)){
 	      		 		 validNumbers[i][j][k]= 0;
 	      		 		 validNumbers[i][j][N] = validNumbers[i][j][N]-1 ;
 	      		 		 cnt--;
 	      		 	 }
 	      		 }

 	      	  }
  		sudoku[i][j] = temp;
 	   return cnt;
    }


    /*
     * randomly chose a value to the cell<i,j> among the valid values that are legal.
     */
 int calculateRand(int j, int i){
	   int index;
	   if (validNumbers[i][j][N] != 0){
		   index = rand();
		   index =index % (validNumbers[i][j][N]);
	   }
	   else{
		   index = 0;
	   }
	   return index;
 }

 /* the procedure sets the i,j indexes in the game board to 0
  * and initializes the array of the valid numbers of that cell to {1,2,3...,N,N}
  * call it when restart the board for every i,j
 */
 void initializeValidNumbers(int i, int j){
	   int k;
	   /*sudoku[i][j]= 0;*/
	   for (k=1;k<(N+1);k++){
	     	validNumbers[i][j][k-1] = k;
	   }
	   validNumbers[i][j][N] = N;
  }


 /* the procedure receives indexes of a cell: i,j and an index
    * and finds the #index number that fits to the cell.
    * example: let be {1,4,7,9} those numbers, and index = 2 then the procedure returns 7. (index 0 stands for the 1st number)
    */
 int findTheIndex(int j, int i,int index){
	   int cnt=(-1), k;
	   for (k=0; k<N; k++){
		   if (validNumbers[i][j][k]!=0){
			   cnt+=1;
			   if (cnt==index){
				   break;
			   }
		   }
	   }
	   return k;
 }

 /*
  * calls the next step of the recursion rec_Random( int j, int i).
  */
 int sub_rec_random( int j, int i){
	 if (j!= (N-1)){
	     			 if (rec_Random (j+1,i)){
	     				 return 1;
	     			 }
	     		 }
	     		 else{
	     			 if (rec_Random(0,i+1)){
	     				 return 1;
	     			 }
	     		 }
	 return 0;
 }

 /*
  *  the procedure is recursive one that sets the fixed cells in the puzzle. more info in: random_backtracking
  * returns 1 if it worked successfully.
  */
    int rec_Random( int j, int i){
    	 int  k,index, cnt=0;
    	 if (i==N){ /* the procedure works top down. therefore if we've reached the 9th row, it succeeded generating a board */
    		 return 1;
    	 }
    	 if (fixed[i][j]){
    		 cnt=validateCell(j,i);
    		 while(cnt > 0 ){
    		 index =calculateRand(j, i);
    		 k=findTheIndex(j,i,index);
    		 sudoku[i][j] = validNumbers[i][j][k];
    		 validNumbers[i][j][k]= 0;
    		 validNumbers[i][j][N] = (validNumbers[i][j][N]-1) ;
    		 cnt --;
    		 return sub_rec_random(j,i);
    	 }
    	initializeValidNumbers( i, j);
    	 }
    	 else{
    		 return sub_rec_random(j,i);
    	 }
    	return 0;
    }


 /*
  * the procedure filling each fixed cell in the puzzle with a legal random value.
  * it does that according to a randomic algorithm learned in class
  */
     int random_backtracking(){
   	 int i, j,k;
   	 for (i=0;i<N;i++){
   	  		 for (j=0;j<N;j++){
   	  			if (fixed[i][j]==1){
   	  				for (k=1;k<(N+1);k++){ /* initialize the possible numbers {1,2,3,4,5,6,7,8,9,9} to each cell i,j. the last index is counter of the valid numbers */
   	  					validNumbers[i][j][k-1] = k;
   	  				}
   	 	 			validNumbers[i][j][N] = N;
   	  			}

   	  			else{

   	  				validNumbers[i][j][N] = 0;
   	  			}
   	  		}

   	  }
   	 return rec_Random(0, 0);
     }


/*
 * Randomly choose X cells from the puzzle.
 * sets those cells to fixed.
 */
void choose_random_cells(int x){
	int i,flag,row,column;
	for (i=0; i<x; i++){
		           flag = 0;
		           while (flag==0){
		               column = rand() % N;
		               row = rand() % N;
		               if (fixed[row][column] == 0){
		                   flag = 1;
		                   fixed[row][column] = 1;
		               }
		           }
		       }
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

int gameOn(){
int i,j;
char str[256];
char * token;
int array2 [2]; /* array2 stands for the arguments in the command hint X Y. for example: if the user writes 'hint 4 5' then array2 - {4,5} */
int array3 [3]; /* array3 stands for the arguments in the command set X Y Z. for example: if the user writes 'set 4 5 5' then array3 - {4,5,5} */
int valid = 1; /* valid indicates if the input in commands 'hint', 'set' is valid */
char c;

SP_BUFF_SET()
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
					save(token);
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
int main(int argc, char* argv[]){
	srand(time(NULL));
	mode=init;
	/*editing(NULL);
	generating(40,10);*/
	gameOn();
	return 1;
 }
