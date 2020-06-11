#ifndef DOTINDIRECTBLOCK_H
#define DOTINDIRECTBLOCK_H

#include "FileSystem/pointerblock.h"
#include "diskentity.h"

class DotPointerBlock
{
private:
    DiskEntity<PointerBlock> pointerBlockEntity;
    std::string dotName;
    std::string dotNodeDeclaration;
    std::string blockPtrPorts[PointerBlock::POINTERS_SIZE];

    void initDotStrings();

public:
    DotPointerBlock(int address, RaidOneFile* file);
    DotPointerBlock(int address, PointerBlock pointerBlock);

    int getBlockPtr(int index) const;
    const std::string& getDotName() const;
    const std::string& getDotNodeDeclaration() const;
    const std::string& getBlockPort(int index) const;//Para los puertos es lo mismo lo
};

#endif // DOTINDIRECTBLOCK_H
