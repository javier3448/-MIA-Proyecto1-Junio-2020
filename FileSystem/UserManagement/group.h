#ifndef GROUP_H
#define GROUP_H

#include<string>

class Group
{
public:
    int id;
    std::string name;

    Group(int id, const std::string& name);
};

#endif // GROUP_H
