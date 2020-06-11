#ifndef MYSTRINGUTIL_H
#define MYSTRINGUTIL_H

#include <string>
#include <vector>
#include <optional>

namespace  MyStringUtil
{
    //Solo funciona si recibe un path con estilo linux valido
    //hace la concatenacion al final si el string no tiene extension
    std::string appendBeforeExtension(const std::string& str, const std::string& suffix);
    std::vector<std::string> tokenize(const std::string s, char separator);
    bool isValidExtPath(const std::string& extPath);
    //Deberia verificar que sea isValidExtPath antes de invocar a splitFolderPathAndFileName
    void splitFolderPathAndFileName(const std::string& absolutePath, std::string& folderPath, std::string& fileName);
    void removeSlash(const std::string& absolutePath, std::string& folderPath);
    void splitSubFolderNameAndRemainingPath(const std::string& absolutePath, std::string& subDirName, std::string& remainingPath);
    std::optional<std::string> readFile(const std::string& path);
    std::string buildDigitString(int size);
    std::string intToHexString(int n);
};

#endif // MYSTRINGUTIL_H
