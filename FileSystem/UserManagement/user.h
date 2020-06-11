#ifndef USER_H
#define USER_H

#include<string>

class User
{
public:
    int id = -1;
    std::string name;
    std::string password;
    std::string groupName;

    User();//Chapuz. Este constructor solo sirve para poder hacer el static User en tryLogin //TODO: Ver si todavia es necsario ese chapuz, viene del proyecto de vacas
    User(int id, const std::string& name, const std::string& groupName, const std::string& password);

    bool isNull();
};

#endif // USER_H
