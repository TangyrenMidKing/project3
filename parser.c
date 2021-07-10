/*
	Author: Noelle Midkiff
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "compiler.h"

symbol *table;
int sym_index;
int error;
lexeme* current_token;

// prototypes
void printtable();
void errorend(int x);
void enter();
void program();
void block();
void const_decl();
void var_decl();
void proc_decl();
void statement();
void expression();
void condition();
void term();
void factor();
int isMultOp(token_type sym);
int isRelation(token_type sym);
void enter();

symbol *parse(lexeme *input)
{
	table = malloc(1000 * sizeof(symbol));
	sym_index = 0;
	error = 0;
	current_token = input;
// 	current_token += 1;
// printf("Start here: %6s %3d %3d\n", input[1].name, input[1].value, input[1].type);
// if (current_token->type == periodsym)
//  printf("Start here: %6s %3d %3d\n", current_token->name, current_token->value, current_token->type);

	program();

	if (error)
	{
		free(table);
		return NULL;
	}
	else
	{
		printtable();
		return table;
	}
}

void get_token()
{
	// Input is empty.
	if (current_token == NULL)
	{
		errorend(0);
		exit(1);
	}

	current_token = current_token + 1;
}

void program()
{
	if (error == 0)
	{
		get_token();

		block();

		if (current_token->type != periodsym)
		{
			errorend(3);
			exit(1);
		}
	}
	else
	{
		errorend(0);
		exit(1);
	}
}

void block()
{
	if (current_token->type == constsym)
		const_decl();
	if (current_token->type == varsym)
		var_decl();
	if (current_token->type == procsym)
		proc_decl();

	statement();
}

void const_decl()
{
	while (current_token->type != commasym)
	{
		get_token();

		if (current_token->type != identsym)
		{
			/**Bug here**/
			errorend(4);
			exit(1);
		}

		get_token();

		if(current_token->type != eqlsym)
		{
			errorend(4);
			exit(1);
		}

		get_token();

		if(current_token->type != numbersym)
		{
			errorend(5);
			exit(1);
		}
		/**Uncompleted**/
		enter();
		get_token();
	}

	if (current_token->type != semicolonsym)
	{
		errorend(6);
		exit(1);
	}
	get_token();
}

void var_decl()
{
	while (current_token->type != commasym)
	{
		get_token();
		if (current_token->type != identsym)
		{
			/**Bug here**/
			errorend(4);
			exit(1);
		}
		get_token();
		enter();
	}

	if (current_token->type != semicolonsym)
	{
		errorend(6);
		exit(1);
	}
	get_token();
}

void proc_decl()
{
	while (current_token->type == procsym)
	{
		get_token();
		if (current_token->type != identsym)
		{
			/**Bug here**/
			errorend(4);
			exit(1);
		}

		enter();
		get_token();

		if (current_token->type != semicolonsym)
		{
			errorend(6);
			exit(1);
		}

		get_token();
  	/** level +1 **/
		block();

		if (current_token->type != semicolonsym)
		{
			errorend(6);
			exit(1);
		}

		get_token();
	}
}

void statement()
{
	// Identifier
	if (current_token->type == identsym)
	{
		get_token();
		if (current_token->type != becomessym)
		{
			errorend(2);
			exit(1);
		}
		get_token();
		expression();
	}
	// Call
	else if (current_token->type == callsym)
	{
		get_token();
		if (current_token->type != identsym)
		{
			errorend(4);
			exit(1);
		}
		get_token();
	}
	// begin
	else if (current_token->type == beginsym)
	{
		get_token();
		statement();
		while (current_token->type == semicolonsym)
		{
			get_token();
			statement();
		}
		if (current_token->type != endsym)
		{
			errorend(10);
			exit(1);
		}
		get_token();
	}
	// if
	else if (current_token->type == ifsym)
	{
		get_token();
		condition();
		if (current_token->type != thensym)
		{
			errorend(9);
			exit(1);
		}
		get_token();
		statement();
	}
	// While
	else if (current_token->type == whilesym)
	{
		get_token();
		condition();
		if (current_token->type != dosym)
		{
			errorend(11);
			exit(1);
		}
		get_token();
		statement();
	}
	// Unknown error
	else
	{
		errorend(0);
		exit(1);
	}
}

void condition()
{
	if (current_token->type == oddsym)
	{
		get_token();
		expression();
	}
	else
	{
		expression();
		if (!isRelation(current_token->type))
		{
			errorend(12);
			exit(1);
		}
		get_token();
		expression();
	}
}

void expression()
{
	if (current_token->type == plussym || current_token->type == minussym)
		get_token();
	term();
	while (current_token->type == plussym || current_token->type == minussym)
	{
		get_token();
		term();
	}
}

void term()
{
	factor();
	while (isMultOp(current_token->type))
	{
		get_token();
		factor();
	}
}

void factor()
{
	if (current_token->type == identsym)
		get_token();
	else if (current_token->type == numbersym)
		get_token();
	else if (current_token->type == lparentsym)
	{
		get_token();
		expression();
		if (current_token->type != rparentsym)
		{
			errorend(13);
			exit(1);
		}
		get_token();
	}
	else
	{
		errorend(4);
		exit(1);
	}
}

int isMultOp(token_type mult_sym)
{
	if (mult_sym == multsym)
		return 1;
	else if (mult_sym == modsym)
		return 1;
	else if (mult_sym == slashsym)
		return 1;
	return 0;
}

int isRelation(token_type rela_sym)
{
	if (rela_sym == eqlsym)
		return 1;
	else if (rela_sym == neqsym)
		return 1;
	else if (rela_sym == lessym)
		return 1;
	else if (rela_sym == leqsym)
		return 1;
	else if (rela_sym == gtrsym)
		return 1;
	else if (rela_sym == geqsym)
		return 1;
	else
		return 0;
}

void errorend(int x)
{
	switch (x)
	{
		case 1:
			printf("Parser Error: Competing Symbol Declarations\n");
			break;
		case 2:
			printf("Parser Error: Unrecognized Statement Form\n");
			break;
		case 3:
			printf("Parser Error: Programs Must Close with a Period\n");
			break;
		case 4:
			printf("Parser Error: Symbols Must Be Declared with an Identifier\n");
			break;
		case 5:
			printf("Parser Error: Constants Must Be Assigned a Value at Declaration\n");
			break;
		case 6:
			printf("Parser Error: Symbol Declarations Must Be Followed By a Semicolon\n");
			break;
		case 7:
			printf("Parser Error: Undeclared Symbol\n");
			break;
		case 8:
			printf("Parser Error: while Must Be Followed By do\n");
			break;
		case 9:
			printf("Parser Error: if Must Be Followed By then\n");
			break;
		case 10:
			printf("Parser Error: begin Must Be Followed By end\n");
			break;
		case 11:
			printf("Parser Error: while and if Statements Must Contain Conditions\n");
			break;
		case 12:
			printf("Parser Error: Conditions Must Contain a Relational-Operator\n");
			break;
		case 13:
			printf("Parser Error: ( Must Be Followed By )\n");
			break;
		case 14:
			printf("Parser Error: call and read Must Be Followed By an Identifier\n");
			break;
		default:
			printf("Implementation Error: Unrecognized Error Code\n");
			break;
	}

}

void printtable()
{
	int i;
	printf("Symbol Table:\n");
	printf("Kind | Name        | Value | Level | Address\n");
	printf("------------------------------------------------------\n");
	for (i = 0; i < sym_index; i++)
		printf("%4d | %11s | %5d | %5d | %5d\n", table[i].kind, table[i].name, table[i].val, table[i].level, table[i].addr);
}
