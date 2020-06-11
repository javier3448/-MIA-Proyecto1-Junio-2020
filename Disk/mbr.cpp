#include "Disk/mbr.h"

Mbr::Mbr()
{
    size = -1;
    diskSignature = -1;
//    diskFit = 0;
}

Mbr::Mbr(int size, char fit)
{
    this->size = size;

    if(fit != 'w' && fit != 'b' && fit != 'f')
        throw "no se puede instanciar un mbr con fit no igual a w, b o f";
    if (size <= 0)
        throw "No se puede instanciar un mbr con size <= 0";

    this->diskFit = fit;
    time_t rawtime;
    time(&rawtime);
    this->fechaCreacion = *localtime(&rawtime);
    this->diskSignature = rand();
}

bool Mbr::isNull()
{
    return size < 0;
}

