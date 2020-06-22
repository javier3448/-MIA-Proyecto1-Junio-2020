#include "superboot.h"

#include <vector>
#include <math.h>
#include <bits/stdc++.h>
#include "Disk/diskmanager.h"
#include "FileSystem/journaling.h"
#include "FileSystem/extutility.h"


//i.e. numero de inodos
int SuperBoot::getNumeroDeEstructuras(int partSize, char fsType)
{
    //talvez no sea necesario usar floor

    //tamaño_particion = sizeof(superblock) + n + 3 * n + n * sizeof(inodos) + 3 * n * sizeof(block)
    if(fsType == FS_TYPE_EXT2)
        return (int)std::floor(partSize / ((int)sizeof(Inode) + ExtUtility::BLOCK_SIZE + 7));
    else
        return (int)std::floor(partSize / ((int)sizeof(JournEntry) + (int)sizeof(Inode) + ExtUtility::BLOCK_SIZE));
}

SuperBoot::SuperBoot(char fsType, int partStart, int partSize)
{
    int numeroDeEstructuras = getNumeroDeEstructuras(partSize, fsType);

    filesystemType = fsType;
    inodesCount = numeroDeEstructuras;
    blocksCount = 3 * numeroDeEstructuras;
    freeInodesCount = inodesCount;
    freeBlocksCount = blocksCount;
    mntCount = 0;
    magic = MAGIC;

    time_t rawtime;
    time(&rawtime);
    this->lastUnmountTime = *localtime(&rawtime);
    this->lastmountTime = *localtime(&rawtime);

    //primer bit disponible despues del journaling (si este fs va a tener journaling
    int start = partStart + sizeof(SuperBoot) + (fsType == FS_TYPE_EXT3 ? numeroDeEstructuras * (int)sizeof(JournEntry) : 0);

    //En el orde de ubicacion en el disco:
    bitmapInodeBegin = start;
    bitmapBlockBegin = bitmapInodeBegin + numeroDeEstructuras;
    inodeStart = bitmapBlockBegin + 3 * numeroDeEstructuras;
    blockStart = inodeStart + (int)sizeof(Inode) * numeroDeEstructuras;
    firstFreeInode  = bitmapInodeBegin;
    firstFreeBlock  = bitmapBlockBegin;
    this->updateLastMountTime();
    this->updateLastUnmountTime();
}

SuperBoot::SuperBoot()
{

}

bool SuperBoot::isNull()
{
    return filesystemType == -1;
}

void SuperBoot::updateLastMountTime()
{
    time_t rawtime;
    time(&rawtime);
    this->lastmountTime = *localtime(&rawtime);
}

void SuperBoot::updateLastUnmountTime()
{
    time_t rawtime;
    time(&rawtime);
    this->lastUnmountTime = *localtime(&rawtime);
}

int SuperBoot::reserveInode(RaidOneFile* file)
{
    if(firstFreeInode == INT_MAX){//Significa que ya no tenemos espacios disponibles
        return -1;
    }

    //Marcamos en el bitmap de inodos del archivo "fisico"
    file->seekp(firstFreeInode, std::ios::beg);
    char one = 1;
    file->write(&one, 1);
    file->flush();

    int freeInode = firstFreeInode;//Este le sirve al metodo nada mas. porque, lo almacenamos porque mas adelante vamos a actualizar firstFreeInode
    int i;
    file->seekg(firstFreeInode + 1, std::ios::beg);
    for (i = firstFreeInode + 1; i < bitmapBlockBegin; i++) {//el siguiente firstFreeInode tiene que estar despues de firstFreeInode actual porque cuando eliminamos un inode verificamos si es menor o no al firstFreeInode actual, de ser asi cambiamos al inode recien eliminado
        char c;
        file->read(&c, 1);
        if(c == 0){
            firstFreeInode = i;
            break;
        }
    }
    if(i >= bitmapBlockBegin){
        firstFreeInode = INT_MAX;//Con esto indicamos que ya no tenemos espacios disponibles
    }

    freeInodesCount--;

    return convertInodeBitmapPtrToInodePtr(freeInode);
}

//La direccion del inodo como tal no de su bitmap
void SuperBoot::deleteInode(RaidOneFile *file, int inodeAddress)
{
    int bitmapInodeAddress = convertInodePtrToBitmapPtr(inodeAddress);

    file->seekp(bitmapInodeAddress, std::ios::beg);
    char zero = 0;
    file->write(&zero, 1);
    file->flush();

    if(bitmapInodeAddress < firstFreeInode){
        firstFreeInode = bitmapInodeAddress;
    }

    freeInodesCount++;
}


int SuperBoot::convertInodeBitmapPtrToInodePtr(int bitmapInodeBegin)
{
    return (bitmapInodeBegin - this->bitmapInodeBegin) * inodeSize + inodeStart;
}

int SuperBoot::convertInodePtrToBitmapPtr(int inodePointer)
{
    return (inodePointer - inodeStart) / inodeSize + bitmapInodeBegin;
}

int SuperBoot::reserveBlock(RaidOneFile *file)
{
    if(firstFreeBlock == INT_MAX){//Significa que ya no tenemos espacios disponibles
        return -1;
    }

    //Marcamos en el bitmap de blocks del archivo "fisico"
    file->seekp(firstFreeBlock, std::ios::beg);
    char one = 1;
    file->write(&one, 1);
    file->flush();

    int freeBlock = firstFreeBlock;//Este le sirve al metodo nada mas. porque, lo almacenamos porque mas adelante vamos a actualizar firstFreeBlock
    int i;
    file->seekg(firstFreeBlock + 1, std::ios::beg);
    for (i = firstFreeBlock + 1; i < inodeStart; i++) {//el siguiente firstFreeInode tiene que estar despues de firstFreeInode actual porque cuando eliminamos un inode verificamos si es menor o no al firstFreeInode actual, de ser asi cambiamos al inode recien eliminado
        char c;
        file->read(&c, 1);
        if(c == 0){
            firstFreeBlock = i;
            break;
        }
    }
    if(i >= inodeStart){
        firstFreeBlock = INT_MAX;//Con esto indicamos que ya no tenemos espacios disponibles
    }
    freeBlocksCount--;

    return convertBlockBitmapPtrToBlockPtr(freeBlock);
}

void SuperBoot::deleteBlock(RaidOneFile *file, int blockPointer)
{
    int bitmapBlockAddress = convertBlockPtrToBitmapPtr(blockPointer);

    file->seekp(bitmapBlockAddress, std::ios::beg);
    char zero = 0;
    file->write(&zero, 1);
    file->flush();

    if(bitmapBlockAddress < firstFreeBlock){
        firstFreeBlock = bitmapBlockAddress;
    }

    freeBlocksCount++;
}

int SuperBoot::convertBlockBitmapPtrToBlockPtr(int bitmapBlockBegin)
{
    return (bitmapBlockBegin - this->bitmapBlockBegin) * ExtUtility::BLOCK_SIZE + blockStart;
}

void SuperBoot::reset()
{
    freeInodesCount = inodesCount;
    freeBlocksCount = blocksCount;
    firstFreeInode  = bitmapInodeBegin;
    firstFreeBlock  = bitmapBlockBegin;
}

int SuperBoot::convertBlockPtrToBitmapPtr(int blockPointer)
{
    return (blockPointer - blockStart) / blockSize + bitmapBlockBegin;
}

//int SuperBoot::setFirstFreeInodePtr(RaidOneFile *file, char byte)
//{
//    int bitmapNewInodePtr = getFirstFreeInodePtr(file);
//    file->seekp(bitmapNewInodePtr, std::ios::beg);
//    file->write(&byte, 1);
//    freeInodesCount--;
//    return bitmapNewInodePtr;
//}

//int SuperBoot::getFirstFreeInodePtr(RaidOneFile* file)
//{
//    int bitmapNewInodePtr = bitmapInodeBegin;
//    std::vector<char> bitmap;
//    bitmap.reserve(inodesCount);
//    file->seekg(bitmapNewInodePtr, std::ios::beg);
//    file->read(&bitmap[0], inodesCount);

//    for (int i = 0; i < inodesCount; i++)
//    {
//        if(bitmap[i] == 0)
//        {
//            bitmapNewInodePtr += i;
//            return bitmapNewInodePtr;
//        }
//    }
//    return -1;
//}

