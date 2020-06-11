#include "dotfileblock.h"

DotFileBlock::DotFileBlock(int address, RaidOneFile *file)
{
    fileBlockEntity = DiskEntity<FileBlock>(address, file);
    initDotStrings();
}

DotFileBlock::DotFileBlock(int address, FileBlock inode)
{
    fileBlockEntity = DiskEntity<FileBlock>(address, inode);
    initDotStrings();
}

void DotFileBlock::initDotStrings()
{
    using std::to_string;

    dotName = "b" + to_string(fileBlockEntity.address);

    dotNodeDeclaration = dotName +"[\n"
                                   "    fontname = \"courier\"\n"
                                   "    label = <\n"
                                   "        <table border=\"0\" cellborder=\"1\" cellspacing=\"2\" color=\"cyan4\">\n"
                                   "            <tr><td height=\"110\" width=\"75\" bgcolor = \"skyblue3\" >\n";

    std::string dataText("");
    dataText.reserve(64);
    int printedCharsCount = 0;
    for (int i = 0; i < FileBlock::CONTENT_SIZE; ++i) {
        char& c = fileBlockEntity.value.content[i];
        if(std::isprint(c)){
            dataText += c;
            printedCharsCount++;
        }
        if(printedCharsCount > 8){
            dataText += "<br/>";
            printedCharsCount = 0;
        }
    }

    dotNodeDeclaration += dataText + "</td></tr>\n"
                                     "        </table>\n"
                                     "    >\n"
                                     "];\n";
}

const std::string &DotFileBlock::getDotName() const
{
    return dotName;
}

const std::string &DotFileBlock::getDotNodeDeclaration() const
{
    return dotNodeDeclaration;
}
