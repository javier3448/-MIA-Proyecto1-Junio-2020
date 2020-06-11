#!/bin/bash
function pause(){
   read -p "$*"
}

clear
pause 'Presione [ENTER] para generar el Lexico...\\n'
echo -e
echo "generando el lexico ..."
flex scanner.l
ls -l scanner*
echo -e
pause '\nPresione [ENTER] para generar el sintactico...'
echo -e
echo "generando el sintactico ..."
bison -t -d -v parser.yy
ls -l parser*
