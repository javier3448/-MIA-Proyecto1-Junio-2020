#ifndef MOUNTEDPART_H
#define MOUNTEDPART_H

#include <string>
#include "Disk/partition.h"
#include "Disk/ebr.h"

//Particion que esta montada en el disco, necesitamos unos datos especifico de ella solamente una vez ya esta montada y tenemos que tener en cuenta si es ebr o no. Por eso existe esta clase
//No necesitamos name de la particion solo su path y su start (por ahora)
//Esta clase la usamos para saber las fronteras de la particion en donde podemos poner el filesystem
//Si tipo es T_EXTENDED, start y size es -1 porque en extended no podemos poner un file system
//^ medio chapuz, talvez solo es de encontrarle un mejor nombre a esta clase
class MountedPart
{
public:
    static const char T_PRIMARY = 0;
    static const char T_EXTENDED = 1;
    static const char T_LOGIC = 2;

    std::string name;
    int start = -1;
    int size = -1;
    std::string path;
    char type = -1;

    //Indican donde se puede empezar a escribir el file system en la particion, tienen en cuenta el tamanno del ebr en caso de ser logica
    int contentStart = -1;//buscarle un mejor nombre
    //Indican el tamanno disponible para el formateo del file system en la particion, tienen en cuenta el tamanno del ebr en caso de ser logica
    int contentSize  = -1;

    MountedPart(const Partition& part, const std::string& path, char type);
    MountedPart(const Ebr& ebr, const std::string& path);
    MountedPart();
};

#endif // MOUNTEDPART_H

