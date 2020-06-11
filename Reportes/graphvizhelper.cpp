#include "graphvizhelper.h"

#include<fstream>
#include<cstdio>

//PATH NO DEBE DE INCLUIR LA EXTENCION DEL ARCHIVO SOLO EL NOMBRE!
int GraphvizHelper::writeAndCompileDot(const std::string &path, const std::string &dotCode, bool open)
{
    if (writeDot(path, dotCode) == -1) {
        return -1;
    }
    if (compileDot(path, open) == -1) {
        return  -1;
    }
    return 0;
}

//PATH NO DEBE DE INCLUIR LA EXTENCION DEL ARCHIVO SOLO EL NOMBRE!
int GraphvizHelper::writeDot(const std::string &path, const std::string &dotCode)
{
    std::ofstream file(path + ".dot");
    if (!file.is_open()) {
        std::string dirPath = path.substr(0, path.find_last_of('/'));
        int status = system((std::string("mkdir -p ") + dirPath).c_str());
        if(status != 0){
            return -1;
        }
        file.open(path + ".dot");
    }
    file << dotCode << std::endl;
    file.close();
    return 0;
}

//solo funciona en linux
//PATH NO DEBE DE INCLUIR LA EXTENCION DEL ARCHIVO SOLO EL NOMBRE!
int GraphvizHelper::compileDot(const std::string &path, bool open)
{
    //No creamos carpetar recursivas porque involucraria una segunda llamada al sistema
    std::string command = "dot -Tpng '" + path + ".dot' -o '" + path + ".png'";
    system(command.c_str());//TODO. Ver que retorna esto en linux cuando hay error

    if (open)
    {
        command = "nohup display '" + path + ".png' &";
        system(command.c_str());
    }

    return 0;
}
