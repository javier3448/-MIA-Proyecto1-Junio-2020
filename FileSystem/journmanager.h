#ifndef JOURNMANAGER_H
#define JOURNMANAGER_H

#include "raidonefile.h"
#include "FileSystem/superboot.h"
#include "journaling.h"

class JournManager
{
public:
    static void initEntry(JournEntry* journaling);
    static void initEntry(JournEntry* journaling, char key);

    static int addLogin(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& usr, const std::string& pwd, const std::string& id);
    static int addLogout(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity);
    static int addMkGrp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& name);
    static int addRmGrp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& name);
    static int addMkUsr(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& name, const std::string& grp , const std::string& pwd);
    static int addRmUsr(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& name);
    static int addChmod(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path, int ugo, bool r);
    static int addMkFile(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path, bool p, int size, const std::string& paramsCont);
    static int addCat(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path);
    static int addRem(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path);
    static int addEdit(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path, const std::string& content);
    static int addRen(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path, const std::string& name);
    static int addMkDir(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path, bool p);

private:
    static int addEntryToSb(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, JournEntry* journEntry);

};

#endif // JOURNMANAGER_H
