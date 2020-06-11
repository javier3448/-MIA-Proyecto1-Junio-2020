#include "usersession.h"

UserSession::UserSession(const MountedPart& part, const User &user)
{
    this->part = part;
    this->user = user;
}

bool UserSession::isRoot()
{
    return user.id == 1;
}
