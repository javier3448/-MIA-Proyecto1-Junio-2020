%option case-insensitive
%option yylineno
%{
#include <string>
#include "Analizador/parser.h"
#include "consola.h"

int yylex();
void yyerror(const char *msg);
int line=0;
int column = 0;
int previous = 0;
%}

%option header-file="scanner.h"
%option outfile="scanner.cpp"
%option noyywrap
%option nounput


InputCharacter  [^\r\n]
ContinueNextLine \\\^\n

LineTerminator  \r|\n|\r\n

LineComment "#"{InputCharacter}*{LineTerminator}?

Integer -?[0-9]+
Text [_[:alnum:]/][_[:alnum:]/\.]*
partType "p"|"e"|"l"
fitType "ff"|"wf"|"bf"
unitType "b"|"k"|"m"
deleteType "fast"|"full"
fsType "2fs"|"3fs"
equals "="


%x STRING_STATE

%%
<INITIAL>
{
    "\"" { column++; yylval.stringPtrValue = new std::string(""); BEGIN STRING_STATE;}

    "mkdisk" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return MK_DISK; }
    "-size" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return SIZE; }
    "-path" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return PATH; }
    "-name" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return NAME; }
    "-unit" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return UNIT; }
    "-fit" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return FIT; }
    "rmDisk" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return RM_DISK; }
    "fDisk" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return F_DISK; }
    "-type"  { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return TYPE; }
    "-delete"  { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return DELETE; }
    "-add"  { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return ADD; }
    "-id"[0-9]*  { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return ID; }

    "mount" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return MOUNT; }
    "unmount" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return UNMOUNT; }
    "pause"  { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return PAUSE; }
    "rep" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return REP; }

    "mkfs" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return MK_FS; }
    "login" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return LOGIN; }
    "logout" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return LOGOUT; }
    "mkgrp" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return MK_GRP; }
    "rmgrp" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return RM_GRP; }
    "mkusr" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return MK_USR; }
    "rmusr" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return RM_USR; }
    "chmod" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return CHMOD; }
    "mkfile" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return MK_FILE; }
    "cat" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return CAT; }
    "rem" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return REM; }
    "edit" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return EDIT; }
    "ren" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return REN; }
    "mkdir" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return MK_DIR; }
    "cp" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return CP; }
    "mv" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return MV; }
    "find" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return FIND; }
    "chown" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return CHOWN; }
    "chgrp" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return CHGRP; }

    "-usr" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return USR; }
    "-pwd" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return PWD; }
    "-grp" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return GRP; }
    "-ugo" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return UGO; }
    "-r" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return R; }
    "-p" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return P; }
    "-cont" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return CONT; }
    "-file" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return _FILE; }
    "-dest" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return DEST; }
    "-ruta" { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return RUTA; }

    {equals} { column = column + previous; previous = strlen(yytext); yylval.intValue = -1; return EQUALS; }
    {LineTerminator} {line++; column=0; previous=0; }
    {LineComment} {line++; column=0; previous=0; }

    {unitType} { column = column + previous; previous = strlen(yytext); yylval.stringPtrValue = new std::string(yytext); return UNIT_TYPE;}
    {partType} { column = column + previous; previous = strlen(yytext); yylval.stringPtrValue = new std::string(yytext); return PART_TYPE;}
    {fitType} { column = column + previous; previous = strlen(yytext); yylval.stringPtrValue = new std::string(yytext); return FIT_TYPE;}
    {deleteType} { column = column + previous; previous = strlen(yytext); yylval.stringPtrValue = new std::string(yytext); return DELETE_TYPE;}
    {Integer} { column = column + previous; previous = strlen(yytext); yylval.intValue = std::stoi(yytext); return INTEGER;}
    {Text} { column = column + previous; previous = strlen(yytext); yylval.stringPtrValue = new std::string(yytext); return TEXT;}

    [[:blank:]] { column++; }
    {ContinueNextLine} { line++; column=0; previous=0; }
}

<STRING_STATE>
{
    "\"" {column++; BEGIN INITIAL; return TEXT;}
    [^\n\r\"\\]+ { column = column + previous; previous = strlen(yytext); *(yylval.stringPtrValue) += yytext; }//Performance--?; Talvez sea mejor incluir caracter por caracter no se
    "\\t" { column = column + previous; previous = strlen(yytext); *(yylval.stringPtrValue) += "\t"; }
    "\\n" { column = column + previous; previous = strlen(yytext); *(yylval.stringPtrValue) += "\n"; }
    "\\r" { column = column + previous; previous = strlen(yytext); *(yylval.stringPtrValue) += "\r"; }
    {LineTerminator}  {
                          yyerror("unclosed string");
                          std::string tmp("Caracter: ");//Chapuz porque no concatenaba
                          tmp += yytext;
                          tmp += " no valido adentro de string";
                          Consola::reportarError(yylineno + 1, column + 1,  tmp);
                          column = 0;
                          line++;
                          BEGIN INITIAL;
                          return TEXT;
                      }
    {ContinueNextLine} { line++; column=0; previous=0; }
}

%%

