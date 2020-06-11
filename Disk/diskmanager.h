#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <fstream>
#include <string>
#include <algorithm>
#include <utility>
#include <vector>
#include <unordered_map>
#include <cstdio>
#include <array>
#include "consola.h"
#include "Disk/mbr.h"
#include "Disk/ebr.h"
#include "Disk/mountedpart.h"

//Deberia estar dentro del nameSpace
extern std::unordered_map<std::string, MountedPart> mountedPartitions;

extern char ZERO;
extern char ONE;
//TODO: arreglar esta 'clase estatica' para que todas las definiciones de metodos esten en el header o en un namespace anonimo en el .cpp
 namespace DiskManager {

    //retorna -1 si hubo error
    int mkDisk(const std::string& path, int size, char fit, char unit);

    //Remueve el disco en la direccion y su (copy)
    //retorna -1 si hubo error
    int rmDisk(const std::string& path);

    int fDiskNew(const std::string& path, const std::string& name, int size, char fit, char unit, char type);

    int fDiskDelete(const std::string& path, const std::string& name, char deleteType);

    int fDiskAdd(const std::string& path, const std::string& name, int size, char unit);

    int mount(const std::string& path, const std::string& name);

    int unMount(const std::string& names);

    //Retorna una particion null si no la ecuentra
    std::optional<MountedPart> getMountedPartition(const std::string& id);


    //Utility
    int toBytes(int size, char unit);
}
#endif // DISKMANAGER_H
