/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

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

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     STRING = 258,
     FLOAT = 259,
     INTEGER = 260,
     BOOLEAN = 261,
     CONST = 262,
     TRUE = 263,
     FALSE = 264,
     STR_LITERAL = 265,
     VOID = 266,
     IDENTIFIER = 267,
     SEMICOLON = 268,
     EQUALS = 269,
     COMMA = 270,
     OPENSQR_BRACKET = 271,
     CLOSESQR_BRACKET = 272,
     OPENRND_BRACKET = 273,
     CLOSERND_BRACKET = 274,
     INT_DIGIT = 275,
     FLT_DIGIT = 276,
     IF = 277,
     END_IF = 278,
     WHILE = 279,
     END_WHILE = 280,
     MULTIPLY = 281,
     DIVIDE = 282,
     LESS_THAN = 283,
     LESS_THAN_EQUAL = 284,
     GREATER_THAN = 285,
     GREATER_THAN_EQUAL = 286,
     LOGICAL_NOT = 287,
     NOT_EQUAL = 288,
     LOGICAL_AND = 289,
     LOGICAL_OR = 290,
     CALL = 291,
     RETURN = 292,
     FUNCTION = 293,
     RETURNS = 294,
     END_FUNCTION = 295,
     MINUS = 296,
     PLUS = 297,
     NEG = 298
   };
#endif
/* Tokens.  */
#define STRING 258
#define FLOAT 259
#define INTEGER 260
#define BOOLEAN 261
#define CONST 262
#define TRUE 263
#define FALSE 264
#define STR_LITERAL 265
#define VOID 266
#define IDENTIFIER 267
#define SEMICOLON 268
#define EQUALS 269
#define COMMA 270
#define OPENSQR_BRACKET 271
#define CLOSESQR_BRACKET 272
#define OPENRND_BRACKET 273
#define CLOSERND_BRACKET 274
#define INT_DIGIT 275
#define FLT_DIGIT 276
#define IF 277
#define END_IF 278
#define WHILE 279
#define END_WHILE 280
#define MULTIPLY 281
#define DIVIDE 282
#define LESS_THAN 283
#define LESS_THAN_EQUAL 284
#define GREATER_THAN 285
#define GREATER_THAN_EQUAL 286
#define LOGICAL_NOT 287
#define NOT_EQUAL 288
#define LOGICAL_AND 289
#define LOGICAL_OR 290
#define CALL 291
#define RETURN 292
#define FUNCTION 293
#define RETURNS 294
#define END_FUNCTION 295
#define MINUS 296
#define PLUS 297
#define NEG 298




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef int YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

