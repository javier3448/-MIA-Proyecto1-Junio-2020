#include "fileblock.h"
#include <algorithm>

FileBlock::FileBlock()
{
    //medio chapus: Ponemos su contenido en 0 para que sea mas facil armar los strings cuando queramos sacarlos del disco
    std::fill_n(content, CONTENT_SIZE, 0);
}

//[!!!] no segura, para nada
FileBlock::FileBlock(const std::string &string, int beg, int size)
{
    std::copy(std::begin(string) + beg, std::begin(string) + beg + size, content);//PROBAR SI FUNCIONA
    std::fill(content + size, std::end(content), 0);
}
