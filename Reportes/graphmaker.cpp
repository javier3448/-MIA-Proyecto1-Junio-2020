#include "graphmaker.h"

#include "Disk/ebr.h"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <vector>
#include <algorithm>
#include "diskentity.h"
#include "consola.h"
#include "mystringutil.h"
#include "FileSystem/pointerblock.h"
#include "FileSystem/foldercontent.h"
#include "FileSystem/folderblock.h"
#include "Reportes/DotObjects/dotfileblock.h"
#include "Reportes/DotObjects/dotfolderblock.h"
#include "Reportes/DotObjects/dotinode.h"
#include "Reportes/DotObjects/dotpointerblock.h"


int GraphMaker::mkDotMbr(std::string &outDotCode, const Mbr& mbr)
{
    outDotCode += "digraph Mbr{\n node[shape = none];\n node[margin = 0];\n";

    using std::to_string;
    using namespace MyStringUtil;

    std::string size = to_string(mbr.size);
    std::string date = MyStringUtil::dateToString(mbr.fechaCreacion);
    std::string signature = to_string(mbr.diskSignature);
    std::string part0_status = charToString(mbr.primaryPartition[0].status);
    std::string part0_type= charToString(mbr.primaryPartition[0].type);
    std::string part0_fit= charToString(mbr.primaryPartition[0].fit);
    std::string part0_start = to_string(mbr.primaryPartition[0].start);
    std::string part0_size = to_string(mbr.primaryPartition[0].size);
    std::string part0_name(mbr.primaryPartition[0].name);
    std::string part1_status= charToString(mbr.primaryPartition[1].status);
    std::string part1_type= charToString(mbr.primaryPartition[1].type);
    std::string part1_fit= charToString(mbr.primaryPartition[1].fit);
    std::string part1_start = to_string(mbr.primaryPartition[1].start);
    std::string part1_size = to_string(mbr.primaryPartition[1].size);
    std::string part1_name(mbr.primaryPartition[1].name);
    std::string part2_status= charToString(mbr.primaryPartition[2].status);
    std::string part2_type= charToString(mbr.primaryPartition[2].type);
    std::string part2_fit= charToString(mbr.primaryPartition[2].fit);
    std::string part2_start = to_string(mbr.primaryPartition[2].start);
    std::string part2_size = to_string(mbr.primaryPartition[2].size);
    std::string part2_name(mbr.primaryPartition[2].name);
    std::string part3_status= charToString(mbr.extendedPartition.status);
    std::string part3_type= charToString(mbr.extendedPartition.type);
    std::string part3_fit= charToString(mbr.extendedPartition.fit);
    std::string part3_start = to_string(mbr.extendedPartition.start);
    std::string part3_size = to_string(mbr.extendedPartition.size);
    std::string part3_name(mbr.extendedPartition.name);

    outDotCode += "MBR_TABLA[ label = \n<"
                  "<table cellspacing=\"0\" cellborder=\"2\">\n"
                  "  <tr>\n"
                  "    <td colspan = \"2\"><b>MBR</b></td>\n"
                  "  </tr>\n"
                  "  <tr>\n"
                  "    <td>size</td>\n"
                  "    <td>" + size + "</td>\n"
                           "  </tr>\n"
                           "  <tr>\n"
                           "    <td>date</td>\n"
                           "    <td>" + date + "</td>\n"
                           "  </tr>\n"
                           "  <tr>\n"
                           "    <td>signature</td>\n"
                           "    <td>" + signature + "</td>\n"
                                "  </tr>\n"
                                "  <tr>\n"
                                "    <td>part0_status</td>\n"
                                "    <td>" + part0_status + "</td>\n"
                                   "  </tr>\n"
                                   "  <tr>\n"
                                   "    <td>part0_type</td>\n"
                                   "    <td>" + part0_type + "</td>\n"
                                 "  </tr>\n"
                                 "  <tr>\n"
                                 "    <td>part0_fit</td>\n"
                                 "    <td>" + part0_fit + "</td>\n"
                                "  </tr>\n"
                                "  <tr>\n"
                                "    <td>part0_start</td>\n"
                                "    <td>" + part0_start + "</td>\n"
                                  "  </tr>\n"
                                  "  <tr>\n"
                                  "    <td>part0_size</td>\n"
                                  "    <td>" + part0_size + "</td>\n"
                                 "  </tr>\n"
                                 "  <tr>\n"
                                 "    <td>part0_name</td>\n"
                                 "    <td>" + part0_name + "</td>\n"
                                 "  </tr>\n"
                                 "  <tr>\n"
                                 "    <td>part1_status</td>\n"
                                 "    <td>" + part1_status + "</td>\n"
                                   "  </tr>\n"
                                   "  <tr>\n"
                                   "    <td>part1_type</td>\n"
                                   "    <td>" + part1_type + "</td>\n"
                                 "  </tr>\n"
                                 "  <tr>\n"
                                 "    <td>part1_fit</td>\n"
                                 "    <td>" + part1_fit + "</td>\n"
                                "  </tr>\n"
                                "  <tr>\n"
                                "    <td>part1_start</td>\n"
                                "    <td>" + part1_start + "</td>\n"
                                  "  </tr>\n"
                                  "  <tr>\n"
                                  "    <td>part1_size</td>\n"
                                  "    <td>" + part1_size + "</td>\n"
                                 "  </tr>\n"
                                 "  <tr>\n"
                                 "    <td>part1_name</td>\n"
                                 "    <td>" + part1_name + "</td>\n"
                                 "  </tr>\n"
                                 "  <tr>\n"
                                 "    <td>part2_status</td>\n"
                                 "    <td>" + part2_status + "</td>\n"
                                   "  </tr>\n"
                                   "  <tr>\n"
                                   "    <td>part2_type</td>\n"
                                   "    <td>" + part2_type + "</td>\n"
                                 "  </tr>\n"
                                 "  <tr>\n"
                                 "    <td>part2_fit</td>\n"
                                 "    <td>" + part2_fit + "</td>\n"
                                "  </tr>\n"
                                "  <tr>\n"
                                "    <td>part2_start</td>\n"
                                "    <td>" + part2_start + "</td>\n"
                                  "  </tr>\n"
                                  "  <tr>\n"
                                  "    <td>part2_size</td>\n"
                                  "    <td>" + part2_size + "</td>\n"
                                 "  </tr>\n"
                                 "  <tr>\n"
                                 "    <td>part2_name</td>\n"
                                 "    <td>" + part2_name + "</td>\n"
                                 "  </tr>\n"
                                 "  <tr>\n"
                                 "    <td>part3_status</td>\n"
                                 "    <td>" + part3_status + "</td>\n"
                                   "  </tr>\n"
                                   "  <tr>\n"
                                   "    <td>part3_type</td>\n"
                                   "    <td>" + part3_type + "</td>\n"
                                 "  </tr>\n"
                                 "  <tr>\n"
                                 "    <td>part3_fit</td>\n"
                                 "    <td>" + part3_fit + "</td>\n"
                                "  </tr>\n"
                                "  <tr>\n"
                                "    <td>part3_start</td>\n"
                                "    <td>" + part3_start + "</td>\n"
                                  "  </tr>\n"
                                  "  <tr>\n"
                                  "    <td>part3_size</td>\n"
                                  "    <td>" + part3_size + "</td>\n"
                                 "  </tr>\n"
                                 "  <tr>\n"
                                 "    <td>part3_name</td>\n"
                                 "    <td>" + part3_name + "</td>\n"
                                 "  </tr>\n"
                                 "</table>\n"
                                 ">\n]"
        ;

    outDotCode += "}\n";
    return 0;
}

int GraphMaker::mkDotDisk(std::string &outDotCode, RaidOneFile* file, Mbr* mbr)
{
    using std::to_string;
    using namespace MyStringUtil;

    float disksize = mbr->size;
    outDotCode += "digraph Disk{\n"
                  "node[shape = none];\n"
                  "node[margin = 0];\n"
                  "DISK[ label = <\n"
                  "<table cellspacing=\"0\" cellborder=\"2\">\n"
                  "  <tr>\n"
                  "    <td width=\"40\" height=\"150\" bgcolor=\"yellow\">MBR</td>\n";

    //Hacer lista de particiones
    std::vector<Partition*> partitions;
    partitions.reserve(4);
    for (int i = 0; i < PART_COUNT; ++i) {
        auto& currPart = mbr->primaryPartition[i];
        if(!currPart.isNull()){
            partitions.push_back(&currPart);
        }
    }
    std::sort(partitions.begin(), partitions.end(), [](Partition* a, Partition* b){  return a->start < b->start;  });

    //Caso especial, si no se tiene ninguna particion
    if (partitions.size() < 1) {
        int freeSize = mbr->size - sizeof(Mbr);
        outDotCode += "    <td height=\"150\" width = \"400\" bgcolor=\"gray\">Libre<br/>\n" + to_string(freeSize) + " bytes<br/>"
                            + floatToString(freeSize/disksize * 100) + "%</td>\n";
    }
    else{
        if(partitions[0]->start != sizeof(Mbr)){
            int freeSize = partitions[0]->start - sizeof(Mbr);
            outDotCode += "    <td height=\"150\" width = \"100\" bgcolor=\"gray\">Libre<br/>" + to_string(freeSize) + " bytes<br/>"
                            + floatToString(freeSize/disksize * 100) + "%</td>\n";
        }
        for (ulong i = 0; i < partitions.size(); i++) {
            auto& currPart = partitions[i];
            if (partitions[i]->type == 'p') {
                outDotCode += "    <td height=\"150\" width = \"100\" bgcolor=\"#388E8E\">Primaria<br/>" + std::string(currPart->name) + "<br/>" + to_string(partitions[i]->size) + " bytes<br/>"
                              + floatToString(currPart->size/disksize * 100) + "%</td>\n";
            }else{
                outDotCode += "    <td height=\"150\" width = \"100\">\n";
                writeTableExtendedPart(outDotCode, file, partitions[i], disksize);
                outDotCode += "    </td>\n";
            }
            if (i != partitions.size() - 1
                && partitions[i + 1]->start != currPart->start + currPart->size) {
                int freeSize = partitions[i + 1]->start - (currPart->start + currPart->size);
                outDotCode += "    <td height=\"150\" width = \"100\" bgcolor=\"gray\">Libre<br/>" + to_string(freeSize) + " bytes<br/>"
                            + floatToString(freeSize/disksize * 100) + "%</td>\n";
            }
        }
        auto& lastPart = partitions.back();
        if(mbr->size != (lastPart->start + lastPart->size)){
            int freeSize = mbr->size - (lastPart->start + lastPart->size);
            outDotCode += "    <td height=\"150\" width = \"100\" bgcolor=\"gray\">Libre<br/>" + to_string(freeSize) + " bytes<br/>"
                            + floatToString(freeSize/disksize * 100) + "%</td>\n";        }
    }

    outDotCode += "  </tr>\n"
                  "</table>\n"
                  ">\n"
                  "]}\n";

    return 0;
}

//suffix: lo que se le agregara al final de cada nombre de atributo en la tabla ej para suffix
int GraphMaker::mkDotEbr(std::string &outDotCode, DiskEntity<Ebr> *ebrEntity, const std::string& suffix)
{
    outDotCode += "digraph Ebr{\n node[shape = none];\n node[margin = 0];\n";

    using std::to_string;
    using namespace MyStringUtil;

    Ebr& ebr = ebrEntity->value;

    std::string status = to_string(ebr.status);
    std::string fit = charToString(ebr.fit);
    std::string start = to_string(ebr.start);
    std::string size = to_string(ebr.size);
    std::string next = to_string(ebr.next);
    std::string name(ebr.name);

    outDotCode += "EBR_TABLA[ label = \n<"
                  "<table cellspacing=\"0\" cellborder=\"2\">\n"
                  "  <tr>\n"
                  "    <td colspan = \"2\"><b>EBR" + suffix + "</b></td>\n"
                             "  </tr>\n"
                             "  <tr>\n"
                             "    <td>status" + suffix + "</td>\n"
                             "    <td>" + status + "</td>\n"
                             "  </tr>\n"
                             "  <tr>\n"
                             "    <td>fit" + suffix + "</td>\n"
                             "    <td>" + fit + "</td>\n"
                          "  </tr>\n"
                          "  <tr>\n"
                          "    <td>start" + suffix + "</td>\n"
                             "    <td>" + start + "</td>\n"
                            "  </tr>\n"
                            "  <tr>\n"
                            "    <td>size" + suffix + "</td>\n"
                             "    <td>" + size + "</td>\n"
                           "  </tr>\n"
                           "  <tr>\n"
                           "    <td>next" + suffix + "</td>\n"
                             "    <td>" + next + "</td>\n"
                           "  </tr>\n"
                           "  <tr>\n"
                           "    <td>name" + suffix + "</td>\n"
                             "    <td>" + name + "</td>\n"
                           "  </tr>\n"
                           "</table>\n"
                           ">\n]"
        ;

    outDotCode += "}\n";
    return 0;
}

int GraphMaker::mkDotSb(std::string &outDotCode, DiskEntity<SuperBoot> *sbEntity)
{
    auto& sb = sbEntity->value;

    using std::to_string;
    using namespace MyStringUtil;

    std::string filesystemType = to_string(sb.filesystemType);
    std::string inodesCount = to_string(sb.inodesCount);
    std::string blocksCount = to_string(sb.blocksCount);
    std::string freeInodesCount = to_string(sb.freeInodesCount);
    std::string freeBlocksCount = to_string(sb.freeBlocksCount);
    std::string lastmountTime = dateToString(sb.lastmountTime);
    std::string lastUnmountTime = dateToString(sb.lastUnmountTime);
    std::string mntCount = to_string(sb.mntCount);
    std::string magic = to_string(sb.magic);
    std::string inodeSize = to_string(sb.inodeSize);
    std::string blockSize = to_string(sb.blockSize);
    std::string firstFreeInode = to_string(sb.firstFreeInode);
    std::string firstFreeBlock = to_string(sb.firstFreeBlock);
    std::string bitmapInodeBegin = to_string(sb.bitmapInodeBegin);
    std::string bitmapBlockBegin = to_string(sb.bitmapBlockBegin);
    std::string inodeStart = to_string(sb.inodeStart);
    std::string blockStart = to_string(sb.blockStart);

    outDotCode += "digraph SuperBoot{"
                  "node[shape = none];"
                  "node[margin = 0];"
                  "SUPERBOOT_TABLA[ label = "
                  "<<table cellspacing=\"0\" cellborder=\"2\" cellpadding=\"2\">"
                  "    <tr>"
                  "        <td colspan = \"2\"><b>SUPERBOOT</b></td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>filesystemType:</td>"
                  "        <td>2</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>inodesCount:</td>"
                  "        <td>32066</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>blocksCount:</td>"
                  "        <td>96198</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>freeInodesCount:</td>"
                  "        <td>32058</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>freeBlocksCount:</td>"
                  "        <td>95829</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>lastmountTime:</td>"
                  "        <td>26-03-2020 15:04:54</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>lastUnmountTime:</td>"
                  "        <td>26-03-2020 15:04:54</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>mntCount:</td>"
                  "        <td>0</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>magic:</td>"
                  "        <td>61267</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>inodeSize:</td>"
                  "        <td>256</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>blockSize:</td>"
                  "        <td>64</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>firstFreeInode:</td>"
                  "        <td>370</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>firstFreeBlock:</td>"
                  "        <td>32796</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>bitmapInodeBegin:</td>"
                  "        <td>360</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>bitmapBlockBegin:</td>"
                  "        <td>32426</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>inodeStart:</td>"
                  "        <td>128624</td>"
                  "    </tr>"
                  "    <tr>"
                  "        <td>blockStart:</td>"
                  "        <td>8337520</td>"
                  "    </tr>"
                  "</table>"
                  ">"
                  "]}";

    return 0;
}

int GraphMaker::mkDotLs(std::string &outDotCode, RaidOneFile *file, DiskEntity<Inode> *fileEntity, const UsersData& usrsData)
{
    outDotCode = "digraph Ls{"
                 " node[shape = none];"
                 " node[margin = 0];"
                 "LS_TABLE[ label = "
                 "<<table cellspacing=\"0\" cellborder=\"1\">"
                 "  <tr>"
                 "    <td><b> PERMISOS </b></td>"
                 "    <td><b> OWNER </b></td>"
                 "    <td><b> GRUPO </b></td>"
                 "    <td><b> SIZE </b></td>"
                 "    <td><b> MODIFICACION </b></td>"
                 "    <td><b> CREACION </b></td>"
                 "    <td><b> TIPO </b></td>"
                 "    <td><b> NAME </b></td>"
                 "  </tr>";

    bool keepVisiting = true;

    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE && keepVisiting; i++) {
        if(fileEntity->value.blocks[i] == -1){
            keepVisiting = false;
            break;
        }
        keepVisiting = mkDotLsImp(outDotCode, file, 0, fileEntity->value.blocks[i], usrsData);
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE && keepVisiting; i++){
        if(fileEntity->value.blocks[i] == -1){
            keepVisiting = false;
            break;
        }
        keepVisiting = mkDotLsImp(outDotCode, file, 1, fileEntity->value.blocks[i], usrsData);
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE && keepVisiting; i++){
        if(fileEntity->value.blocks[i] == -1){
            keepVisiting = false;
            break;
        }
        keepVisiting = mkDotLsImp(outDotCode, file, 2, fileEntity->value.blocks[i], usrsData);
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE && keepVisiting; i++){
        if(fileEntity->value.blocks[i] == -1){
            keepVisiting = false;
            break;
        }
        keepVisiting = mkDotLsImp(outDotCode, file, 3, fileEntity->value.blocks[i], usrsData);
    }//Nos podriamos ahorar un monton de lineas si lo metemos todo de un for que vaya de 0 a Indoe ::blks_size. Pero siento que asi se lee mejor. No se

    outDotCode += "</table>"
                  ">"
                  "]}";
    return 0;
}

int GraphMaker::mkDotTree(std::string &outDotCode, RaidOneFile *file, DiskEntity<Inode> *rootEntity)
{
    outDotCode = "digraph tree{\n"
                 "node[shape = none];\n";

    mkDotTreeInode(outDotCode, file, rootEntity->address);

    outDotCode += "}";

    return 0;
}

int GraphMaker::mkDotBlock(std::string &outDotCode, RaidOneFile *file, DiskEntity<Inode> *rootEntity)
{
    outDotCode = "digraph tree{\n"
                 "node[shape = none];\n";

    mkDotBlockInode(outDotCode, file, rootEntity->address);

    outDotCode += "}";

    return 0;
}

int GraphMaker::mkDotInode(std::string &outDotCode, RaidOneFile *file, DiskEntity<Inode> *rootEntity)
{
    outDotCode = "digraph tree{\n"
                 "node[shape = none];\n";

    mkDotInodeInode(outDotCode, file, rootEntity->address);

    outDotCode += "}";

    return 0;
}

//Retorna verdadero si todavia hay inodos que obtener
bool GraphMaker::mkDotLsImp(std::string &outDotCode, RaidOneFile *file, char depth, int address, const UsersData& usrsData)
{
    if(depth == 0){
        DiskEntity<FolderBlock> folderBlock(address, file);
        auto& folderContents = folderBlock.value.contents;
        for (int i = 0; i < FolderBlock::CONTENTS_SIZE; i++) {
            if(folderContents[i].isNull()){
                continue;
            }
            DiskEntity<Inode> inode(folderContents[i].inodePtr, file);
            outDotCode += mkInodeTr(&inode, folderContents[i].name, usrsData);
        }
    }
    else {//Significa que depth es 1 o mas
        DiskEntity<PointerBlock> pointerBlock(address, file);
        auto& pointers = pointerBlock.value.pointers;
        for (int i = 0; i < PointerBlock::POINTERS_SIZE; i++) {
            if(pointers[i] == -1){
                return false;
            }
            bool keepVisiting = mkDotLsImp(outDotCode, file, depth - 1, pointers[i], usrsData);
            if(!keepVisiting){
                return keepVisiting;
            }
        }
    }
    return true;
}

std::string GraphMaker::mkInodeTr(DiskEntity<Inode>* inodeEntity, const std::string &name, const UsersData& usrsData)
{
    using std::to_string;
    using namespace MyStringUtil;

    Inode& inode = inodeEntity->value;

    std::string permissions = intToPermissionsHtmlString(inode.permissions);
    std::string usrId = usrsData.getUsrName(inode.usrId);
    std::string grpId = usrsData.getGroupName(inode.grpId);
    std::string size = to_string(inode.size);
    std::string creationTime = dateToString(inode.creationTime);
    std::string modificationTime = dateToString(inode.modificationTime);
    std::string type = intToInodeTypeHtmlString(inode.type);

    std::string st = "  <tr>"
                     "    <td><b> " + permissions + " </b></td>\n"
                                     "    <td><b> " + usrId + " </b></td>\n"
                               "    <td><b> " + grpId + " </b></td>\n"
                               "    <td><b> " + size + " </b></td>\n"
                              "    <td><b> " + creationTime + " </b></td>\n"
                                      "    <td><b> " + modificationTime + " </b></td>\n"
                                          "    <td><b> " + type + " </b></td>\n"
                              "    <td><b> " + name + " </b></td>\n"
                              "  </tr>";

    return st;
}

std::string GraphMaker::intToPermissionsHtmlString(int permissions)
{
    std::string st;
    int aux;
    for (int i = 0; i < 3; i++) {
        aux = permissions & (0xf << ((2 - i) * 4));
        if(aux & 0b100){
            st += "r";
        }
        else{
            st += "-";
        }
        if(aux & 0b010){
            st += "w";
        }
        else{
            st += "-";
        }
        if(aux & 0b001){
            st += "x";
        }
        else{
            st += "-";
        }
        if(i != 2){//Chapuz super feo pero no afecta mucho
            st += "<br/>";
        }
    }
    return st;
}

std::string GraphMaker::intToInodeTypeHtmlString(int type)
{
    if(type == Inode::TYPE_FILE){
        return "file";
    }
    else{
        return "folder";
    }
}

void GraphMaker::mkDotTreeInode(std::string &outDotCode, RaidOneFile *file, int inodeAdress)
{
    using std::to_string;

    DotInode dotInode(inodeAdress, file);
    int type = dotInode.getType();

    outDotCode += dotInode.getDotNodeDeclaration();

    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        int blockAddress = dotInode.getBlockPtr(i);
        if(blockAddress == -1){
            continue;
        }
        outDotCode += dotInode.getBlockPort(i) + "->" + "b" + to_string(blockAddress) + ";\n";

        mkDotTreeBlock(outDotCode, file, type, 0, blockAddress);
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        int blockAddress = dotInode.getBlockPtr(i);
        if(blockAddress == -1){
            return;
        }
        outDotCode += dotInode.getBlockPort(i) + "->" + "b" + to_string(blockAddress) + ";\n";

        mkDotTreeBlock(outDotCode, file, type, 1, blockAddress);
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        int blockAddress = dotInode.getBlockPtr(i);
        if(blockAddress == -1){
            return;
        }
        outDotCode += dotInode.getBlockPort(i) + "->" + "b" + to_string(blockAddress) + ";\n";
        mkDotTreeBlock(outDotCode, file, type, 2, blockAddress);
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        int blockAddress = dotInode.getBlockPtr(i);
        if(blockAddress == -1){
            return;
        }
        outDotCode += dotInode.getBlockPort(i) + "->" + "b" + to_string(blockAddress) + ";\n";

        mkDotTreeBlock(outDotCode, file, type, 3, blockAddress);
    }
}

void GraphMaker::mkDotTreeBlock(std::string &outDotCode, RaidOneFile *file, int type, char depth, int blockAdress)
{
    if(depth == 0){
        if(type == Inode::TYPE_FILE){
            DotFileBlock dotFileBlock(blockAdress, file);
            outDotCode += dotFileBlock.getDotNodeDeclaration();
        }
        else{
            DotFolderBlock dotFolderBlock(blockAdress, file);
            outDotCode += dotFolderBlock.getDotNodeDeclaration();
            for(int i = 0; i < FolderBlock::CONTENTS_SIZE; i++){
                int inodeAddress = dotFolderBlock.getInodePtr(i);
                if(dotFolderBlock.getInodePtr(i) == -1){
                    continue;
                }
                outDotCode += dotFolderBlock.getInodePort(i) + "->" + "i"  + std::to_string(inodeAddress) + ";\n";

                mkDotTreeInode(outDotCode, file, inodeAddress);
            }
        }
    }
    else{
        DotPointerBlock dotPointerBlock(blockAdress, file);
        outDotCode += dotPointerBlock.getDotNodeDeclaration();
        for(int i = 0; i < PointerBlock::POINTERS_SIZE; i++){
            int subBlockAddress = dotPointerBlock.getBlockPtr(i);
            if(subBlockAddress == -1){
                return;
            }
            outDotCode += dotPointerBlock.getBlockPort(i) + "->" + "b" + std::to_string(subBlockAddress) + ";\n";

            mkDotTreeBlock(outDotCode, file, type, depth - 1, subBlockAddress);
        }
    }
}

void GraphMaker::mkDotBlockInode(std::string &outDotCode, RaidOneFile *file, int inodeAdress)
{
    using std::to_string;
    DotInode dotInode(inodeAdress, file);
    int type = dotInode.getType();

    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        int blockAddress = dotInode.getBlockPtr(i);
        if(blockAddress == -1){
            continue;
        }
        mkDotBlockBlock(outDotCode, file, type, 0, blockAddress);
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        int blockAddress = dotInode.getBlockPtr(i);
        if(blockAddress == -1){
            return;
        }

        mkDotBlockBlock(outDotCode, file, type, 1, blockAddress);
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        int blockAddress = dotInode.getBlockPtr(i);
        if(blockAddress == -1){
            return;
        }
        mkDotBlockBlock(outDotCode, file, type, 2, blockAddress);
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        int blockAddress = dotInode.getBlockPtr(i);
        if(blockAddress == -1){
            return;
        }
        mkDotBlockBlock(outDotCode, file, type, 3, blockAddress);
    }
}

void GraphMaker::mkDotBlockBlock(std::string &outDotCode, RaidOneFile *file, int type, char depth, int blockAdress)
{
    if(depth == 0){
        if(type == Inode::TYPE_FILE){
            DotFileBlock dotFileBlock(blockAdress, file);
            outDotCode += dotFileBlock.getDotNodeDeclaration();
        }
        else{
            DotFolderBlock dotFolderBlock(blockAdress, file);
            outDotCode += dotFolderBlock.getDotNodeDeclaration();
            for(int i = 0; i < FolderBlock::CONTENTS_SIZE; i++){
                int inodeAddress = dotFolderBlock.getInodePtr(i);
                if(dotFolderBlock.getInodePtr(i) == -1){
                    continue;
                }

                mkDotBlockInode(outDotCode, file, inodeAddress);
            }
        }
    }
    else{
        DotPointerBlock dotPointerBlock(blockAdress, file);
        outDotCode += dotPointerBlock.getDotNodeDeclaration();
        for(int i = 0; i < PointerBlock::POINTERS_SIZE; i++){
            int subBlockAddress = dotPointerBlock.getBlockPtr(i);
            if(subBlockAddress == -1){
                return;
            }

            outDotCode += dotPointerBlock.getBlockPort(i) + "->" + "b" + std::to_string(subBlockAddress) + ";\n";

            mkDotBlockBlock(outDotCode, file, type, depth - 1, subBlockAddress);
        }
    }
}

void GraphMaker::mkDotInodeInode(std::string &outDotCode, RaidOneFile *file, int inodeAdress)
{
    using std::to_string;

    DotInode dotInode(inodeAdress, file);
    int type = dotInode.getType();

    outDotCode += dotInode.getDotNodeDeclaration();

    int i = 0;
    for (i = 0; i < Inode::DIRECT_BLK_BEG + Inode::DIRECT_BLK_SIZE; i++) {
        int blockAddress = dotInode.getBlockPtr(i);
        if(blockAddress == -1){
            continue;
        }
        mkDotInodeBlock(outDotCode, file, type, 0, blockAddress);
    }
    for(; i < Inode::SIMPLE_INDIRECT_BLK_BEG + Inode::SIMPLE_INDIRECT_BLK_SIZE; i++){
        int blockAddress = dotInode.getBlockPtr(i);
        if(blockAddress == -1){
            return;
        }
        mkDotInodeBlock(outDotCode, file, type, 1, blockAddress);
    }
    for(; i < Inode::DOUBLE_INDIRECT_BLK_BEG + Inode::DOUBLE_INDIRECT_BLK_SIZE; i++){
        int blockAddress = dotInode.getBlockPtr(i);
        if(blockAddress == -1){
            return;
        }
        mkDotInodeBlock(outDotCode, file, type, 2, blockAddress);
    }
    for(; i < Inode::TRIPLE_INDIRECT_BLK_BEG + Inode::TRIPLE_INDIRECT_BLK_SIZE; i++){
        int blockAddress = dotInode.getBlockPtr(i);
        if(blockAddress == -1){
            return;
        }
        mkDotInodeBlock(outDotCode, file, type, 3, blockAddress);
    }
}

void GraphMaker::mkDotInodeBlock(std::string &outDotCode, RaidOneFile *file, int type, char depth, int blockAdress)
{
    if(depth == 0){
        if(type == Inode::TYPE_FILE){
            DotFileBlock dotFileBlock(blockAdress, file);
        }
        else{
            DotFolderBlock dotFolderBlock(blockAdress, file);
            for(int i = 0; i < FolderBlock::CONTENTS_SIZE; i++){
                int inodeAddress = dotFolderBlock.getInodePtr(i);
                if(dotFolderBlock.getInodePtr(i) == -1){
                    continue;
                }
                mkDotInodeInode(outDotCode, file, inodeAddress);
            }
        }
    }
    else{
        DotPointerBlock dotPointerBlock(blockAdress, file);
        for(int i = 0; i < PointerBlock::POINTERS_SIZE; i++){
            int subBlockAddress = dotPointerBlock.getBlockPtr(i);
            if(subBlockAddress == -1){
                return;
            }
            mkDotInodeBlock(outDotCode, file, type, depth - 1, subBlockAddress);
        }
    }
}

void GraphMaker::splitDirPathAndFileName(const std::string &absolutePath, std::string &dirPath, std::string &fileName)
{
    dirPath = absolutePath;
    if (dirPath.size() > 0 && dirPath[0] == '/') {
        dirPath = dirPath.substr(1);//Quitamos el primer /, ya que, ya conseguimos el avd del root
    }
    auto lastIndexOfSlash = dirPath.find_last_of("/");
    if (lastIndexOfSlash == std::string::npos) {
        fileName = dirPath;
        dirPath = "";
    }
    else{
        fileName = dirPath.substr(lastIndexOfSlash + 1);
        dirPath = dirPath.substr(0, lastIndexOfSlash);
    }
}

void GraphMaker::splitSubDirNameAndRemainingPath(const std::string& absolutePath, std::string& subDirName, std::string& remainingPath)
{
    auto lastIndexOfSlash = absolutePath.find_first_of("/");
    if (lastIndexOfSlash == std::string::npos) {
        remainingPath = "";
        subDirName= absolutePath;
    }
    else{
        remainingPath = absolutePath.substr(lastIndexOfSlash + 1);
        subDirName = absolutePath.substr(0, lastIndexOfSlash);
    }
}

void GraphMaker::writeTableExtendedPart(std::string &outDotCode, RaidOneFile* file, Partition *extendedPart, float disksize)
{
    using std::to_string;
    using namespace MyStringUtil;

    int colSpan = 1;

    std::string intermediate;

    std::vector<Ebr> ebrs;
    ebrs.reserve(16);

    //Meter todos los ebr en un vector para ordenarlos
    DiskEntity<Ebr> ebrIter(extendedPart->start, file);
    while (true) {
        ebrs.push_back(ebrIter.value);
        if(ebrIter.value.next == -1){
            break;
        }
        ebrIter = DiskEntity<Ebr>(ebrIter.value.next, file);
    }
    std::sort(ebrs.begin(), ebrs.end(), [](Ebr a, Ebr b){  return a.start < b.start;  });

    //caso si especial, si no tenemos ninguna particion logica
    if (ebrs.size() < 2) {
        colSpan++;
        int freeSize = extendedPart->start + extendedPart->size - sizeof(Ebr);
        intermediate += "                <td height=\"150\" width = \"200\" bgcolor=\"gray\">Libre<br/>\n"
                        + to_string(freeSize) + " bytes<br/>\n"
                        + floatToString(freeSize/disksize * 100) + "%</td>\n";
    }
    else{
        //caso especial, si hay un espacio libre ebre $TopEbr y la primera particion logica
        if(ebrs[1].start != (sizeof(Ebr) + extendedPart->start)){
            colSpan++;
            int freeSize = ebrs[1].start - ((int)sizeof(Ebr) + extendedPart->start);
            intermediate += "                <td height=\"150\" width = \"100\" bgcolor=\"gray\">Libre<br/>"
                            + to_string(freeSize) + " bytes<br/>\n"
                            + floatToString(freeSize/disksize * 100) + "%</td>\n";
        }
        for (ulong i = 1; i < ebrs.size(); i++) {
            auto currEbr = ebrs[i];
            colSpan += 2;
            intermediate += "                <td height=\"150\" width=\"40\" bgcolor=\"orange\">EBR</td>\n"
                            "                <td height=\"150\" width = \"100\" bgcolor=\"brown\">Logica<br/>"
                            + std::string(currEbr.name) + "<br/>" + to_string(currEbr.size) + " bytes<br/>\n"
                            + floatToString(currEbr.size/disksize * 100) + "%</td>\n";

            //Si hay espacio disponible entre particiones logicas
            if (i != ebrs.size() - 1
                && ebrs[i + 1].start != (currEbr.start + currEbr.size)) {
                colSpan++;
                int freeSize = ebrs[i + 1].start - (currEbr.start + currEbr.size);
                intermediate += "                <td height=\"150\" width = \"100\" bgcolor=\"gray\">Libre<br/>"
                                + to_string(freeSize) + " bytes<br/>\n"
                                + floatToString(freeSize/disksize * 100) + "%</td>\n";
            }
        }
        auto lastEbr = ebrs.back();

        //Caso especial si hay espacio disponible al final de la particion extendida
        if((extendedPart->size + extendedPart->start)!= (lastEbr.start + lastEbr.size)){
            colSpan++;
            int freeSize = (extendedPart->size + extendedPart->start) - (lastEbr.start + lastEbr.size);
            intermediate += "                <td height=\"150\" width = \"100\" bgcolor=\"gray\">Libre<br/>"
                            + to_string(freeSize) + " bytes<br/>\n"
                            + floatToString(freeSize/disksize * 100) + "%</td>\n";
        }
    }

    intermediate += "              </tr>\n"
                    "        </table>";

    //Incluye el primer ebr tambien
    //El princpio del string se agrega de ultimo porque tenemosq que saber cual va ser el colspan de Extendida

    intermediate =  "        <table cellspacing=\"0\" cellborder=\"1\">\n"
                   "              <tr>\n"
                   "                <td colspan = \"" + to_string(colSpan) + "\" bgcolor=\"red\">Extendida</td>\n"
                                          "              </tr>\n"
                                          "              <tr>\n"
                                          "                <td height=\"150\" width=\"40\" bgcolor=\"orange\">EBR</td>\n" +
                   intermediate;

    outDotCode += intermediate;
}
