TEMPLATE = app
CONFIG += console c++17
CONFIG += c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        Analizador/extparams.cpp \
        Analizador/interprete.cpp \
        Analizador/parser.cpp \
        Analizador/scanner.cpp \
        Disk/diskmanager.cpp \
        Disk/ebr.cpp \
        Disk/mbr.cpp \
        Disk/mountedpart.cpp \
        Disk/partition.cpp \
        FileSystem/UserManagement/group.cpp \
        FileSystem/UserManagement/user.cpp \
        FileSystem/UserManagement/usersdata.cpp \
        FileSystem/UserManagement/usersession.cpp \
        FileSystem/extmanager.cpp \
        FileSystem/extutilty.cpp \
        FileSystem/fileblock.cpp \
        FileSystem/folderblock.cpp \
        FileSystem/foldercontent.cpp \
        FileSystem/inode.cpp \
        FileSystem/journaling.cpp \
        FileSystem/journmanager.cpp \
        FileSystem/pointerblock.cpp \
        FileSystem/superboot.cpp \
        Reportes/DotObjects/dotfileblock.cpp \
        Reportes/DotObjects/dotfolderblock.cpp \
        Reportes/DotObjects/dotinode.cpp \
        Reportes/DotObjects/dotpointerblock.cpp \
        Reportes/graphmaker.cpp \
        Reportes/graphvizhelper.cpp \
        Reportes/reportes.cpp \
        consola.cpp \
        main.cpp \
        mystringutil.cpp \
        raidonefile.cpp

DISTFILES += \
    Analizador/generador.sh \
    Analizador/parser.output \
    Analizador/zComandosUtiles

HEADERS += \
    Analizador/extparams.h \
    Analizador/interprete.h \
    Analizador/parser.h \
    Analizador/parser.yy \
    Analizador/scanner.h \
    Analizador/scanner.l \
    Disk/diskmanager.h \
    Disk/ebr.h \
    Disk/mbr.h \
    Disk/mountedpart.h \
    Disk/partition.h \
    FileSystem/UserManagement/group.h \
    FileSystem/UserManagement/user.h \
    FileSystem/UserManagement/usersdata.h \
    FileSystem/UserManagement/usersession.h \
    FileSystem/extmanager.h \
    FileSystem/extutility.h \
    FileSystem/fileblock.h \
    FileSystem/folderblock.h \
    FileSystem/foldercontent.h \
    FileSystem/inode.h \
    FileSystem/journaling.h \
    FileSystem/journmanager.h \
    FileSystem/pointerblock.h \
    FileSystem/superboot.h \
    Reportes/DotObjects/dotfileblock.h \
    Reportes/DotObjects/dotfolderblock.h \
    Reportes/DotObjects/dotinode.h \
    Reportes/DotObjects/dotpointerblock.h \
    Reportes/graphmaker.h \
    Reportes/graphvizhelper.h \
    Reportes/reportes.h \
    consola.h \
    diskentity.h \
    diskentity.hpp \
    mystringutil.h \
    raidonefile.h
