#ifndef GRAPHMAKER_H
#define GRAPHMAKER_H

#include <ctime>
#include "diskentity.h"
#include "Disk/mbr.h"
#include "Disk/ebr.h"
#include "FileSystem/superboot.h"
#include "FileSystem/UserManagement/usersdata.h"
#include "Reportes/DotObjects/dotfileblock.h"
#include "Reportes/DotObjects/dotfolderblock.h"
#include "Reportes/DotObjects/dotinode.h"
#include "Reportes/DotObjects/dotpointerblock.h"

class GraphMaker
{
public:
    static int mkDotMbr(std::string& outDotCode, const Mbr& mbr);
    static int mkDotDisk(std::string& outDotCode, RaidOneFile* file, Mbr* mbr);
    static int mkDotEbr(std::string& outDotCode, DiskEntity<Ebr>* ebrEntity, const std::string& suffix = std::string());

    static int mkDotSb(std::string& outDotCode, DiskEntity<SuperBoot>* sbEntity);
    static int mkDotLs(std::string& outDotCode, RaidOneFile* file, DiskEntity<Inode>* fileEntity, const UsersData& usrsData);
    static int mkDotTree(std::string& outDotCode, RaidOneFile* file, DiskEntity<Inode>* rootEntity);
    static int mkDotBlock(std::string& outDotCode, RaidOneFile* file, DiskEntity<Inode>* rootEntity);
    static int mkDotInode(std::string& outDotCode, RaidOneFile* file, DiskEntity<Inode>* rootEntity);

private:

    //Name vacio si el depth no es 0
    static bool mkDotLsImp(std::string& outDotCode, RaidOneFile* file, char depth, int address, const UsersData& usrsData);
    static std::string mkInodeTr(DiskEntity<Inode>* inodeEntity, const std::string& name, const UsersData& usrsData);
    static std::string intToPermissionsHtmlString(int permissions);
    static std::string intToInodeTypeHtmlString(int type);

    //el nombre de los siguientes mkDot sigue la siguiente convesion: mkDot<NombreDeReporte><EntityQueManejaElMetodo>
    static void mkDotTreeInode(std::string& outDotCode, RaidOneFile* file, int inodeAdress);
    static void mkDotTreeBlock(std::string& outDotCode, RaidOneFile *file, int type, char depth, int blockAdress);

    static void mkDotBlockInode(std::string& outDotCode, RaidOneFile* file, int inodeAdress);
    static void mkDotBlockBlock(std::string& outDotCode, RaidOneFile *file, int type, char depth, int blockAdress);

    static void mkDotInodeInode(std::string& outDotCode, RaidOneFile* file, int inodeAdress);
    static void mkDotInodeBlock(std::string& outDotCode, RaidOneFile *file, int type, char depth, int blockAdress);

    static void writeTableExtendedPart(std::string& outDotCode, RaidOneFile* file, Partition* extendedPart, float disksize);
    static void splitSubDirNameAndRemainingPath(const std::string& absolutePath, std::string& subDirName, std::string& remainingPath);
    static void splitDirPathAndFileName(const std::string& absolutePath, std::string& dirPath, std::string& fileName);
    static std::string dateToString(const std::tm& time, const std::string& fmt = "%d-%m-%Y %H:%M:%S");
    static std::string charToString(char c);
    static std::string floatToString(float f);
};

#endif // GRAPHMAKER_H
