/* Copyright (c) 2010 the authors listed at the following URL, and/or
the authors of referenced articles or incorporated external code:
http://en.literateprograms.org/Shunting_yard_algorithm_(C)?action=history&offset=20080201043325

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Retrieved from: http://en.literateprograms.org/Shunting_yard_algorithm_(C)?oldid=12454

** edited by James Rubingh for zasm2 (
*/

#include<stdlib.h>
#include<stdio.h>
#include<ctype.h>

#define MAXOPSTACK 64
#define MAXNUMSTACK 64

struct op_s *opstack[MAXOPSTACK];
int nopstack;

int numstack[MAXNUMSTACK];
int nnumstack;
int shunt_error;

enum {ASSOC_NONE=0, ASSOC_LEFT, ASSOC_RIGHT};

int eval_uminus(int a1, int a2) 
{
	return -a1;
}
int eval_exp(int a1, int a2)
{
	return a2<0 ? 0 : (a2==0?1:a1*eval_exp(a1, a2-1));
}
int eval_mul(int a1, int a2) 
{
	return a1*a2;
}
int eval_div(int a1, int a2) 
{
	if(!a2) {
		fprintf(stderr, "ERROR: Division by zero\n");
    shunt_error=1;
    return EXIT_FAILURE;
	}
	return a1/a2;
}
int eval_mod(int a1, int a2) 
{
	if(!a2) {
		fprintf(stderr, "ERROR: Division by zero\n");
    shunt_error=1;
    return EXIT_FAILURE;
	}
	return a1%a2;
}
int eval_add(int a1, int a2) 
{
	return a1+a2;
}
int eval_sub(int a1, int a2) 
{
	return a1-a2;
}

struct op_s {
	char op;
	int prec;
	int assoc;
	int unary;
	int (*eval)(int a1, int a2);
} ops[]={
	{'_', 10, ASSOC_RIGHT, 1, eval_uminus},
	{'^', 9, ASSOC_RIGHT, 0, eval_exp},
	{'*', 8, ASSOC_LEFT, 0, eval_mul},
	{'/', 8, ASSOC_LEFT, 0, eval_div},
	{'%', 8, ASSOC_LEFT, 0, eval_mod},
	{'+', 5, ASSOC_LEFT, 0, eval_add},
	{'-', 5, ASSOC_LEFT, 0, eval_sub},
	{'(', 0, ASSOC_NONE, 0, NULL},
	{')', 0, ASSOC_NONE, 0, NULL}
};

struct op_s *getop(char ch)
{
	int i;
	for(i=0; i<sizeof ops/sizeof ops[0]; ++i) {
		if(ops[i].op==ch) return ops+i;
	}
	return NULL;
}

void push_opstack(struct op_s *op)
{
	if(nopstack>MAXOPSTACK-1) {
		fprintf(stderr, "ERROR: Operator stack overflow\n");
    shunt_error=1;
		return;
	}
	opstack[nopstack++]=op;
}

struct op_s *pop_opstack()
{
	if(!nopstack) {
		fprintf(stderr, "ERROR: Operator stack empty\n");
    shunt_error=1;
		return NULL;
	}
	return opstack[--nopstack];
}

void push_numstack(int num)
{
	if(nnumstack>MAXNUMSTACK-1) {
		fprintf(stderr, "ERROR: Number stack overflow\n");
    shunt_error=1;
		return;
	}
	numstack[nnumstack++]=num;
}

int pop_numstack()
{
	if(!nnumstack) {
		fprintf(stderr, "ERROR: Number stack empty\n");
    shunt_error=1;
		return EXIT_FAILURE;
	}
	return numstack[--nnumstack];
}

void shunt_op(struct op_s *op)
{
	struct op_s *pop;
	int n1, n2;

	if(op->op=='(') {
		push_opstack(op);
		return;

	} else if(op->op==')') {
		while(nopstack>0 && opstack[nopstack-1]->op!='(') {
			pop=pop_opstack();
			n1=pop_numstack();

			if(pop->unary) push_numstack(pop->eval(n1, 0));
			else {
				n2=pop_numstack();
				push_numstack(pop->eval(n2, n1));
			}
		}

		if(!(pop=pop_opstack()) || pop->op!='(') {
			fprintf(stderr, "ERROR: Stack error. No matching \'(\'\n");
      shunt_error=1;
      return;
		}
		return;
	}

	if(op->assoc==ASSOC_RIGHT) {
		while(nopstack && op->prec<opstack[nopstack-1]->prec) {
			pop=pop_opstack();
			n1=pop_numstack();
			if(pop->unary) push_numstack(pop->eval(n1, 0));
			else {
				n2=pop_numstack();
				push_numstack(pop->eval(n2, n1));
			}
		}
	} else {
		while(nopstack && op->prec<=opstack[nopstack-1]->prec) {
			pop=pop_opstack();
			n1=pop_numstack();
			if(pop->unary) push_numstack(pop->eval(n1, 0));
			else {
				n2=pop_numstack();
				push_numstack(pop->eval(n2, n1));
			}
		}
	}
	push_opstack(op);
}

int shunt(char *expression)
{
	char *expr;
	char *tstart=NULL;
	struct op_s startop={'X', 0, ASSOC_NONE, 0, NULL};	/* Dummy operator to mark start */
	struct op_s *op=NULL;
	int n1, n2;
	struct op_s *lastop=&startop;

  nnumstack=0;
  nopstack=0;
  shunt_error=0;

	for(expr=expression; *expr; ++expr) {
		if(!tstart) {

			if((op=getop(*expr))) {

				if(lastop && (lastop==&startop || lastop->op!=')')) {
					if(op->op=='-') op=getop('_');
					else if(op->op!='(') {
						fprintf(stderr, "ERROR: Illegal use of binary operator (%c)\n", op->op);
            shunt_error=1;
            return EXIT_FAILURE;
					}
				}
				shunt_op(op);
				lastop=op;
			} else if(isdigit(*expr)) tstart=expr;
			else if(!isspace(*expr)) {
				fprintf(stderr, "ERROR: Syntax error\n");
        shunt_error=1;
				return EXIT_FAILURE;
			}
		} else {
			if(isspace(*expr)) {
				push_numstack(atoi(tstart));
				tstart=NULL;
				lastop=NULL;
			} else if((op=getop(*expr))) {
				push_numstack(atoi(tstart));
				tstart=NULL;
				shunt_op(op);
				lastop=op;
			} else if(!isdigit(*expr)) {
				fprintf(stderr, "ERROR: Syntax error\n");
				return EXIT_FAILURE;
			}
		}
	}
	if(tstart) push_numstack(atoi(tstart));

	while(nopstack) {
		op=pop_opstack();
		n1=pop_numstack();
		if(op->unary) push_numstack(op->eval(n1, 0));
		else {
			n2=pop_numstack();
			push_numstack(op->eval(n2, n1));
		}
	}

	if(nnumstack!=1) {
		fprintf(stderr, "ERROR: Number stack has %d elements after evaluation. Should be 1.\n", nnumstack);
		return EXIT_FAILURE;
	}

	return numstack[0];
}




