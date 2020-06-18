#include "inode.h"
#include <algorithm>

Inode::Inode()
{
    std::fill_n(blocks, BLKS_SIZE, -1);
}

Inode Inode::createFolderInode(int usrId, int grpId, int permissions)
{
    Inode inode = Inode();
    inode.usrId = usrId;
    inode.grpId = grpId;
    inode.size = 0;
    time_t rawtime;
    time(&rawtime);
    inode.creationTime = *localtime(&rawtime);
    inode.accessTime = inode.creationTime;
    inode.modificationTime = inode.creationTime;
    inode.type = TYPE_FOLDER;
    inode.permissions = permissions;

    return inode;
}

Inode Inode::createFolderInode(int usrId, int grpId, int size, int permissions)
{
    Inode inode = Inode();
    inode.usrId = usrId;
    inode.grpId = grpId;
    inode.size = size;
    time_t rawtime;
    time(&rawtime);
    inode.creationTime = *localtime(&rawtime);
    inode.accessTime = inode.creationTime;
    inode.modificationTime = inode.creationTime;
    inode.type = TYPE_FOLDER;
    inode.permissions = permissions;

    return inode;
}

Inode Inode::createFileInode(int usrId, int grpId, int size, int permissions)
{
    Inode inode = Inode();
    inode.usrId = usrId;
    inode.grpId = grpId;
    inode.size = size;
    time_t rawtime;
    time(&rawtime);
    inode.creationTime = *localtime(&rawtime);
    inode.accessTime = inode.creationTime;
    inode.modificationTime = inode.creationTime;
    inode.type = TYPE_FILE;
    inode.permissions = permissions;

    return inode;
}

bool Inode::isFolder()
{
    return type == Inode::TYPE_FOLDER;
}

bool Inode::isFile()
{
    return type == Inode::TYPE_FILE;
}

void Inode::updateAccessTime()
{
    time_t rawtime;
    time(&rawtime);
    this->accessTime = *localtime(&rawtime);
}

void Inode::updateModificationTime()
{
    time_t rawtime;
    time(&rawtime);
    this->modificationTime = *localtime(&rawtime);
}

bool Inode::isUsrOwner(int usrId){
    return usrId == this->usrId;
}

bool Inode::canUsrWrite(int usrId, int usrGrpId)
{
    if(usrId == 1){
        return true;
    }
    //Determinar si es owner, parte del group o other
    if(isUsrOwner(usrId)){
        if(getOwnerPermissions() & RWX_WRITE){
            return true;
        }
    }
    if(this->grpId == usrGrpId){
        if(getGrpPermissions() & RWX_WRITE){
            return true;
        }
    }
    if(getOtherPermissions() & RWX_WRITE){
        return true;
    }
    return false;
}

bool Inode::canUsrRead(int usrId, int usrGrpId)
{
    if(usrId == 1){
        return true;
    }

    if(isUsrOwner(usrId)){
        if(getOwnerPermissions() & RWX_READ){
            return true;
        }
    }
    if(this->grpId == usrGrpId){
        if(getGrpPermissions() & RWX_READ){
            return true;
        }
    }
    if(getOtherPermissions() & RWX_READ){
        return true;
    }
    return false;
}

bool Inode::canUsrExecute(int usrId, int usrGrpId)
{
    if(usrId == 1){
        return true;
    }

    if(isUsrOwner(usrId)){
        if(getOwnerPermissions() & RWX_EXECUTE){
            return true;
        }
    }
    if(this->grpId == usrGrpId){
        if(getGrpPermissions() & RWX_EXECUTE){
            return true;
        }
    }
    if(getOtherPermissions() & RWX_EXECUTE){
        return true;
    }
    return false;
}

std::string Inode::getTypeString()
{
    if(this->type == TYPE_FOLDER){
        return "Directorio";
    }
    return "Archivo";
}

int Inode::getOwnerPermissions()
{
    return (this->permissions & UGO_OWNER) >> 8;
}

int Inode::getGrpPermissions()
{
    return (this->permissions & UGO_GRP) >> 4;
}

int Inode::getOtherPermissions()
{
    return (this->permissions & UGO_OTHER);
}
