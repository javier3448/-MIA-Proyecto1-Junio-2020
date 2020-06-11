#ifndef EXTUTILITY_H
#define EXTUTILITY_H

#include "FileSystem/fileblock.h"
#include "FileSystem/folderblock.h"
#include "FileSystem/folderblock.h"

//Esto parece ser estupido
namespace ExtUtility {
    const int BLOCK_SIZE = 64;//Tamanno en bytes de cada clase bloque

    //Para un archivo de tamano <bytes>, retorna el numero de bloques necesarios
    //que ocuparia ese archivo en el sistema de archivos
    int neededBlocks(int bytes);

    //TODO: Dado un inodo y un n-esimo byte, conseguir un el bloque que contiene ese byte
}
#endif // BLOCK_H
