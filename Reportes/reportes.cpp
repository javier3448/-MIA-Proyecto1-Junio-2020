#include "reportes.h"

#include <fstream>
#include "Disk/diskmanager.h"
#include "FileSystem/extmanager.h"
#include "Reportes/graphmaker.h"
#include "Reportes/graphvizhelper.h"
#include "diskentity.h"
#include "mystringutil.h"

int Reportes::repMbr(const std::string& idPart, const std::string &path)
{
    auto mountedPart = DiskManager::getMountedPartition(idPart);
    if(!mountedPart)
    {
        Consola::reportarError("No se ha montado una particion con el id: " + idPart);
        return -1;
    }

    //Conseguir mbr
    RaidOneFile file(mountedPart->path, std::ios::binary | std::ios::in);

    Mbr mbr;
    file.seekg(0, std::ios::beg);
    file.read((char*)&mbr, sizeof(Mbr));

    std::string dotSrc;
    int result = 0;
    if (GraphMaker::mkDotMbr(dotSrc, mbr) == -1) {
        Consola::reportarError("Dot no generado");
        result = -1;
    }

    if (GraphvizHelper::writeAndCompileDot(path, dotSrc) == -1) {
        Consola::reportarError("No se pudo compilar el codigo dot");
        result  = -1;
    }

    Partition& extendedPart = mbr.extendedPartition;
    int count = 0;
    if(!mbr.extendedPartition.isNull())
    {
        DiskEntity<Ebr> ebrIter(extendedPart.start, &file);
        while (true) {
            dotSrc = std::string();
            if (GraphMaker::mkDotEbr(dotSrc, &ebrIter, std::string("_") + std::to_string(count)) == -1) {
                Consola::reportarError("Dot no generado");
                result = -1;
            }

            if (GraphvizHelper::writeAndCompileDot(path + "_ebr" + std::to_string(count), dotSrc) == -1) {
                Consola::reportarError("No se pudo compilar el codigo dot");
                result  = -1;
            }

            if(ebrIter.value.next == -1){
                break;
            }
            ebrIter = DiskEntity<Ebr>(ebrIter.value.next, &file);
            count++;
        }
    }

    file.close();
    return result;
}

int Reportes::repDisk(const std::string& idPart, const std::string &path)
{
    auto mountedPart = DiskManager::getMountedPartition(idPart);
    if(!mountedPart)
    {
        Consola::reportarError("No se ha montado una particion con el id: " + idPart);
        return -1;
    }
    std::string& partPath = mountedPart->path;
    RaidOneFile file(partPath, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<Partition> partEntity = DiskEntity<Partition>(mountedPart->start, &file);
    Partition& partition = partEntity.value;


    Mbr mbr;
    file.seekg(0, std::ios::beg);
    file.read((char*)&mbr, sizeof(Mbr));


    std::string dotSrc;
    if (GraphMaker::mkDotDisk(dotSrc, &file, &mbr) == -1) {
        Consola::reportarError("Dot no generado");
        return -1;
    }

    if (GraphvizHelper::writeAndCompileDot(path, dotSrc) == -1) {
        Consola::reportarError("No se pudo compilar el codigo dot");
        return -1;
    }

    file.close();
    return 0;
}

int Reportes::repSb(const std::string &idPart, const std::string &path)
{
    auto mountedPart = DiskManager::getMountedPartition(idPart);
    if(!mountedPart)
    {
        Consola::reportarError("No se ha montado una particion con el id: " + idPart);
        return -1;
    }
    RaidOneFile file(mountedPart->path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot>sbEntity(mountedPart->contentStart, &file);

    std::string dotSrc;
    if (GraphMaker::mkDotSb(dotSrc, &sbEntity) == -1) {
        Consola::reportarError("No se pudo generar el codigo dot");
        return -1;
    }

    if (GraphvizHelper::writeAndCompileDot(path, dotSrc) == -1) {
        Consola::reportarError("No se pudo compilar el codigo dot");
        return -1;
    }

    file.close();
    return 0;
}

int Reportes::repBmInode(const std::string &idPart, const std::string &path)
{
    auto mountedPart = DiskManager::getMountedPartition(idPart);
    if(!mountedPart)
    {
        Consola::reportarError("No se ha montado una particion con el id: " + idPart);
        return -1;
    }
    RaidOneFile file(mountedPart->path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot>sbEntity(mountedPart->contentStart, &file);

    if (sbEntity.value.magic != SuperBoot::MAGIC) {
        Consola::reportarError("el montaje no es un sistema de archivos");
        return -1;
    }

    std::vector<char> bytes = getBytes(&file, sbEntity.value.bitmapInodeBegin, sbEntity.value.inodesCount);
    if (writeBytesInCsvFile(path, bytes) == -1) {
        return -1;
    }

    return 0;
}

int Reportes::repBmBlock(const std::string &idPart, const std::string &path)
{
    auto mountedPart = DiskManager::getMountedPartition(idPart);
    if(!mountedPart)
    {
        Consola::reportarError("No se ha montado una particion con el id: " + idPart);
        return -1;
    }
    RaidOneFile file(mountedPart->path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot>sbEntity(mountedPart->contentStart, &file);

    if (sbEntity.value.magic != SuperBoot::MAGIC) {
        Consola::reportarError("El montaje no es un sistema de archivos");
        return -1;
    }

    std::vector<char> bytes = getBytes(&file, sbEntity.value.bitmapBlockBegin, sbEntity.value.blocksCount);
    if (writeBytesInCsvFile(path, bytes) == -1) {
        return -1;
    }

    return 0;
}

int Reportes::repFile(const std::string &idPart, const std::string &path, const std::string &ruta)
{
    auto mountedPart = DiskManager::getMountedPartition(idPart);
    if(!mountedPart){
        Consola::reportarError("La particion: " + idPart + " no esta montada");
        return -1;
    }
    RaidOneFile file(mountedPart->path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(mountedPart->contentStart, &file);

    DiskEntity<Inode> rootEntity = ExtManager::getRoot(&file, &sbEntity);

    std::string relativePath;
    MyStringUtil::removeSlash(ruta, relativePath);//Crea el path relativo al root (i.e. quita el primer slash)

    auto[closestInode, remainingPath] = ExtManager::folderGetClosestInode(&file, &rootEntity, relativePath);

    if(remainingPath.length() != 0){
        Consola::reportarError("No existe el archivo: <" + ruta + ">");
        return -1;
    }

    if(closestInode.value.isFolder()){
        std::string wrongFilePath = ruta.substr(0, ruta.length() - remainingPath.length());
        Consola::reportarError("El path: </" + wrongFilePath + "> es un folder, no un archivo");
        return -1;
    }

    std::string folderPath;
    std::string fileName;
    MyStringUtil::splitFolderPathAndFileName(relativePath, folderPath, fileName);
    std::string content = ExtManager::fileGetContent(&file, &closestInode);

    content = fileName + "\n\n*********************************************************\n\n" +
              content;

    return writeStringInFile(path, content);
}

int Reportes::repLs(const std::string &idPart, const std::string &path, const std::string &ruta)
{
    auto mountedPart = DiskManager::getMountedPartition(idPart);
    if(!mountedPart){
        Consola::reportarError("La particion: " + idPart + " no esta montada");
        return -1;
    }
    RaidOneFile file(mountedPart->path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(mountedPart->contentStart, &file);
    DiskEntity<Inode> rootEntity = ExtManager::getRoot(&file, &sbEntity);
    DiskEntity<Inode> usersEntity = ExtManager::getUsers(&file, &sbEntity);
    std::string usersFileContent = ExtManager::fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    std::string relativePath;
    MyStringUtil::removeSlash(ruta, relativePath);//Crea el path relativo al root (i.e. quita el primer slash)

    auto[closestInode, remainingPath] = ExtManager::folderGetClosestInode(&file, &rootEntity, relativePath);

    if(remainingPath.length() != 0){
        Consola::reportarError("No existe el archivo: <" + ruta + ">");
        return -1;
    }

    if(closestInode.value.isFile()){
        std::string wrongFilePath = ruta.substr(0, ruta.length() - remainingPath.length());
        Consola::reportarError("El path: <" + wrongFilePath + "> es un archivo, no una carpeta");
        return -1;
    }

    std::string dotSrc;
    if (GraphMaker::mkDotLs(dotSrc, &file, &closestInode, usersData) == -1) {
        Consola::reportarError("Dot no generado");
        return -1;
    }

    if (GraphvizHelper::writeAndCompileDot(path, dotSrc) == -1) {
        Consola::reportarError("No se pudo compilar el codigo dot");
        return -1;
    }

    return 0;
}

int Reportes::repTree(const std::string &idPart, const std::string &path)
{
    auto mountedPart = DiskManager::getMountedPartition(idPart);
    if(!mountedPart){
        Consola::reportarError("La particion: " + idPart + " no esta montada");
        return -1;
    }
    RaidOneFile file(mountedPart->path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(mountedPart->contentStart, &file);

    DiskEntity<Inode> rootEntity = ExtManager::getRoot(&file, &sbEntity);

    std::string dotSrc;
    if (GraphMaker::mkDotTree(dotSrc, &file, &rootEntity) == -1) {
        Consola::reportarError("Dot no generado");
        return -1;
    }

    if (GraphvizHelper::writeAndCompileDot(path, dotSrc) == -1) {
        Consola::reportarError("No se pudo compilar el codigo dot");
        return -1;
    }

    return 0;
}

int Reportes::repBlock(const std::string &idPart, const std::string &path)
{
    auto mountedPart = DiskManager::getMountedPartition(idPart);
    if(!mountedPart){
        Consola::reportarError("La particion: " + idPart + " no esta montada");
        return -1;
    }
    RaidOneFile file(mountedPart->path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(mountedPart->contentStart, &file);

    DiskEntity<Inode> rootEntity = ExtManager::getRoot(&file, &sbEntity);

    std::string dotSrc;
    if (GraphMaker::mkDotBlock(dotSrc, &file, &rootEntity) == -1) {
        Consola::reportarError("Dot no generado");
        return -1;
    }

    if (GraphvizHelper::writeAndCompileDot(path, dotSrc) == -1) {
        Consola::reportarError("No se pudo compilar el codigo dot");
        return -1;
    }

    return 0;
}

int Reportes::repInode(const std::string &idPart, const std::string &path)
{
    auto mountedPart = DiskManager::getMountedPartition(idPart);
    if(!mountedPart){
        Consola::reportarError("La particion: " + idPart + " no esta montada");
        return -1;
    }
    RaidOneFile file(mountedPart->path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(mountedPart->contentStart, &file);

    DiskEntity<Inode> rootEntity = ExtManager::getRoot(&file, &sbEntity);

    std::string dotSrc;
    if (GraphMaker::mkDotInode(dotSrc, &file, &rootEntity) == -1) {
        Consola::reportarError("Dot no generado");
        return -1;
    }

    if (GraphvizHelper::writeAndCompileDot(path, dotSrc) == -1) {
        Consola::reportarError("No se pudo compilar el codigo dot");
        return -1;
    }

    return 0;
}

int Reportes::writeBytesInCsvFile(const std::string &path, const std::vector<char> &bytes)
{
    std::ofstream file(path);

    if(!file.is_open()){
        return -1;
    }

    file.seekp(0, std::ios::beg);

    int byteCount = 0; //Llega hasta 20
    for (auto _byte : bytes) {
        file << std::to_string(_byte) + " ";
        byteCount++;
        if(byteCount == 20){
            byteCount = 0;
            file << "\n";
        }
    }

    file.close();

    return 0;
}

int Reportes::writeStringInFile(const std::string &path, const std::string &content)
{
    std::ofstream file(path);

    if(!file.is_open()){
        Consola::reportarError("No se pudo escribir el archivo: " + path);
        return -1;
    }

    file.seekp(0, std::ios::beg);
    file.write(&content[0], content.length());

    file.close();

    return 0;
}

std::vector<char> Reportes::getBytes(RaidOneFile* file, int start, int size)
{
    std::vector<char> byteMap(size);
    file->seekg(start, std::ios::beg);
    file->read(&byteMap[0], size);

    return byteMap;
}

