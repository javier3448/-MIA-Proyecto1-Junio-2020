#ifndef EXTMANAGER_H
#define EXTMANAGER_H

#include <string>
#include <cstring>
#include <vector>
#include <optional>
#include "FileSystem/superboot.h"
#include "FileSystem/UserManagement/usersession.h"
#include "FileSystem/UserManagement/usersdata.h"
#include "Disk/mountedpart.h"
#include "diskentity.h"

#define ROOT_USER 0

//using SuperBootPair = std::pair<int, SuperBoot>;
//using AvdPair = std::pair<int, Avd>;
//using DdPair = std::pair<int, Dd>;
//using DdFilePair = std::pair<int, DdFile>;
//using InodePair = std::pair<int, Inode>;

class ExtManager
{
public:
    static int mkfs(const std::string& id, char formatType);
    static int login(const std::string& usr, const std::string& pwd, const std::string& id);
    static int logout();
    static int mkGrp(const std::string& name);
    static int rmGrp(const std::string& name);
    static int mkUsr(const std::string& usr, const std::string& grp , const std::string& pwd);
    static int rmUsr(const std::string& usr);
    static int chmod(const std::string& path, int ugo, bool r, int paramsUgo);//paramsUgo es un chapuz horrible para que se pueda hacer el journaling chmod facil
    static int chown(const std::string& path, bool r, const std::string& usr);//paramsUgo es un chapuz horrible para que se pueda hacer el journaling chmod facil
    static int chgrp(const std::string& usr, const std::string& grp);//paramsUgo es un chapuz horrible para que se pueda hacer el journaling chmod facil
    static int mkFile(const std::string& path, bool p, const std::string& content, int paramsSize, const std::string& paramsCont);//paramsSize y paramsCont son un chapuz horrible para que se pueda hacer el journaling mkFile facil
    static int cp(const std::string& path, const std::string& dest);
    static int mv(const std::string& path, const std::string& dest);
    static int find(const std::string& path, const std::string& name);
    static int cat(const std::string& path);
    static int rem(const std::string& path);
    static int edit(const std::string& path, const std::string& content);
    static int ren(const std::string& path, const std::string& name);
    static int mkDir(const std::string& path, bool p);

    //Verifica si la particion tiene un sb, de ser asi actualiza su lastMountTime
    static bool updateMountTime(RaidOneFile* file, const MountedPart& mountedPart);
    //Verifica si la particion tiene un sb, de ser asi actualiza su lastUnmountTime
    static bool updateUnmountTime(const MountedPart& mountedPart);

    static DiskEntity<Inode> getRoot(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity);//Chapuz medio bajo, este metodo deberia ser privado
    static DiskEntity<Inode> getUsers(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity);//Chapuz medio bajo, este metodo deberia ser privado

    //Para manejar files:
    static std::string fileGetContent(RaidOneFile* file, DiskEntity<Inode> *fileEntity);
    static void fileGetContentImp(RaidOneFile* file, std::string& content, int& size, char depth, int address);
    static std::tuple<DiskEntity<Inode>, std::string> folderGetClosestInode(RaidOneFile* file, DiskEntity<Inode> *inodeEntity, const std::string& path);
    //igual a noimp pero pasa el string por ref
    static DiskEntity<Inode> folderGetClosestInodeImp(RaidOneFile* file, DiskEntity<Inode> *inodeEntity, std::string& remainingPath);
    static std::optional<DiskEntity<Inode>> folderGetSubInode(RaidOneFile* file, DiskEntity<Inode> *folderEntity, const std::string& name);
    //Comporlamiento no definido cuando se manda un inode que no sea directorio
    //nitpick
    static std::vector<std::pair<std::string, DiskEntity<Inode>>> folderGetSubNamedInodes(RaidOneFile *file, DiskEntity<Inode> *folderEntity);
    static std::vector<DiskEntity<Inode>> folderGetSubInodes(RaidOneFile *file, DiskEntity<Inode> *folderEntity);

private:

    static std::optional<UserSession> currUserSession;

    static int createRootAndUsrsTxt(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity);

    static int recursiveChmod(RaidOneFile* file, DiskEntity<Inode>* inode, int usrId, int ugo);
    static int recursiveChown(RaidOneFile* file, DiskEntity<Inode>* inodeEntity, int usrId, int newUsrId);

    //para manejar files:
    static int fileConcat(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity, const std::string& content);//-1 si se queda sin espacio
    static int fileConcatImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int &beg, std::string &content, char depth, int address, int& fileblockBeg);
    static int fileWrite(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity, const std::string& content);//-1 si se queda sin espacio
    static int fileWriteImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int &beg, std::string &content, char depth, int address);
    static int fileClear(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity);
    static int fileClearImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, char depth, int address);
    static int fileEdit(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity, const std::string& content);//-1 si se queda sin espacio
    //Bad design: A diferencia de los otros metodos file no revisa que haya suficiente espacio
    //disponible, se espera que esta revision ocurra antes de llamar a este metodo. se tomo esa decision
    //porque este metodo se llamara recursivamente y es preferible hacer esos chequeos afuera de la recursion
    static DiskEntity<Inode> fileCopy(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity, int usrId, int grpId, int permissions);
    //retorna el puntero al nuevo bloque. El nuevo bloque es una compia del bloque en address
    static int fileCopyImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int usrId, int grpId, int permissions, char depth, int sourceAddress);

    static bool folderCanUsrRemove(RaidOneFile* file, const std::string& inodeName, DiskEntity<Inode>* inodeEntity, int usrId, int grpId, const std::string& usrName);
    static bool folderCanUsrCopy(RaidOneFile* file, const std::string& inodeName, DiskEntity<Inode>* inodeEntity, int usrId, int grpId, const std::string& usrName);
    //retorna 0 si no ocurrio error, -1 si ocurrio error
    static int folderClear(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity);
    //retorna 1 si no ocurrio error pero no encontro el target: name; 0 si lo encontro y lo elimino
    //con exito; y -1 si no le elimino, ocurrio un error y no puede continuar la busqueda
    static int folderClearImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, char depth, int address);
    //Retorna 0 si no hay error; -1 si se quedo sin espacio inodeEntity
    //No revisa que tenga espacio en el disco.
    //Si no hay espacio suficiente en folderEntity tira exception
    static int folderAddSubInode(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, DiskEntity<Inode>* folderEntity, const std::string& name, int subInodeAddress);
    //Retorna 0 si logro agregar el subInodo.
    //Retorna 1 si no se ha agregado el subinodo
    //Retorna -1 si ocurrio error
    static int folderAddSubInodeImp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& name, int subInodeAddress, char depth, bool isNew, int blockAddress);
    static bool folderMkfileRecursively(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions, const std::string &path, const std::string &fileName, const std::string &content);
    //igual a noimp pero pasa el string por ref
    static bool folderMkfileRecursivelyImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions, std::string &remainingPath, const std::string &fileName, const std::string &content);
    static bool folderMkfolderRecursively(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions, const std::string &path);
    //igual a noimp pero pasa el string por ref
    static bool folderMkfolderRecursivelyImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions, std::string &remainingPath);
    //Retorna el folder creado o empty si no pudo crearlo
    static std::optional<DiskEntity<Inode>> folderMkSubfolder(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions, const std::string &name);
    static std::optional<DiskEntity<Inode>> folderMkSubfolderImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int usrId, int grpId, int permissions, const std::string &name, char depth, bool isNew, int address);//isNew nos indica si el address que recibe es de un nuevo bloque que se acaba de reservar con reserveBlock
    static std::optional<DiskEntity<Inode>> folderMkSubfile(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions, const std::string &name, const std::string &content);
    static std::optional<DiskEntity<Inode>> folderMkSubfileImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int usrId, int grpId, int permissions, const std::string &name, const std::string &content, char depth, bool isNew, int address);
    static std::optional<DiskEntity<Inode>> folderGetSubInodeImp(RaidOneFile* file, const std::string& name, char depth, int address);
    //subInodeCount: how many subInodes have yet to be found according to folderEntity.size
    static void folderGetSubNamedInodesImp(RaidOneFile *file, std::vector<std::pair<std::string, DiskEntity<Inode>>> &container, int &subInodeCount, char depth, int address);
    static void folderGetSubInodesImp(RaidOneFile *file, std::vector<DiskEntity<Inode>> &container, int &subInodeCount, char depth, int address);
    //retorna 0 si no ocurrio error, -1 si ocurrio error
    static int folderRemoveSubInode(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, const std::string& name);
    //retorna 1 si no ocurrio error pero no encontro el target: name; 0 si lo encontro y lo elimino
    //con exito; y -1 si no le elimino, ocurrio un error y no puede continuar la busqueda
    static int folderRemoveSubInodeImp(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, const std::string& name, char depth, int address);
    //Bad design: A diferencia de los otros metodos file no revisa que haya suficiente espacio
    //disponible, se espera que esta revision ocurra antes de llamar a este metodo. se tomo esa decision
    //porque este metodo se llamara recursivamente y es preferible hacer esos chequeos afuera de la recursion
    static DiskEntity<Inode> folderCopy(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *folderEntity, int usrId, int grpId, int permissions);
    //retorna el puntero al nuevo bloque. El nuevo bloque es una compia del bloque en address
    static int folderCopyImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int usrId, int grpId, int permissions, char depth, int address);

    //Reporta error de una vez en caso de ocurrir.
    //Es un chapuz porque adentro de este metodo pueden ocurrir dos tipos de errores diferentes
    //de permisos o de no existe el subinodo
    static std::optional<DiskEntity<Inode>> folderPopSubInode(RaidOneFile* file, DiskEntity<Inode> *folderEntity, const std::string& name);
    static std::optional<DiskEntity<Inode>> folderPopSubInodeImp(RaidOneFile* file, const std::string& name, char depth, int address);

    //No verifica que no exista otro subInodo con nombre igual a newName
    static int folderChangeSubInodeName(RaidOneFile* file, DiskEntity<Inode> *folderEntity, const std::string& oldName, const std::string& newName, int usrId, int grpId);
    static int folderChangeSubInodeNameImp(RaidOneFile* file, const std::string& oldName, const std::string& newName, int usrId, int grpId, char depth, int address);
    //bad design
    //pasamos dos int como ref para llevar conteo en vez de retorna un pair<int, int>
    //es porque no se como hacer para mantener un retornar un pair sin tener que crear
    //otro a cada rato
    //TODO: testear bien. (sacarle inodeCount blockCount a / ver si es igual a lo que dice el sb
    static void getInodeAndBlockCount(RaidOneFile* file, DiskEntity<Inode>* inodeEntity, int& inodeCount, int& blockCount);
    //address es un puntero a bloque en el disco
    static void getInodeAndBlockCountImp(RaidOneFile* file, int& inodeCount, int& blockCount, char depth, int address);

    //Returns true if a file with that name was found in this "branch"
    //Acepta inodes file e inodes directory
    static bool findImp(RaidOneFile *file, const std::string& inodeName, DiskEntity<Inode> *inode, std::string& buffer, const std::string& name, size_t indentation);
    //Para manejar el journaling:

friend int main();//Solo para debugging
};

#endif // EXTMANAGER_H
