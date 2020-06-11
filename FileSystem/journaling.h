#ifndef JOURNALING_H
#define JOURNALING_H

#include <ctime>

#include "raidonefile.h"
#include "FileSystem/superboot.h"
#include "diskentity.h"

//TODO: VER QUE VA AQUI. Creo que seria algo muy parecido al lwhLog
class JournEntry
{
public:
    static constexpr unsigned int USER_SIZE = 12;
    static constexpr unsigned int PART_NAME_SIZE = 12;
    static constexpr unsigned int OPERATION_SIZE = 256;
    static constexpr unsigned int FILE_SYSTEM_SIZE = 12;

    std::tm time;
    char user[USER_SIZE];
    char operation[OPERATION_SIZE];

    JournEntry();
    JournEntry(const std::string& user, const std::string& operation, const std::string& fileSystem);
};

#endif // JOURNALING_H
