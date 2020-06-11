#ifndef EXTPARAMS_H
#define EXTPARAMS_H

#include<string>
#include<vector>
#include<bits/stdc++.h>
#include"Analizador/parser.h"

//Creo que esto lo deberiamos de definir desde un archivo con nombre mas o menos
//DiskConstants y FileSystemConstants
namespace ExtParamsConstants {
    const char PART_TYPE_P = 'p';
    const char PART_TYPE_E = 'e';
    const char PART_TYPE_L = 'l';

    const char FIT_TYPE_FF = 'f';
    const char FIT_TYPE_WF = 'w';
    const char FIT_TYPE_BF = 'b';

    const char UNIT_TYPE_B = 'b';
    const char UNIT_TYPE_K = 'k';
    const char UNIT_TYPE_M = 'm';

    const char DELETE_TYPE_FAST = 'q';
    const char DELETE_TYPE_FULL = 'f';

    const char FS_EXT2 = 2;
    const char FS_EXT3 = 3;

    const char FORMAT_TYPE_FAST = 'q';
    const char FORMAT_TYPE_FULL = 'f';
}

class ExtParams
{
public:
    bool r = false;
    bool p = false;
    bool rf = false;
    char unit = '\0';
    char fitType = '\0';
    char deleteType = '\0';
    char type = '\0';//partType, formatType. CHAPUZ MAXIMO. Puede representar el char que especifica el tipo de particion, o en caso de ser una instruccion de formateo el tipo de formato
    char fs = '\0';
    int size = -1;
    int add = INT_MIN;//Chapuz int min va a representar el NULL
    int ugo = -1;
    std::string* id = nullptr;
    std::string* file = nullptr;
    std::string* path = nullptr;//Talvez aqui sea nullptr por default, no estoy seguro
    std::string* name = nullptr;
    std::string* usr = nullptr;
    std::string* pwd = nullptr;
    std::string* grp = nullptr;
    std::string* cont = nullptr;
    std::string* ruta = nullptr;
    std::string* dest = nullptr;


    void deepClearVector(std::vector<std::string*> &vector);

    void setR(int val);
    void setP(int val);
    void setRf(int val);
    void setUnit(std::string* val);
    void setFitType(std::string* val);
    void setDeleteType(std::string* val);
    void setType(std::string* val);
    void setSize(int val);
    void setUgo(int val);
    void setId(std::string* val);
    void setFiles(std::string* val);
    void setPath(std::string* val);
    void setName(std::string* val);
    void setUsr(std::string* val);
    void setPwd(std::string* val);
    void setCont(std::string* val);
    void setRuta(std::string* val);
    void setAdd(int val);

    //nuevos:
    void setFs(char val);
    void setGrp(std::string* val);
    void setDest(std::string* val);

    void reset();

    ExtParams();

    ~ExtParams();
};

#endif // LWHPARAMS_H
