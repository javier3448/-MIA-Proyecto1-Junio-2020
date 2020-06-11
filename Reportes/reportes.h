#ifndef REPORTES_H
#define REPORTES_H

#include <string>
#include <vector>
#include "raidonefile.h"

class Reportes
{
public:
    static int repMbr(const std::string& idPart, const std::string& path);
    static int repDisk(const std::string& idPart, const std::string& path);

    static int repSb(const std::string& idPart, const std::string& path);
    static int repBmInode(const std::string& idPart, const std::string& path);
    static int repBmBlock(const std::string& idPart, const std::string& path);
    static int repFile(const std::string& idPart, const std::string& path, const std::string& ruta);
    static int repLs(const std::string& idPart, const std::string& path, const std::string& ruta);
    static int repTree(const std::string& idPart, const std::string& path);
    static int repBlock(const std::string &idPart, const std::string& path);
    static int repInode(const std::string &idPart, const std::string& path);

private:
    static std::vector<char> getBytes(RaidOneFile* file, int start, int size);//TODO: quitar, esta de mas
    static int writeBytesInCsvFile(const std::string& path, const std::vector<char>& bytes);//TODO: quitar, esta de mas
    static int writeStringInFile(const std::string& path, const std::string& content);
friend int main();//TODO: quitar, solo sirve para debugging
};

#endif // LWHMANAGER_H
