#ifndef DISKENTITY_H
#define DISKENTITY_H

#include "raidonefile.h"

//Clase es medio chapus. Solo se puede usar si el file que se envia como parametro esta abierto
//Puede retornar basura
//Super unsafe no hace ningun tipo de cheque. ni en read ni en write
template<typename DiskStruct>
class DiskEntity
{
public:
    //Pointer los bytes en el disco, Supone que ambos discos son identicos y
    int address = -1;
    //'Ram'Value
    DiskStruct value;

    DiskEntity();

    //Utiliza el constructor default para DiskStruct
    DiskEntity(int address);

    //Llama a updateRamValue despues de setear el address
    DiskEntity(int address, RaidOneFile* file);

    DiskEntity(int address, DiskStruct value);

    //Supone que file esta abierto, toma this->value y lo escribe en address del file
    void updateDiskValue(RaidOneFile* file);//<--- este es el que mas se va usar
    //Supone que file esta abierto, toma el valor que esta en address del binario y lo escribe en value
    void updateRamValue(RaidOneFile* file);

    bool isNull();//Chapuz maximo No deberia de existir. Hay casos en los que no se sabe la direccion de un entity hasta mucho despues de que haya sido creado, no significa que sea nulo :(
};

#include "diskentity.hpp"

#endif // DISKENTITY_H
