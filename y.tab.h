/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    XOR = 258,
    AND = 259,
    OR = 260,
    GE = 261,
    LE = 262,
    EQ = 263,
    NE = 264,
    FALSE = 265,
    TRUE = 266,
    IF = 267,
    ELSE = 268,
    QUES = 269,
    DOT = 270,
    WHILE = 271,
    PRINT = 272,
    LOOP = 273,
    BOOL_TYPE = 274,
    INT_TYPE = 275,
    ID = 276,
    VAL = 277,
    IF_ALONE = 278,
    MINUS = 279,
    PLUS = 280
  };
#endif
/* Tokens.  */
#define XOR 258
#define AND 259
#define OR 260
#define GE 261
#define LE 262
#define EQ 263
#define NE 264
#define FALSE 265
#define TRUE 266
#define IF 267
#define ELSE 268
#define QUES 269
#define DOT 270
#define WHILE 271
#define PRINT 272
#define LOOP 273
#define BOOL_TYPE 274
#define INT_TYPE 275
#define ID 276
#define VAL 277
#define IF_ALONE 278
#define MINUS 279
#define PLUS 280

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 24 "parser.y" /* yacc.c:1909  */

  int value;
  size_t id;
  struct expr *expr;
  enum value_type {
    ERROR = -1,
    UNTYPED = 0,
    INTEGER = 1,
    BOOLEAN = 2,
  } type;
  struct stmt *stmt;

#line 117 "y.tab.h" /* yacc.c:1909  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void *module, void *builder);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
