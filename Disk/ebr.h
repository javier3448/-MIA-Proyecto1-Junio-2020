#ifndef EBR_H
#define EBR_H
#include <string>
#include <cstring>

class Ebr
{
public:
    int start;
    int size;
    int next;
    char status;
    char fit;
    char name[16] = "null";

    Ebr();

    Ebr(int start, int size, int next, char status, char fit, const std::string &name);

    bool isNull() const;

    bool isTop() const;
};

#endif // EBR_H
