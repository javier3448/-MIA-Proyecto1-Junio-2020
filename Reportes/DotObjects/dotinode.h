#ifndef DOTINODE_H
#define DOTINODE_H

#include "FileSystem/inode.h"
#include "diskentity.h"

class DotInode
{
private:
    DiskEntity<Inode> inodeEntity;
    std::string dotName;
    std::string dotNodeDeclaration;
    std::string blockPtrPorts[Inode::BLKS_SIZE];

    void initDotStrings();

public:
    DotInode(int address, RaidOneFile* file);
    DotInode(int address, Inode inode);

    int getBlockPtr(int index) const;
    int getType() const;
    const std::string& getDotName() const;
    const std::string& getDotNodeDeclaration() const;
    const std::string& getBlockPort(int index) const;//Para los puertos es lo mismo lo
};

#endif // DOTINODE_H
