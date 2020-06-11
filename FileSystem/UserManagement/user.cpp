#include "user.h"

User::User(int id, const std::string& name, const std::string& groupName, const std::string& password)
{
    this->id = id;
    this->name = name;
    this->groupName = groupName;
    this->password = password;
}

User::User()
{
    this->id = -1;
    this->name = "null";
    this->password = "null";
    this->groupName = "null";
}

bool User::isNull(){
    return id == -1;
}
