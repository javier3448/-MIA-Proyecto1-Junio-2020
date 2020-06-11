#ifndef INTERPRETE_H
#define INTERPRETE_H

#include "Analizador/extparams.h"
#include "consola.h"
#include "Disk/diskmanager.h"
#include "Analizador/scanner.h"
#include "Analizador/parser.h"
#include "FileSystem/journaling.h"

extern int column;

namespace Interprete
{
    //Param flag
    namespace pf{
        const int R = 1 << 1;
        const int P = 1 << 2;
        const int RF = 1 << 3;
        const int UNIT = 1 << 4;
        const int FIT_TYPE = 1 << 5;
        const int DELETE_TYPE = 1 << 6;
        const int TYPE = 1 << 7;
        const int SIZE = 1 << 8;
        const int UGO = 1 << 9;
        const int ADD = 1 << 10;
        const int ID = 1 << 11;
        const int FILE = 1 << 12;
        const int PATH = 1 << 13;
        const int NAME = 1 << 14;
        const int USR = 1 << 15;
        const int PWD = 1 << 16;
        const int CONT = 1 << 17;
        const int RUTA = 1 << 18;

        const int GRP = 1 << 19;
        const int DEST = 1 << 20;
    }

    //TODO: AGREGAR Y QUITAR LOS COMANDOS NECESARIOS PARA PREPARAR LA FASE 2
    enum Cmd { //Para el lwhLog, creo
        NONE = 0, LOGIN, LOGOUT, MKGRP, RMGRP, MKUSR,
        RMUSR, CHMOD, MKFILE, CAT, REM,
        EDIT, REN, MKDIR, CP, MV, FIND, CHOWN, CHGRP
    };

    int compile(const std::string& srcCode);

    int mkDisk(ExtParams* params);
    int rmDisk(ExtParams* params);
    int fDisk(ExtParams* params);

    int mount(ExtParams* params);
    int unmount(ExtParams* params);
    int pause(ExtParams* params);
//    int exec(LwhParams* params);

    //De fase 2:
    int mkfs(ExtParams* params);
    int login(ExtParams* params);
    int logout(ExtParams* params);
    int mkgrp(ExtParams* params);
    int rmgrp(ExtParams* params);
    int mkusr(ExtParams* params);
    int rmusr(ExtParams* params);
    int chmod(ExtParams* params);
    int mkfile(ExtParams* params);
    int cat(ExtParams* params);
    int rem(ExtParams* params);
    int edit(ExtParams* params);
    int ren(ExtParams* params);
    int mkdir(ExtParams* params);
    int cp(ExtParams* params);
    int mv(ExtParams* params);
    int find(ExtParams* params);
    int chown(ExtParams* params);
    int chgrp(ExtParams* params);

    int rep(ExtParams* params);

    bool hasEnding(std::string const &fullString, std::string const &ending);

    //Verifica si params contiene una definicion NO NULL para todos los parametros listados en flags
    //Retorna el mensaje de error en caso que un parametro este mal definido
    std::string defines(ExtParams* params, int paramFlag);

    //Verifica si params contiene una definicion NULL para todos los parametros listados en flags
    std::string nullDefines(ExtParams* params, int paramFlag);

    //Verifica que params que solo defina los parametros en
    std::string exclusiveDefines(ExtParams* params, int requiredParms, int optionalParams);
}

#endif // INTERPRETE_H
