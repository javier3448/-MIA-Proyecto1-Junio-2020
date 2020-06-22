#include "extmanager.h"

#include "Disk/diskmanager.h"
#include "FileSystem/superboot.h"
#include "pointerblock.h"
#include "consola.h"
#include "Analizador/extparams.h"
#include "mystringutil.h"
#include "FileSystem/journaling.h"
#include "FileSystem/journmanager.h"
#include "FileSystem/extutility.h" //DEBUG ONLY!!! TODO  QUITAR!!!!!

std::optional<UserSession> ExtManager::currUserSession = {};

//make file system

int ExtManager::mkfs(const std::string& id, char formatType)
{
    //Conseguir partition:
    auto mountedPart = DiskManager::getMountedPartition(id);
    if(!mountedPart)
    {
        Consola::reportarError("No se ha montado una particion con el id: " + id);
        return -1;
    }
    if(mountedPart->type == MountedPart::T_EXTENDED){
        Consola::reportarError("No se puede hacer un file system dentro de una particion extendida");
        return -1;
    }

    RaidOneFile file(mountedPart->path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(mountedPart->contentStart, SuperBoot(SuperBoot::FS_TYPE_EXT3, mountedPart->contentStart, mountedPart->contentSize));
    if(formatType == ExtParamsConstants::FORMAT_TYPE_FULL){
        file.seekp(mountedPart->contentStart + sizeof(SuperBoot), std::ios::beg);
        std::vector<char> zeros(mountedPart->contentSize - sizeof(SuperBoot));
        fill(zeros.begin(), zeros.end(), 0);
        file.write(&zeros[0], mountedPart->contentSize);
    }else{
        //tenemos que limpear los bitmaps y los journalings
        int bitmapsBeg = sizeof(SuperBoot) + mountedPart->contentStart;
        int bitmapsSize = sbEntity.value.inodeStart - bitmapsBeg;
        file.seekp(bitmapsBeg, std::ios::beg);
        std::vector<char> zeros((ulong)bitmapsSize);
        fill(zeros.begin(), zeros.end(), 0);
        file.write(&zeros[0], bitmapsSize);
    }
    sbEntity.updateDiskValue(&file);

    return createRootAndUsrsTxt(&file, &sbEntity);
}

int ExtManager::login(const std::string &usr, const std::string &pwd, const std::string &id)
{
    if(currUserSession)
    {
        Consola::reportarError("Solo se puede tener un usuario logeado");
        return -1;
    }
    auto mountedPart = DiskManager::getMountedPartition(id);
    if(!mountedPart)
    {
        Consola::reportarError("No se ha montado una particion con el id: " + id);
        return -1;
    }
    if(mountedPart->type == MountedPart::T_EXTENDED){
        Consola::reportarError("No se puede hacer un file system dentro de una particion extendida");
        return -1;
    }

    RaidOneFile file(mountedPart->path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(mountedPart->contentStart, &file);
    //Agregamos entrada al journaling
    JournManager::addLogin(&file, &sbEntity, "null", usr, pwd, id);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    auto loggedInUser = usersData.tryLogin(usr, pwd);
    if(!loggedInUser)
    {
        Consola::reportarError("No existe ningun usuario con el nombre: " + usr + " y contrasenna: " + pwd);
        return -1;
    }

    currUserSession = UserSession(mountedPart.value(), loggedInUser.value());

    return 1;
}

int ExtManager::logout()
{
    if(!currUserSession){
        Consola::reportarError("Ningun usuario loggeado");
        return -1;
    }
    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addLogout(&file, &sbEntity, currUserSession->user.name);

    currUserSession.reset();
    return 0;
}

int ExtManager::mkGrp(const std::string &name)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }
    if(!currUserSession->isRoot()){
        Consola::reportarError("El usuario debe ser root");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addMkGrp(&file, &sbEntity, currUserSession->user.name, name);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    if(usersData.hasGroup(name)){
        Consola::reportarError("Ya existe un grupo con el nombre: " + name);
        return -1;
    }

    int grpId = usersData.getMaxGroupId();
    std::string newGrpText = usersData.getGroupString(++grpId, name);

    if(fileConcat(&file, &sbEntity, &usersEntity, newGrpText) == -1){
        Consola::reportarError("No hay espacio disponible");
        return -1;
    }

    sbEntity.updateDiskValue(&file);

    return 0;
}

int ExtManager::rmGrp(const std::string &name)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }
    if(!currUserSession->isRoot()){
        Consola::reportarError("El usuario debe ser root");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addRmGrp(&file, &sbEntity, currUserSession->user.name, name);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    if(!usersData.tryRemoveGroup(name)){
        Consola::reportarError("No existe ningun grupo con el nombre: " + name);
        return -1;
    }

    if(fileEdit(&file, &sbEntity, &usersEntity, usersData.getUsersTxt()) == -1){
        Consola::reportarError("No hay espacio disponible");
        return -1;
    }

    sbEntity.updateDiskValue(&file);

    return 0;
}

int ExtManager::mkUsr(const std::string &usr, const std::string &grp, const std::string &pwd)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }
    if(!currUserSession->isRoot()){
        Consola::reportarError("El usuario debe ser root");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addMkUsr(&file, &sbEntity, currUserSession->user.name, usr, grp, pwd);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    if(usersData.hasUser(usr)){
        Consola::reportarError("Ya existe un usuario con el nombre: " + usr);
        return -1;
    }

    if(!usersData.hasGroup(grp)){
        Consola::reportarError("No existe ningun grupo con el nombre: " + usr);
        return -1;
    }

    int usrId = usersData.getMaxUserId();
    std::string newUsrText = usersData.getUserString(++usrId, usr, grp, pwd);

    if(fileConcat(&file, &sbEntity, &usersEntity, newUsrText) == -1){
        Consola::reportarError("No hay espacio disponible");
        return -1;
    }

    sbEntity.updateDiskValue(&file);

    return 0;
}

int ExtManager::rmUsr(const std::string &usr)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }
    if(!currUserSession->isRoot()){
        Consola::reportarError("El usuario debe ser root");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addRmUsr(&file, &sbEntity, currUserSession->user.name, usr);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    if(!usersData.tryRemoveUser(usr)){
        Consola::reportarError("No existe ningun grupo con el nombre: " + usr);
        return -1;
    }

    if(fileEdit(&file, &sbEntity, &usersEntity, usersData.getUsersTxt()) == -1){
        Consola::reportarError("No hay espacio disponible");
        return -1;
    }

    sbEntity.updateDiskValue(&file);

    return 0;
}

//
int ExtManager::chmod(const std::string &path, int ugo, bool r, int paramsUgo)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addChmod(&file, &sbEntity, currUserSession->user.name, path, paramsUgo, r);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    DiskEntity<Inode> rootEntity = getRoot(&file, &sbEntity);

    std::string relativePath;
    MyStringUtil::removeSlash(path, relativePath);//Crea el path relativo al root (i.e. quita el primer slash)

    auto[closestInode, remainingPath] = folderGetClosestInode(&file, &rootEntity, relativePath);

    if(remainingPath.length() > 0){
        Consola::reportarError("No existe la direccion: <" + path + ">");
        return -1;
    }

    int usrId = currUserSession->user.id;

    if(!closestInode.value.isUsrOwner(usrId) &&
        !currUserSession->isRoot()){
        Consola::reportarError("El usuario: " + currUserSession->user.name + " no es propietario de: "
                                                                             "<" + path + ">");
        return -1;
    }

    if(r){
        if(recursiveChmod(&file, &closestInode, usrId, ugo) == -1){
            Consola::reportarError("No se pudo aplicar chmod a todos los archivos dentro de " + path +
                                   "\n el usuario " + currUserSession->user.name + " no es propietarios");
            return -1;
        }
        return 0;
    }
    else{
        closestInode.value.permissions = ugo;
        closestInode.value.updateModificationTime();
        closestInode.updateDiskValue(&file);
        return 0;
    }
}

int ExtManager::chown(const std::string &path, bool r, const std::string &usr)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addChown(&file, &sbEntity, currUserSession->user.name, path, r, usr);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    DiskEntity<Inode> rootEntity = getRoot(&file, &sbEntity);

    int newUsrId = usersData.getUserId(usr);
    if(newUsrId == -1){
        Consola::reportarError("No existe un usuario con el nombre: " + usr);
        return -1;
    }

    std::string relativePath;
    MyStringUtil::removeSlash(path, relativePath);//Crea el path relativo al root (i.e. quita el primer slash)

    auto[closestInode, remainingPath] = folderGetClosestInode(&file, &rootEntity, relativePath);

    if(remainingPath.length() > 0){
        Consola::reportarError("No existe la direccion: <" + path + ">");
        return -1;
    }

    int usrId = currUserSession->user.id;

    if(!closestInode.value.isUsrOwner(usrId) &&
        !currUserSession->isRoot()){
        Consola::reportarError("El usuario: " + currUserSession->user.name + " no es propietario de: "
                                                                             "<" + path + ">");
        return -1;
    }

    if(r){
        if(recursiveChown(&file, &closestInode, usrId, newUsrId) == -1){
            Consola::reportarError("No se pudo aplicar chmod a todos los archivos dentro de " + path +
                                   "\n el usuario " + currUserSession->user.name + " no es propietarios");
            return -1;
        }
        return 0;
    }
    else{
        closestInode.value.usrId = newUsrId;
        closestInode.value.updateModificationTime();
        closestInode.updateDiskValue(&file);
        return 0;
    }
}

int ExtManager::chgrp(const std::string &usr, const std::string &grp)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }
    if(!currUserSession->isRoot()){
        Consola::reportarError("El usuario debe ser root");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addChgrp(&file, &sbEntity, currUserSession->user.name, usr, grp);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    if(!usersData.tryChangeGrp(usr, grp)){
        //Chapuz, deberia de decir cual de los dos es el que no existe
        Consola::reportarError("No existe usuario o grupo");
        return -1;
    }

    return fileEdit(&file, &sbEntity, &usersEntity, usersData.getUsersTxt());
}

int ExtManager::mkFile(const std::string &path, bool p, const std::string &content, int paramsSize, const std::string &paramsCont)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addMkFile(&file, &sbEntity, currUserSession->user.name, path, p, paramsSize, paramsCont);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    DiskEntity<Inode> rootEntity = getRoot(&file, &sbEntity);

    std::string relativePath;
    MyStringUtil::removeSlash(path, relativePath);//Crea el path relativo al root (i.e. quita el primer slash)
    std::string folderPath;
    std::string fileName;
    MyStringUtil::splitFolderPathAndFileName(path, folderPath, fileName);

    auto[closestInode, remainingPath] = folderGetClosestInode(&file, &rootEntity, folderPath);

    if(remainingPath.length() != 0 && !p){
        std::string folderPath;
        std::string folderName;
        MyStringUtil::splitFolderPathAndFileName(path, folderPath, folderName);
        Consola::reportarError("No existe el directorio: <" + folderPath + ">");
        return -1;
    }

    if(closestInode.value.isFile()){
        std::string wrongFilePath = path.substr(0, path.length() - remainingPath.length());
        Consola::reportarError("El path: </" + wrongFilePath + "> es un archivo, no una carpeta");
        return -1;
    }

    if(remainingPath.length() < 1 &&
        folderGetSubInode(&file, &closestInode, fileName)){//
        Consola::reportarError("El archivo: <" + path + "> ya existe");
        return -1;
    }


    int usrId = currUserSession->user.id;
    int grpId= usersData.getGroupId(currUserSession->user.groupName);
    int permissions = 0x664;

    if(!closestInode.value.canUsrWrite(usrId, grpId)){
        //wrongPath seria el path de el lastInode
        std::string wrongFilePath = folderPath.substr(0, folderPath.length() - remainingPath.length());
        Consola::reportarError("El usuario: " + currUserSession->user.name + " no tiene permisos de escritura en: "
                                                                             "</" + wrongFilePath + ">");
        return -1;
    }

    if(!folderMkfileRecursively(&file, &sbEntity, &closestInode, usrId, grpId, permissions, remainingPath, fileName, content)){
        Consola::reportarError("No se pudo crear el archivo: <" + path + ">");
        return -1;
    }

    sbEntity.updateDiskValue(&file);

    return 0;
}

int ExtManager::cp(const std::string &path, const std::string &dest)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addCp(&file, &sbEntity, currUserSession->user.name, path, dest);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    DiskEntity<Inode> rootEntity = getRoot(&file, &sbEntity);

    std::string relativeSourcePath;
    MyStringUtil::removeSlash(path, relativeSourcePath);//Crea el path relativo al root (i.e. quita el primer slash)
    std::string relativeDestPath;
    MyStringUtil::removeSlash(dest, relativeDestPath);//Crea el path relativo al root (i.e. quita el primer slash)

    auto[sourceInode, sourceRemainingPath] = folderGetClosestInode(&file, &rootEntity, relativeSourcePath);
    if(sourceRemainingPath.length() > 0){
        Consola::reportarError("No existe el path source <" + path + ">");
        return -1;
    }
    auto[destParentInode, destRemainingPath] = folderGetClosestInode(&file, &rootEntity, relativeDestPath);
    if(destRemainingPath.length() > 0){
        Consola::reportarError("No existe el path dest <" + dest + ">");
        return -1;
    }

    //Chequeamos que el directorio destino tenga suficiente espacio
    if(destParentInode.value.size + 1 >= Inode::MAX_FILE_SIZE){
        Consola::reportarError("directorio: " + dest + " lleno");
        return -1;
    }

    if(destParentInode.value.isFile()){
        Consola::reportarError("dest <" + dest + "> deber ser un directorio");
        return -1;
    }
    int usrId = currUserSession->user.id;
    int grpId= usersData.getGroupId(currUserSession->user.groupName);
    if(!destParentInode.value.canUsrWrite(usrId, grpId)){
        Consola::reportarError("No tiene permiso de escritura en dest <" + dest + ">");
        return -1;
    }

    std::string sourceName = path.substr(path.find_last_of("/") + 1);
    std::string destName = sourceName;
    //En caso que el usuario loco este tratando de copiar la carpeta root
    if(destName.length() < 1){
        destName = "root";
    }
    //Revisamos que dest no tenga un subinodo con el mismo nombre
    //Bad performance. Recorremos dos veces la lista de subinodos
    //Mucha mem innecesaria, solo necesitamos consultar los nombre
    auto subInodes = folderGetSubNamedInodes(&file, &destParentInode);
    for(auto[name, inode]: subInodes){
        if(name == destName){
            Consola::reportarError("Ya existe el archivo: " + dest + "/" + destName);
            return -1;
        }
    }
    //Chequeamos que tenga persmisos de lectura en en Todo source.
    //Chapuz: en linux y el enunciado -_- no revisa antes y solo se salta los archivos
    //que no tiene permiso de lectura
    if(!folderCanUsrCopy(&file, sourceName, &sourceInode, usrId, grpId, currUserSession->user.name)){
        Consola::reportarError("No tiene permisos de lectura en los subarchivos de " + path);
        return -1;
    }

    int sourceInodeCount = 0;
    int sourceBlockCount = 0;
    getInodeAndBlockCount(&file, &sourceInode, sourceInodeCount, sourceBlockCount);
    if(sourceInodeCount > sbEntity.value.freeInodesCount){
        Consola::reportarError("No tiene espacio suficiente espacio para crear " + std::to_string(sourceInodeCount) + " inodos");
        return -1;
    }
    //chapuz: +4 porque agregar el inodo copiado a la carpeta destino puede
    //requerir un maximo de 4 bloques adicionales
    if(sourceBlockCount + 4 > sbEntity.value.freeBlocksCount){
        Consola::reportarError("No tiene espacio suficiente espacio para crear " + std::to_string(sourceBlockCount) + " inodos");
        return -1;
    }

    DiskEntity<Inode> destInode;
    if(sourceInode.value.isFile()){
        destInode = fileCopy(&file, &sbEntity, &sourceInode, usrId, grpId, 0x664);
    }
    else{
        destInode = folderCopy(&file, &sbEntity, &sourceInode, usrId, grpId, 0x664);
    }

    folderAddSubInode(&file, &sbEntity, &destParentInode, destName, destInode.address);

    sbEntity.updateDiskValue(&file);

    return 0;
}

int ExtManager::mv(const std::string &path, const std::string &dest)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addMv(&file, &sbEntity, currUserSession->user.name, path, dest);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    DiskEntity<Inode> rootEntity = getRoot(&file, &sbEntity);

    std::string relativeSourcePath;
    MyStringUtil::removeSlash(path, relativeSourcePath);//Crea el path relativo al root (i.e. quita el primer slash)
    std::string sourceParentPath;
    std::string sourceName;
    MyStringUtil::splitFolderPathAndFileName(path, sourceParentPath, sourceName);
    std::string relativeDestPath;
    MyStringUtil::removeSlash(dest, relativeDestPath);//Crea el path relativo al root (i.e. quita el primer slash)

    auto[sourceParentInode, sourceRemainingPath] = folderGetClosestInode(&file, &rootEntity, sourceParentPath);
    if(sourceRemainingPath.length() > 0){
        Consola::reportarError("No existe el path source <" + path + ">");
        return -1;
    }
    auto[destParentInode, destRemainingPath] = folderGetClosestInode(&file, &rootEntity, relativeDestPath);
    if(destRemainingPath.length() > 0){
        Consola::reportarError("No existe el path dest <" + dest + ">");
        return -1;
    }

    //Chequeamos que el directorio destino tenga suficiente espacio
    if(destParentInode.value.size + 1 >= Inode::MAX_FILE_SIZE){
        Consola::reportarError("directorio: " + dest + " lleno");
        return -1;
    }

    if(destParentInode.value.isFile()){
        Consola::reportarError("dest <" + dest + "> deber ser un directorio");
        return -1;
    }
    int usrId = currUserSession->user.id;
    int grpId= usersData.getGroupId(currUserSession->user.groupName);
    if(!destParentInode.value.canUsrWrite(usrId, grpId)){
        Consola::reportarError("No tiene permiso de escritura en dest <" + dest + ">");
        return -1;
    }

    std::string destName = sourceName;
    //En caso que el usuario loco este tratando de copiar la carpeta root
    if(destName.length() < 1){
        destName = "root";
    }
    //Revisamos que dest no tenga un subinodo con el mismo nombre
    //Bad performance. Recorremos dos veces la lista de subinodos
    //Mucha mem innecesaria, solo necesitamos consultar los nombre
    auto subInodes = folderGetSubNamedInodes(&file, &destParentInode);
    for(auto[name, inode]: subInodes){
        if(name == destName){
            Consola::reportarError("Ya existe el archivo: " + dest + "/" + destName);
            return -1;
        }
    }

    //Chequeamos que tenga persmisos de escritura solo en la carpeta origen
    //asi indica el enunciado y con las pruebas superficiales que hicimos parece que asi
    //lo hace linux tambien
    auto sourceInode = folderGetSubInode(&file, &sourceParentInode, sourceName);
    if(!sourceInode){
        Consola::reportarError("No existe la carpeta: " + path);
        return -1;
    }
    if(!sourceInode->value.canUsrWrite(usrId, grpId)){
        Consola::reportarError("No tiene permisos de escritura en " + path);
        return -1;
    }

    //Bad performanace
    //Chapuz conseguimos el source inode dos veces!!! Una vez para revisar permisos y
    //la otra para romper el enlace con su inodo padre
    //Deberiamos de hacer un popSubInode que revise permisos de una vez
    folderPopSubInode(&file, &sourceParentInode, sourceName);

    folderAddSubInode(&file, &sbEntity, &destParentInode, destName, sourceInode->address);

    sbEntity.updateDiskValue(&file);

    return 0;
}

int ExtManager::find(const std::string &path, const std::string &name)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addFind(&file, &sbEntity, currUserSession->user.name, path, name);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    DiskEntity<Inode> rootEntity = getRoot(&file, &sbEntity);

    std::string relativePath;
    MyStringUtil::removeSlash(path, relativePath);//Crea el path relativo al root (i.e. quita el primer slash)

    auto[closestInode, remainingPath] = folderGetClosestInode(&file, &rootEntity, relativePath);

    if(remainingPath.length() != 0){
        Consola::reportarError("No existe el archivo: <" + path + ">");
        return -1;
    }

    if(!closestInode.value.isFolder()){
        Consola::reportarError("El path: </" + path + "> es un archivo, no un directorio");
        return -1;
    }

    int usrId = currUserSession->user.id;
    int grpId= usersData.getGroupId(currUserSession->user.groupName);

    if(!closestInode.value.canUsrRead(usrId, grpId)){
        Consola::reportarError("El usuario: " + currUserSession->user.name + " no tiene permisos de lectura en: "
                                                                             "<" + path + ">");
        return -1;
    }

    std::string inodeName = path.substr(path.find_last_of('/') + 1);

    std::string buffer("");

    //Bad practice to use a reference but I cant think of another way to use the
    //same string in the whole recursion
    findImp(&file, inodeName, &closestInode, buffer, name, 0);

    Consola::printLine(buffer);

    return 0;
}

int ExtManager::cat(const std::string &path)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addCat(&file, &sbEntity, currUserSession->user.name, path);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    DiskEntity<Inode> rootEntity = getRoot(&file, &sbEntity);

    std::string relativePath;
    MyStringUtil::removeSlash(path, relativePath);//Crea el path relativo al root (i.e. quita el primer slash)

    auto[closestInode, remainingPath] = folderGetClosestInode(&file, &rootEntity, relativePath);

    if(remainingPath.length() != 0){
        Consola::reportarError("No existe el archivo: <" + path + ">");
        return -1;
    }

    if(closestInode.value.isFolder()){
        std::string wrongFilePath = path.substr(0, path.length() - remainingPath.length());
        Consola::reportarError("El path: </" + wrongFilePath + "> es un folder, no una carpeta");
        return -1;
    }

    int usrId = currUserSession->user.id;
    int grpId= usersData.getGroupId(currUserSession->user.groupName);

    if(!closestInode.value.canUsrRead(usrId, grpId)){
        Consola::reportarError("El usuario: " + currUserSession->user.name + " no tiene permisos de lectura en: "
                                                                             "<" + path + ">");
        return -1;
    }

    std::string content = fileGetContent(&file, &closestInode);

    Consola::printLine(content);
    return 0;
}

int ExtManager::rem(const std::string &path)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addRem(&file, &sbEntity, currUserSession->user.name, path);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    DiskEntity<Inode> rootEntity = getRoot(&file, &sbEntity);

    std::string relativePath;
    MyStringUtil::removeSlash(path, relativePath);//Crea el path relativo al root (i.e. quita el primer slash)
    std::string folderPath;
    //El nombre del archivo/directorio que vamos a borrar
    std::string targetName;
    MyStringUtil::splitFolderPathAndFileName(path, folderPath, targetName);

    //Tenemos que conseguir el inodo del padre primero porque tenemos que eliminar
    //El puntero que apunta a la carpeta que se va a elimanar, dicho puntero esta en el inodo del padre
    auto[parentInode, remainingPath] = folderGetClosestInode(&file, &rootEntity, folderPath);

    if(remainingPath.length() != 0){
        Consola::reportarError("No existe el directorio: <" + path + ">");
        return -1;
    }

    if(parentInode.value.isFile()){
        Consola::reportarError("El path: </" + folderPath + "> es un archivo, no una carpeta");
        return -1;
    }

    //Chapuz: conseguimos target inode 2 veces, unca con folderGetSubInode y otra andentro de
    //folderRemoveSubInode, deberiamos de hacer la verificacion de folderCanUsrRemove desde
    //adentro de folderRemvoerSubInode
    auto targetInode = folderGetSubInode(&file, &parentInode, targetName);
    if(!targetInode){
        Consola::reportarError("NO existe el directorio: <" + path + ">");
        return -1;
    }

    int usrId = currUserSession->user.id;
    int grpId= usersData.getGroupId(currUserSession->user.groupName);
    //El isRoot al principio del && hace que no tengamos que hacer todo el canUsrRemove
    //En caso de que el usuario sea root. for Better performace
    if(!currUserSession->isRoot() &&
        !folderCanUsrRemove(&file, targetName, &targetInode.value(), usrId, grpId, currUserSession->user.name)){
        Consola::reportarError("No tiene todos los permisos sobre: <" + path + ">");
        return -1;
    }

    if(folderRemoveSubInode(&file, &sbEntity, &parentInode, targetName) == -1){
        Consola::reportarError("No se pudo elimnar: <" + path + ">");
        return -1;
    }

    sbEntity.updateDiskValue(&file);

    return 0;
}

int ExtManager::edit(const std::string &path, const std::string &content)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addEdit(&file, &sbEntity, currUserSession->user.name, path, content);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    DiskEntity<Inode> rootEntity = getRoot(&file, &sbEntity);
    std::string relativePath;
    MyStringUtil::removeSlash(path, relativePath);//Crea el path relativo al root (i.e. quita el primer slash)

    auto[fileInode, remainingPath] = folderGetClosestInode(&file, &rootEntity, relativePath);

    if(remainingPath.length() > 0){
        Consola::reportarError("No existe el archivo: " + path);
        return -1;
    }
    if(!fileInode.value.isFile()){
        Consola::reportarError(path + " no es un archivo");
        return -1;
    }

    //permisos. El enunciado pide de escritura y lectura
    int usrId = currUserSession->user.id;
    int grpId= usersData.getGroupId(currUserSession->user.groupName);
    if(!fileInode.value.canUsrRead(usrId, grpId)){
        Consola::reportarError("No tiene permisos de lectura en: " + path);
        return -1;
    }
    if(!fileInode.value.canUsrWrite(usrId, grpId)){
        Consola::reportarError("No tiene permisos de escritura en: " + path);
        return -1;
    }

    if(fileConcat(&file, &sbEntity, &fileInode, content) == -1){
        Consola::reportarError("No hay espacio disponible");
        return -1;
    }

    sbEntity.updateDiskValue(&file);

    return 0;
}

int ExtManager::ren(const std::string &path, const std::string &name)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addRen(&file, &sbEntity, currUserSession->user.name, path, name);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    DiskEntity<Inode> rootEntity = getRoot(&file, &sbEntity);

    std::string parentPath;
    std::string oldName;
    MyStringUtil::splitFolderPathAndFileName(path, parentPath, oldName);

    auto[parentInode, parentRemainingPath] = folderGetClosestInode(&file, &rootEntity, parentPath);
    if(parentRemainingPath.length() > 0){
        Consola::reportarError("No existe el path dest <" + path + ">");
        return -1;
    }

    if(parentInode.value.isFile()){
        Consola::reportarError("No existe el path dest <" + path + ">");
        return -1;
    }

    auto subInodes = folderGetSubNamedInodes(&file, &parentInode);
    for(auto[inodeName, inode]: subInodes){
        if(inodeName == name){
            Consola::reportarError("Ya existe el archivo: " + name);
            return -1;
        }
    }

    int usrId = currUserSession->user.id;
    int grpId= usersData.getGroupId(currUserSession->user.groupName);
    return folderChangeSubInodeName(&file, &parentInode, oldName, name, usrId, grpId);
}

int ExtManager::mkDir(const std::string &path, bool p)
{
    if(!currUserSession){
        Consola::reportarError("No se a loggeado ningun usuario");
        return -1;
    }

    RaidOneFile file(currUserSession->part.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(currUserSession->part.contentStart, &file);
    //Agregamos al journaling
    JournManager::addMkDir(&file, &sbEntity, currUserSession->user.name, path, p);

    DiskEntity<Inode> usersEntity = getUsers(&file, &sbEntity);
    std::string usersFileContent = fileGetContent(&file, &usersEntity);
    UsersData usersData(usersFileContent);

    DiskEntity<Inode> rootEntity = getRoot(&file, &sbEntity);

    std::string relativePath;
    MyStringUtil::removeSlash(path, relativePath);//Crea el path relativo al root (i.e. quita el primer slash)

    auto[closestInode, remainingPath] = folderGetClosestInode(&file, &rootEntity, relativePath);

    if(remainingPath.length() < 1){
        Consola::reportarError("Ya existe el directorio: <" + path + ">");
        return -1;
    }

    if(std::count(remainingPath.begin(), remainingPath.end(), '/') > 0
        && !p){
        std::string folderPath;
        std::string folderName;
        MyStringUtil::splitFolderPathAndFileName(path, folderPath, folderName);
        Consola::reportarError("No existe el directorio: </" + folderPath + ">");
        return -1;
    }

    if(closestInode.value.isFile()){
        std::string wrongFilePath = path.substr(0, path.length() - remainingPath.length());
        Consola::reportarError("El path: </" + wrongFilePath + "> es un archivo, no una carpeta");
        return -1;
    }

    int usrId = currUserSession->user.id;
    int grpId= usersData.getGroupId(currUserSession->user.groupName);
    int permissions = 0x664;

    if(!closestInode.value.canUsrWrite(usrId, grpId)){
        //TODO: revisar este error
        std::string wrongFilePath = relativePath.substr(0, relativePath.length() - remainingPath.length());
        Consola::reportarError("El usuario: " + currUserSession->user.name + " no tiene permisos de escritura en: "
                                                                             "</" + wrongFilePath + ">");
        return -1;
    }

    if(!folderMkfolderRecursively(&file, &sbEntity, &closestInode, usrId, grpId, permissions, remainingPath)){
        Consola::reportarError("No se pudo crear la carpeta: <" + path + ">");
        return -1;
    }

    sbEntity.updateDiskValue(&file);

    return 0;
}

bool ExtManager::updateMountTime(RaidOneFile *file, const MountedPart& mountedPart)
{
    DiskEntity<SuperBoot> sbEntity(mountedPart.contentStart, file);
    if(sbEntity.value.magic != SuperBoot::MAGIC){
        return false;
    }
    sbEntity.value.updateLastMountTime();
    sbEntity.updateDiskValue(file);
    return true;
}

bool ExtManager::updateUnmountTime(const MountedPart& mountedPart)
{
    RaidOneFile file(mountedPart.path, std::ios::binary | std::ios::in | std::ios::out);
    DiskEntity<SuperBoot> sbEntity(mountedPart.contentStart, &file);
    if(sbEntity.value.magic != SuperBoot::MAGIC){
        return false;
    }
    sbEntity.value.updateLastMountTime();
    sbEntity.updateDiskValue(&file);
    return true;
}

DiskEntity<Inode> ExtManager::getRoot(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity)
{
    SuperBoot& sb = sbEntity->value;
    return DiskEntity<Inode>(sb.inodeStart, file);//El primer inodo siempre sera el root
}

DiskEntity<Inode> ExtManager::getUsers(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity)
{
    SuperBoot& sb = sbEntity->value;
    return DiskEntity<Inode>(sb.inodeStart + (int)sizeof(Inode), file);//El segundo inodo siempre sera el users.txt
}


int ExtManager::recursiveChmod(RaidOneFile *file, DiskEntity<Inode> *inodeEntity, int usrId, int ugo)
{
    auto& inode = inodeEntity->value;
    //usrId != 1. is the same as saying isUsrNotRoot
    if(!inode.isUsrOwner(usrId) &&
        usrId != 1){
        return -1;
    }
    int result = 0;
    inode.permissions = ugo;
    inode.updateModificationTime();
    inodeEntity->updateDiskValue(file);

    if(inode.isFolder()){
        auto subInodes = folderGetSubInodes(file, inodeEntity);
        for (auto subInodeEntity : subInodes) {
            if(recursiveChmod(file, &subInodeEntity, usrId, ugo) == -1){
                result = -1;
            }
        }
    }
    return result;
}

int ExtManager::recursiveChown(RaidOneFile *file, DiskEntity<Inode> *inodeEntity, int usrId, int newUsrId)
{
    auto& inode = inodeEntity->value;
    //usrId != 1. is the same as saying isUsrNotRoot
    if(!inode.isUsrOwner(usrId) &&
        usrId != 1){
        return -1;
    }
    int result = 0;
    inode.usrId = newUsrId;
    inode.updateModificationTime();
    inodeEntity->updateDiskValue(file);

    if(inode.isFolder()){
        auto subInodes = folderGetSubInodes(file, inodeEntity);
        for (auto subInodeEntity : subInodes) {
            if(recursiveChown(file, &subInodeEntity, usrId, newUsrId) == -1){
                result = -1;
            }
        }
    }
    return result;
}

std::string ExtManager::fileGetContent(RaidOneFile *file, DiskEntity<Inode> *fileEntity)
{
    fileEntity->value.updateAccessTime();

    std::string content("");
    int size = fileEntity->value.size;
    content.reserve((ulong)size);//Podriamos usar resize pero tendriamos que indexar en los metodos recursivos y no se si resize tendria tiene mejor performance que reserve
    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE && size > 0; i++) {
        fileGetContentImp(file, content, size, 0, fileEntity->value.blocks[i]);
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE && size > 0; i++){
        fileGetContentImp(file, content, size, 1, fileEntity->value.blocks[i]);
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE && size > 0; i++){
        fileGetContentImp(file, content, size, 2, fileEntity->value.blocks[i]);
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE && size > 0; i++){
        fileGetContentImp(file, content, size, 3, fileEntity->value.blocks[i]);
    }

    return content;
}

void ExtManager::fileGetContentImp(RaidOneFile *file, std::string &content, int &size, char depth, int address)
{
    if(depth == 0){
        DiskEntity<FileBlock> fileBlock(address, file);
        auto& blockContent = fileBlock.value.content;
        if(size < FileBlock::CONTENT_SIZE){
            content.append(blockContent, 0, size);
            size = 0;
        }
        else{
            content.append(blockContent, 0, FileBlock::CONTENT_SIZE);
            size -= FileBlock::CONTENT_SIZE;
        }
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock(address, file);
        auto& pointers = pointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE && pointers[i] != -1; i++) {
            fileGetContentImp(file, content, size, depth - 1, pointers[i]);
        }
    }
}

int ExtManager::fileConcat(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity, const std::string &content)
{
    std::string contentCopy = content;
    SuperBoot& sb = sbEntity->value;

    if(content.length() < 1){
        return 0;
    }

    //TODO: REvisar que esta formula este bien
    int neededBlocks = ExtUtility::neededBlocks(content.length() + fileEntity->value.size) - ExtUtility::neededBlocks(fileEntity->value.size);
    if(neededBlocks == -1 || neededBlocks > sbEntity->value.freeBlocksCount) {
        return -1;
    }

    int i = 0;
    int beg = 0;//Desde donde en el string cont se escribira en el archivo
    int fileblockBeg = fileEntity->value.size % FileBlock::CONTENT_SIZE;

    fileEntity->value.updateModificationTime();
    fileEntity->value.size += content.length();

    //Encontramos el primer bloque antes del primer -1 o el ultimo bloque de este inode
    //Empezamos en i = 1 en caso que blocks[0] sea el primer bloque que puede no estar lleno o que esta en -1
    for(i = 1; i < Inode::BLKS_SIZE; i++){
        if(fileEntity->value.blocks[i] == -1){
            break;
        }
    }//Despues de este loop i-1 es el primer bloque que puede no estar lleno
    i--;

    for (; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(fileEntity->value.blocks[i] == -1){
            int address = sb.reserveBlock(file);
            fileEntity->value.blocks[i] = address;
            DiskEntity<FolderContent>(address, FolderContent());
        }
        int result = fileConcatImp(file, sbEntity, beg, contentCopy, 0, fileEntity->value.blocks[i], fileblockBeg);
        if(result == 0){
            fileEntity->updateDiskValue(file);
            return result;
        }
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(fileEntity->value.blocks[i] == -1){
            int address = sb.reserveBlock(file);
            fileEntity->value.blocks[i] = address;
            DiskEntity<PointerBlock>(address, PointerBlock());
        }
        int result = fileConcatImp(file, sbEntity, beg, contentCopy, 1, fileEntity->value.blocks[i], fileblockBeg);
        if(result == 0){
            fileEntity->updateDiskValue(file);
            return result;
        }
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(fileEntity->value.blocks[i] == -1){
            int address = sb.reserveBlock(file);
            fileEntity->value.blocks[i] = address;
            DiskEntity<PointerBlock>(address, PointerBlock());
        }
        int result = fileConcatImp(file, sbEntity, beg, contentCopy, 2, fileEntity->value.blocks[i], fileblockBeg);
        if(result == 0){
            fileEntity->updateDiskValue(file);
            return result;
        }
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(fileEntity->value.blocks[i] == -1){
            int address = sb.reserveBlock(file);
            fileEntity->value.blocks[i] = address;
            DiskEntity<PointerBlock>(address, PointerBlock());
        }
        int result = fileConcatImp(file, sbEntity, beg, contentCopy, 3, fileEntity->value.blocks[i], fileblockBeg);
        if(result == 0){
            fileEntity->updateDiskValue(file);
            return result;
        }
    }//Nos podriamos ahorar un monton de lineas si lo metemos todo de un for que vaya de 0 a Indoe ::blks_size. Pero siento que asi se lee mejor. No se

    fileEntity->updateDiskValue(file);

    return -1;//Este result es diferente a los -1 anteriores porque este significa que el archivo es demasiado grande para el inodo
}

//retorna el length restante, o -1 si ocurrio algun error
//fileBlockBeg es un pequenno chapuz para que empiece a escribir en el primer caracter disponible del primer fileblock no lleno
//fileblockBeg affects performance por que va a ser 0 para toda la recursion menos para la primera vez que escribamos sobre un fileblock
int ExtManager::fileConcatImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int &beg, std::string &content, char depth, int address, int& fileblockBeg)
{
    //AQUI AQUI, PONER UN IF(-1) PARA QUE EL DEBUGGER SE DETENGA DONDE HAY PROBLEMA
    SuperBoot& sb = sbEntity->value;
    if(depth == 0){
        DiskEntity<FileBlock> fileBlock(address, file);
        auto& blockContent = fileBlock.value.content;
        if(content.length() <  FileBlock::CONTENT_SIZE - fileblockBeg){
            beg += content.copy(&blockContent[fileblockBeg], content.length() - beg, (uint)beg);
        }
        else{
            beg += content.copy(&blockContent[fileblockBeg], FileBlock::CONTENT_SIZE - fileblockBeg, (uint)beg);
        }
        fileBlock.updateDiskValue(file);
        fileblockBeg = 0;
        return (int) content.length() - beg;
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock(address, file);
        auto& pointers = pointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(pointers[i] == -1){
                int address = sb.reserveBlock(file);
                pointers[i] = address;
                if(depth - 1 > 0){
                    DiskEntity<PointerBlock>(address, PointerBlock());
                }
                else{
                    DiskEntity<FileBlock>(address, FileBlock());
                }
            }
            int blockAddress = sb.reserveBlock(file);
            pointers[i] = blockAddress;
            int result = fileConcatImp(file, sbEntity, beg, content, depth - 1, pointers[i], fileblockBeg);
            pointerBlock.updateDiskValue(file);
            if(result == 0){
                return result;
            }
        }
    }
    return (int) content.length() - beg;
}
//Suponen que el fileEntity esta totalmente vacio
//TODO PROBARLO CON ALGO GRANDE, QUE REQUIERA APUNTADORES TRIPLES
int ExtManager::fileWrite(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity, const std::string& content)
{
    std::string contentCopy = content;
    SuperBoot& sb = sbEntity->value;

    if(content.length() < 1){
        return 0;
    }

    int neededBlocks = ExtUtility::neededBlocks(content.length());
    if(neededBlocks == -1 || neededBlocks > sbEntity->value.freeBlocksCount) {
        return -1;
    }

    fileEntity->value.updateModificationTime();
    fileEntity->value.size = content.length();

    int i = 0;
    int beg = 0;//Desde donde en el string cont se escribira en el archivo
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        int address = sb.reserveBlock(file);
        fileEntity->value.blocks[i] = address;
        int result = fileWriteImp(file, sbEntity, beg, contentCopy, 0, fileEntity->value.blocks[i]);
        if(result == 0){
            fileEntity->updateDiskValue(file);
            return result;
        }
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        int address = sb.reserveBlock(file);
        fileEntity->value.blocks[i] = address;
        int result = fileWriteImp(file, sbEntity, beg, contentCopy, 1, fileEntity->value.blocks[i]);
        if(result == 0){
            fileEntity->updateDiskValue(file);
            return result;
        }
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        int address = sb.reserveBlock(file);
        fileEntity->value.blocks[i] = address;
        int result = fileWriteImp(file, sbEntity, beg, contentCopy, 2, fileEntity->value.blocks[i]);
        if(result == 0){
            fileEntity->updateDiskValue(file);
            return result;
        }
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        int address = sb.reserveBlock(file);
        fileEntity->value.blocks[i] = address;
        int result = fileWriteImp(file, sbEntity, beg, contentCopy, 3, fileEntity->value.blocks[i]);
        if(result == 0){
            fileEntity->updateDiskValue(file);
            return result;
        }
    }//Nos podriamos ahorar un monton de lineas si lo metemos todo de un for que vaya de 0 a Indoe ::blks_size. Pero siento que asi se lee mejor. No se

    fileEntity->updateDiskValue(file);

    return -1;//Este result es diferente a los -1 anteriores porque este significa que el archivo es demasiado grande para el inodo
}

//retorna el length restante, o -1 si ocurrio algun error
int ExtManager::fileWriteImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int &beg, std::string &content, char depth, int address)
{
    //AQUI AQUI, PONER UN IF(-1) PARA QUE EL DEBUGGER SE DETENGA DONDE HAY PROBLEMA
    SuperBoot& sb = sbEntity->value;
    if(depth == 0){
        DiskEntity<FileBlock> fileBlock(address, FileBlock());
        auto& blockContent = fileBlock.value.content;
        if(content.length() < FileBlock::CONTENT_SIZE){
            beg += content.copy(blockContent, content.length() - beg, (uint)beg);
        }
        else{
            beg += content.copy(blockContent, FileBlock::CONTENT_SIZE, (uint)beg);
        }
        fileBlock.updateDiskValue(file);
        return (int) content.length() - beg;
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock(address, PointerBlock());
        auto& pointers = pointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            int blockAddress = sb.reserveBlock(file);
            pointers[i] = blockAddress;
            int result = fileWriteImp(file, sbEntity, beg, content, depth - 1, pointers[i]);
            pointerBlock.updateDiskValue(file);
            if(result == 0){
                return result;
            }
        }
    }
    return (int) content.length() - beg;
}

//retorna 0 si no ocurrio error, -1 si ocurrio error
int ExtManager::fileClear(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity)
{
    SuperBoot& sb = sbEntity->value;

    fileEntity->value.updateModificationTime();
    fileEntity->value.size = 0;

    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(fileEntity->value.blocks[i] == -1){
            fileEntity->updateDiskValue(file);
            return 0;
        }
        sb.deleteBlock(file, fileEntity->value.blocks[i]);
        int result = fileClearImp(file, sbEntity, 0, fileEntity->value.blocks[i]);
        if(result == 0){
            fileEntity->value.blocks[i] = -1;
            fileEntity->updateDiskValue(file);
            return result;
        }
        fileEntity->value.blocks[i] = -1;
        fileEntity->updateDiskValue(file);
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(fileEntity->value.blocks[i] == -1){
            fileEntity->updateDiskValue(file);
            return 0;
        }
        sb.deleteBlock(file, fileEntity->value.blocks[i]);
        int result = fileClearImp(file, sbEntity, 1, fileEntity->value.blocks[i]);
        if(result == 0){
            fileEntity->value.blocks[i] = -1;
            fileEntity->updateDiskValue(file);
            return result;
        }
        fileEntity->value.blocks[i] = -1;
        fileEntity->updateDiskValue(file);
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(fileEntity->value.blocks[i] == -1){
            fileEntity->updateDiskValue(file);
            return 0;
        }
        sb.deleteBlock(file, fileEntity->value.blocks[i]);
        int result = fileClearImp(file, sbEntity, 2, fileEntity->value.blocks[i]);
        if(result == 0){
            fileEntity->value.blocks[i] = -1;
            fileEntity->updateDiskValue(file);
            return result;
        }
        fileEntity->value.blocks[i] = -1;
        fileEntity->updateDiskValue(file);
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(fileEntity->value.blocks[i] == -1){
            fileEntity->updateDiskValue(file);
            return 0;
        }
        sb.deleteBlock(file, fileEntity->value.blocks[i]);
        int result = fileClearImp(file, sbEntity, 3, fileEntity->value.blocks[i]);
        if(result == 0){
            fileEntity->value.blocks[i] = -1;
            fileEntity->updateDiskValue(file);
            return result;
        }
        fileEntity->value.blocks[i] = -1;
        fileEntity->updateDiskValue(file);
    }//Nos podriamos ahorar un monton de lineas si lo metemos todo de un for que vaya de 0 a Indoe ::blks_size. Pero siento que asi se lee mejor. No se

    fileEntity->updateDiskValue(file);

    return 0;
}

int ExtManager::fileClearImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, char depth, int address)
{
    SuperBoot& sb = sbEntity->value;
    if(depth == 0){
        return 1;
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock(address, file);
        auto& pointers = pointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(pointers[i] == -1){
                pointerBlock.updateDiskValue(file);
                return 0;
            }
            sb.deleteBlock(file, pointers[i]);
            int result = fileClearImp(file, sbEntity, depth - 1, pointers[i]);
            if(result == 0){
                pointers[i] = -1;
                pointerBlock.updateDiskValue(file);
                return result;
            }
            pointers[i] = -1;
            pointerBlock.updateDiskValue(file);
        }
    }
    return 1;
}

int ExtManager::fileEdit(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity, const std::string &content)
{
    int result = fileClear(file, sbEntity, fileEntity);
    if(result == -1){
        return result;
    }
    return fileWrite(file, sbEntity, fileEntity, content);
}

DiskEntity<Inode> ExtManager::fileCopy(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity, int usrId, int grpId, int permissions)
{
    SuperBoot& sb = sbEntity->value;
    auto& sourceInode = fileEntity->value;

    fileEntity->value.updateAccessTime();
    fileEntity->updateDiskValue(file);

    //Creamos el inodo copia
    //Chapuz: constante quemada y restringimos a fileCopy a no poder crear una copia con un permiso no default
    int destInodeAddress = sb.reserveInode(file);
    DiskEntity<Inode> destInodeEntity(destInodeAddress, Inode::createFileInode(usrId, grpId, sourceInode.size, 0x664));
    auto& destInode = destInodeEntity.value;

    //Nota: como es inode file podemos asegura que al llegar a un apuntador -1 se termino de copiar el contenido del archivo
    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(sourceInode.blocks[i] == -1){
            destInodeEntity.updateDiskValue(file);
            return destInodeEntity;
        }
        destInode.blocks[i] = fileCopyImp(file, sbEntity, usrId, grpId, permissions, 0, sourceInode.blocks[i]);
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(sourceInode.blocks[i] == -1){
            destInodeEntity.updateDiskValue(file);
            return destInodeEntity;
        }
        destInode.blocks[i] = fileCopyImp(file, sbEntity, usrId, grpId, permissions, 1, sourceInode.blocks[i]);
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(sourceInode.blocks[i] == -1){
            destInodeEntity.updateDiskValue(file);
            return destInodeEntity;
        }
        destInode.blocks[i] = fileCopyImp(file, sbEntity, usrId, grpId, permissions, 2, sourceInode.blocks[i]);
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(sourceInode.blocks[i] == -1){
            destInodeEntity.updateDiskValue(file);
            return destInodeEntity;
        }
        destInode.blocks[i] = fileCopyImp(file, sbEntity, usrId, grpId, permissions, 3, sourceInode.blocks[i]);
    }//Nos podriamos ahorar un monton de lineas si lo metemos todo de un for que vaya de 0 a Indoe ::blks_size. Pero siento que asi se lee mejor. No se

    destInodeEntity.updateDiskValue(file);
    return destInodeEntity;//Este result es diferente a los -1 anteriores porque este significa que el archivo es demasiado grande para el inodo
}

int ExtManager::fileCopyImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int usrId, int grpId, int permissions, char depth, int address)
{
    if(depth == 0){
        DiskEntity<FileBlock> sourceFileBlock(address, file);
        int destAddress = sbEntity->value.reserveBlock(file);
        DiskEntity<FileBlock> destFolderBlock(destAddress, FileBlock());
        std::memcpy(destFolderBlock.value.content, sourceFileBlock.value.content, FileBlock::CONTENT_SIZE);
        destFolderBlock.updateDiskValue(file);
        return destAddress;
    }
    else{
        DiskEntity<PointerBlock> sourcePointerBlock(address, file);
        auto& sourcePointers = sourcePointerBlock.value.pointers;
        int destAddress = sbEntity->value.reserveBlock(file);
        DiskEntity<PointerBlock> destPointerBlock(destAddress, PointerBlock());
        auto& destPointers = destPointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            destPointers[i] = fileCopyImp(file, sbEntity, usrId, grpId, permissions, depth - 1, sourcePointers[i]);
        }
        destPointerBlock.updateDiskValue(file);
        return destAddress;
    }
}

bool ExtManager::folderCanUsrRemove(RaidOneFile *file, const std::string& inodeName, DiskEntity<Inode> *inodeEntity, int usrId, int grpId, const std::string &usrName)
{
    auto& inode = inodeEntity->value;

    if(!inode.canUsrWrite(usrId, grpId)){
        Consola::reportarError("El usuario " + usrName + " no puede eliminar el archivo: " + inodeName);
        return false;
    }

    if(inode.isFolder()){
        auto subInodes = folderGetSubNamedInodes(file, inodeEntity);
        for (int i=subInodes.size() - 1; i >= 0; i--) {
            if(!folderCanUsrRemove(file, subInodes[i].first, &subInodes[i].second, usrId, grpId, usrName))
                return false;
        }
    }

    return true;
}

bool ExtManager::folderCanUsrCopy(RaidOneFile *file, const std::string &inodeName, DiskEntity<Inode> *inodeEntity, int usrId, int grpId, const std::string &usrName)
{
    auto& inode = inodeEntity->value;

    if(!inode.canUsrRead(usrId, grpId)){
        Consola::reportarError("El usuario " + usrName + " no puede copiar el archivo: " + inodeName);
        return false;
    }

    if(inode.isFolder()){
        auto subInodes = folderGetSubNamedInodes(file, inodeEntity);
        for (int i=subInodes.size() - 1; i >= 0; i--) {
            if(!folderCanUsrCopy(file, subInodes[i].first, &subInodes[i].second, usrId, grpId, usrName))
                return false;
        }
    }

    return true;
}

int ExtManager::folderClear(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity)
{
    SuperBoot& sb = sbEntity->value;

    folderEntity->value.updateModificationTime();
    folderEntity->value.size = 0;

    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(folderEntity->value.blocks[i] == -1){
            folderEntity->updateDiskValue(file);
            return 0;
        }
        sb.deleteBlock(file, folderEntity->value.blocks[i]);
        int result = folderClearImp(file, sbEntity, 0, folderEntity->value.blocks[i]);
        if(result == 0){
            folderEntity->value.blocks[i] = -1;
            folderEntity->updateDiskValue(file);
            return result;
        }
        folderEntity->value.blocks[i] = -1;
        folderEntity->updateDiskValue(file);
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] == -1){
            folderEntity->updateDiskValue(file);
            return 0;
        }
        sb.deleteBlock(file, folderEntity->value.blocks[i]);
        int result = folderClearImp(file, sbEntity, 1, folderEntity->value.blocks[i]);
        if(result == 0){
            folderEntity->value.blocks[i] = -1;
            folderEntity->updateDiskValue(file);
            return result;
        }
        folderEntity->value.blocks[i] = -1;
        folderEntity->updateDiskValue(file);
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] == -1){
            folderEntity->updateDiskValue(file);
            return 0;
        }
        sb.deleteBlock(file, folderEntity->value.blocks[i]);
        int result = folderClearImp(file, sbEntity, 2, folderEntity->value.blocks[i]);
        if(result == 0){
            folderEntity->value.blocks[i] = -1;
            folderEntity->updateDiskValue(file);
            return result;
        }
        folderEntity->value.blocks[i] = -1;
        folderEntity->updateDiskValue(file);
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] == -1){
            folderEntity->updateDiskValue(file);
            return 0;
        }
        sb.deleteBlock(file, folderEntity->value.blocks[i]);
        int result = folderClearImp(file, sbEntity, 3, folderEntity->value.blocks[i]);
        if(result == 0){
            folderEntity->value.blocks[i] = -1;
            folderEntity->updateDiskValue(file);
            return result;
        }
        folderEntity->value.blocks[i] = -1;
        folderEntity->updateDiskValue(file);
    }//Nos podriamos ahorar un monton de lineas si lo metemos todo de un for que vaya de 0 a Indoe ::blks_size. Pero siento que asi se lee mejor. No se

    folderEntity->updateDiskValue(file);

    return 0;
}

int ExtManager::folderClearImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, char depth, int address)
{
    SuperBoot& sb = sbEntity->value;
    if(depth == 0){
        DiskEntity<FolderBlock> folderBlock(address, file);
        auto& contents = folderBlock.value.contents;
        for(size_t i = 0; i < FolderBlock::CONTENTS_SIZE; i++){
            if(contents[i].isNull()){
                continue;
            }
            DiskEntity<Inode> subInodeEntity(contents[i].inodePtr, file);
            sb.deleteInode(file, contents[i].inodePtr);
//            std::memset(contents[i].name, '\0', FolderContent::NAME_SIZE);
//            contents[i].inodePtr = -1;
            if(subInodeEntity.value.isFolder()){
                if(folderClear(file, sbEntity, &subInodeEntity) == -1){
                    return -1;
                }
            }
            else{
                if(fileClear(file, sbEntity, &subInodeEntity) == -1){
                    return -1;
                }
            }
        }
        return 1;
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock(address, file);
        auto& pointers = pointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(pointers[i] == -1){
                pointerBlock.updateDiskValue(file);
                return 0;
            }
            sb.deleteBlock(file, pointers[i]);
            int result = folderClearImp(file, sbEntity, depth - 1, pointers[i]);
            if(result == 0){
                pointers[i] = -1;
                pointerBlock.updateDiskValue(file);
                return result;
            }
            pointers[i] = -1;
            pointerBlock.updateDiskValue(file);
        }
    }
    return 1;
}

int ExtManager::folderAddSubInode(RaidOneFile *file, DiskEntity<SuperBoot>* sbEntity, DiskEntity<Inode>* parentEntity, const std::string &name, int subInodeAddress)
{
    int i = 0;
    auto& sb = sbEntity->value;
    auto& blocks = parentEntity->value.blocks;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        int result;
        if(blocks[i] != -1){
            result = folderAddSubInodeImp(file, sbEntity, name, subInodeAddress, 0, false, parentEntity->value.blocks[i]);
        }
        else{
            int blockAddress = sb.reserveBlock(file);
            blocks[i] = blockAddress;
            result = folderAddSubInodeImp(file, sbEntity, name, subInodeAddress, 0, true, blockAddress);
            parentEntity->updateDiskValue(file);
        }
        if(result == 0){
            //Updates size and modification time
            parentEntity->value.updateModificationTime();
            parentEntity->value.size++;
            parentEntity->updateDiskValue(file);
            return 0;
        }
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        int result;
        if(blocks[i] != -1){
            result = folderAddSubInodeImp(file, sbEntity, name, subInodeAddress, 1, false, parentEntity->value.blocks[i]);
        }
        else{
            int blockAddress = sb.reserveBlock(file);
            blocks[i] = blockAddress;
            result = folderAddSubInodeImp(file, sbEntity, name, subInodeAddress, 1, true, blockAddress);
            parentEntity->updateDiskValue(file);
        }
        if(result == 0){
            //Updates size and modification time
            parentEntity->value.updateModificationTime();
            parentEntity->value.size++;
            parentEntity->updateDiskValue(file);
            return 0;
        }
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        int result;
        if(blocks[i] != -1){
            result = folderAddSubInodeImp(file, sbEntity, name, subInodeAddress, 2, false, parentEntity->value.blocks[i]);
        }
        else{
            int blockAddress = sb.reserveBlock(file);
            blocks[i] = blockAddress;
            result = folderAddSubInodeImp(file, sbEntity, name, subInodeAddress, 2, true, blockAddress);
            parentEntity->updateDiskValue(file);
        }
        if(result == 0){
            //Updates size and modification time
            parentEntity->value.updateModificationTime();
            parentEntity->value.size++;
            parentEntity->updateDiskValue(file);
            return 0;
        }
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        int result;
        if(blocks[i] != -1){
            result = folderAddSubInodeImp(file, sbEntity, name, subInodeAddress, 3, false, parentEntity->value.blocks[i]);
        }
        else{
            int blockAddress = sb.reserveBlock(file);
            blocks[i] = blockAddress;
            result = folderAddSubInodeImp(file, sbEntity, name, subInodeAddress, 3, true, blockAddress);
            parentEntity->updateDiskValue(file);
        }
        if(result == 0){
            //Updates size and modification time
            parentEntity->value.updateModificationTime();
            parentEntity->value.size++;
            parentEntity->updateDiskValue(file);
            return 0;
        }
    }

    throw std::length_error("No se pudo completar addSubInode");
}

int ExtManager::folderAddSubInodeImp(RaidOneFile *file, DiskEntity<SuperBoot>* sbEntity, const std::string &name, int subInodeAddress, char depth, bool isNew, int blockAddress)
{
    SuperBoot& sb = sbEntity->value;
    if(depth == 0){
        DiskEntity<FolderBlock> folderBlock;
        if(isNew){
            folderBlock = DiskEntity<FolderBlock>(blockAddress, FolderBlock());
        }
        else{
            folderBlock = DiskEntity<FolderBlock>(blockAddress, file);
        }
        auto& blockContent = folderBlock.value.contents;

        for (int i = 0; i < FolderBlock::CONTENTS_SIZE; i++) {
            if (blockContent[i].isNull()) {
                std::strcpy(blockContent[i].name, name.c_str());
                blockContent[i].inodePtr = subInodeAddress;
                folderBlock.updateDiskValue(file);

                return 0;
            }
        }
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock;
        if(isNew){
            pointerBlock= DiskEntity<PointerBlock>(blockAddress, PointerBlock());
        }
        else{
            pointerBlock = DiskEntity<PointerBlock>(blockAddress, file);
        }
        auto& pointers = pointerBlock.value.pointers;

        int result;
        for(int i = 0; i < PointerBlock::POINTERS_SIZE; i++){
            if(pointers[i] != -1){
                result = folderAddSubInodeImp(file, sbEntity, name, subInodeAddress, depth - 1, false, pointers[i]);
            }
            else{
                int blockAddress = sb.reserveBlock(file);
                pointers[i] = blockAddress;
                result = folderAddSubInodeImp(file, sbEntity, name, subInodeAddress, depth - 1, true, blockAddress);
                pointerBlock.updateDiskValue(file);
            }
            if(result == 0){
                pointerBlock.updateDiskValue(file);
                return 0;
            }
        }
    }
    return 1;
}

std::tuple<DiskEntity<Inode>, std::string> ExtManager::folderGetClosestInode(RaidOneFile *file, DiskEntity<Inode> *inodeEntity, const std::string &path)
{
    std::string remainingPath = path;
    DiskEntity<Inode> closestInode= folderGetClosestInodeImp(file, inodeEntity, remainingPath);
    return {closestInode, remainingPath};
}

DiskEntity<Inode> ExtManager::folderGetClosestInodeImp(RaidOneFile *file, DiskEntity<Inode> *inodeEntity, std::string &remainingPath)
{
    if(remainingPath.length() == 0 || inodeEntity->value.isFile()){
        return *inodeEntity;
    }
    std::string absolutePath = remainingPath;
    std::string subInodeName;
    MyStringUtil::splitSubFolderNameAndRemainingPath(absolutePath, subInodeName, remainingPath);
    auto subInode = folderGetSubInode(file, inodeEntity, subInodeName);
    if(!subInode){
        remainingPath = absolutePath;
        return *inodeEntity;
    }
    auto& debRef = subInode.value();//todo: quitar
    if(subInode->value.isFile()){
        return subInode.value();
    }

    return  folderGetClosestInodeImp(file, &subInode.value(), remainingPath);
}

std::optional<DiskEntity<Inode>> ExtManager::folderGetSubInode(RaidOneFile *file, DiskEntity<Inode> *folderEntity, const std::string &name)
{
    folderEntity->value.updateAccessTime();

    std::optional<DiskEntity<Inode>> subInode;
    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(folderEntity->value.blocks[i] == -1){
            return {};
        }
        subInode = folderGetSubInodeImp(file, name, 0, folderEntity->value.blocks[i]);
        if(subInode){
            return subInode;
        }
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] == -1){
            return {};
        }
        subInode = folderGetSubInodeImp(file, name, 1, folderEntity->value.blocks[i]);
        if(subInode){
            return subInode;
        }
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] == -1){
            return {};
        }
        subInode = folderGetSubInodeImp(file, name, 2, folderEntity->value.blocks[i]);
        if(subInode){
            return subInode;
        }
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] == -1){
            return {};
        }
        subInode = folderGetSubInodeImp(file, name, 3, folderEntity->value.blocks[i]);
        if(subInode){
            return subInode;
        }
    }

    return {}; //Despues de recorrer todos los punteros, si todavia tiene null significa que esta lleno y ninguno de sus subInodos
}

std::vector<std::pair<std::string, DiskEntity<Inode>>> ExtManager::folderGetSubNamedInodes(RaidOneFile *file, DiskEntity<Inode> *folderEntity)
{
    folderEntity->value.updateAccessTime();

    //Tipo demasiado largo y complejo. Hay que ver como podemos evitar estos tipos
    std::vector<std::pair<std::string, DiskEntity<Inode>>> subInodes;
    subInodes.reserve(folderEntity->value.size);
    int subInodesCount = folderEntity->value.size;
    int i = 0;
    //Tiramos exception si llegamos a un apuntador a bloque -1 y no hemos terminado de
    //conseguir folderContents segun subInodesCount.
    //Si ocurre dicho error lo mas seguro es no se mantuvo inode.size correctamente despues
    //de ejecutar algun metodo folder
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(subInodesCount <= 0){
            return subInodes;
        }
        if(folderEntity->value.blocks[i] == -1){
            throw std::length_error("Se termino de recorrer el inodo sin conseguir todos sus subInodos");
        }
        folderGetSubNamedInodesImp(file, subInodes, subInodesCount, 0, folderEntity->value.blocks[i]);
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(subInodesCount <= 0){
            return subInodes;
        }
        if(folderEntity->value.blocks[i] == -1){
            throw std::length_error("Se termino de recorrer el inodo sin conseguir todos sus subInodos");
        }
        folderGetSubNamedInodesImp(file, subInodes, subInodesCount, 1, folderEntity->value.blocks[i]);
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(subInodesCount <= 0){
            return subInodes;
        }
        if(folderEntity->value.blocks[i] == -1){
            throw std::length_error("Se termino de recorrer el inodo sin conseguir todos sus subInodos");
        }
        folderGetSubNamedInodesImp(file, subInodes, subInodesCount, 2, folderEntity->value.blocks[i]);
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(subInodesCount <= 0){
            return subInodes;
        }
        if(folderEntity->value.blocks[i] == -1){
            throw std::length_error("Se termino de recorrer el inodo sin conseguir todos sus subInodos");
        }
        folderGetSubNamedInodesImp(file, subInodes, subInodesCount, 3, folderEntity->value.blocks[i]);
    }

    throw std::length_error("Se termino de recorrer el inodo sin conseguir todos sus subInodos");
}

std::vector<DiskEntity<Inode> > ExtManager::folderGetSubInodes(RaidOneFile *file, DiskEntity<Inode> *folderEntity)
{
    folderEntity->value.updateAccessTime();

    //Tipo demasiado largo y complejo. Hay que ver como podemos evitar estos tipos
    std::vector<DiskEntity<Inode>> subInodes;
    subInodes.reserve(folderEntity->value.size);
    int subInodesCount = folderEntity->value.size;
    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(subInodesCount <= 0){
            return subInodes;
        }
        if(folderEntity->value.blocks[i] == -1){
            continue;
        }
        folderGetSubInodesImp(file, subInodes, subInodesCount, 0, folderEntity->value.blocks[i]);
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(subInodesCount <= 0){
            return subInodes;
        }
        if(folderEntity->value.blocks[i] == -1){
            continue;
        }
        folderGetSubInodesImp(file, subInodes, subInodesCount, 1, folderEntity->value.blocks[i]);
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(subInodesCount <= 0){
            return subInodes;
        }
        if(folderEntity->value.blocks[i] == -1){
            continue;
        }
        folderGetSubInodesImp(file, subInodes, subInodesCount, 2, folderEntity->value.blocks[i]);
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(subInodesCount <= 0){
            return subInodes;
        }
        if(folderEntity->value.blocks[i] == -1){
            continue;
        }
        folderGetSubInodesImp(file, subInodes, subInodesCount, 3, folderEntity->value.blocks[i]);
    }

    //en otras palabras, size esta correcto o alguno de los links entre bloques esta super corroto
    //Todo esta disennado para que no de este error nunca!
    //Es posible que no actualizaramos el inode.size cuando debimos
    throw std::length_error("Se termino de recorrer el inodo sin conseguir todos sus subInodos");
}

std::optional<DiskEntity<Inode>> ExtManager::folderGetSubInodeImp(RaidOneFile *file, const std::string& name, char depth, int address)
{
    if(depth == 0){
        DiskEntity<FolderBlock> folderBlock(address, file);
        auto& folderContents = folderBlock.value.contents;
        for (int i = 0; i < FolderBlock::CONTENTS_SIZE; i++) {
            if(folderContents[i].isNull()){
                continue;
            }
            if(folderContents[i].name == name){
                return DiskEntity<Inode>(folderContents[i].inodePtr, file);
            }
        }
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock(address, file);
        auto& pointers = pointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(pointers[i] == -1){
                return {};
            }
            auto subInode = folderGetSubInodeImp(file, name, depth - 1, pointers[i]);
            if(subInode){
                return subInode;
            }
        }
    }
    return {};
}

void ExtManager::folderGetSubNamedInodesImp(RaidOneFile *file, std::vector<std::pair<std::string, DiskEntity<Inode>>> &container, int &subInodeCount, char depth, int address)
{
    if(depth == 0){
        DiskEntity<FolderBlock> folderBlock(address, file);
        auto& folderContents = folderBlock.value.contents;
        for (int i = 0; i < FolderBlock::CONTENTS_SIZE; i++) {
            if(subInodeCount <= 0){
                return;
            }
            if(folderContents[i].isNull()){
                continue;
            }
            //bad performance. Deveriamos buscar como hacer emplace_back con un std::pair
            std::string subInodeName(folderContents[i].name);
            DiskEntity<Inode> subInode(folderContents[i].inodePtr, file);
            //WHY IS THE std::pair constructor taking non consts references?!
            container.push_back(std::pair<std::string, DiskEntity<Inode>>(subInodeName, subInode));
            subInodeCount--;
        }
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock(address, file);
        auto& pointers = pointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(subInodeCount <= 0){
                return;
            }
            if(pointers[i] == -1){
                continue;
            }
            folderGetSubNamedInodesImp(file, container, subInodeCount, depth - 1, pointers[i]);
        }
    }
}

void ExtManager::folderGetSubInodesImp(RaidOneFile *file, std::vector<DiskEntity<Inode> > &container, int &subInodeCount, char depth, int address)
{
    if(depth == 0){
        DiskEntity<FolderBlock> folderBlock(address, file);
        auto& folderContents = folderBlock.value.contents;
        for (int i = 0; i < FolderBlock::CONTENTS_SIZE; i++) {
            if(subInodeCount <= 0){
                return;
            }
            if(folderContents[i].isNull()){
                continue;
            }
            container.emplace_back(folderContents[i].inodePtr, file);
            subInodeCount--;
        }
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock(address, file);
        auto& pointers = pointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(subInodeCount <= 0){
                return;
            }
            if(pointers[i] == -1){
                continue;
            }
            folderGetSubInodesImp(file, container, subInodeCount, depth - 1, pointers[i]);
        }
    }
}

int ExtManager::folderRemoveSubInode(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, const std::string &name)
{
    folderEntity->value.updateAccessTime();

    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(folderEntity->value.blocks[i] == -1){
            continue;
        }
        int result = folderRemoveSubInodeImp(file, sbEntity, name, 0, folderEntity->value.blocks[i]);
        if(result != 1){
            if(result == 0){
                folderEntity->value.size--;
                folderEntity->updateDiskValue(file);
            }
            return result;
        }
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] == -1){
            return -1;
        }
        int result = folderRemoveSubInodeImp(file, sbEntity, name, 1, folderEntity->value.blocks[i]);
        if(result != 1){
            if(result == 0){
                folderEntity->value.size--;
                folderEntity->updateDiskValue(file);
            }
            return result;
        }
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] == -1){
            return -1;
        }
        int result = folderRemoveSubInodeImp(file, sbEntity, name, 2, folderEntity->value.blocks[i]);
        if(result != 1){
            if(result == 0){
                folderEntity->value.size--;
                folderEntity->updateDiskValue(file);
            }
            return result;
        }
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] == -1){
            return -1;
        }
        int result = folderRemoveSubInodeImp(file, sbEntity, name, 3, folderEntity->value.blocks[i]);
        if(result != 1){
            if(result == 0){
                folderEntity->value.size--;
                folderEntity->updateDiskValue(file);
            }
            return result;
        }
    }

    return -1; //Despues de recorrer todos los punteros, si todavia tiene null significa que esta lleno y ninguno de sus subInodos
}

int ExtManager::folderRemoveSubInodeImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, const std::string &name, char depth, int address)
{
    if(depth == 0){
        DiskEntity<FolderBlock> folderBlock(address, file);
        auto& folderContents = folderBlock.value.contents;
        for (int i = 0; i < FolderBlock::CONTENTS_SIZE; i++) {
            if(folderContents[i].isNull()){
                continue;
            }
            if(folderContents[i].name == name){
                //Quitamos el puntero al inodo por eliminar
                DiskEntity<Inode> subInodeEntity(folderContents[i].inodePtr, file);
                sbEntity->value.deleteInode(file, folderContents[i].inodePtr);
                std::strcpy(folderContents[i].name, "null");
                folderContents[i].inodePtr = -1;
                folderBlock.updateDiskValue(file);

                if(subInodeEntity.value.isFolder()){
                    if(folderClear(file, sbEntity, &subInodeEntity) == -1){
                        return -1;
                    }
                }
                else{
                    if(fileClear(file, sbEntity, &subInodeEntity) == -1){
                        return -1;
                    }
                }
            }
        }
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock(address, file);
        auto& pointers = pointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(pointers[i] == -1){
                return -1;
            }
            int result = folderRemoveSubInodeImp(file, sbEntity, name, depth - 1, pointers[i]);
            if(result != 1){
                return result;
            }
        }
    }
    return {};
}

DiskEntity<Inode> ExtManager::folderCopy(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions)
{
    SuperBoot& sb = sbEntity->value;
    auto& sourceInode = folderEntity->value;

    folderEntity->value.updateAccessTime();
    folderEntity->updateDiskValue(file);

    //Creamos el inodo copia
    //Chapuz: constante quemada y restringimos a fileCopy a no poder crear una copia con un permiso no default
    int destInodeAddress = sb.reserveInode(file);
    DiskEntity<Inode> destInodeEntity(destInodeAddress, Inode::createFolderInode(usrId, grpId, sourceInode.size, 0x664));
    auto& destInode = destInodeEntity.value;

    //Nota: como es inode file podemos asegura que al llegar a un apuntador -1 se termino de copiar el contenido del archivo
    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(sourceInode.blocks[i] == -1){
            destInodeEntity.updateDiskValue(file);
            return destInodeEntity;
        }
        destInode.blocks[i] = folderCopyImp(file, sbEntity, usrId, grpId, permissions, 0, sourceInode.blocks[i]);
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(sourceInode.blocks[i] == -1){
            destInodeEntity.updateDiskValue(file);
            return destInodeEntity;
        }
        destInode.blocks[i] = folderCopyImp(file, sbEntity, usrId, grpId, permissions, 1, sourceInode.blocks[i]);
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(sourceInode.blocks[i] == -1){
            destInodeEntity.updateDiskValue(file);
            return destInodeEntity;
        }
        destInode.blocks[i] = folderCopyImp(file, sbEntity, usrId, grpId, permissions, 2, sourceInode.blocks[i]);
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(sourceInode.blocks[i] == -1){
            destInodeEntity.updateDiskValue(file);
            return destInodeEntity;
        }
        destInode.blocks[i] = folderCopyImp(file, sbEntity, usrId, grpId, permissions, 3, sourceInode.blocks[i]);
    }//Nos podriamos ahorar un monton de lineas si lo metemos todo de un for que vaya de 0 a Indoe ::blks_size. Pero siento que asi se lee mejor. No se

    destInodeEntity.updateDiskValue(file);
    return destInodeEntity;//Este result es diferente a los -1 anteriores porque este significa que el archivo es demasiado grande para el inodo
}

int ExtManager::folderCopyImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int usrId, int grpId, int permissions, char depth, int address)
{
    if(depth == 0){
        DiskEntity<FolderBlock> sourceFolderBlock(address, file);
        int destAddress = sbEntity->value.reserveBlock(file);
        DiskEntity<FolderBlock> destFolderBlock(destAddress, FolderBlock());
        auto& sourceContents = sourceFolderBlock.value.contents;
        auto& destContens = destFolderBlock.value.contents;
        for(size_t i = 0; i < FolderBlock::CONTENTS_SIZE; i++){
            if(sourceContents[i].isNull()){
                continue;
            }
            std::strcpy(destContens[i].name, sourceContents[i].name);
            DiskEntity<Inode> sourceInode(sourceContents[i].inodePtr, file);
            if(sourceInode.value.isFile()){
                DiskEntity<Inode> destInode = fileCopy(file, sbEntity, &sourceInode, usrId, grpId, permissions);
                destContens[i].inodePtr = destInode.address;
            }
            else{
                DiskEntity<Inode> destInode = folderCopy(file, sbEntity, &sourceInode, usrId, grpId, permissions);
                destContens[i].inodePtr = destInode.address;
            }
        }
        destFolderBlock.updateDiskValue(file);
        return destAddress;
    }
    else{
        DiskEntity<PointerBlock> sourcePointerBlock(address, file);
        auto& sourcePointers = sourcePointerBlock.value.pointers;
        int destAddress = sbEntity->value.reserveBlock(file);
        DiskEntity<PointerBlock> destPointerBlock(destAddress, PointerBlock());
        auto& destPointers = destPointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            destPointers[i] = folderCopyImp(file, sbEntity, usrId, grpId, permissions, depth - 1, sourcePointers[i]);
        }
        destPointerBlock.updateDiskValue(file);
        return destAddress;
    }
}

std::optional<DiskEntity<Inode> > ExtManager::folderPopSubInode(RaidOneFile *file, DiskEntity<Inode> *folderEntity, const std::string &name)
{
    folderEntity->value.updateAccessTime();

    std::optional<DiskEntity<Inode>> subInode;
    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(folderEntity->value.blocks[i] == -1){
            return {};
        }
        subInode = folderPopSubInodeImp(file, name, 0, folderEntity->value.blocks[i]);
        if(subInode){
            folderEntity->value.size--;
            folderEntity->updateDiskValue(file);
            return subInode;
        }
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] == -1){
            return {};
        }
        subInode = folderPopSubInodeImp(file, name, 1, folderEntity->value.blocks[i]);
        if(subInode){
            folderEntity->value.size--;
            folderEntity->updateDiskValue(file);
            return subInode;
        }
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] == -1){
            return {};
        }
        subInode = folderPopSubInodeImp(file, name, 2, folderEntity->value.blocks[i]);
        if(subInode){
            folderEntity->value.size--;
            folderEntity->updateDiskValue(file);
            return subInode;
        }
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] == -1){
            return {};
        }
        subInode = folderPopSubInodeImp(file, name, 3, folderEntity->value.blocks[i]);
        if(subInode){
            folderEntity->value.size--;
            folderEntity->updateDiskValue(file);
            return subInode;
        }
    }

    return {}; //Despues de recorrer todos los punteros, si todavia tiene null significa que esta lleno y ninguno de sus subInodos
}

std::optional<DiskEntity<Inode> > ExtManager::folderPopSubInodeImp(RaidOneFile *file, const std::string &name, char depth, int address)
{
    if(depth == 0){
        DiskEntity<FolderBlock> folderBlock(address, file);
        auto& folderContents = folderBlock.value.contents;
        for (int i = 0; i < FolderBlock::CONTENTS_SIZE; i++) {
            if(folderContents[i].isNull()){
                continue;
            }
            if(folderContents[i].name == name){
                int popedInodeAddress = folderContents[i].inodePtr;
                //vaciamos el puntero
                std::strcpy(folderContents[i].name, "null");
                folderContents[i].inodePtr = -1;
                folderBlock.updateDiskValue(file);
                return DiskEntity<Inode>(popedInodeAddress, file);
            }
        }
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock(address, file);
        auto& pointers = pointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(pointers[i] == -1){
                return {};
            }
            auto subInode = folderPopSubInodeImp(file, name, depth - 1, pointers[i]);
            if(subInode){
                return subInode;
            }
        }
    }
    return {};
}

int ExtManager::folderChangeSubInodeName(RaidOneFile *file, DiskEntity<Inode> *folderEntity, const std::string &oldName, const std::string &newName, int usrId, int grpId)
{
    int result;
    int i = 0;
    auto& blocks = folderEntity->value.blocks;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(blocks[i] == -1){
            Consola::reportarError("archivos: " + oldName + " no encontrado");
            return -1;
        }
        result = folderChangeSubInodeNameImp(file, oldName, newName, usrId, grpId, 0, blocks[i]);
        if(result != 1){
            return result;
        }
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(blocks[i] == -1){
            Consola::reportarError("archivos: " + oldName + " no encontrado");
            return -1;
        }
        result = folderChangeSubInodeNameImp(file, oldName, newName, usrId, grpId, 1, blocks[i]);
        if(result != 1){
            return result;
        }
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(blocks[i] == -1){
            Consola::reportarError("archivos: " + oldName + " no encontrado");
            return -1;
        }
        result = folderChangeSubInodeNameImp(file, oldName, newName, usrId, grpId, 2, blocks[i]);
        if(result != 1){
            return result;
        }
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(blocks[i] == -1){
            Consola::reportarError("archivos: " + oldName + " no encontrado");
            return -1;
        }
        result = folderChangeSubInodeNameImp(file, oldName, newName, usrId, grpId, 3, blocks[i]);
        if(result != 1){
            return result;
        }
    }

    Consola::reportarError("archivos: " + oldName + " no encontrado");
    return -1;
}

int ExtManager::folderChangeSubInodeNameImp(RaidOneFile *file, const std::string &oldName, const std::string &newName, int usrId, int grpId, char depth, int address)
{
    if(depth == 0){
        DiskEntity<FolderBlock> folderBlock(address, file);
        auto& folderContents = folderBlock.value.contents;
        for (int i = 0; i < FolderBlock::CONTENTS_SIZE; i++) {
            if(folderContents[i].isNull()){
                continue;
            }
            if(oldName == folderContents[i].name){
                DiskEntity<Inode> subInode(folderContents[i].inodePtr, file);
                if(!subInode.value.canUsrWrite(usrId, grpId)){
                    Consola::reportarError("No tiene permisos de escritura en " + oldName);
                    return -1;
                }
                std::strcpy(folderContents[i].name, newName.c_str());
                folderBlock.updateDiskValue(file);
                return 0;
            }
        }
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock(address, file);
        auto& pointers = pointerBlock.value.pointers;
        int result;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(pointers[i] == -1){
                Consola::reportarError("archivos: " + oldName + " no encontrado");
                return -1;
            }
            result = folderChangeSubInodeNameImp(file, oldName, newName, usrId, grpId, 3, pointers[i]);
            if(result != 1){
                return result;
            }
        }
    }
    return 1;
}

void ExtManager::getInodeAndBlockCount(RaidOneFile *file, DiskEntity<Inode> *inodeEntity, int &inodeCount, int &blockCount)
{
    inodeCount++;
    auto& inode = inodeEntity->value;
    if(inode.isFile()){
        blockCount += ExtUtility::neededBlocks(inode.size);
    }
    else{
        int i = 0;
        for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
            if(inode.blocks[i] == -1){
                continue;
            }
            getInodeAndBlockCountImp(file, inodeCount, blockCount, 0, inode.blocks[i]);
        }
        for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
            if(inode.blocks[i] == -1){
                return;
            }
            getInodeAndBlockCountImp(file, inodeCount, blockCount, 1, inode.blocks[i]);

        }
        for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
            if(inode.blocks[i] == -1){
                return;
            }
            getInodeAndBlockCountImp(file, inodeCount, blockCount, 2, inode.blocks[i]);
        }
        for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
            if(inode.blocks[i] == -1){
                return;
            }
            getInodeAndBlockCountImp(file, inodeCount, blockCount, 3, inode.blocks[i]);
        }//Nos podriamos ahorar un monton de lineas si lo metemos todo de un for que vaya de 0 a Indoe ::blks_size. Pero siento que asi se lee mejor. No se
    }

}

//[ !!! ] ASUME QUE EL ADDRESS ES UN BLOQUE DE UN INODE TIPO DIRECTORIO!!!
void ExtManager::getInodeAndBlockCountImp(RaidOneFile *file, int &inodeCount, int &blockCount, char depth, int address)
{
    blockCount++;
    if(depth == 0){
        DiskEntity<FolderBlock> folderBlock(address, file);
        auto& contents = folderBlock.value.contents;
        for(size_t i = 0; i < FolderBlock::CONTENTS_SIZE; i++){
            if(contents[i].isNull()){
                continue;
            }
            DiskEntity<Inode> subInodeEntity(contents[i].inodePtr, file);
            getInodeAndBlockCount(file, &subInodeEntity, inodeCount, blockCount);
        }
    }
    else{
        DiskEntity<PointerBlock> pointerBlock(address, file);
        auto& pointers = pointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(pointers[i] == -1){
                return;
            }
            getInodeAndBlockCountImp(file, inodeCount, blockCount, depth - 1, pointers[i]);
        }
    }
}

int ExtManager::createRootAndUsrsTxt(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity)
{
    //hacer el Root:
    int rootInodeStart = sbEntity->value.reserveInode(file);
    DiskEntity<Inode> rootInodeEntity(rootInodeStart, Inode::createFolderInode(1, 1, 0x664));
    Inode& rootInode = rootInodeEntity.value;
    //Hacer el primer bloque folder de root
    int rootFolderBlockStart = sbEntity->value.reserveBlock(file);
    DiskEntity<FolderBlock> rootFolderBlockEntity(rootFolderBlockStart, FolderBlock());
    FolderBlock& rootFolderBlock = rootFolderBlockEntity.value;
    rootInode.blocks[0] = rootFolderBlockStart;
    rootInode.size = 1;

    //hacer el users.txt
    int usersInodeStart = sbEntity->value.reserveInode(file);
    std::string content = std::string("1,G,root\n") +
                          "1,U,root,root,123\n";
    DiskEntity<Inode> usersTxtInodeEntity(usersInodeStart, Inode::createFileInode(1, 1, content.length(), 0x000));
    Inode& usersTextInode = usersTxtInodeEntity.value;

    //Que el primer folderContent del primer bloque de carpetas apunte al users.txt
    std::fill(rootFolderBlock.contents[0].name, rootFolderBlock.contents[0].name + FolderContent::NAME_SIZE, 0);
    strcpy(rootFolderBlock.contents[0].name, "users.txt");
    rootFolderBlock.contents[0].inodePtr = usersInodeStart;

    //Quemamos el bloque necesario para almacenar el archivo users.txt
    int usersTextFileBlockStart = sbEntity->value.reserveBlock(file);
    DiskEntity<FileBlock> usersTextFileBlockEntity(usersTextFileBlockStart, FileBlock(content, 0, (int) content.length()));
    FileBlock& usersTextFileBlock = usersTextFileBlockEntity.value;
    usersTextInode.blocks[0] = usersTextFileBlockStart;

    //Actualizamos en disco todas las entidades
    rootInodeEntity.updateDiskValue(file);
    rootFolderBlockEntity.updateDiskValue(file);
    usersTxtInodeEntity.updateDiskValue(file);
    usersTextFileBlockEntity.updateDiskValue(file);
    sbEntity->updateDiskValue(file);

    return 0;
}

bool ExtManager::folderMkfileRecursively(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions, const std::string &path, const std::string &fileName, const std::string &content)
{
    std::string remainingPath = path;
    return folderMkfileRecursivelyImp(file, sbEntity, folderEntity, usrId, grpId, permissions, remainingPath, fileName, content);
}

bool ExtManager::folderMkfileRecursivelyImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions, std::string &remainingPath, const std::string &fileName, const std::string &content)
{
    if(remainingPath.length() == 0){
        auto subFile = folderMkSubfile(file, sbEntity, folderEntity, usrId, grpId, permissions, fileName, content);
        return subFile.has_value();
    }
    std::string absolutePath = remainingPath;
    std::string subFolderName;
    MyStringUtil::splitSubFolderNameAndRemainingPath(absolutePath, subFolderName, remainingPath);
    auto subFolder = folderMkSubfolder(file, sbEntity, folderEntity, usrId, grpId, permissions, subFolderName);
    if(!subFolder){
        return false;
    }

    return folderMkfileRecursivelyImp(file, sbEntity, &subFolder.value(), usrId, grpId, permissions, remainingPath, fileName, content);
}

bool ExtManager::folderMkfolderRecursively(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions, const std::string &path)
{
    std::string remainingPath = path;
    return folderMkfolderRecursivelyImp(file, sbEntity, folderEntity, usrId, grpId, permissions, remainingPath);
}

bool ExtManager::folderMkfolderRecursivelyImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions, std::string &remainingPath)
{
    if(remainingPath.length() < 1){
        return true;
    }
    std::string absolutePath = remainingPath;
    std::string subFolderName;
    MyStringUtil::splitSubFolderNameAndRemainingPath(absolutePath, subFolderName, remainingPath);
    auto subFolder = folderMkSubfolder(file, sbEntity, folderEntity, usrId, grpId, permissions, subFolderName);
    if(!subFolder){
        return false;
    }

    return folderMkfolderRecursivelyImp(file, sbEntity, &subFolder.value(), usrId, grpId, permissions, remainingPath);
}

std::optional<DiskEntity<Inode>> ExtManager::folderMkSubfolder(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions, const std::string &name)
{
    SuperBoot& sb = sbEntity->value;

    std::optional<DiskEntity<Inode>> subInode;
    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(folderEntity->value.blocks[i] != -1){
            subInode = folderMkSubfolderImp(file, sbEntity, usrId, grpId, permissions, name, 0, false, folderEntity->value.blocks[i]);
        }
        else{
            int address = sb.reserveBlock(file);
            if(address == -1){
                folderEntity->updateDiskValue(file);
                return {};//ya no hay espacio en el bitmap de inodos
            }
            folderEntity->value.blocks[i] = address;
            subInode = folderMkSubfolderImp(file, sbEntity, usrId, grpId, permissions, name, 0, true, folderEntity->value.blocks[i]);
            folderEntity->updateDiskValue(file);
        }
        if(subInode){
            //Updates size and modification time
            folderEntity->value.updateModificationTime();
            folderEntity->value.size++;
            folderEntity->updateDiskValue(file);
            return subInode;
        }
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] != -1){
            subInode = folderMkSubfolderImp(file, sbEntity, usrId, grpId, permissions, name, 1, false, folderEntity->value.blocks[i]);
        }
        else{
            int address = sb.reserveBlock(file);
            if(address == -1){
                folderEntity->updateDiskValue(file);
                return {};//ya no hay espacio en el bitmap de inodos
            }
            folderEntity->value.blocks[i] = address;

            subInode = folderMkSubfolderImp(file, sbEntity, usrId, grpId, permissions, name, 1, true, folderEntity->value.blocks[i]);
            folderEntity->updateDiskValue(file);
        }
        if(subInode){
            //Updates size and modification time
            folderEntity->value.updateModificationTime();
            folderEntity->value.size++;
            folderEntity->updateDiskValue(file);
            return subInode;
        }
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] != -1){
            subInode = folderMkSubfolderImp(file, sbEntity, usrId, grpId, permissions, name, 2, false, folderEntity->value.blocks[i]);
        }
        else{
            int address = sb.reserveBlock(file);
            if(address == -1){
                folderEntity->updateDiskValue(file);
                return {};//ya no hay espacio en el bitmap de inodos
            }
            folderEntity->value.blocks[i] = address;
            subInode = folderMkSubfolderImp(file, sbEntity, usrId, grpId, permissions, name, 2, true, folderEntity->value.blocks[i]);
            folderEntity->updateDiskValue(file);
        }
        if(subInode){
            //Updates size and modification time
            folderEntity->value.updateModificationTime();
            folderEntity->value.size++;
            folderEntity->updateDiskValue(file);
            return subInode;
        }
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] != -1){
            subInode = folderMkSubfolderImp(file, sbEntity, usrId, grpId, permissions, name, 3, false, folderEntity->value.blocks[i]);
        }
        else{
            int address = sb.reserveBlock(file);
            if(address == -1){
                folderEntity->updateDiskValue(file);
                return {}; //ya no hay espacio en el bitmap de inodos
            }
            folderEntity->value.blocks[i] = address;
            subInode = folderMkSubfolderImp(file, sbEntity, usrId, grpId, permissions, name, 3, true, folderEntity->value.blocks[i]);
            folderEntity->updateDiskValue(file);
        }
        if(subInode){
            //Updates size and modification time
            folderEntity->value.updateModificationTime();
            folderEntity->value.size++;
            folderEntity->updateDiskValue(file);
            return subInode;
        }
    }

    return {};
}

std::optional<DiskEntity<Inode>> ExtManager::folderMkSubfolderImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int usrId, int grpId, int permissions, const std::string &name, char depth, bool isNew, int address)
{
    SuperBoot& sb = sbEntity->value;
    if(depth == 0){
        DiskEntity<FolderBlock> folderBlock;
        if(isNew){
            folderBlock = DiskEntity<FolderBlock>(address, FolderBlock());
        }
        else{
            folderBlock = DiskEntity<FolderBlock>(address, file);
        }
        auto& blockContent = folderBlock.value.contents;

        for (int i = 0; i < FolderBlock::CONTENTS_SIZE; i++) {
            if (blockContent[i].isNull()) {
                int inodeStart = sb.reserveInode(file);
                if(inodeStart == -1){
                    return {};
                }
                std::strcpy(blockContent[i].name, name.c_str());
                blockContent[i].inodePtr = inodeStart;
                DiskEntity<Inode> subFolderEntity(blockContent[i].inodePtr, Inode::createFolderInode(usrId, grpId, permissions));
                subFolderEntity.updateDiskValue(file);
                folderBlock.updateDiskValue(file);//Si es new siempre pasa por aqui, si no es new y no pasa por aqui no hay necesidad de actualizar el disco

                return subFolderEntity;
            }
        }
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock;
        if(isNew){
            pointerBlock= DiskEntity<PointerBlock>(address, PointerBlock());
        }
        else{
            pointerBlock = DiskEntity<PointerBlock>(address, file);
        }
        auto& pointers = pointerBlock.value.pointers;
        std::optional<DiskEntity<Inode>> subInode;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(pointers[i] != -1){
                subInode = folderMkSubfolderImp(file, sbEntity, usrId, grpId, permissions, name, depth - 1, false, pointers[i]);
            }
            else{
                int address = sb.reserveBlock(file);
                if(address == -1){
                    pointerBlock.updateDiskValue(file);
                    return {};//ya no hay espacio en el bitmap de inodos
                }
                pointers[i] = address;
                subInode = folderMkSubfolderImp(file, sbEntity, usrId, grpId, permissions, name, depth - 1, true, pointers[i]);
                pointerBlock.updateDiskValue(file);
            }
            if(subInode){
                return subInode;
            }
        }
    }
    return {};
}

std::optional<DiskEntity<Inode>> ExtManager::folderMkSubfile(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions, const std::string &name, const std::string& content)
{
    SuperBoot& sb = sbEntity->value;
    folderEntity->value.updateModificationTime();

    std::optional<DiskEntity<Inode>> subInode;
    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        if(folderEntity->value.blocks[i] != -1){
            subInode = folderMkSubfileImp(file, sbEntity, usrId, grpId, permissions, name, content, 0, false, folderEntity->value.blocks[i]);
        }
        else{
            int address = sb.reserveBlock(file);
            if(address == -1){
                folderEntity->updateDiskValue(file);
                return {};//ya no hay espacio en el bitmap de inodos
            }
            folderEntity->value.blocks[i] = address;
            subInode = folderMkSubfileImp(file, sbEntity, usrId, grpId, permissions, name, content, 0, true, folderEntity->value.blocks[i]);
            folderEntity->updateDiskValue(file);
        }
        if(subInode){
            folderEntity->value.updateModificationTime();
            folderEntity->value.size++;
            folderEntity->updateDiskValue(file);
            return subInode;
        }
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] != -1){
            subInode = folderMkSubfileImp(file, sbEntity, usrId, grpId, permissions, name, content, 1, false, folderEntity->value.blocks[i]);
        }
        else{
            int address = sb.reserveBlock(file);
            if(address == -1){
                folderEntity->updateDiskValue(file);
                return {};//ya no hay espacio en el bitmap de inodos
            }
            folderEntity->value.blocks[i] = address;

            subInode = folderMkSubfileImp(file, sbEntity, usrId, grpId, permissions, name, content, 1, true, folderEntity->value.blocks[i]);
            folderEntity->updateDiskValue(file);
        }
        if(subInode){
            folderEntity->value.updateModificationTime();
            folderEntity->value.size++;
            folderEntity->updateDiskValue(file);
            return subInode;
        }
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] != -1){
            subInode = folderMkSubfileImp(file, sbEntity, usrId, grpId, permissions, name, content, 2, false, folderEntity->value.blocks[i]);
        }
        else{
            int address = sb.reserveBlock(file);
            if(address == -1){
                folderEntity->updateDiskValue(file);
                return {};//ya no hay espacio en el bitmap de inodos
            }
            folderEntity->value.blocks[i] = address;
            subInode = folderMkSubfileImp(file, sbEntity, usrId, grpId, permissions, name, content, 2, true, folderEntity->value.blocks[i]);
            folderEntity->updateDiskValue(file);
        }
        if(subInode){
            folderEntity->value.updateModificationTime();
            folderEntity->value.size++;
            folderEntity->updateDiskValue(file);
            return subInode;
        }
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        if(folderEntity->value.blocks[i] != -1){
            subInode = folderMkSubfileImp(file, sbEntity, usrId, grpId, permissions, name, content, 3, false, folderEntity->value.blocks[i]);
        }
        else{
            int address = sb.reserveBlock(file);
            if(address == -1){
                folderEntity->updateDiskValue(file);
                return {};//ya no hay espacio en el bitmap de inodos
            }
            folderEntity->value.blocks[i] = address;
            subInode = folderMkSubfileImp(file, sbEntity, usrId, grpId, permissions, name, content, 3, true, folderEntity->value.blocks[i]);
            folderEntity->updateDiskValue(file);
        }
        if(subInode){
            folderEntity->value.updateModificationTime();
            folderEntity->value.size++;
            folderEntity->updateDiskValue(file);
            return subInode;
        }
    }

    return {};
}

std::optional<DiskEntity<Inode>> ExtManager::folderMkSubfileImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int usrId, int grpId, int permissions, const std::string &name, const std::string& content, char depth, bool isNew, int address)
{
    SuperBoot& sb = sbEntity->value;
    if(depth == 0){
        DiskEntity<FolderBlock> folderBlock;
        if(isNew){
            folderBlock = DiskEntity<FolderBlock>(address, FolderBlock());
        }
        else{
            folderBlock = DiskEntity<FolderBlock>(address, file);
        }
        auto& blockContent = folderBlock.value.contents;

        for (int i = 0; i < FolderBlock::CONTENTS_SIZE; i++) {
            if (blockContent[i].isNull()) {
                int inodeStart = sb.reserveInode(file);//Debug: con name = prueba2.txt, retorna 13696que es el inodo de carpeta users
                if(inodeStart == -1){
                    return {};
                }
                std::strcpy(blockContent[i].name, name.c_str());
                blockContent[i].inodePtr = inodeStart;
                DiskEntity<Inode> subFileEntity(blockContent[i].inodePtr, Inode::createFileInode(usrId, grpId, content.length(), permissions));
                subFileEntity.updateDiskValue(file);

                //file->flush();//Creo que para que funcione fileWrite el inodo tienen que estar en el disco fisico, hacemos un flush por si acaso
                if(fileWrite(file, sbEntity, &subFileEntity, content) == -1){
                    folderBlock.updateDiskValue(file);//Si es new siempre pasa por aqui, si no es new y no pasa por aqui no hay necesidad de actualizar el disco
                    Consola::reportarError("No se pudo escribir el archivo: " + name + " se ha quedado sin espacio el disco");
                    return subFileEntity;
                }
                folderBlock.updateDiskValue(file);//Si es new siempre pasa por aqui, si no es new y no pasa por aqui no hay necesidad de actualizar el disco
                return subFileEntity;
            }
        }
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock;
        if(isNew){
            pointerBlock= DiskEntity<PointerBlock>(address, PointerBlock());
        }
        else{
            pointerBlock = DiskEntity<PointerBlock>(address, file);
        }
        auto& pointers = pointerBlock.value.pointers;
        std::optional<DiskEntity<Inode>> subInode;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(pointers[i] != -1){
                subInode = folderMkSubfileImp(file, sbEntity, usrId, grpId, permissions, name, content, depth - 1, false, pointers[i]);
            }
            else{
                int address = sb.reserveBlock(file);
                if(address == -1){
                    pointerBlock.updateDiskValue(file);
                    return {};//ya no hay espacio en el bitmap de inodos
                }
                pointers[i] = address;
                subInode = folderMkSubfileImp(file, sbEntity, usrId, grpId, permissions, name, content, depth - 1, true, pointers[i]);
                pointerBlock.updateDiskValue(file);
            }
            if(subInode){
                return subInode;
            }
        }
    }
    return {};
}

bool ExtManager::findImp(RaidOneFile *file, const std::string& inodeName, DiskEntity<Inode> *inodeEntity, std::string &buffer, const std::string &name, size_t indentation)
{
    bool foundInThisBranch = false;
    auto& inode = inodeEntity->value;
    //chapuz. para poner <- en caso que este inode especificamente sea el archivo que estamos buscando
    //bad performance too
    std::string suffix("\n");

    if(inodeName == name){
        foundInThisBranch = true;
        suffix = " <-\n";
    }
    if(inode.isFolder()){
        auto subInodes = folderGetSubNamedInodes(file, inodeEntity);
        for (int i=subInodes.size() - 1; i >= 0; i--) {
            foundInThisBranch = findImp(file, subInodes[i].first, &subInodes[i].second, buffer, name, indentation + 2) || foundInThisBranch;
        }
    }
    if(foundInThisBranch){
        //bad performance, buncho o strings concats that I dont think can be optimized by the compiler
        buffer = std::string(indentation, ' ') + "|_" + inodeName + suffix + buffer;
    }

    return foundInThisBranch;
}



