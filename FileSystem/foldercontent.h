#ifndef FOLDERCONTENT_H
#define FOLDERCONTENT_H


class FolderContent
{
public:
    static constexpr int NAME_SIZE = 12;

    char name[NAME_SIZE] = "null";
    int inodePtr = -1;

    FolderContent();

    bool isNull();
};

#endif // FOLDERCONTENT_H
