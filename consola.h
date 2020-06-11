#ifndef CONSOLA_H
#define CONSOLA_H

#include <string>
#include <iostream>

namespace Consola
{
    void reportarError(const std::string& msg);
    void reportarError(int line, int column, std::string msg);
    void reportarLinea(int line, int column);//Chapuz
    void reportarComando(const std::string& cmdName);
    void printLine(const std::string& msg);
    void printCommandLine(const std::string& msg);
    void pause();
    int readInt();
}

#endif // CONSOLA_H
