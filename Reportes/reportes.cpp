#include "reportes.h"

#include <fstream>
#include "Disk/diskmanager.h"
#include "FileSystem/extmanager.h"
#include "Reportes/graphmaker.h"
#include "Reportes/graphvizhelper.h"
#include "diskentity.h"
#include "mystringutil.h"
#include "Analizador/interprete.h"

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

int Reportes::repJournaling(const std::string &idPart, const std::string &path)
{
    auto mountedPart = DiskManager::getMountedPartition(idPart);
    if(!mountedPart)
    {
        Consola::reportarError("No se ha montado una particion con el id: " + idPart);
        return -1;
    }
    RaidOneFile file(mountedPart->path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot>sbEntity(mountedPart->contentStart, &file);
    auto& sb = sbEntity.value;

    if (sb.magic != SuperBoot::MAGIC) {
        Consola::reportarError("el montaje no es un sistema de archivos");
        return -1;
    }

    std::ofstream outfile(path);
    if (!outfile.is_open()) {
        std::string dirPath = path.substr(0, path.find_last_of('/'));
        int status = system((std::string("mkdir -p ") + dirPath).c_str());
        if(status != 0){
            return -1;
        }
        outfile.open(path);
    }

    std::string header("*********************************************************\n"
                       "*----------------------Journaling-----------------------*\n"
                       "*********************************************************\n\n"
                       "FS: EXT3\n"
                       "PART: ");
    header += idPart + "\n\n";

    //Bad performance. No han necesida de hacer otro string ademas de header pero facilita hacer los resize. bleh!
    std::string tableHeader("");
    tableHeader += "DATE";
    tableHeader.resize(17, ' ');
    tableHeader += "USER";
    tableHeader.resize(29, ' ');
    tableHeader += "COMMAND\n";
    header += tableHeader;

    outfile << header;
    std::cout << header;
    int journAddress = sbEntity.address + sizeof(SuperBoot);
    while(true){
        if(journAddress >= sb.bitmapInodeBegin){
            break;
        }
        file.seekg(journAddress, std::ios::beg);
        JournEntry journEntry;
        file.read((char*)&journEntry, sizeof(JournEntry));
        if(journEntry.key==0){
            break;
        }
        std::string journEntryString = journEntryToString(&journEntry);
        outfile << journEntryString;
        std::cout << journEntryString;
        journAddress += sizeof (JournEntry);
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

std::string Reportes::journEntryToString(JournEntry *journEntry)
{
    using namespace Interprete;
    using namespace MyStringUtil;

    auto& args = journEntry->args;//bleh: codigo algo feito
    std::string s("");
    s += dateToString(journEntry->time, "%d-%m-%y %H:%M");
    s.resize(17, ' ');
    s = s + journEntry->user;
    s.resize(29, ' ');

    switch(journEntry->key){
        case Cmd::LOGIN:
            s = s + "login ";
            s = s + "-name=" + args.login.name + " ";
            s = s + "-pwd=" + args.login.pwd + " ";
            s = s + "-id=" + args.login.id + " ";
            s = s + "\n";
            return s;

        case Cmd::LOGOUT:
            s = s + "logout ";
            s = s + "\n";
            return s;

        case Cmd::MKGRP:
            s = s + "mkgrp ";
            s = s + "-name=" + args.mkGrp.name + " ";
            s = s + "\n";
            return s;

        case Cmd::RMGRP:
            s = s + "rmgrp ";
            s = s + "-name=" + args.rmGrp.name + " ";
            s = s + "\n";
            return s;

        case Cmd::MKUSR:
            s = s + "mkusr ";
            s = s + "-name=" + args.mkUsr.name + " ";
            s = s + "-pwd=" + args.mkUsr.pwd + " ";
            s = s + "-grp=" + args.mkUsr.grp + " ";
            s = s + "\n";
            return s;

        case Cmd::RMUSR:
            s = s + "rmusr ";
            s = s + "-name=" + args.rmUsr.name + " ";
            s = s + "\n";
            return s;

        case Cmd::CHMOD:
            s = s + "chmod ";
            s = s + "-path=" + args.chMod.path + " ";
            s = s + "-ugo=" + std::to_string(args.chMod.ugo) + " ";
            s = s + "-r=" + std::to_string(args.chMod.r) + " ";
            s = s + "\n";
            return s;

        case Cmd::MKFILE:
            s = s + "mkfile ";
            s = s + "-path=" + args.mkFile.path + " ";
            s = s + "-cont=" + args.mkFile.cont + " ";
            s = s + "-p=" + std::to_string(args.mkFile.p) + " ";
            s = s + "-size=" + std::to_string(args.mkFile.size) + " ";
            s = s + "\n";
            return s;

        case Cmd::CAT:
            s = s + "cat ";
            s = s + "-file=" + args.cat.file + " ";
            s = s + "\n";
            return s;

        case Cmd::REM:
            s = s + "rem ";
            s = s + "-path=" + args.rem.path + " ";
            s = s + "\n";
            return s;

        case Cmd::EDIT:
            s = s + "edit ";
            s = s + "-path=" + args.edit.path + " ";
            s = s + "-cont=" + args.edit.cont + " ";
            s = s + "\n";
            return s;

        case Cmd::REN:
            s = s + "ren ";
            s = s + "-path=" + args.ren.path + " ";
            s = s + "-name=" + args.ren.name + " ";
            s = s + "\n";
            return s;

        case Cmd::MKDIR:
            s = s + "mkdir ";
            s = s + "-path=" + args.mkDir.path + " ";
            s = s + "-p=" + std::to_string(args.mkDir.p) + " ";
            s = s + "\n";
            return s;

        case Cmd::CP:
            s = s + "cp ";
            s = s + "-path=" + args.cp.path + " ";
            s = s + "-dest=" + args.cp.dest + " ";
            s = s + "\n";
            return s;

        case Cmd::MV:
            s = s + "mv ";
            s = s + "-path=" + args.mv.path + " ";
            s = s + "-dest=" + args.mv.dest + " ";
            s = s + "\n";
            return s;

        case Cmd::FIND:
            s = s + "find ";
            s = s + "-path=" + args.find.path + " ";
            s = s + "-name=" + args.find.name + " ";
            s = s + "\n";
            return s;

        case Cmd::CHOWN:
            s = s + "chown ";
            s = s + "-path=" + args.chown.path + " ";
            s = s + "-r=" + std::to_string(args.chown.r) + " ";
            s = s + "-usr=" + args.chown.usr + " ";
            s = s + "\n";
            return s;

        case Cmd::CHGRP:
            s = s + "chgrp ";
            s = s + "-usr=" + args.chgrp.usr + " ";
            s = s + "-grp=" + args.chgrp.grp + " ";
            s = s + "\n";
            return s;
    }
}

std::vector<char> Reportes::getBytes(RaidOneFile* file, int start, int size)
{
    std::vector<char> byteMap(size);
    file->seekg(start, std::ios::beg);
    file->read(&byteMap[0], size);

    return byteMap;
}

