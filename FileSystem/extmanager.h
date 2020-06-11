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
    static int mkfs(const std::string& id, char fsType, char formatType);
    static int login(const std::string& usr, const std::string& pwd, const std::string& id);
    static int logout();
    static int mkGrp(const std::string& name);
    static int rmGrp(const std::string& name);
    static int mkUsr(const std::string& name, const std::string& grp , const std::string& pwd);
    static int rmUsr(const std::string& name);
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
    static int loss(const std::string& id);
    static int recovery(const std::string& id);

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
    static std::optional<DiskEntity<Inode>> folderGetSubInodeImp(RaidOneFile* file, const std::string& name, char depth, int address);

private:

    static std::optional<UserSession> currUserSession;

    static int createRootAndUsrsTxt(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity);

    static int chmod(const std::string& path, int ugo);
    static int recursiveChmod(const std::string& path, int ugo);

    //para manejar files:
    static int fileConcat(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity, const std::string& content);//-1 si se queda sin espacio
    static int fileConcatImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int &beg, std::string &content, char depth, int address, int& fileblockBeg);
    static int fileWrite(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity, const std::string& content);//-1 si se queda sin espacio
    static int fileWriteImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, int &beg, std::string &content, char depth, int address);
    static int fileClear(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity);
    static int fileClearImp(RaidOneFile *file, DiskEntity<SuperBoot> *sbEntity, char depth, int address);
    static int fileEdit(RaidOneFile* file, DiskEntity<SuperBoot> *sbEntity, DiskEntity<Inode> *fileEntity, const std::string& content);//-1 si se queda sin espacio

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


    //Para manejar el journaling:

friend int main();//Solo para debugging
};

#endif // EXTMANAGER_H