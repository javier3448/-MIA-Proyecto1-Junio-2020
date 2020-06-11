#include "Disk/ebr.h"

Ebr::Ebr()
{
    this->start = -1;
    this->size = -1;
    this->next = -1;
    this->status = 0;
    this->fit = 0;
}

Ebr::Ebr(int partStart, int partSize, int partNext, char partStatus, char partFit, const std::string &partName)
{
    //TODO: Quitar esto en version final
    if(partStart < 0)
        throw "partStart menor a 0";
    if(partSize < 0)
        throw "partsize menor a 0";
    if(partFit != 'w' && partFit != 'b' && partFit != 'f')
        throw "partfit no igual a w b o f";
    this->start = partStart;
    this->size = partSize;
    this->next = partNext;
    this->status = partStatus;
    this->fit = partFit;
    std::fill(this->name, this->name + 16, 0);
    strcpy(this->name, partName.c_str());
}

bool Ebr::isNull() const
{
    return size < 0;
}

bool Ebr::isTop() const
{
    return fit == '\0' && size >= 0;
}
