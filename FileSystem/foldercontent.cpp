#include "foldercontent.h"

FolderContent::FolderContent()
{

}

bool FolderContent::isNull()
{
    return inodePtr < 0;
}
