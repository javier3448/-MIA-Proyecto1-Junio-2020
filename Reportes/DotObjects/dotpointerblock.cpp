#include "dotpointerblock.h"

DotPointerBlock::DotPointerBlock(int address, RaidOneFile *file)
{
    pointerBlockEntity = DiskEntity<PointerBlock>(address, file);
    initDotStrings();
}

DotPointerBlock::DotPointerBlock(int address, PointerBlock pointerBlock)
{
    pointerBlockEntity = DiskEntity<PointerBlock>(address, pointerBlock);
    initDotStrings();
}

void DotPointerBlock::initDotStrings()
{
    using std::to_string;
    auto& pointerBlock = pointerBlockEntity.value;

    dotName = "b" + to_string(pointerBlockEntity.address);

    dotNodeDeclaration = dotName + "[\n"
                                   "        label = <\n"
                                   "            <table border=\"0\" cellborder=\"1\" cellspacing=\"2\" cellpadding=\"2\" color=\"cyan4\">\n"
                                   "                <tr><td width=\"75\" bgcolor = \"skyblue3\">Pointer Block</td></tr>\n";

    for(int i = 0; i < PointerBlock::POINTERS_SIZE; i++){
        std::string ptrVal = to_string(pointerBlock.pointers[i]);
        std::string portName = "b" + std::to_string(i);
        blockPtrPorts[i] = dotName + ":" + portName;
        dotNodeDeclaration += "                <tr>\n"
                              "                    <td port=\"" + portName + "\" bgcolor = \"cyan2\">" + ptrVal + "</td>\n"
                              "                </tr>\n";
    }
    dotNodeDeclaration += "            </table>\n"
                          "        >\n"
                          "    ];\n";
}

int DotPointerBlock::getBlockPtr(int index) const
{
    return pointerBlockEntity.value.pointers[index];
}

const std::string& DotPointerBlock::getDotName() const{
    return dotName;
}

const std::string& DotPointerBlock::getDotNodeDeclaration() const{
    return dotNodeDeclaration;
}

const std::string& DotPointerBlock::getBlockPort(int index) const{
    return blockPtrPorts[index];
}
