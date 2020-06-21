/*
 * david_gurobi.c
 *
 *  Created on: 12 בספט 2018
 *      Author: ליטל
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gurobi_c.h"
#include "Parser.h"
#include "Soduko.h"
#include "Solver.h"


/*
 * 1 = solution exist
 * 0 = NO solution exist
 * -1 = Gurobi error
 *
 */
int ILP(double ** solution)
{

  GRBenv   *env   = NULL;
  GRBmodel *model = NULL;
  int X=m*n;

  int* ind=calloc((size_t)(X),sizeof(int));
    double *sol = (double*)calloc(X*X*X, sizeof(double));
  double* val=calloc((size_t)(X),sizeof(double));
  double* lb=calloc((size_t)(X*X*X),sizeof(double));
  char* vtype=calloc((size_t)(X*X*X),sizeof(char));
  char** names=calloc((size_t)(X*X*X),12);
  char* namestorage=calloc((size_t)(12*X*X*X),sizeof(char*));
  char* cursor="";

  int       optimstatus;
  double    objval;
  int       i, j, v, k,ig, jg, count, offset;
  int       errorr = 0;
  /* Create an empty model */
  cursor = namestorage;
  for (i = 0; i < X; i++) {
    for (j = 0; j < X; j++) {
      for (v = 0; v < X; v++) {
        if (sudoku[i][j] == v+1){
         *(lb+(i*X*X+j*X+v)) = 1;
         }
        else{
          *(lb+(i*X*X+j*X+v)) = 0;}
       *(vtype+(i*X*X+j*X+v)) = GRB_BINARY;

        *(names+(i*X*X+j*X+v)) = cursor;
        sprintf(*(names+(i*X*X+j*X+v)), "x[%d,%d,%d]", i+1, j+1, v+1);
        cursor += strlen(*(names+(i*X*X+j*X+v))) + 1;
      }
    }
  }


  /* Create environment */

  errorr = GRBloadenv(&env, "sudoku.log");
  if (errorr) goto QUIT;


  /* Create new model */

  errorr = GRBnewmodel(env, &model, "sudoku", X*X*X, NULL, lb, NULL, vtype, names);
  if (errorr) goto QUIT;



  /* Each cell gets a value */

  for (i = 0; i < X; i++) {
    for (j = 0; j < X; j++) {
      for (v = 0; v < X; v++) {
        *(ind+v) = i*X*X + j*X + v;
        *(val+v) = 1.0;
      }

      errorr = GRBaddconstr(model, X, ind, val, GRB_EQUAL, 1.0, NULL);
      if (errorr) goto QUIT;
    }
  }

  /* disable gurobi prints */

  errorr =GRBsetintparam(GRBgetenv(model), "OutputFlag", 0);
  if (errorr) goto QUIT;


  /* Each value must appear once in each row */

  for (v = 0; v < X; v++) {
    for (j = 0; j < X; j++) {
      for (i = 0; i < X; i++) {
       *(ind+i) = i*X*X + j*X + v;
        *(val+i) = 1.0;
      }

      errorr = GRBaddconstr(model, X, ind, val, GRB_EQUAL, 1.0, NULL);
      if (errorr) goto QUIT;
    }
  }


  /* Each value must appear once in each column */

  for (v = 0; v < X; v++) {
    for (i = 0; i < X; i++) {
      for (j = 0; j < X; j++) {
       *(ind+j) = i*X*X + j*X + v;
        *(val+j) = 1.0;
      }

      errorr = GRBaddconstr(model, X, ind, val, GRB_EQUAL, 1.0, NULL);
      if (errorr) goto QUIT;
    }
  }


  /* Each value must appear once in each subgrid */

  for (v = 0; v < X; v++) {
    for (ig = 0; ig < n; ig++) {
      for (jg = 0; jg < m; jg++) {
        count = 0;
        for (i = ig*m; i < (ig+1)*m; i++) {
          for (j = jg*n; j < (jg+1)*n; j++) {
            *(ind+count) = i*X*X + j*X + v;
           *(val+count) = 1.0;
            count++;
          }
        }
        errorr = GRBaddconstr(model, X, ind, val, GRB_EQUAL, 1.0, NULL);
        if (errorr) goto QUIT;
      }
    }
  }


  /* Optimize model */

  errorr = GRBoptimize(model);
  if (errorr) goto QUIT;

  /* Capture solution information */

  /*if (optimstatus == GRB_OPTIMAL) {*/
	  errorr = GRBgetintattr(model, GRB_INT_ATTR_STATUS, &optimstatus);
	  if (errorr) goto QUIT;

	 if (optimstatus == GRB_OPTIMAL) {
		 errorr = GRBgetdblattr(model, GRB_DBL_ATTR_OBJVAL, &objval);
		 if (errorr) goto QUIT;

  /* get the solution - the assignment to each variable */
	  errorr = GRBgetdblattrarray(model, GRB_DBL_ATTR_X, 0, X*X*X, sol);
	  if (errorr) goto QUIT;
}

  /* solution found */
  if (optimstatus == GRB_OPTIMAL) {
	for(i=0; i< N; i++){
              	  for(j=0; j<N; j++){
              		  for(k=0; k<N; k++){
              			  if (sol[(i*(N*N)+ j*N + k)]==1){
              				solution[i][j] = k+1;
              			  }
              		  }
              	  }
              }
	GRBfreemodel(model);
	GRBfreeenv(env);
	free(sol);
	free(ind);
	free(val);
	free(lb);
	free(vtype);
	free(names);
	free(namestorage);



	return 1;
  }
  /* no solution found */
  else if (optimstatus == GRB_INF_OR_UNBD) {
	printf("Model is infeasible or unbounded\n");
	GRBfreemodel(model);
	GRBfreeenv(env);
	free(ind);
	free(sol);
	free(val);
	free(lb);
	free(vtype);
	free(names);
	free(namestorage);
	return 0;
  }
  /* error or calculation stopped */
  else {
	  goto QUIT;
  }

QUIT:

  /* Error reporting */

  if (errorr) {
    printf("ERROR: %s\n", GRBgeterrormsg(env));
    return -1;
  }

  /* Free model */

  GRBfreemodel(model);

  /* Free environment */

  	  GRBfreeenv(env);

  /* Free allocated arrays */
  	  free(sol);
  	free(ind);
	free(val);
	free(lb);
	free(vtype);
	free(names);
	free(namestorage);

  return 0;
}

