// Jiahao Zhu
// Shibo Ding

#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Size of a stack representing all activation records */
#define STACK_SIZE 16

symbol *table;
int sym_index;
int error;

/* The list data structure to represent symbols for an activation record */
typedef struct symlist
{
    /* The array to store all symbols */
    symbol symbols[128];
    /* Number of symbols */
    int length;
    /* The M address */
    int addr;
} symlist;

/* A list of tokens returned by the lexer */
lexeme *tokens;
/* Current token being analysed */
lexeme *tok;
/* Activation record stack */
symlist symstack[STACK_SIZE];
/* Stack top */
int top;

void printtable();
void errorend(int x);

/* Helper function prototypes */

/* Functions associated with each non-terminal */
void program();
void block();
void constdec();
void constinit();
void vardec();
void procdec();
void statement();
void asstat();
void callstat();
void body();
void ifstat();
void whilestat();
void readstat();
void writestat();
void condition();
void expression();
void term();
void factor();
lexeme *scan();
/* Match/accept tokens */
int match(token_type expected);
/* Check if the name already exists in current activation record */
int exist(const char *name);
/* Check if the name is already declared */
int declared(const char *name);
/* Add main procedure by default to the symbol table */
void addmain();
/* Add consts to symbol table */
void addconst(char *name, int val);
/* Add variables to symbol table */
void addvar(char *name);
/* Add procedures to symbol table */
void addproc(char *name);

symbol *parse(lexeme *input)
{
    /* Initialize the globals */
    table = calloc(1000, sizeof(symbol));
    sym_index = 0;
    error = 0;

    tokens = input;
    for (int i = 0; i < STACK_SIZE; i++)
    {
        symstack[i].addr = 3;
    }
    top = -1;

    /* Start the parsing process */
    tok = scan();
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
    exit(0);
}

void printtable()
{
    int i;
    printf("Symbol Table:\n");
    printf("Kind | Name        | Value | Level | Address\n");
    printf("--------------------------------------------\n");
    for (i = 0; i < sym_index; i++)
        printf("%4d | %11s | %5d | %5d | %5d\n",
               table[i].kind,
               table[i].name,
               table[i].val,
               table[i].level,
               table[i].addr);
}

// Helper function implementations
void program()
{
    switch (tok->type)
    {
    case constsym:
    case varsym:
    case procsym:
    case identsym:
    case callsym:
    case beginsym:
    case ifsym:
    case whilesym:
    case readsym:
    case writesym:
        addmain();
        block();
        if (!match(periodsym))
            errorend(3);
        break;
    default:
        errorend(0);
    }
}

void block()
{
    switch (tok->type)
    {
    case constsym:
    case varsym:
    case procsym:
    case identsym:
    case callsym:
    case beginsym:
    case ifsym:
    case whilesym:
    case readsym:
    case writesym:
        constdec();
        vardec();
        procdec();
        statement();
        break;
    default:
        errorend(0);
    }
}

void constdec()
{
    switch (tok->type)
    {
    case constsym:
        match(constsym);
        constinit();
        while (tok->type == commasym)
        {
            match(commasym);
            constinit();
        }
        if (!match(semicolonsym))
            errorend(6);
        break;
    case varsym:
    case procsym:
    case identsym:
    case callsym:
    case beginsym:
    case ifsym:
    case whilesym:
    case readsym:
    case writesym:
        break;
    default:
        errorend(0);
    }
}

void constinit()
{
    char *name = tok->name;
    if (!match(identsym))
    {
        errorend(4);
    }
    if (!match(becomessym))
    {
        errorend(5);
    }
    int val = tok->value;
    if (!match(numbersym))
    {
        errorend(5);
    }
    if (exist(name))
        errorend(1);
    addconst(name, val);
}

void vardec()
{
    switch (tok->type)
    {
    case varsym:
        match(varsym);
        if (exist(tok->name))
            errorend(1);
        addvar(tok->name);
        if (!match(identsym))
            errorend(4);
        while (tok->type == commasym)
        {
            match(commasym);
            if (exist(tok->name))
                errorend(1);
            addvar(tok->name);
            if (!match(identsym))
                errorend(4);
        }
        if (!match(semicolonsym))
            errorend(6);
        break;
    case procsym:
    case identsym:
    case callsym:
    case beginsym:
    case ifsym:
    case whilesym:
    case readsym:
    case writesym:
        break;
    default:
        errorend(0);
    }
}

void procdec()
{
    switch (tok->type)
    {
    case procsym:
        while (tok->type == procsym)
        {
            match(procsym);
            if (exist(tok->name))
                errorend(1);
            addproc(tok->name);
            if (!match(identsym))
                errorend(4);
            if (!match(semicolonsym))
                errorend(6);
            block();
            if (!match(semicolonsym))
                errorend(6);
            /*
             * The activation record must be popped after procedure declaration
             * ends
             */
            --top;
        }
        break;
    case identsym:
    case callsym:
    case beginsym:
    case ifsym:
    case whilesym:
    case readsym:
    case writesym:
        break;
    default:
        errorend(0);
    }
}

void statement()
{
    switch (tok->type)
    {
    case identsym:
        asstat();
        break;
    case callsym:
        callstat();
        break;
    case beginsym:
        body();
        break;
    case ifsym:
        ifstat();
        break;
    case whilesym:
        whilestat();
        break;
    case readsym:
        readstat();
        break;
    case writesym:
        writestat();
        break;
    case periodsym:
    case semicolonsym:
    case endsym:
    case elsesym:
        break;
    default:
        errorend(2);
    }
}

void asstat()
{
    char *name = tok->name;
    if (!declared(name))
        errorend(7);
    match(identsym);
    if (!match(becomessym))
        errorend(7);
    expression();
}

void callstat()
{
    match(callsym);
    char *name = tok->name;
    if (!match(identsym))
        errorend(14);
    if (!declared(name))
        errorend(7);
}

void body()
{
    match(beginsym);
    statement();
    while (tok->type == semicolonsym)
    {
        match(semicolonsym);
        statement();
    }
    if (!match(endsym))
        errorend(10);
}

void ifstat()
{
    match(ifsym);
    condition();
    if (!match(thensym))
        errorend(9);
    statement();
    if (tok->type == elsesym)
    {
        match(elsesym);
        statement();
    }
}

void whilestat()
{
    match(whilesym);
    condition();
    if (!match(dosym))
        errorend(8);
    statement();
}

void readstat()
{
    match(readsym);
    char *name = tok->name;
    if (!match(identsym))
        errorend(14);
    if (!declared(name))
        errorend(7);
}

void writestat()
{
    match(writesym);
    expression();
}

void condition()
{
    switch (tok->type)
    {
    case oddsym:
        match(oddsym);
        expression();
        break;
    case numbersym:
    case identsym:
    case lparentsym:
    case plussym:
    case minussym:
    {
        expression();
        switch (tok->type)
        {
        case eqlsym:
        case neqsym:
        case lessym:
        case leqsym:
        case gtrsym:
        case geqsym:
            match(tok->type);
            break;
        default:
            errorend(12);
        }
        expression();
    }
    break;
    default:
        errorend(11);
    }
}

void expression()
{
    switch (tok->type)
    {
    case numbersym:
    case identsym:
    case lparentsym:
    case plussym:
    case minussym:
        term();
        while (tok->type == plussym || tok->type == minussym)
        {
            match(tok->type);
            term();
        }
        break;
    default:
        errorend(0);
    }
}

void term()
{
    switch (tok->type)
    {
    case numbersym:
    case identsym:
    case lparentsym:
    case plussym:
    case minussym:
        factor();
        while (tok->type == multsym || tok->type == slashsym || tok->type == modsym)
        {
            match(tok->type);
            factor();
        }
        break;
    default:
        errorend(0);
    }
}

void factor()
{
    switch (tok->type)
    {
    case numbersym:
    case identsym:
        match(tok->type);
        break;
    case lparentsym:
        match(lparentsym);
        expression();
        if (!match(rparentsym))
            errorend(13);
        break;
    case plussym:
    case minussym:
        match(tok->type);
        expression();
        break;
    default:
        errorend(0);
    }
}

lexeme *scan()
{
    static int i = 0;
    if (tokens[i].type != 0)
        return &tokens[i++];
    return NULL;
}

int match(token_type expected)
{
    if (tok->type == expected)
    {
        tok = scan();
        return 1;
    }
    error = 1;
    return 0;
}

int exist(const char *name)
{
    symlist *list = &symstack[top];
    for (int i = 0; i < list->length; i++)
    {
        if (strcmp(name, list->symbols[i].name) == 0)
            return 1;
    }

    return 0;
}

int declared(const char *name)
{
    int sp = top;
    /* Trace the activation record stack to find if it is declared */
    while (sp >= 0)
    {
        symlist *list = &symstack[sp];
        for (int i = 0; i < list->length; i++)
        {
            if (strcmp(name, list->symbols[i].name) == 0)
                return 1;
        }
        sp--;
    }

    return 0;
}

void addmain()
{
    /* The main procedure should be added by default */
    table[sym_index].kind = 3;
    strcpy(table[sym_index].name, "main");
    table[sym_index].level = 0;

    sym_index++;
    top = 0;
}

void addconst(char *name, int val)
{
    table[sym_index].kind = 1;
    strcpy(table[sym_index].name, name);
    table[sym_index].val = val;
    table[sym_index].level = top;

    /* The const should be added to the activation record as well for checking */
    symlist *list = &symstack[top];
    list->symbols[list->length++] = table[sym_index];

    sym_index++;
}

void addvar(char *name)
{
    symlist *list = &symstack[top];

    table[sym_index].kind = 2;
    strcpy(table[sym_index].name, name);
    table[sym_index].level = top;
    table[sym_index].addr = list->addr++;

    /* The vars should be added to the activation record as well for checking */
    list->symbols[list->length++] = table[sym_index];

    sym_index++;
}

void addproc(char *name)
{
    symlist *list = &symstack[top];

    table[sym_index].kind = 3;
    strcpy(table[sym_index].name, name);
    table[sym_index].level = top;

    /* The procedures should be added to the activation record as well for checking */
    list->symbols[list->length++] = table[sym_index];

    sym_index++;

    /* The new procedure must be pushed into the stack */
    top++;
}
