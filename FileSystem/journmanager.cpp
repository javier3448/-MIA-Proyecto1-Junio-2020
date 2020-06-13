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

void JournManager::initEntry(struct JournEntry *journaling, char key, const std::string& author)
{
    //Vaciamos todos los parametros, esto es solo por si acado y no deberia ser necesario en lo absoluto
    memset(&journaling->args, 0, sizeof (journaling->args));
    journaling->key = key;
    std::strcpy(journaling->user, author.c_str());
    time_t rawtime;
    time(&rawtime);
    journaling->time = *localtime(&rawtime);
}

void JournManager::initEntry(struct JournEntry *journaling)
{
    //Vaciamos todo el journaling, lo unico que es necesario es poner en 0 key pero lo vacio todo solo por si acaso
    memset(&journaling, 0, sizeof(JournEntry));
}

int JournManager::addLogin(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string& name, const std::string& pwd, const std::string& id)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::LOGIN, author);
    std::strcpy(entry.args.login.id, id.c_str());
    std::strcpy(entry.args.login.name, name.c_str());
    std::strcpy(entry.args.login.pwd, pwd.c_str());
    return addEntryToSb(file, sbEntity, &entry);}

int JournManager::addLogout(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::LOGOUT, author);
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addMkGrp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string& name)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::MKGRP, author);
    std::strcpy(entry.args.mkGrp.name, name.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addRmGrp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string& name)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::RMGRP, author);
    std::strcpy(entry.args.rmGrp.name, name.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addMkUsr(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string& name, const std::string& grp , const std::string& pwd)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::MKUSR, author);
    std::strcpy(entry.args.mkUsr.grp, grp.c_str());
    std::strcpy(entry.args.mkUsr.pwd, pwd.c_str());
    std::strcpy(entry.args.mkUsr.name, name.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addRmUsr(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string& name)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::RMUSR, author);
    std::strcpy(entry.args.rmUsr.name, name.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addChmod(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string& path, int ugo, bool r)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::CHMOD, author);
    entry.args.chMod.r = r;
    entry.args.chMod.ugo = ugo;
    std::strcpy(entry.args.chMod.path, path.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addMkFile(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string& path, bool p, int paramsSize, const std::string& paramsCont)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::MKFILE, author);
    entry.args.mkFile.p = p;
    std::strcpy(entry.args.mkFile.cont, paramsCont.c_str());
    std::strcpy(entry.args.mkFile.path, path.c_str());
    entry.args.mkFile.size = paramsSize;
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addCat(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string& path)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::CAT, author);
    std::strcpy(entry.args.cat.file, path.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addRem(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string& path)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::REM, author);
    std::strcpy(entry.args.rem.path, path.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addEdit(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string& path, const std::string& content)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::EDIT, author);
    std::strcpy(entry.args.edit.path, path.c_str());
    std::strcpy(entry.args.edit.cont, content.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addRen(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string& path, const std::string& name)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::REN, author);
    std::strcpy(entry.args.ren.path, path.c_str());
    std::strcpy(entry.args.ren.name, name.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addMkDir(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string& path, bool p)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::MKDIR, author);
    entry.args.mkDir.p = p;
    std::strcpy(entry.args.mkDir.path, path.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addCp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string &path, const std::string &dest)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::CP, author);
    std::strcpy(entry.args.cp.path, path.c_str());
    std::strcpy(entry.args.cp.dest, dest.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addMv(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string &path, const std::string &dest)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::MV, author);
    std::strcpy(entry.args.mv.path, path.c_str());
    std::strcpy(entry.args.mv.dest, dest.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addFind(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string &path, const std::string &name)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::FIND, author);
    std::strcpy(entry.args.find.path, path.c_str());
    std::strcpy(entry.args.find.name, name.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addChown(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string &path, bool r, const std::string &usr)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::CHOWN, author);
    std::strcpy(entry.args.chown.path, path.c_str());
    entry.args.chown.r = r;
    std::strcpy(entry.args.chown.usr, usr.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}

int JournManager::addChgrp(RaidOneFile* file, DiskEntity<SuperBoot>* sbEntity, const std::string& author,
                            const std::string &usr, const std::string &grp)
{
    struct JournEntry entry;
    initEntry(&entry, Interprete::Cmd::CP, author);
    std::strcpy(entry.args.chgrp.grp, grp.c_str());
    std::strcpy(entry.args.chgrp.usr, usr.c_str());
    return addEntryToSb(file, sbEntity, &entry);
}
