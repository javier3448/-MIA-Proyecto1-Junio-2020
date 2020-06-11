#include "consola.h"

#include<limits>

using std::string;
using std::cout;
using std::endl;

void Consola::reportarError(const std::string& msg)
{
    std::cout << "\033[1;31m" + msg + " \033[0m" << std::endl;
}

void Consola::reportarError(int line, int column, std::string msg)
{
    std::cout << "\033[1;31m" <<
                 msg <<
                 "\n    Linea: " << line <<
                 "\n    Column: " << column <<
                 " \033[0m" << std::endl;
}

void Consola::reportarLinea(int line, int column)
{
    std::cout << "\033[1;31m" <<
                 "\n    Linea: " << line <<
                 "\n    Column: " << column <<
                 " \033[0m" << std::endl;
}

void Consola::reportarComando(const std::string& cmdName)
{
    std::cout << "\033[1;31m" <<
        "\n    Error al ejecutar Comando: " << cmdName <<
        " \033[0m" << std::endl;
}


void Consola::printLine(const std::string&msg)
{
    std::cout << msg << std::endl;
}

void Consola::printCommandLine(const std::string& msg)
{
    std::cout << "\033[1;32m" + msg + " \033[0m" << std::endl;
}

void Consola::pause()
{
    std::cin.get();
}

int Consola::readInt(){//TODO: Testear si se traba como en vacas de junio
    int userInput = -1;
    std::cin >> userInput;
    while(1)
    {
        if(std::cin.fail())
        {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            std::cin>>userInput;
        }
        if(!std::cin.fail())
            break;
    }
    return userInput;
}
