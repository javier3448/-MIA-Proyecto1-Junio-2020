#ifndef INODE_H
#define INODE_H

#include <ctime>
#include <string>
#include "FileSystem/fileblock.h"
#include "FileSystem/folderblock.h"
#include "FileSystem/pointerblock.h"

class Inode
{
public:
    static constexpr int DIRECT_BLK_BEG = 0;
    static constexpr int DIRECT_BLK_SIZE = 12;
    static constexpr int SIMPLE_INDIRECT_BLK_BEG = 12;
    static constexpr int SIMPLE_INDIRECT_BLK_SIZE = 1;
    static constexpr int DOUBLE_INDIRECT_BLK_BEG = 13;
    static constexpr int DOUBLE_INDIRECT_BLK_SIZE = 1;
    static constexpr int TRIPLE_INDIRECT_BLK_BEG = 14;
    static constexpr int TRIPLE_INDIRECT_BLK_SIZE = 1;
    static constexpr int BLKS_SIZE = DIRECT_BLK_SIZE + SIMPLE_INDIRECT_BLK_SIZE + DOUBLE_INDIRECT_BLK_SIZE + TRIPLE_INDIRECT_BLK_SIZE;

    //Creo que oficialmente se nos fue la mano con las constantes :(
    //chapuz: Constante quemada. No se que es peor si tener una operacion super larga llena de constantes
    //operacion: FileBlock::CONTENT_SIZE*(DIRECT_BLK_SIZE + SIMPLE_INDIRECT_BLK_SIZE * PointerBlock::POINTERS_SIZE + DOUBLE_INDIRECT_BLK_SIZE * PointerBlock::POINTERS_SIZE^2 + TRIPLE_INDIRECT_BLK_SIZE * PointerBlock::POINTERS_SIZE^3);
    static constexpr int MAX_FILE_SIZE = 280320;
    //operacion: FileBlock::CONTENTS_SIZE*(DIRECT_BLK_SIZE + SIMPLE_INDIRECT_BLK_SIZE * PointerBlock::POINTERS_SIZE + DOUBLE_INDIRECT_BLK_SIZE * PointerBlock::POINTERS_SIZE^2 + TRIPLE_INDIRECT_BLK_SIZE * PointerBlock::POINTERS_SIZE^3);
    static constexpr int MAX_FOLDER_SIZE = 17520;

    static const char TYPE_FOLDER = 0;
    static const char TYPE_FILE = 1;

    int usrId = -1;
    int grpId = -1;
    int size = -1;
    tm accessTime;
    tm creationTime;
    tm modificationTime;
    int blocks[BLKS_SIZE];
    char type = -1;
    int permissions = -1;

    Inode();
    static Inode createFolderInode(int usrId, int grpId, int permissions);
    static Inode createFolderInode(int usrId, int grpId, int size, int permissions);
    static Inode createFileInode(int usrId, int grpId, int size, int permissions);

    bool isNull();
    bool isFolder();
    bool isFile();
    void updateAccessTime();
    void updateModificationTime();

    bool isUsrOwner(int usrId);//No hacemos isGrp, ni isOther, proque isUsrOwner es el unico que nos va a servir afuera de la clase. No se si es mala practica?, Talvez seria bueno hacer los metodos y dejarlos privados
    bool canUsrWrite(int usrId, int usrGrpId);
    bool canUsrRead(int usrId, int usrGrpId);
    bool canUsrExecute(int usrId, int usrGrpId);

    std::string getTypeString();

private:
    static const int RWX_READ = 1 << 2;
    static const int RWX_WRITE = 1 << 1;
    static const int RWX_EXECUTE = 1;

    static const int UGO_OWNER = 0xf << 8;
    static const int UGO_GRP = 0xf << 4;
    static const int UGO_OTHER = 0xf;

    //    int setOwnerPermissions();
    //    int setGroupPermissions();
    //    int setOtherPermissions();

    int getOwnerPermissions();
    int getGrpPermissions();
    int getOtherPermissions();
};


#endif // INODE_H
