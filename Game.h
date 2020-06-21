/*
 * Game.h
 *
 *  Created on: 23 בספט 2018
 *      Author: ליטל
 */

#ifndef GAME_H_
#define GAME_H_

#include "Parser.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gurobi.h"
#include "gurobi_c.h"
#include "Solver.h"
#include <assert.h>
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




int validate(int flag);
int check_error(int i, int j, int**board);
void update_matrix_errors();
int has_error();
void printboard();
void solving (char* path);
void editing(char* path);
int set (int y, int x, int z);
void hint (int y, int x);
int save_to_file(char*path);
void save(char* path);
void print_undo();
void undo();
void new_undo(int flag);
void new_redo();
int redo();
void redo_print();
void read(char * p);
void generating(int x, int y);
int num_solutions();
void autofill();
void reset();
void exiting();
#endif /* GAME_H_ */
