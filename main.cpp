#include <iostream>
#include <fstream>

#include "consola.h"
#include "Analizador/interprete.h"
#include "FileSystem/journaling.h"

//DEBUG INCLUDES:
#include "mystringutil.h"

//revisa si es comando exec y lo ejecuta. Si no es comando exec retorna falso y no ejecuta nada. CHAPUZ MAXIMO
bool tryExec(const std::string userInput){
    static const std::string exec = "exec";

    std::string source = userInput.substr(0, userInput.find_first_of("\n"));

    if (source.length() < exec.length() ||
        source.find_first_of("\"") == std::string::npos ||
        source.find_last_of("\"") - source.find_first_of("\"") <= 1) {
        return false;
    }

    std::string sourceExec = source.substr(0, exec.length());
    std::transform(sourceExec.begin(), sourceExec.end(), sourceExec.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    if (sourceExec != exec) {
        return false;
    }

    Consola::printCommandLine("EXEC: ");

    ulong firstQuote = source.find_first_of("\"");
    ulong lastQuote = source.find_last_of("\"") - 1;
    std::string path = source.substr(firstQuote + 1, lastQuote - firstQuote);

    std::string content;
    std::ifstream file(path);

    if (!file.is_open()) {
        Consola::reportarError("Error al ejecutar exec. El directorio: " + path + " no se puede abrir");
        Consola::reportarLinea(0, 0);
        return true;
    }

    content.assign((std::istreambuf_iterator<char>(file)),
                   (std::istreambuf_iterator<char>()));

    Interprete::compile(content);
    return true;
}

int main()
{
    srand(time(nullptr));

    std::string userInput = "EXIT";
    do
    {
        std::getline(std::cin, userInput);

        //Chapuz Maximo,
        if (!tryExec(userInput)) {
            Interprete::compile(userInput);
        }
    }while (userInput != "EXIT");
}
