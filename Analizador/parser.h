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

#ifndef YY_YY_PARSER_H_INCLUDED
# define YY_YY_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 1
#endif
#if YYDEBUG
extern int yydebug;
#endif
/* "%code requires" blocks.  */
#line 1 "parser.yy" /* yacc.c:1909  */

  #include <string>

#line 48 "parser.h" /* yacc.c:1909  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    MK_DISK = 258,
    SIZE = 259,
    PATH = 260,
    UNIT = 261,
    FIT = 262,
    NAME = 263,
    RM_DISK = 264,
    F_DISK = 265,
    TYPE = 266,
    DELETE = 267,
    ADD = 268,
    MOUNT = 269,
    UNMOUNT = 270,
    PAUSE = 271,
    REP = 272,
    ID = 273,
    RUTA = 274,
    MK_FS = 275,
    LOGIN = 276,
    MK_GRP = 277,
    RM_GRP = 278,
    MK_USR = 279,
    RM_USR = 280,
    CHMOD = 281,
    MK_FILE = 282,
    CAT = 283,
    REM = 284,
    EDIT = 285,
    REN = 286,
    MK_DIR = 287,
    CP = 288,
    MV = 289,
    FIND = 290,
    CHOWN = 291,
    CHGRP = 292,
    LOSS = 293,
    RECOVERY = 294,
    FS = 295,
    USR = 296,
    PWD = 297,
    GRP = 298,
    UGO = 299,
    R = 300,
    P = 301,
    CONT = 302,
    _FILE = 303,
    DEST = 304,
    LOGOUT = 305,
    EQUALS = 306,
    LINE_BREAK = 307,
    INTEGER = 308,
    TEXT = 309,
    PART_TYPE = 310,
    UNIT_TYPE = 311,
    FIT_TYPE = 312,
    DELETE_TYPE = 313,
    FS_TYPE = 314
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
#line 32 "parser.yy" /* yacc.c:1909  */
union SymbolType
{
#line 32 "parser.yy" /* yacc.c:1909  */

    int intValue;
    std::string* stringPtrValue;

#line 125 "parser.h" /* yacc.c:1909  */
};
#line 32 "parser.yy" /* yacc.c:1909  */
typedef union SymbolType YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


extern YYSTYPE yylval;
extern YYLTYPE yylloc;
int yyparse (void);

#endif /* !YY_YY_PARSER_H_INCLUDED  */
