/*
 * Solver.c
 *
 *  Created on: 23 בספט 2018
 *      Author: ליטל
 */

#include "Solver.h"
#include "Parser.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gurobi.h"
#include "gurobi_c.h"
#include <assert.h>
#include "Game.h"
#include "SPBufferset.h"
#define solve 0
#define edit 1
#define init 2
#define backtrack 3
#define forwardtrack 7


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

 	}	while(1);
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













