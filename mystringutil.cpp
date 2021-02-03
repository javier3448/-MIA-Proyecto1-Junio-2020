#include "mystringutil.h"

#include <algorithm>
#include <bits/stdc++.h>

std::string MyStringUtil::appendBeforeExtension(const std::string& str, const std::string& suffix)
{
    using std::string;

    //nambeBegin indice inicial del nombre del archivo
    ulong nameBegin = str.find_last_of("/");
    if(nameBegin == string::npos)//No tiene / entonce su nombre es todo antes del punto de la extension
    {
        nameBegin = 0;
    }
    else
    {
        nameBegin++;//empieza una posicion despues del ultimo /
    }

    ulong nameEnd = str.find_last_of(".");
    if(nameEnd == string::npos | nameEnd < nameBegin)//Si no tiene . entonces end es el size ofstring | tiene un punto pero esta en una carpeta padre, no en el nombre
    {
        return str + suffix;
    }

    std::string result = str;
    result.replace(nameEnd, 1, suffix + ".");
    return result;
}

std::vector<std::string> MyStringUtil::tokenize(const std::string s, char separator)
{
    std::vector<std::string> tokens;
    int tokenCount = std::count(s.begin(), s.end(), separator) + 1;
    tokens.reserve(tokenCount);

    std::stringstream check1(s);
    std::string intermediate;
    while(getline(check1, intermediate, separator))
        tokens.push_back(intermediate);

    return tokens;
}

bool MyStringUtil::isValidExtPath(const std::string &extPath)
{
    if(extPath.length() == 0){
        return false;
    }
    if(extPath[0] != '/'){
        return false;
    }
    if(extPath.length() == 1){//Caso especial path = "/"
        return true;
    }

    size_t lastSlashIndex = 0;
    //Este for y el if que le sigue es un algoritmo algo criptico para asegurarnos
    //que los nombre esten siempre [0, 12) chars
    //endOfLastFileName es el indice que 'cierra' el ultimo archivo en un extPath
    //*Si tiene el estilo /aaa/aaaa/aaa/. endOfLastFileName es el indice del
    // ultimo caracter porque el ultimo '/' no es parte del nombre del ultimo archivo
    //*Si tiene el estilo /aaa/aaaa/aaa. endOfLastFileName es extPath.length()
    size_t endOfLastFileName = (extPath[extPath.length() - 1] == '/' ? extPath.length() - 1 : extPath.length());
    size_t i;
    for (i = 1; i < endOfLastFileName; i++)
    {
        if(extPath[i] == '/'){
            int nameLength = i - lastSlashIndex - 1;
            if(nameLength > 11 ||
               nameLength < 1){
                //No es extpath porque el nombre es muy largo o muy corto
                return false;
            }
            lastSlashIndex = i;
        }
        //Aqui podriamos revisar caracteres no validos y todo eso
    }
    //En este momento i esta en endOfLastFileName
    int nameLength = i - lastSlashIndex;
    if(nameLength > 12 ||
       nameLength < 1){
        //No es extpath porque el nombre es muy largo o muy corto
        return false;
    }

    return true;
}

void MyStringUtil::splitFolderPathAndFileName(const std::string &absolutePath, std::string &folderPath, std::string &fileName)
{
    folderPath = absolutePath.substr(1);//Quitamos el primer /, ya que, ya conseguimos el avd del root
    if(folderPath.length() > 0 && folderPath[folderPath.length() - 1] == '/'){
        folderPath.pop_back();//Quitamos el ultimo / en caso que sea un path con el estilo /aaaa/aaaa/aaaa/
    }
    auto lastIndexOfSlash = folderPath.find_last_of("/");
    if (lastIndexOfSlash == std::string::npos) {
        fileName = folderPath;
        folderPath = "";
    }
    else{
        fileName = folderPath.substr(lastIndexOfSlash + 1);
        folderPath = folderPath.substr(0, lastIndexOfSlash);
    }
}

//Sirve para el mkDir, solo remueve el / inicial porque ese se consigue al conseguir el root
void MyStringUtil::removeSlash(const std::string &absolutePath, std::string& folderPath)
{
    folderPath = absolutePath.substr(1);//Quitamos el primer /, ya que, ya conseguimos el inode del root
    if(folderPath.length() > 0 && folderPath[folderPath.length() - 1] == '/'){
        folderPath.pop_back();//Quitamos el ultimo / en caso que sea un path con el estilo /aaaa/aaaa/aaaa/
    }
}

void MyStringUtil::splitSubFolderNameAndRemainingPath(const std::string &absolutePath, std::string &subDirName, std::string &remainingPath)
{
    auto firstIndexOfSlash = absolutePath.find_first_of("/");
    if (firstIndexOfSlash == std::string::npos) {
        remainingPath = "";
        subDirName= absolutePath;
    }
    else{
        remainingPath = absolutePath.substr(firstIndexOfSlash + 1);
        subDirName = absolutePath.substr(0, firstIndexOfSlash);
    }
}

std::optional<std::string> MyStringUtil::readFile(const std::string &path)
{
    std::ifstream file(path);
    if (file.is_open()) {
        std::string content;
        content.assign((std::istreambuf_iterator<char>(file)),
                       (std::istreambuf_iterator<char>()));
        return {content};
    }else{
        return {};
    }
}

std::string MyStringUtil::buildDigitString(int size)
{
    std::string content;
    content.reserve(size);
    while (size > 0) {
        for (char c = '0'; c <= '9' && size > 0; c++) {
            content += c;
            size--;
        }
    }
    return content;
}


std::string MyStringUtil::intToHexString(int n)
{
    std::stringstream stream;
    stream << std::hex << std::setfill('0') << std::setw(3) << n;
    return stream.str();
}

std::string MyStringUtil::dateToString(const std::tm& time, const std::string& fmt)
{
    std::ostringstream oss;
    oss << std::put_time(&time, fmt.c_str());
    return oss.str();
}

std::string MyStringUtil::dateToJsonString(const tm &time)
{
    std::ostringstream oss;
    oss << std::put_time(&time, "%Y-%m-%dT%H:%M:%S.000Z");
    return oss.str();
}

//Retorna un "null" si el caracter es 0
std::string MyStringUtil::charToString(char c)
{
    if (c == '\0') {
        return std::string("null");
    }
    if(std::isprint(c)){
        return std::string(1, c);
    }
    return std::string("non printable");
}

std::string MyStringUtil::floatToString(float f)
{
    std::stringstream stream;
    stream << std::fixed << std::setprecision(2) << f;
    return stream.str();
}


