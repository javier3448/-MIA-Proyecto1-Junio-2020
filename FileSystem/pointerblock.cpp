#include "pointerblock.h"
#include <algorithm>

PointerBlock::PointerBlock()
{
    std::fill_n(pointers, POINTERS_SIZE, -1);
}
