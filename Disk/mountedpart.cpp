#include "mountedpart.h"

#include "ebr.h"

MountedPart::MountedPart(const Partition &part, const std::string& path, char type)
{
    this->name = std::string(part.name);
    if(type == T_PRIMARY){
        this->start = part.start;
        this->size = part.size;
    }
    this->path = path;
    this->type = type;
    contentStart = start;
    contentSize = size;
}

MountedPart::MountedPart(const Ebr &ebr, const std::string& path)
{
    this->name = std::string(ebr.name);
    this->start = ebr.start;
    this->size = ebr.size;
    this->path = path;
    this->type = T_LOGIC;

    contentStart = ebr.start + (int) sizeof(Ebr);
    contentSize = ebr.size - (int) sizeof(Ebr);
}

MountedPart::MountedPart()
{

}
