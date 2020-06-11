%code requires {
  #include <string>
}
%{

    #include "Analizador/scanner.h"//se importa el header del analisis sintactico
    #include "consola.h"
    #include "Analizador/extparams.h"
    #include "Analizador/interprete.h"

    extern int yylineno; //linea actual donde se encuentra el parser (analisis lexico) lo maneja BISON
    extern int column; //columna actual donde se encuentra el parser (analisis lexico) lo maneja BISON
    extern char *yytext; //lexema actual donde esta el parser (analisis lexico) lo maneja BISON

    ExtParams* lwhParams;

    int yyerror(const char* msg)
    {
        Consola::reportarError(yylineno + 1, column + 1, msg);
        return -1;
    }
%}
%initial-action
{
    lwhParams = new ExtParams();
}
%defines "parser.h"
%output "parser.cpp"
%define parse.error verbose
%locations

%union SymbolType{
    int intValue;
    std::string* stringPtrValue;
}

//-----------TERMINALS-----------
//Keywords
%token<intValue> MK_DISK;
%token<intValue> SIZE;
%token<intValue> PATH;
%token<intValue> UNIT;
%token<intValue> FIT;
%token<intValue> NAME;
%token<intValue> RM_DISK;
%token<intValue> F_DISK;
%token<intValue> TYPE;
%token<intValue> DELETE;
%token<intValue> ADD;
%token<intValue> MOUNT;
%token<intValue> UNMOUNT;
%token<intValue> PAUSE;
%token<intValue> REP;
%token<intValue> ID;
%token<intValue> RUTA;

    //Keywords fase 2
%token<intValue> MK_FS;
%token<intValue> LOGIN;
%token<intValue> MK_GRP;
%token<intValue> RM_GRP;
%token<intValue> MK_USR;
%token<intValue> RM_USR;
%token<intValue> CHMOD;
%token<intValue> MK_FILE;
%token<intValue> CAT;
%token<intValue> REM;
%token<intValue> EDIT;
%token<intValue> REN;
%token<intValue> MK_DIR;
%token<intValue> CP;
%token<intValue> MV;
%token<intValue> FIND;
%token<intValue> CHOWN;
%token<intValue> CHGRP;
%token<intValue>USR;
%token<intValue>PWD;
%token<intValue>GRP;
%token<intValue>UGO;
%token<intValue>R;
%token<intValue>P;
%token<intValue>CONT;
%token<intValue>_FILE;
%token<intValue>DEST;
%token<intValue>LOGOUT;
//Punctuation
%token<intValue>EQUALS;
%token<intValue>LINE_BREAK;
//Literals
%token<intValue> INTEGER;
%token<stringPtrValue> TEXT;
%token<stringPtrValue> PART_TYPE;
%token<stringPtrValue> UNIT_TYPE;
%token<stringPtrValue> FIT_TYPE;
%token<stringPtrValue> DELETE_TYPE;

//---------NON TERMINALS---------
%start S;

%type<intValue> l_commands;
%type<intValue> command;
%type<intValue> l_params;
%type<intValue> param;
%type<stringPtrValue> _type;
%%

S:  l_commands
    {
        delete(lwhParams);
    }
;

l_commands: command l_commands
            |
            command
;

command: MK_DISK l_params
         {
             Consola::printCommandLine("MK_DISK: ");
             if(Interprete::mkDisk(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         RM_DISK l_params
         {
             Consola::printCommandLine("RM_DISK: ");
             if(Interprete::rmDisk(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         F_DISK l_params
         {
             Consola::printCommandLine("F_DISK: ");
             if(Interprete::fDisk(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         MOUNT l_params
         {
             Consola::printCommandLine("MOUNT: ");
             if(Interprete::mount(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         UNMOUNT l_params
         {
             Consola::printCommandLine("UNMOUNT: ");
             if(Interprete::unmount(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         PAUSE
         {
             Consola::printCommandLine("PAUSE: ");
             if(Interprete::pause(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         REP l_params
         {
             Consola::printCommandLine("REP: ");
             if(Interprete::rep(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         MK_FS l_params
         {
             Consola::printCommandLine("MK_FS: ");
             if(Interprete::mkfs(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         LOGIN l_params
         {
             Consola::printCommandLine("LOGIN: ");
             if(Interprete::login(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         LOGOUT
         {
             Consola::printCommandLine("LOGOUT: ");
             if(Interprete::logout(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         MK_GRP l_params
         {
             Consola::printCommandLine("MK_GROUP: ");
             if(Interprete::mkgrp(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         RM_GRP l_params
         {
             Consola::printCommandLine("RM_GROP: ");
             if(Interprete::rmgrp(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         MK_USR l_params
         {
             Consola::printCommandLine("MK_USR: ");
             if(Interprete::mkusr(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         RM_USR l_params
         {
             Consola::printCommandLine("RM_USR: ");
             if(Interprete::rmusr(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         CHMOD l_params
         {
             Consola::printCommandLine("CHMOD: ");
             if(Interprete::chmod(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         MK_FILE l_params
         {
             Consola::printCommandLine("MK_FILE: ");
             if(Interprete::mkfile(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         CAT l_params
         {
             Consola::printCommandLine("CAT: ");
             if(Interprete::cat(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         REM l_params
         {
             Consola::printCommandLine("REM: ");
             if(Interprete::rem(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         EDIT l_params
         {
             Consola::printCommandLine("EDIT: ");
             if(Interprete::edit(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         REN l_params
         {
             Consola::printCommandLine("REN: ");
             if(Interprete::ren(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         MK_DIR l_params
         {
             Consola::printCommandLine("MK_DIR: ");
             if(Interprete::mkdir(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         CP l_params
         {
             Consola::printCommandLine("CP: ");
             if(Interprete::cp(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         MV l_params
         {
             Consola::printCommandLine("MV: ");
             if(Interprete::mv(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         FIND l_params
         {
             Consola::printCommandLine("FIND: ");
             if(Interprete::find(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         CHOWN l_params
         {
             Consola::printCommandLine("CHOWN: ");
             if(Interprete::chown(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
         |
         CHGRP l_params
         {
             Consola::printCommandLine("CHGRP: ");
             if(Interprete::chgrp(lwhParams) < 0){
                 Consola::reportarLinea(yylineno, column);
             }else{
                 Consola::printCommandLine("    DONE");
             }
             lwhParams->reset();
         }
;

l_params: param l_params
          |
          param
;

param: SIZE EQUALS INTEGER
       {
           lwhParams->setSize($3);
       }
       |
       PATH EQUALS TEXT
       {
           lwhParams->setPath($3);
       }
       |
       NAME EQUALS TEXT
       {
           lwhParams->setName($3);
       }
       |
       UNIT EQUALS UNIT_TYPE
       {
           lwhParams->setUnit($3);
       }
       |
       FIT EQUALS FIT_TYPE
       {
           lwhParams->setFitType($3);
       }
       |
       TYPE EQUALS _type
       {
           lwhParams->setType($3);
       }
       |
       DELETE EQUALS DELETE_TYPE
       {
           lwhParams->setDeleteType($3);
       }
       |
       ADD EQUALS INTEGER
       {
           lwhParams->setAdd($3);
       }
       |
       ID EQUALS TEXT
       {
           lwhParams->setId($3);
       }
       |
       USR EQUALS TEXT
       {
           lwhParams->setUsr($3);
       }
       |
       PWD EQUALS TEXT
       {
           lwhParams->setPwd($3);
       }
       |
       GRP EQUALS TEXT
       {
           lwhParams->setGrp($3);
       }
       |
       UGO EQUALS INTEGER
       {
           lwhParams->setUgo($3);
       }
       |
       R
       {
           lwhParams->setR(true);
       }
       |
       P
       {
           lwhParams->setP(true);
       }
       |
       CONT EQUALS TEXT
       {
           lwhParams->setCont($3);
       }
       |
       _FILE EQUALS TEXT
       {
           lwhParams->setFiles($3);
       }
       |
       DEST EQUALS TEXT
       {
           lwhParams->setDest($3);
       }
       |
       RUTA EQUALS TEXT
       {
           lwhParams->setRuta($3);
       }
;

_type: DELETE_TYPE
       {
           $$ = $1;
       }
       |
       PART_TYPE
       {
           $$ = $1;
       }
;
%%
