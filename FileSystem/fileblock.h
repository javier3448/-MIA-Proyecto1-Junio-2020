#ifndef FILEBLOCK_H
#define FILEBLOCK_H

#include<string>

class FileBlock
{
public:

    static constexpr unsigned int CONTENT_SIZE = 64;

    char content[CONTENT_SIZE];

    FileBlock();
    FileBlock(const std::string& string, int beg, int size);//Escibe en content de la pos beg del string
    FileBlock(const std::string &string);
};

#endif // FILEBLOCK_H
