#ifndef FOLDERBLOCK_H
#define FOLDERBLOCK_H

#include "FileSystem/foldercontent.h"

//TODO TODO: Sacarle size!
class FolderBlock
{
public:
    static constexpr unsigned int CONTENTS_SIZE = 4;

    FolderContent contents[CONTENTS_SIZE];

    FolderBlock();
};

#endif // FOLDERBLOCK_H
