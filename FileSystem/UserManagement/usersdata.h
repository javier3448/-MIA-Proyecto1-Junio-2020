#ifndef USERSDATA_H
#define USERSDATA_H

#include <vector>
#include <optional>
#include "FileSystem/UserManagement/user.h"
#include "FileSystem/UserManagement/group.h"

//USERS
class UsersData
{
private:
    std::vector<std::string> lines;

public:

    UsersData(std::string usersTxt);

    std::string getUsersTxt() const;
    bool trySetUsersTxt(const std::string& usersTxt);

    bool hasGroup(const std::string& name) const;
    bool tryRemoveGroup(const std::string& name);
    std::string getGroupString(int id, const std::string& name) const;
    int getGroupId(const std::string& name) const;
    std::string getGroupName(int id) const;
    int getMaxGroupId() const;

    bool hasUser(const std::string& name) const;
    bool tryRemoveUser(const std::string& name);
    std::string getUserString(int id, const std::string& name, const std::string& grp, const std::string& password) const;
    int getUserId(const std::string& name)const;
    std::string getUsrName(int id) const;
    int getMaxUserId()const;

    std::optional<User> tryLogin(const std::string& name, const std::string& password)const;
};

#endif // USERSDATA_H
