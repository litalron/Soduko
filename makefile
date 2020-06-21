CC = gcc
OBJS = gurobi.o Game.o Parser.o Solver.o
EXEC = project
COMP_FLAGS = -ansi -O3 -Wall -Wextra \
-Werror -pedantic-errors 
GUROBI_COMP = -I/usr/local/lib/gurobi563/include
GUROBI_LIB = -L/usr/local/lib/gurobi563/lib -lgurobi56

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(OBJS) $(GUROBI_LIB) -o $@ -lm
gurobi.o: gurobi.c gurobi_c.h Game.h Parser.h Solver.h SPBufferset.h
	$(CC) $(COMP_FLAG) $(GUROBI_COMP) -c $*.c
Game.o:  Game.c gurobi.h gurobi_c.h Parser.h Solver.h SPBufferset.h
	$(CC) $(COMP_FLAG) $(GUROBI_COMP) -c $*.c
Parser.o:  Parser.c gurobi.h gurobi_c.h Game.h Solver.h SPBufferset.h
	$(CC) $(COMP_FLAG) $(GUROBI_COMP) -c $*.c
Solver.o:  Solver.c gurobi.h gurobi_c.h Game.h Solver.h SPBufferset.h
	$(CC) $(COMP_FLAG) $(GUROBI_COMP) -c $*.c
main.o: main.c gurobi.h Solver.h gurobi_c.h Game.h Solver.h SPBufferset.h
	$(CC) $(COMP_FLAGS) $(GUROBI_COMP) -c $*.c
clean:
	rm -f *.o $(EXEC)


