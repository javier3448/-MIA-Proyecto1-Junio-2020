#ifndef MBR_H
#define MBR_H

#define PRIMARY_COUNT 3
#define PART_COUNT 4

#include <time.h>
#include <stdlib.h>
#include "Disk/partition.h"

class Mbr
{
public:
    int size;
    char diskFit;
    tm fechaCreacion; //TODO: Probar que esto no truene, tal vez sea necesario almacenar el puntero y no la structura como tal?
    int diskSignature;
    Partition primaryPartition[PRIMARY_COUNT];//TODO: Pasar esto a pointer porque puede ser null
    Partition extendedPartition;//CHAPUZ PELIGROSO. ESTA PARTICION TIENE QUE IR DESPUES DE EL ARRAY DE PARTICIONES PRINCIPALES PARA QUE SE PUEDA HACER FACIL UN ARRAY DE 4 CON TODAS LAS PARTICIONES DEL MBR!!!!

    Mbr();
    Mbr(int size, char fit);

    bool isNull();
};

#endif // MBR_H
