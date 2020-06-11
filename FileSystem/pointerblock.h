#ifndef POINTERBLOCK_H
#define POINTERBLOCK_H


class PointerBlock
{
public:
    static constexpr int POINTERS_SIZE = 16;

    int pointers[POINTERS_SIZE];

    PointerBlock();
};

#endif // POINTERBLOCK_H
