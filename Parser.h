/*
 * Parser.h
 *
 *  Created on: 23 בספט 2018
 *      Author: ליטל
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gurobi.h"
#include "gurobi_c.h"
#include "Solver.h"
#include "Game.h"
#include <assert.h>
#include "SPBufferset.h"
#define solve 0
#define edit 1
#define init 2
#define backtrack 3
#define forwardtrack 7

int gameOn();
void invalid_command();
void mark_errors(int x);


#endif /* PARSER_H_ */
