#include "dotinode.h"

#include "mystringutil.h"

DotInode::DotInode(int address, RaidOneFile *file)
{
    inodeEntity = DiskEntity<Inode>(address, file);
    initDotStrings();
}

DotInode::DotInode(int address, Inode inode)
{
    inodeEntity = DiskEntity<Inode>(address, inode);
    initDotStrings();
}

void DotInode::initDotStrings()
{
    using std::to_string;
    auto& inode = inodeEntity.value;

    dotName = "i" + to_string(inodeEntity.address);

    std::string tipo = inode.getTypeString();
    std::string usuario = to_string(inode.usrId);
    std::string grupo = to_string(inode.grpId);
    std::string ugo = MyStringUtil::intToHexString(inode.permissions);
    std::string size = to_string(inode.size);

    dotNodeDeclaration = dotName + "[\n"
                                   "        label = <\n"
                                   "        <table border=\"0\" cellborder=\"1\" cellspacing=\"2\" cellpadding=\"3\" color=\"cyan4\">\n"
                                   "            <tr><td colspan=\"2\" bgcolor=\"white\" ><b>Inode</b></td></tr>\n"
                                   "                <tr>\n"
                                   "                    <td bgcolor = \"lightcyan\">Tipo</td>\n"
                                   "                    <td bgcolor = \"cyan2\">" + tipo + "</td>\n"
                                   "                </tr>\n"
                                   "                <tr>\n"
                                   "                    <td bgcolor = \"lightcyan\">Usuario</td>\n"
                                   "                    <td bgcolor = \"cyan2\">" + usuario + "</td>\n"
                                   "                </tr>\n"
                                   "                <tr>\n"
                                   "                    <td bgcolor = \"lightcyan\">Grupo</td>\n"
                                   "                    <td bgcolor = \"cyan2\">" + grupo + "</td>\n"
                                   "                </tr>\n"
                                   "                <tr>\n"
                                   "                    <td bgcolor = \"lightcyan\">UGO</td>\n"
                                   "                    <td bgcolor = \"cyan2\">" + ugo + "</td>\n"
                                   "                </tr>\n"
                                   "                <tr>\n"
                                   "                    <td bgcolor = \"lightcyan\">size</td>\n"
                                   "                    <td bgcolor = \"cyan2\">" + size + "</td>\n"
                                   "                </tr>\n";
    int i;
    for(i = 0; i < Inode::DIRECT_BLK_SIZE; i++){
        std::string portName = "b" + std::to_string(i);
        std::string ptrName = portName;
        blockPtrPorts[i] = dotName + ":" + portName;
        std::string ptrVal = to_string(inode.blocks[i]);
        dotNodeDeclaration += "                <tr>\n"
                              "                    <td bgcolor = \"lightcyan\">" + ptrName + "</td>\n"
                              "                    <td port=\"" + portName + "\" bgcolor = \"lightskyblue1\">" + ptrVal + "</td>\n"
                              "                </tr>\n";
    }
    for(i = 0; i < Inode::BLKS_SIZE - Inode::DIRECT_BLK_SIZE; i++){//i va de 0 a 2
        std::string portName = "b" + std::to_string(i + Inode::DIRECT_BLK_SIZE);
        std::string ptrName = "ad" + std::to_string(i + 1);
        blockPtrPorts[i + Inode::DIRECT_BLK_SIZE] = dotName + ":" + portName;
        std::string ptrVal = to_string(inode.blocks[i + Inode::DIRECT_BLK_SIZE]);
        dotNodeDeclaration +=  "                <tr>\n"
                               "                    <td bgcolor = \"lightcyan\">" + ptrName + "</td>\n"
                               "                    <td port=\"" + portName + "\" bgcolor = \"lightskyblue1\">" + ptrVal + "</td>\n"
                               "                </tr>\n";
    }

    dotNodeDeclaration += "        </table>\n"
                          "    >\n"
                          "];\n";
}


int DotInode::getBlockPtr(int index) const
{
    return inodeEntity.value.blocks[index];
}

int DotInode::getType() const
{
    return inodeEntity.value.type;
}

const std::string& DotInode::getDotName() const{
    return dotName;
}

const std::string& DotInode::getDotNodeDeclaration() const{
    return dotNodeDeclaration;
}

const std::string& DotInode::getBlockPort(int index) const{
    return blockPtrPorts[index];
}
