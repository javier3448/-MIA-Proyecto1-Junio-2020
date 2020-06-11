#ifndef SUPERBLOCK_H
#define SUPERBLOCK_H

#include<time.h>

#include "inode.h"
#include "extutility.h"
#include "Disk/partition.h"
#include "raidonefile.h"
#include "Analizador/interprete.h"

class SuperBoot
{
private:
    int getNumeroDeEstructuras(int partSize, char fsType);

public:
    static const char FS_TYPE_EXT2 = 2;
    static const char FS_TYPE_EXT3 = 3;
    static const int MAGIC = 0xef53;

    int filesystemType  = -1;
    int inodesCount  = -1;
    int blocksCount  = -1;
    int freeInodesCount  = -1;
    int freeBlocksCount  = -1;
    tm lastmountTime;
    tm lastUnmountTime;
    int mntCount  = -1;
    int magic  = -1;
    int inodeSize  = sizeof (Inode);
    int blockSize  = ExtUtility::BLOCK_SIZE;
    int firstFreeInode  = -1;
    int firstFreeBlock  = -1;
    int bitmapInodeBegin  = -1;
    int bitmapBlockBegin  = -1;
    int inodeStart  = -1;
    int blockStart  = -1;

    //formatType es fast o full
    SuperBoot(char fsType, int partStart, int partSize);

    //Solo para que se pueda poner en DiskEntity
    SuperBoot();

    bool isNull();

    void updateLastMountTime();
    void updateLastUnmountTime();

    //Utiliza el bitmap de inodos para ver si hay espacios disponibles
    //Si hay: Ocupa la posicion disponible en el bitmap y retorna la posicion equivalente en la tabla de inodos
    //Si no: Retorna -1
    int reserveInode(RaidOneFile* file);
    void deleteInode(RaidOneFile *file, int inodeAddress);//No se porque no esta definido en el proyecto de vacas
//    int setFirstFreeInodePtr(RaidOneFile* file, char byte = 1);
//    int getFirstFreeInodePtr(RaidOneFile* file);
    int convertInodePtrToBitmapPtr(int inodePointer);
    int convertInodeBitmapPtrToInodePtr(int bitmapInodePtr);

    int reserveBlock(RaidOneFile* file);
    void deleteBlock(RaidOneFile *file, int blockAddress);//No se porque no esta definido en el proyecto de vacas
//    int setFirstFreeBlockPtr(RaidOneFile* file, char byte = 1);
//    int getFirstFreeBlockPtr(RaidOneFile* file);
    int convertBlockPtrToBitmapPtr(int blockPointer);
    int convertBlockBitmapPtrToBlockPtr(int bitmapBlockPtr);

    //Antes de hacer una recuperacion sb debe de empezar a hacer las reservas y todo eso
    //como si estuviera vacio. Para eso utilizamos reset
    void reset();
};

#endif // SUPERBLOCK_H
