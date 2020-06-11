#include "Disk/partition.h"

Partition::Partition()
{
    status = 0;
    type = 0;
    fit = 0;
    start = -1;
    size = -1;
}

bool Partition::isNull()
{
    return size < 0;
}

void Partition::Init(char partType, char partFit, int partStart, int partSize, const std::string &name)
{
    //TODO: Quitar en version final
    if(partStart < 0)
        throw "partStart menor a 0";
    if(partSize < 0)
        throw "partsize menor a 0";
    if(partType != 'p' && partType != 'e')
        throw "partype no igual a p o e";
    if(partFit != 'w' && partFit != 'b' && partFit != 'f')
        throw "partfit no igual a w b o f";

    this->type = partType;
    this->fit = partFit;
    this->start = partStart;
    this->size = partSize;
    std::fill(this->name, this->name + 16, 0);
    strcpy(this->name, name.c_str());
}
