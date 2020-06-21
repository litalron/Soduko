/*
 * Soduko.h
 *
 *  Created on: 9 בספט 2018
 *      Author: ליטל
 */

#ifndef SODUKO_H_
#define SODUKO_H_
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "gurobi.h"
#include "gurobi_c.h"
#include "SPBufferset.h"

#define solve 0
#define edit 1
#define init 2
#define backtrack 3
#define forwardtrack 7


typedef struct node {
    int n_a ;/*num of relevant actions*/
	int val;
    int fill;
    int old_val;
    int i;
    int j;
    struct node * next;
    struct node * prev;
} node;
typedef struct list {
	/*int p=0;*/
    node* first;
    node* last;
    node* pointer;
} list;

typedef struct stack_node {
	int i;
	int j;
	int k;
	struct stack_node* next;
	struct stack_node *prev;
} stack_node;

typedef struct stack{
	stack_node* last;
	int num_of_elements;
} stack;

int m ;
int n ;
int N ;
int a,ac;
int** sudoku;
int ** a_f;

void free_set_nodes();
void free_metrixes();
void invalid_command();
int has_error();
int validate();
int is_empty();
void initialize_list();
void initialize();
void mark_errors(int x);
stack_node* pop(stack *s);
void freestack(stack * s);
void initialize_copy_of(int **copy_of_sudoku, int **copy_of_fixed);
int find_new_val(int i,int j, int ** copy_of_sudoku, stack *s,stack_node *s_n);
int check_error(int i, int j, int**board);
void insert(stack*s, stack_node* node);
void initialize_copy_of_soduko(int **copy_of_sudoku, int **copy_of_fixed,  int i, int j);
int nextFreeCell(int **copy_of_fixed , int i, int j, int location[2]);
int exhaustive_backtracking();
void printboard();
void solving (char* path );
void editing(char* path );
void set_all_cells_to_fixed();
int set (int x, int y, int z);
void hint (int x, int y);
int  save_to_file(char*path);
void save(char* path);
void undo();

int redo();
void free_list_rec(node* n);
void free_list();
int validateCell(int j, int i);
int calculateRand(int j, int i);
void initializeValidNumbers(int i, int j);
int findTheIndex(int j, int i,int index);
int sub_rec_random( int j, int i);
int rec_Random( int j, int i);
int random_backtracking();
void choose_random_cells(int x);
void generating(int x, int y);
int num_solutions();
void autofill();
void reset();
void exiting();
int gameOn();
void pre_redo();
void redo_print();
void print_undo();
void printsolution();
void read(char * path);
void new_undo();
void new_redo();

#endif /* SODUKO_H_ */
