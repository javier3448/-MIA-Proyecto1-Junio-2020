#include "interprete.h"

#include <algorithm>
#include "Reportes/reportes.h"
#include "FileSystem/extmanager.h"
#include "mystringutil.h"

//'Private' methods
namespace  {
    //decUgo debe de tener 3 digitos decimales
    std::optional<int> toHexUgo(int decUgo){
        if(decUgo > 777 ||
            decUgo < 0){
            return {};
        }
        int hexUgo = 0;
        std::string s = std::to_string(decUgo);
        hexUgo += s[s.length() - 1] - '0';
        if(s.length() > 1){
            hexUgo += (s[s.length() - 2] - '0') * 16;
        }
        if(s.length() > 2){
            hexUgo += (s[s.length() - 3] - '0') * 16 * 16;
        }
        return hexUgo;
    }
}

int Interprete::mkDisk(ExtParams* params)
{
    //Chapuz se supone que disk no tiene fit pero en el archivo de entrada viene con fit?
    std::string msg = exclusiveDefines(params,
                                       pf::SIZE | pf::PATH,
                                       pf::UNIT | pf::FIT_TYPE);

    if(msg.length())
    {
        msg = "Parametros no validos para comando mkDisk: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    //Verificacion adicional para este comando. unit tiene que ser m o k
    if(params->unit == ExtParamsConstants::UNIT_TYPE_B){
        msg += "\n    unit no valido";
    }
    std::string absolutePath = *params->path;

    //setea los default
    char unit = (params->unit ? params->unit : ExtParamsConstants::UNIT_TYPE_K);

    char fit = (params->fitType ? params->fitType : ExtParamsConstants::FIT_TYPE_FF);

    return DiskManager::mkDisk(absolutePath, params->size, fit, unit);
}

int Interprete::rmDisk(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::PATH,
                                       0);

    if(params->unit == ExtParamsConstants::UNIT_TYPE_B){
        msg += "\n    size debe ser mayor a 0";
    }

    if(msg.length())
    {
        msg = "Parametros no validos para comando rmDisk: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    Consola::printLine(std::string("Esta seguro que desea eliminar el archivo: ") + *params->path + " y su copia?\n1. si\n2. no");
    int input = Consola::readInt();
    std::cin.ignore();//ignoramos el enter, creo. [?]
    //TODO: No se si necesitamos hacer un ignore despues de usar get int
    if(input != 1){
        return 0;
    }

    return DiskManager::rmDisk(*(params->path));
}

int Interprete::fDisk(ExtParams* params)
{
    std::string msgGeneral = exclusiveDefines(params,
                                              pf::SIZE | pf::PATH,
                                              pf::NAME | pf::DELETE_TYPE | pf::ADD | pf::UNIT | pf::TYPE | pf::FIT_TYPE);

    std::string msgNewF = exclusiveDefines(params,
                                            pf::SIZE | pf::PATH | pf::NAME,
                                            pf::UNIT | pf::FIT_TYPE | pf::TYPE);

    std::string msgDeleteF = exclusiveDefines(params,
                                              pf::PATH | pf::NAME | pf::DELETE_TYPE,
                                              pf::UNIT);

    std::string msgAddF = exclusiveDefines(params,
                                           pf::ADD | pf::PATH | pf::NAME,
                                           pf::UNIT);

    char type = (params->type ? params->type : ExtParamsConstants::PART_TYPE_P);
    char unit = (params->unit ? params->unit : ExtParamsConstants::UNIT_TYPE_K);
    char fitType = (params->fitType ? params->fitType : ExtParamsConstants::FIT_TYPE_WF );

    if(msgNewF.size() == 0){
        if (params->name->length() >= 16)
        {
            Consola::reportarError("No se puede eliminar una particion con nombre de longitud mayor o igual a 16" + *params->name);
            return -1;
        }
        return DiskManager::fDiskNew(*params->path, *params->name, params->size, fitType, unit, type);
    }
    else if(msgDeleteF.size() == 0){
        if (params->name->length() >= 16)
        {
            Consola::reportarError("No se puede eliminar una particion con nombre de longitud mayor o igual a 16" + *params->name);
            return -1;
        }
        return DiskManager::fDiskDelete(*params->path, *params->name, params->deleteType);
    }
    else if (msgAddF.size() == 0) {
        if (params->name->length() >= 16)
        {
            Consola::reportarError("No se puede crear una particion con nombre de longitud mayor o igual a 16" + *params->name);
            return -1;
        }
        return DiskManager::fDiskAdd(*params->path, *params->name, params->add, params->unit);
    }

    msgGeneral = "Parametros no validos para comando fDisk: " + msgGeneral;
    Consola::reportarError(msgGeneral);
    return -1;
}

int Interprete::mount(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::ID | pf::NAME,
                                       0);

    if(msg.size()==0){
        Consola::printLine("Particiones montadas: ");
        for (auto part : mountedPartitions) {
            Consola::printLine(part.first + ": " + part.second.path);
        }

        return 0;
    }

    msg = exclusiveDefines(params,
                           pf::PATH | pf::NAME,
                           0);

    if(msg.length()){
        msg = "Parametros no validos para comando mount: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    if (params->name->length() >= 16)
    {
        Consola::reportarError("No se puede montar una particion con nombre de longitud mayor o igual a 16. " + *params->name);
        return -1;
    }

    return DiskManager::mount(*params->path, *params->name);
}
int Interprete::unmount(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::ID,
                                       0);

    if(msg.length()){
        msg = "Parametros no validos para comando unmount: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    if (params->id->length() >= 16)
    {
        Consola::reportarError(std::string() + "No se puede desmontar una particion con nombre de longitud mayor o igual a 16. " + *params->id);
        return -1;
    }

    return DiskManager::unMount(*params->id);
}

int Interprete::compile(const std::string& srcCode)
{
    yy_scan_string(srcCode.c_str());
    column = 0;
    yylineno = 0;
    if(yyparse()<0)
        return -1;

    return 0;
}

int Interprete::rep(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::NAME | pf::PATH | pf::ID,
                                       pf::RUTA);

    if(msg.length()){
        msg = "Parametros no validos para comando rep: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    std::string repName = *(params->name);
    std::transform(repName.begin(), repName.end(), repName.begin(), tolower);
    std::string id = *params->id;
    std::string path = params->path->substr(0, params->path->find_last_of("."));

    if (repName == "mbr") {
        Consola::printCommandLine("mbr");
        return Reportes::repMbr(id, path);
    }
    if(repName == "dsk" || repName == "disk"){
        Consola::printCommandLine("dsk");
        return Reportes::repDisk(id, path);
    }
    if(repName == "sb"){
        Consola::printCommandLine("sb");
        return Reportes::repSb(id, path);
    }
    if(repName == "bm_inode"){
        Consola::printCommandLine("bm_inode");
        return Reportes::repBmInode(id, path);
    }
    if(repName == "bm_block"){
        Consola::printCommandLine("bm_block");
        return Reportes::repBmBlock(id, path);
    }
    if(repName == "file"){
        if(!params->ruta){
            Consola::reportarError("Se necesita definir ruta para usar el comando rep file");
            return -1;
        }
        if(!MyStringUtil::isValidExtPath(*params->ruta)){
            Consola::reportarError("Ruta no es un extpath: <" + *params->ruta + ">");
            return -1;
        }
        Consola::printCommandLine("file");
        return Reportes::repFile(id, path, *params->ruta);
    }
    if(repName == "ls"){
        if(!params->ruta){
            Consola::reportarError("Se necesita definir ruta para usar el comando rep ls");
            return -1;
        }
        if(!MyStringUtil::isValidExtPath(*params->ruta)){
            Consola::reportarError("Path no es un extpath: <" + *params->ruta + ">");
            return -1;
        }
        Consola::printCommandLine("ls");
        return Reportes::repLs(id, path, *params->ruta);
    }
    if(repName == "tree"){
        Consola::printCommandLine("tree");
        return Reportes::repTree(id, path);
    }
    if(repName == "inode"){
        Consola::printCommandLine("inode");
        return Reportes::repInode(id, path);
    }
    if(repName == "journaling"){
        Consola::printCommandLine("journaling");
        return Reportes::repJournaling(id, path);
    }
    if(repName == "block"){
        Consola::printCommandLine("block");
        return Reportes::repBlock(id, path);
    }

    Consola::printLine(repName + " no es un nombre de reporte valido");
    return -1;
}

int Interprete::pause(ExtParams* params)
{
    Consola::pause();
    return 0;
}

//************************************************
//------------------DE LA FASE 2------------------
//************************************************

int Interprete::mkfs(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::ID,
                                       pf::TYPE);
    if(msg.length()){
        msg = "Parametros no validos para comando mkFs: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    char type = (params->type ? params->type : ExtParamsConstants::FORMAT_TYPE_FULL);

    return ExtManager::mkfs(*(params->id), type);
}

int Interprete::login(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::ID | pf::USR | pf::PWD,
                                       0);
    if(msg.length()){
        msg = "Parametros no validos para comando login: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    return ExtManager::login(*params->usr, *params->pwd, *params->id);
}

int Interprete::logout(ExtParams *params)
{
    return ExtManager::logout();
}

int Interprete::mkgrp(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::NAME,
                                       0);
    if(msg.length()){
        msg = "Parametros no validos para comando mkgrp: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    return ExtManager::mkGrp(*params->name);
}

int Interprete::rmgrp(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::NAME,
                                       0);
    if(msg.length()){
        msg = "Parametros no validos para comando rmpgrp: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    return ExtManager::rmGrp(*params->name);
}


int Interprete::mkusr(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::NAME | pf::PWD | pf::GRP,
                                       0);
    if(msg.length()){
        msg = "Parametros no validos para comando mkusr: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    return ExtManager::mkUsr(*params->name, *params->grp, *params->pwd);
}

int Interprete::rmusr(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::NAME,
                                       0);
    if(msg.length()){
        msg = "Parametros no validos para comando rmusr: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    return ExtManager::rmUsr(*params->name);
}

int Interprete::chmod(ExtParams* params)
{
    std::string msg  = exclusiveDefines(params,
                                        pf::PATH | pf::UGO,
                                        pf::R);

    if(msg.length()){
        msg = "Parametros no validos para comando chmod: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    auto hexUgo = toHexUgo(params->ugo);

    if(!hexUgo){
        Consola::reportarError("ugo no valido: " + std::to_string(params->ugo));
        return -1;
    }

    if(!MyStringUtil::isValidExtPath(*params->path)){
        Consola::reportarError("Path no es un extpath: <" + *params->path + ">");
        return -1;
    }

    return ExtManager::chmod(*params->path, hexUgo.value(), params->r, params->ugo);
}

int Interprete::mkfile(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::PATH,
                                       pf::P | pf::SIZE | pf::CONT);
    if(msg.length()){
        msg = "Parametros no validos para comando mkfile: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    if(!MyStringUtil::isValidExtPath(*params->path)){
        Consola::reportarError("Path no es un extpath: <" + *params->path + ">");
        return -1;
    }

    std::optional<std::string> content;
    std::string paramsCont;//Para hacer el journ mas adelante
    if(params->cont){
        paramsCont = *params->cont;
        content = MyStringUtil::readFile(*params->cont);
        if(!content){
            Consola::reportarError("cont no es un archivo valido: <" + *params->cont + ">");
            return -1;
        }
    }else{
        paramsCont = "";
        int size = (params->size == -1 ? 0 : params->size);
        content = MyStringUtil::buildDigitString(size);
    }

    int paramsSize;
    if(params->size != -1){
        paramsSize = params->size;
    }else{
        paramsSize = 0;
    }

    return ExtManager::mkFile(*params->path, params->p, content.value(), paramsSize, paramsCont);
}

int Interprete::cat(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::FILE,
                                       0);
    if(msg.length()){
        msg = "Parametros no validos para comando cat: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    if(!MyStringUtil::isValidExtPath(*params->file)){
        Consola::reportarError("Path no es un extpath: <" + *params->file+ ">");
        return -1;
    }

    return ExtManager::cat(*params->file);
}

int Interprete::rem(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::PATH,
                                       0);

    if(msg.length()){
        msg = "Parametros no validos para comando rem: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    if(!MyStringUtil::isValidExtPath(*params->path)){
        Consola::reportarError("Path no es un extpath: <" + *params->file+ ">");
        return -1;
    }

    return ExtManager::rem(*params->path);
}

int Interprete::edit(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::PATH | pf::CONT,
                                       0);

    if(msg.length()){
        msg = "Parametros no validos para comando edit: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    if(!MyStringUtil::isValidExtPath(*params->path)){
        Consola::reportarError("Path no es un extpath: <" + *params->file+ ">");
        return -1;
    }

    std::optional<std::string> content;
    std::string paramsCont;//Para hacer el journ mas adelante
    if(params->cont){
        paramsCont = *params->cont;
        content = MyStringUtil::readFile(*params->cont);
        if(!content){
            Consola::reportarError("cont no es un archivo valido: <" + *params->cont + ">");
            return -1;
        }
    }else{
        paramsCont = "";
        int size = (params->size == -1 ? 0 : params->size);
        content = MyStringUtil::buildDigitString(size);
    }

    return ExtManager::edit(*params->path, content.value());
}

int Interprete::ren(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::PATH | pf::NAME,
                                       0);

    if(msg.length()){
        msg = "Parametros no validos para comando rem: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    if(!MyStringUtil::isValidExtPath(*params->path)){
        Consola::reportarError("Path no es un extpath: <" + *params->file+ ">");
        return -1;
    }

    return ExtManager::ren(*params->path, *params->name);
}

int Interprete::mkdir(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::PATH,
                                       pf::P);
    if(msg.length()){
        msg = "Parametros no validos para comando mkfile: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    if(!MyStringUtil::isValidExtPath(*params->path)){
        Consola::reportarError("Path no es un extpath: <" + *params->path + ">");
        return -1;
    }

    return ExtManager::mkDir(*params->path, params->p);
}

int Interprete::cp(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::PATH | pf::DEST,
                                       0);
    if(msg.length()){
        msg = "Parametros no validos para comando mkfile: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    if(!MyStringUtil::isValidExtPath(*params->path)){
        Consola::reportarError("Path no es un extpath: <" + *params->path + ">");
        return -1;
    }
    if(!MyStringUtil::isValidExtPath(*params->dest)){
        Consola::reportarError("Path no es un extpath: <" + *params->path + ">");
        return -1;
    }

    return ExtManager::cp(*params->path, *params->dest);
}

int Interprete::mv(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::PATH | pf::DEST,
                                       0);
    if(msg.length()){
        msg = "Parametros no validos para comando mkfile: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    if(!MyStringUtil::isValidExtPath(*params->path)){
        Consola::reportarError("Path no es un extpath: <" + *params->path + ">");
        return -1;
    }
    if(!MyStringUtil::isValidExtPath(*params->dest)){
        Consola::reportarError("Path no es un extpath: <" + *params->path + ">");
        return -1;
    }

    return ExtManager::mv(*params->path, *params->dest);
}

int Interprete::find(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::PATH | pf::NAME,
                                       0);
    if(msg.length()){
        msg = "Parametros no validos para comando mkfile: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    if(!MyStringUtil::isValidExtPath(*params->path)){
        Consola::reportarError("Path no es un extpath: <" + *params->path + ">");
        return -1;
    }

    return ExtManager::find(*params->path, *params->name);
}

int Interprete::chown(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::PATH | pf::USR,
                                       pf::R);
    if(msg.length()){
        msg = "Parametros no validos para comando mkfile: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    if(!MyStringUtil::isValidExtPath(*params->path)){
        Consola::reportarError("Path no es un extpath: <" + *params->path + ">");
        return -1;
    }

    return ExtManager::chown(*params->path, params->r, *params->usr);
}

int Interprete::chgrp(ExtParams* params)
{
    std::string msg = exclusiveDefines(params,
                                       pf::USR | pf::GRP,
                                       0);
    if(msg.length()){
        msg = "Parametros no validos para comando mkfile: " + msg;
        Consola::reportarError(msg);
        return -1;
    }

    return ExtManager::chgrp(*params->usr, *params->grp);
}

//Verifica si params contiene una definicion NO NULL para todos los parametros listados en flags
//Retorna el mensaje de error en caso que un parametro este mal definido
std::string Interprete::nullDefines(ExtParams* params, int paramFlag)
{
    std::string returnString;

    if(paramFlag & pf::R)
    {
        //Bool nunca es null
    }
    if(paramFlag & pf::P)
    {
        //Bool nunca es null
    }
    if(paramFlag & pf::RF)
    {
        //Bool nunca es null
    }
    if(paramFlag & pf::UNIT)
    {
        if(params->unit != '\0'){
            returnString += "\n    unit no es un parametro valido";
        }
    }
    if(paramFlag & pf::FIT_TYPE)
    {
        if(params->fitType != '\0'){
            returnString +=  "\n    fit no es un parametro valido";
        }
    }
    if(paramFlag & pf::DELETE_TYPE)
    {
        if(params->deleteType != '\0'){
            returnString +=  "\n    delete no es un parametro valido";
        }
    }
    if(paramFlag & pf::TYPE)
    {
        if(params->type != '\0'){
            returnString +=  "\n    type no es un parametro valido";
        }
    }
    if(paramFlag & pf::SIZE)
    {
        if(params->size > -1){
            returnString += "\n    size no es un parametro valido";
        }
    }
    if(paramFlag & pf::UGO)
    {
        if(params->ugo > -1){
            returnString += "\n    ugo no es un parametro valido";
        }
    }
    if(paramFlag & pf::ADD)
    {
        if(params->add > INT_MIN){
            returnString += "\n    ugo no es un parametro valido";
        }
    }
    if(paramFlag & pf::ID)
    {
        if(params->id){
            returnString += "\n    id no es un parametro valido";
        }
    }
    if(paramFlag & pf::FILE)
    {
        if(params->file){
            returnString += "\n    filen no es un parametro valido";
        }
    }
    if(paramFlag & pf::PATH)
    {
        if(params->path){
            returnString += "\n    path no es un parametro valido";
        }
    }
    if(paramFlag & pf::NAME)
    {
        if(params->name){
            returnString += "\n    name no es un parametro valido";
        }
    }
    if(paramFlag & pf::USR)
    {
        if(params->usr){
            returnString += "\n    usr no es un parametro valido";
        }
    }
    if(paramFlag & pf::PWD)
    {
        if(params->pwd){
            returnString += "\n    pwd no es un parametro valido";
        }
    }
    if(paramFlag & pf::GRP)
    {
        if(params->grp){
            returnString += "\n    grp no es un parametro valido";
        }
    }
    if(paramFlag & pf::CONT)
    {
        if(params->cont){
            returnString += "\n    cont no es un parametro valido";
        }
    }
    if(paramFlag & pf::RUTA)
    {
        if(params->ruta){
            returnString += "\n    ruta no es un parametro valido";
        }
    }
    if(paramFlag & pf::DEST)
    {
        if(params->dest){
            returnString += "\n    dest no es un parametro valido";
        }
    }

    return returnString;
}

std::string Interprete::defines(ExtParams* params, int paramFlag)
{
    std::string returnString;

    if(paramFlag & pf::R)
    {
        //Bool nunca es null
    }
    if(paramFlag & pf::P)
    {
        //Bool nunca es null
    }
    if(paramFlag & pf::RF)
    {
        //Bool nunca es null
    }
    if(paramFlag & pf::UNIT)
    {
        if(params->unit == '\0'){
            returnString += "\n    unit no definido";
        }
    }
    if(paramFlag & pf::FIT_TYPE)
    {
        if(params->fitType == '\0'){
            returnString +=  "\n    fit no definido";
        }
    }
    if(paramFlag & pf::DELETE_TYPE)
    {
        if(params->deleteType == '\0'){
            returnString +=  "\n    delete no definido";
        }
    }
    if(paramFlag & pf::TYPE)
    {
        if(params->type == '\0'){
            returnString +=  "\n    type no definido";
        }
    }
    if(paramFlag & pf::SIZE)
    {
        if(params->size < 0){
            returnString += "\n    size no definido";
        }
        else if(params->size == 0){
            returnString += "\n    size debe ser mayor a 0";
        }
    }
    if(paramFlag & pf::UGO)
    {
        if(params->ugo < 0){
            returnString += "\n    ugo no definido";
        }
    }
    if(paramFlag & pf::ADD)
    {
        if(params->add == INT_MIN){
            returnString += "\n    add no definido";
        }
    }
    if(paramFlag & pf::ID)
    {
        if(!params->id){
            returnString += "\n    id no definido";
        }
    }
    if(paramFlag & pf::FILE)
    {
        if(!params->file){
            returnString += "\n    filen no definido";
        }
    }
    if(paramFlag & pf::PATH)
    {
        if(!params->path){
            returnString += "\n    path no definido";
        }
    }
    if(paramFlag & pf::NAME)
    {
        if(!params->name){
            returnString += "\n    name no definido";
        }
    }
    if(paramFlag & pf::USR)
    {
        if(!params->usr){
            returnString += "\n    usr no definido";
        }
    }
    if(paramFlag & pf::PWD)
    {
        if(!params->pwd){
            returnString += "\n    pwd no definido";
        }
    }
    if(paramFlag & pf::GRP)
    {
        if(!params->grp){
            returnString += "\n    grp no definido";
        }
    }
    if(paramFlag & pf::CONT)
    {
        if(!params->cont){
            returnString += "\n    cont no definido";
        }
    }
    if(paramFlag & pf::RUTA)
    {
        if(!params->ruta){
            returnString += "\n    ruta no definido";
        }
    }
    if(paramFlag & pf::DEST)
    {
        if(!params->dest){
            returnString += "\n    dest no definido";
        }
    }

    return returnString;
}

std::string Interprete::exclusiveDefines(ExtParams* params, int requiredParms, int optionalParams)
{
    int nullParams = ~(optionalParams | requiredParms);
    std::string returnString = defines(params, requiredParms) + nullDefines(params, nullParams);
    return returnString;
}
