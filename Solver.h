/*
 * Solver.h
 *
 *  Created on: 23 בספט 2018
 *      Author: ליטל
 */

#ifndef SOLVER_H_
#define SOLVER_H_

#include "Parser.h"
#include "Game.h"
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

int is_empty();
void free_set_nodes();
void free_matrixes();
void initialize_list();
void initialize();
stack_node* pop(stack *s);
void freestack(stack * s);
void insert(stack*s, stack_node* node);
int nextFreeCell(int ** copy_of_fixed, int i, int j, int location[2]);
void initialize_copy_of_soduko(int **copy_of_sudoku, int **copy_of_fixed,  int i, int j);
int find_new_val(int i,int j, int ** copy_of_sudoku, stack *s,stack_node *s_n);
void initialize_copy_of(int **copy_of_sudoku, int **copy_of_fixed);
int exhaustive_backtracking();
void set_all_cells_to_fixed();
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



#endif /* SOLVER_H_ */
