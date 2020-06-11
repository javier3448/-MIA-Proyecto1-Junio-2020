#include "dotfolderblock.h"

DotFolderBlock::DotFolderBlock(int address, RaidOneFile *file)
{
    folderBlockEntity = DiskEntity<FolderBlock>(address, file);
    initDotStrings();
}

DotFolderBlock::DotFolderBlock(int address, FolderBlock folderBlock)
{
    folderBlockEntity = DiskEntity<FolderBlock>(address, folderBlock);
    initDotStrings();
}

void DotFolderBlock::initDotStrings()
{
    using std::to_string;
    auto& folderBlock = folderBlockEntity.value;

    dotName = "b" + to_string(folderBlockEntity.address);

    dotNodeDeclaration = dotName + "[\n"
                                   "        label = <\n"
                                   "            <table border=\"0\" cellborder=\"1\" cellspacing=\"2\" cellpadding=\"2\" color=\"cyan4\">\n"
                                   "                <tr><td width=\"75\" bgcolor = \"skyblue3\" colspan=\"2\">Folder Block</td></tr>\n";

    for(int i = 0; i < FolderBlock::CONTENTS_SIZE; i++){
        std::string inodeName = folderBlock.contents[i].name;
        std::string inodePtr = to_string(folderBlock.contents[i].inodePtr);
        std::string portName = "i" + to_string(i);
        inodePtrPorts[i] = dotName + ":" + portName;
        dotNodeDeclaration += "                <tr>\n"
                              "                    <td bgcolor = \"cyan\">" + inodeName + "</td>\n"
                              "                    <td port=\"" + portName + "\" bgcolor = \"cyan3\">" + inodePtr + "</td>\n"
                              "                </tr>\n";
    }

    dotNodeDeclaration += "            </table>\n"
                          "        >\n"
                          "    ];\n";


}

std::string DotFolderBlock::getInodeName(int index) const
{
    return std::string(folderBlockEntity.value.contents[index].name);
}

int DotFolderBlock::getInodePtr(int index) const
{
    return folderBlockEntity.value.contents[index].inodePtr;
}

const std::string &DotFolderBlock::getDotName() const
{
    return dotNodeDeclaration;
}

const std::string &DotFolderBlock::getDotNodeDeclaration() const
{
    return dotNodeDeclaration;
}

const std::string &DotFolderBlock::getInodePort(int index) const
{
    return inodePtrPorts[index];
}
