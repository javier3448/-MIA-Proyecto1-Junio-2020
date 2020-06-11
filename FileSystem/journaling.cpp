#include "journaling.h"

#include <cstring>


JournEntry::JournEntry()
{
    //Chapux: deberiamos de hacer un memset por arregl
    std::memset(user , '\0', USER_SIZE + PART_NAME_SIZE + OPERATION_SIZE + FILE_SYSTEM_SIZE);
}

JournEntry::JournEntry(const std::string &user, const std::string &partName, const std::string &operation, const std::string &fileSystem)
{
    time_t rawtime;
    std::time(&rawtime);
    this->time = *localtime(&rawtime);

    std::strncpy(this->user, user.c_str(), USER_SIZE - 1);
    this->user[USER_SIZE - 1] = '\0';

    std::strncpy(this->partName, partName.c_str(), PART_NAME_SIZE - 1);
    this->partName[PART_NAME_SIZE - 1] = '\0';

    std::strncpy(this->operation, operation.c_str(), OPERATION_SIZE - 1);
    this->operation[OPERATION_SIZE - 1] = '\0';

    std::strncpy(this->fileSystem, fileSystem.c_str(), FILE_SYSTEM_SIZE - 1);
    this->fileSystem[FILE_SYSTEM_SIZE - 1] = '\0';
}
