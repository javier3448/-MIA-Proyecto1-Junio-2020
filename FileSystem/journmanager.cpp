#include "journmanager.h"

int JournManager::addEntryToSb(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, struct JournEntry* journEntry)
{
    int journAddress = sbEntity->address + (int)sizeof(SuperBoot);
    while(true){
        char key;
        if(journAddress >= sbEntity->value.bitmapInodeBegin){
            return -1;
        }
        file->seekg((uint)journAddress, std::ios::beg);
        file->read(&key, 1);
        if(key == 0){
            file->seekp((uint)journAddress, std::ios::beg);
            file->write((char*)journEntry, sizeof (JournEntry));
            return 0;
        }
        journAddress += sizeof (JournEntry);
    }
}

void JournManager::initEntry(struct JournEntry *journaling, char key)
{
    memset(&journaling->key, 0, sizeof (JournEntry));
    journaling->key = key;
}

void JournManager::initEntry(struct JournEntry *journaling)
{
    memset(&journaling->key, 0, sizeof (JournEntry));
}

int JournManager::addLogin(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& name, const std::string& pwd, const std::string& id)
{
    struct  JournEntry entry;
    entry.key = Interprete::Cmd::LOGIN;
    std::strcpy(entry.u.login.id, id.c_str());
    std::strcpy(entry.u.login.name, name.c_str());
    std::strcpy(entry.u.login.pwd, pwd.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addLogout(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity)
{
    struct JournEntry entry;
    entry.key = Interprete::Cmd::LOGOUT;
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addMkGrp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& name)
{
    struct JournEntry entry;
    entry.key = Interprete::Cmd::MKGRP;
    std::strcpy(entry.u.mkGrp.name, name.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addRmGrp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& name)
{
    struct JournEntry entry;
    entry.key = Interprete::Cmd::RMGRP;
    std::strcpy(entry.u.rmGrp.name, name.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addMkUsr(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& name, const std::string& grp , const std::string& pwd)
{
    struct JournEntry entry;
    entry.key = Interprete::Cmd::MKUSR;
    std::strcpy(entry.u.mkUsr.grp, grp.c_str());
    std::strcpy(entry.u.mkUsr.pwd, pwd.c_str());
    std::strcpy(entry.u.mkUsr.name, name.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addRmUsr(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& name)
{
    struct JournEntry entry;
    entry.key = Interprete::Cmd::RMUSR;
    std::strcpy(entry.u.rmUsr.name, name.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addChmod(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path, int ugo, bool r)
{
    struct JournEntry entry;
    entry.key = Interprete::Cmd::CHMOD;
    entry.u.chMod.r = r;
    entry.u.chMod.ugo = ugo;
    std::strcpy(entry.u.chMod.path, path.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addMkFile(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path, bool p, int paramsSize, const std::string& paramsCont)
{
    struct JournEntry entry;
    entry.key = Interprete::Cmd::MKFILE;
    entry.u.mkFile.p = p;
    std::strcpy(entry.u.mkFile.cont, paramsCont.c_str());
    std::strcpy(entry.u.mkFile.path, path.c_str());
    entry.u.mkFile.size = paramsSize;
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addCat(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path)
{
    struct JournEntry entry;
    entry.key = Interprete::Cmd::CAT;
    std::strcpy(entry.u.cat.file, path.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addRem(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path)
{
    struct JournEntry entry;
    entry.key = Interprete::Cmd::REM;
    std::strcpy(entry.u.rem.path, path.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addEdit(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path, const std::string& content)
{
    struct JournEntry entry;
    entry.key = Interprete::Cmd::EDIT;
    std::strcpy(entry.u.edit.cont, content.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addRen(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path, const std::string& name)
{
    struct JournEntry entry;
    entry.key = Interprete::Cmd::REN;
    std::strcpy(entry.u.ren.path, path.c_str());
    std::strcpy(entry.u.ren.name, name.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addMkDir(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& path, bool p)
{
    struct JournEntry entry;
    entry.key = Interprete::Cmd::MKDIR;
    entry.u.mkDir.p = p;
    std::strcpy(entry.u.mkDir.path, path.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}
