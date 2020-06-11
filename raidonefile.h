#ifndef RAIDONEFILE_H
#define RAIDONEFILE_H

#include <fstream>
#include <tuple>

//Nota: deberiamos de disennar la clase de manera tal que crashee si a uno de los archivos no se le puede hacer lo del otro. Pero por ahora el chapus de retornar una const ref esta bien
//Wrapper para una pareja de archivos read write binarios (i.e. las banderas de ambos archivos siempre seran: (std::ios::binary | std::ios::in | std::ios::out)
//Asume que la direccion del segundo archivo es la misma del primero con "(copy)" concatenado antes de la extension, esto es mal disenno pero para motivos de la practica facilita un poco la progra
//No acepta archivos de mas de 4 gigas
//No acepta offsets en seekg y seekp
//los metodos deberia de retornar una tuple con el resultado de la operacion de ambos files, pero no se porque la tupla no me deja
//TRATA de que ambos archivos sean iguales pero no lo garantiza
//Solo lee de firstFile y solo hace seekg en firstFile
class RaidOneFile
{
private:
    std::fstream firstFile;
    std::fstream secondFile;

public:
    RaidOneFile(const std::string& path, std::_Ios_Openmode mode);
    RaidOneFile(const std::string& path0, const std::string& path, std::_Ios_Openmode mode);

    void open(const std::string& path, std::_Ios_Openmode mode);
    void open(const std::string& path1, const std::string& path2, std::_Ios_Openmode mode);

    //cierra ambos
    void close();

    //and de ambos is_open
    bool areOpen() const;

    //Metodos de lectura y escritura:
    void seekg(unsigned int pos, std::ios_base::seekdir way);//Chapuz minimo usa unsigned int en vez de fpos, esto va a afectar el tamanno maximo del archivo
    void seekg(unsigned int pos);
    void read(char* beg, std::streamsize size); //;_; no se porque no me deja return make_tuple(firstFile.read(beg,size), secondFile.read(beg,size); como que no me deja crear la tupla

    void seekp(unsigned int pos, std::ios_base::seekdir way);//Chapuz minimo usa unsigned int en vez de fpos, esto va a afectar el tamanno maximo del archivo
    void seekp(unsigned int pos);
    void write(char* beg, std::streamsize size);
    void flush();

    //Revisa si ambos archivos son identicos. Cuidado: mucho overhead
    bool isCorrupted();//TODO: buscar un mejor nombre y que retorne info de como esta corrota la pareja de archivos

    const std::fstream& getFirstFile() const;//Para isOpen y ese tipo de huecadas? ... Es como un chapus para poder ver las flag bits tambien
    const std::fstream& getSecondFile() const;
};

#endif // RAIDONEFILE_H
