/*
 * gurobi.h
 *
 *  Created on: 9 בספט 2018
 *      Author: ליטל
 */

#ifndef GUROBI_H_
#define GUROBI_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "gurobi_c.h"
#include "Soduko.h"
#include "Parser.h"
#include "Solver.h"
#include "Game.h"
#include "SPBufferset.h"

#define V (N*N*N)

int ILP(double ** solution);

#endif /* GUROBI_H_ */
