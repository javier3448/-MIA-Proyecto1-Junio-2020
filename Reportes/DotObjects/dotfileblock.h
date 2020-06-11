#ifndef DOTFILEBLOCK_H
#define DOTFILEBLOCK_H


#include "FileSystem/fileblock.h"
#include "diskentity.h"

class DotFileBlock
{
private:
    DiskEntity<FileBlock> fileBlockEntity;
    std::string dotName;
    std::string dotNodeDeclaration;

    void initDotStrings();

public:
    DotFileBlock(int address, RaidOneFile* file);
    DotFileBlock(int address, FileBlock inode);

    const std::string& getDotName() const;
    const std::string& getDotNodeDeclaration() const;
};

#endif // DOTFILEBLOCK_H
