#include "raidonefile.h"

#include <functional>
#include "mystringutil.h"

RaidOneFile::RaidOneFile(const std::string &path, std::_Ios_Openmode mode)
{
    firstFile = std::fstream(path, mode);
    bool b1 = firstFile.is_open();
    std::string secondPath = MyStringUtil::appendBeforeExtension(path, "raid");
    secondFile = std::fstream(secondPath, mode);
    bool b2 = secondFile.is_open();
}

RaidOneFile::RaidOneFile(const std::string &path1, const std::string &path2, std::_Ios_Openmode mode)
{
    firstFile = std::fstream(path1, mode);
    secondFile = std::fstream(path2, mode);
}

void RaidOneFile::open(const std::string &path, std::_Ios_Openmode mode)
{
    firstFile.open(path, mode);
    std::string secondPath = MyStringUtil::appendBeforeExtension(path, "raid");
    secondFile.open(secondPath, mode);
}

void RaidOneFile::open(const std::string &path1, const std::string &path2, std::_Ios_Openmode mode)
{
    firstFile.open(path1, mode);
    secondFile.open(path2, mode);
}

void RaidOneFile::close()
{
    firstFile.close();
    secondFile.close();
}

bool RaidOneFile::areOpen() const
{
    return firstFile.is_open() && secondFile.is_open();
}

void RaidOneFile::seekg(unsigned int pos, std::ios_base::seekdir way)
{
    firstFile.seekg(pos, way);
}

void RaidOneFile::read(char *beg, std::streamsize size)
{
    firstFile.read(beg, size);
}

void RaidOneFile::seekp(unsigned int pos, std::ios_base::seekdir way)
{
    firstFile.seekp(pos, way);
    secondFile.seekp(pos, way);
}

void RaidOneFile::seekp(unsigned int pos)
{
    firstFile.seekp(pos);
    secondFile.seekp(pos);
}

void RaidOneFile::write(char *beg, std::streamsize size)
{
    firstFile.write(beg, size);
    secondFile.write(beg, size);
}

void RaidOneFile::flush()
{
    firstFile.flush();
    secondFile.flush();
}

//Solo para debugging
bool RaidOneFile::isCorrupted()
{
    std::istreambuf_iterator<char>first (firstFile.seekg(0, std::ios_base::beg));
    std::istreambuf_iterator<char>second (secondFile.seekg(0, std::ios_base::beg));

    std::istreambuf_iterator<char> end;

    while(first != end && second != end)
    {
        if(*first != *second)
            return true;

        first++;
        second++;
    }
    return !((first == end) && (second == end));
}

const std::fstream &RaidOneFile::getFirstFile() const
{
    return firstFile;
}

const std::fstream &RaidOneFile::getSecondFile() const
{
    return secondFile;
}

