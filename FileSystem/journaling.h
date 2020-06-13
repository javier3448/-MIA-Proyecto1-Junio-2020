#ifndef JOURNALING_H
#define JOURNALING_H

#include "raidonefile.h"
#include "FileSystem/superboot.h"
#include "diskentity.h"

//TODO: VER QUE VA AQUI. Creo que seria algo muy parecido al lwhLog
struct JournEntry{
    std::tm time;
    char user[12];


    char key = 0;//0 si esta vacia
    union{
        struct login{
            char name[16];
            char pwd[16];
            char id[16];
        }login;

        struct logout{
            char c;
        }logout;

        struct mkGrp{
            char name[16];
        }mkGrp;

        struct rmGrp{
            char name[16];
        }rmGrp;

        struct mkUsr{
            char name[16];
            char pwd[16];
            char grp[16];
        }mkUsr;

        struct rmUsr{
            char name[16];
        }rmUsr;

        struct chMod{
            char path[124];
            int ugo;
            bool r;
        }chMod;

        struct mkFile{
            char path[124];
            char cont[124];
            bool p;
            int size;
        }mkFile;

        struct cat{
            char file[124];
        }cat;

        struct rem{
            char path[124];
        }rem;

        struct edit{
            char path[124];
            char cont[124];
        }edit;

        struct ren{
            char path[124];
            char name[16];
        }ren;

        struct mkDir{
            char path[124];
            bool p;
        }mkDir;

        struct cp{
            char path[124];
            char dest[124];
        }cp;

        struct mv{
            char path[124];
            char dest[124];
        }mv;

        struct find{
            char path[124];
            char name[16];
        }find;

        struct chown{
            char path[124];
            bool r;
            char usr[16];
        }chown;

        struct chgrp{
            char usr[16];
            char grp[16];
        }chgrp;

    }args;
};

#endif // JOURNALING_H
