#ifndef JOURNALING_H
#define JOURNALING_H

#include "raidonefile.h"
#include "FileSystem/superboot.h"
#include "diskentity.h"

//TODO: VER QUE VA AQUI. Creo que seria algo muy parecido al lwhLog
struct JournEntry{
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
            char path[126];
            int ugo;
            bool r;
        }chMod;

        struct mkFile{
            char path[126];
            bool p;
            int size;
            char cont[126];
        }mkFile;

        struct cat{
            char file[126];
        }cat;

        struct rem{
            char path[126];
        }rem;

        struct edit{
            char path[126];
            char cont[126];
        }edit;

        struct ren{
            char path[126];
            char name[16];
        }ren;

        struct mkDir{
            char path[126];
            bool p;
        }mkDir;

        struct cp{
            char path[126];
            char dest[126];
        }cp;

        struct mv{
            char path[126];
            char dest[126];
        }mv;

        struct find{
            char path[126];
            char name[16];
        }find;

        struct chown{
            char path[126];
            bool r;
            char usr[16];
        }chown;

        struct chgrp{
            char usr[16];
            char grp[16];
        }chgrp;
    }u;
};

#endif // JOURNALING_H
