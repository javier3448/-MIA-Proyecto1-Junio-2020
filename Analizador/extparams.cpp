#include "extparams.h"

#include <cctype>

ExtParams::ExtParams()
{
}

ExtParams::~ExtParams()
{
    reset();
}

//Retorna a valores originales (i.e. nulos o default)
void ExtParams::reset()
{
    r = false;
    p = false;
    rf = false;
    unit = '\0';
    fitType = '\0';
    deleteType = '\0';
    type = '\0';
    fs = '\0';
    size = -1;
    add = INT_MIN;
    ugo = -1;
    if(id)
        delete(id);
    id = nullptr;
    if(file)
        delete(file);
    file = nullptr;
    if(grp)
        delete(grp);
    grp = nullptr;
    if(path)
        delete(path);
    path = nullptr;
    if(name)
        delete(name);
    name = nullptr;
    if(usr)
        delete(usr);
    usr = nullptr;
    if(pwd)
        delete(pwd);
    pwd = nullptr;
    if(cont)
        delete(cont);
    cont = nullptr;
    if(ruta)
        delete(ruta);
    ruta = nullptr;
    if(dest)
        delete(dest);
    dest = nullptr;
}

//TODO AGREGAR UN DELETE QUE HAGA RESET TAMBIEN

void ExtParams::deepClearVector(std::vector<std::string*> &vector)
{
    for(auto stringPtr : vector)
        delete(stringPtr);
    vector.clear();
}

void ExtParams::setSize(int val)
{
    size = val;
}

void ExtParams::setPath(std::string* val)
{
    if(path)
        delete(path);
    path = val;
}

void ExtParams::setName(std::string* val)
{
    if(name)
        delete(name);
    name = val;
}

void ExtParams::setUnit(std::string* val)
{
    unit = (char)std::tolower((*val)[0]);
}

void ExtParams::setFitType(std::string* val)
{
    fitType = (char)std::tolower((*val)[0]);
}

void ExtParams::setDeleteType(std::string* val)
{
    if (val->compare("fast")) {
        deleteType = ExtParamsConstants::DELETE_TYPE_FAST;
    }
    else
    {
        deleteType = ExtParamsConstants::DELETE_TYPE_FULL;
    }
    delete(val);//Chapuz horrible para que no deje dangling pointer. Significa que despues de hacer la reduccion en la que se hace setType ya no se puede usar ese puntero/ PELIGROSISIMO!
}

void ExtParams::setType(std::string* val)//partType, deleteType,
{
    //Chapuz:
    if(val->length() > 2){//Significa que es tipo
        if((char)std::tolower((*val)[1]) == 'a'){//fast
            type = ExtParamsConstants::DELETE_TYPE_FAST;
        }
        if((char)std::tolower((*val)[1]) == 'u'){//full
            type = ExtParamsConstants::DELETE_TYPE_FULL;
        }
    }
    else{
        type = (char)std::tolower((*val)[0]);
    }
    delete(val);//Chapuz horrible para que no deje dangling pointer. Significa que despues de hacer la reduccion en la que se hace setType ya no se puede usar ese puntero/ PELIGROSISIMO!
}

void ExtParams::setId(std::string* val)
{
    id = val;//TODO: agregar a una lista de string pointers
}

void ExtParams::setUsr(std::string* val)
{
    if(usr)
        delete(usr);
    usr = val;
}

void ExtParams::setPwd(std::string* val)
{
    if(pwd)
        delete(pwd);
    pwd = val;
}

void ExtParams::setUgo(int val)
{
    ugo = val;
}

void ExtParams::setR(int val)
{
    r = val;
}

void ExtParams::setP(int val)
{
    p = val;
}

void ExtParams::setCont(std::string* val)
{
    if(cont)
        delete(cont);
    cont = val;
}

void ExtParams::setFiles(std::string* val)
{
    file = val;
}

void ExtParams::setRf(int val)
{
    rf = val;
}

void ExtParams::setRuta(std::string* val)
{
    if(ruta)
        delete(ruta);
    ruta = val;
}

void ExtParams::setAdd(int val)
{
    add = val;
}

void ExtParams::setFs(char val)
{
    fs = val;
}

void ExtParams::setGrp(std::string *val)
{
    grp = val;
}

void ExtParams::setDest(std::string *val)
{
    dest = val;
}
