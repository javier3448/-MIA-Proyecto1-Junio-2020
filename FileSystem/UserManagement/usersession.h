#ifndef USERSESSION_H
#define USERSESSION_H

#include "FileSystem/UserManagement/user.h"
#include "Disk/mountedpart.h"

class UserSession
{
public:

    MountedPart part;//Particion donde se inicio la sesion
    User user;

    UserSession(const MountedPart& part, const User &user);

    bool isRoot();
};

#endif // USERSESSION_H
