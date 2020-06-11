#ifndef PARTITION_H
#define PARTITION_H
#include <cstring>
#include <string>

class Partition
{
public:
    int start;
    int size;
    char type;
    char fit;
    char status;
    char name[16] = "null";

    Partition();

    void Init(char type, char fit, int start, int size, std::string const &name);

    bool isNull();
};

#endif // PARTITION_H
