#include "FileSystem/extutility.h"
#include "FileSystem/fileblock.h"
#include "FileSystem/pointerblock.h"
#include "FileSystem/inode.h"

namespace  {
    //Chapuz deberia estar en un mymathutil o algo asi
    //[ !!! ] No lo probe para numero negativos ni numeros con los x + y de overflow
    inline int ceil(int x, int y){
        return (x + y - 1) / y;
    }
}

//Return 0 if the the file system doesnt support a file that big
//bleh!
//This is extremely hard to read. I think the constants are not helping at all D:
int ExtUtility::neededBlocks(int bytes){
    //TODO: REvisar que las costantes esten bien:
    static const int full_lvl_0_dir_block_count = Inode::DIRECT_BLK_SIZE;
    static const int full_lvl_1_dir_block_count = Inode::SIMPLE_INDIRECT_BLK_SIZE * (PointerBlock::POINTERS_SIZE) + full_lvl_0_dir_block_count;
    static const int full_lvl_2_dir_block_count = Inode::DOUBLE_INDIRECT_BLK_SIZE * (PointerBlock::POINTERS_SIZE * PointerBlock::POINTERS_SIZE) + full_lvl_1_dir_block_count;
    static const int full_lvl_3_dir_block_count = Inode::TRIPLE_INDIRECT_BLK_SIZE * (PointerBlock::POINTERS_SIZE * PointerBlock::POINTERS_SIZE * PointerBlock::POINTERS_SIZE) + full_lvl_2_dir_block_count;

    static const int full_lvl_0_indir_block_count = 0;
    static const int full_lvl_1_indir_block_count = 1;
    static const int full_lvl_2_indir_block_count = full_lvl_1_indir_block_count + (1 + PointerBlock::POINTERS_SIZE);

    int neededContentBlocks = ceil(bytes, FileBlock::CONTENT_SIZE);

    if(neededContentBlocks <= full_lvl_0_dir_block_count){
        return neededContentBlocks;
    }
    else if(neededContentBlocks <= full_lvl_1_dir_block_count){
        int currLevelContentBlocks = neededContentBlocks - full_lvl_0_dir_block_count;//este numero deberia ser de 1 a 16
        return full_lvl_0_dir_block_count + full_lvl_0_indir_block_count + (currLevelContentBlocks + 1);
    }
    else if(neededContentBlocks <= full_lvl_2_dir_block_count){
        int currLevelContentBlocks = neededContentBlocks - full_lvl_1_dir_block_count;//este numero deberia ser de 1 a 256
        return full_lvl_1_dir_block_count + full_lvl_1_indir_block_count +
               (currLevelContentBlocks + ceil(currLevelContentBlocks, PointerBlock::POINTERS_SIZE) + 1);
    }
    else if(neededContentBlocks <= full_lvl_3_dir_block_count){
        int currLevelContentBlocks = neededContentBlocks - full_lvl_2_dir_block_count;//este numero deberia ser de 1 a 4096
        int lvl1IndirBlocksCount = ceil(currLevelContentBlocks, PointerBlock::POINTERS_SIZE);
        return full_lvl_2_dir_block_count + full_lvl_2_indir_block_count +
               (currLevelContentBlocks + lvl1IndirBlocksCount + ceil(lvl1IndirBlocksCount, PointerBlock::POINTERS_SIZE) + 1);
    }

    return -1;
}
