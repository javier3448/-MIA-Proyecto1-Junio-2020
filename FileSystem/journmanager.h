#ifndef JOURNMANAGER_H
#define JOURNMANAGER_H

#include "raidonefile.h"
#include "FileSystem/superboot.h"
#include "journaling.h"

class JournManager
{
public:
    static void initEntry(JournEntry* journaling);
    //Author is the usr that executed the command, we didnt just name it usr because that name might be used by an
    //argument of a command
    static void initEntry(JournEntry* journaling, char key, const std::string& author);

    //Los primeros 3 params de un add siempre bienen, entonces deberian de estar todos juntitos en un
    //objeto o algo asi. De cualquier modo hacen mucho bulto y se ven feos, hay que buscar una solucion
    static int addLogin(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& usr, const std::string& pwd, const std::string& id);
    static int addLogout(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author);
    static int addMkGrp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& name);
    static int addRmGrp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& name);
    static int addMkUsr(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& name, const std::string& grp , const std::string& pwd);
    static int addRmUsr(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& name);
    static int addChmod(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& path, int ugo, bool r);
    static int addMkFile(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& path, bool p, int size, const std::string& paramsCont);
    static int addCat(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& path);
    static int addMkDir(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& path, bool p);
    static int addCp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& path, const std::string& dest);
    static int addRem(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& path);
    static int addEdit(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& path, const std::string& content);
    static int addRen(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& path, const std::string& name);
    static int addMv(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& path, const std::string& dest);
    static int addFind(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& path, const std::string& name);
    static int addChown(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& path, bool r, const std::string& usr);
    static int addChgrp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                        const std::string& usr, const std::string& grp);

    static int addEntryToSb(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, JournEntry* journEntry);

};

#endif // JOURNMANAGER_H
