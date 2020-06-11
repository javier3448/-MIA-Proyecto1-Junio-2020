#ifndef DISKENTITY_HPP
#define DISKENTITY_HPP

#include "diskentity.h"

template<typename DiskStruct>
DiskEntity<DiskStruct>::DiskEntity()
{

}

template<typename DiskStruct>
DiskEntity<DiskStruct>::DiskEntity(int address)
{
    this->address = address;
}

template<typename DiskStruct>
DiskEntity<DiskStruct>::DiskEntity(int address, DiskStruct value)
{
    this->address = address;
    this->value = value;
}


template<typename DiskStruct>
DiskEntity<DiskStruct>::DiskEntity(int address, RaidOneFile* file)
{
    this->address = address;
    file->seekg((uint)address, std::ios::beg);
    file->read((char*)&value, sizeof(DiskStruct));
}

template<typename DiskStruct>
void DiskEntity<DiskStruct>::updateDiskValue(RaidOneFile* file)
{
    file->seekp((uint)address, std::ios::beg);
    file->write((char*)&value, sizeof(DiskStruct));
}
//Supone que file esta abierto, toma el valor que esta en address del binario y lo escribe en value
template <typename DiskStruct>
void DiskEntity<DiskStruct>::updateRamValue(RaidOneFile* file)
{
    file->seekg(address, std::ios::beg);
    file->read((char*)&value, sizeof(DiskStruct));
}

template <typename DiskStruct>
bool DiskEntity<DiskStruct>::isNull()
{
    return address == -1;
}

#endif // DISKENTITY_HPP
