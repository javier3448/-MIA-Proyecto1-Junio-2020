#ifndef DOTFOLDERBLOCK_H
#define DOTFOLDERBLOCK_H

#include "FileSystem/folderblock.h"
#include "diskentity.h"

class DotFolderBlock
{
private:
    DiskEntity<FolderBlock> folderBlockEntity;
    std::string dotName;
    std::string dotNodeDeclaration;
    std::string inodePtrPorts[FolderBlock::CONTENTS_SIZE];

    void initDotStrings();

public:
    DotFolderBlock(int address, RaidOneFile* file);
    DotFolderBlock(int address, FolderBlock folderBlock);

    std::string getInodeName(int index) const;
    int getInodePtr(int index) const;
    const std::string& getDotName() const;
    const std::string& getDotNodeDeclaration() const;
    const std::string& getInodePort(int index) const;//Para los puertos es lo mismo lo
};

#endif // DOTFOLDERBLOCK_H
