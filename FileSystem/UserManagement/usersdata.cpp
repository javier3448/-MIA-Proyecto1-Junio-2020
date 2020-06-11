#include "usersdata.h"

#include "mystringutil.h"

//ASUMIMOS QUE: "1,G,root\n1,U,root,root,123\n"
//              *usr={id,type,name,pwd} grp={id,type,name}

UsersData::UsersData(std::string usersTxt)
{
    lines = MyStringUtil::tokenize(usersTxt, '\n');
}

std::string UsersData::getUsersTxt() const
{
    std::string usersTxt("");
    usersTxt.reserve(lines[1].length() * lines.size());//algo chapuceado: asume que el string mas largo posible va estar en la seguna pos del vector y que el string mas largo es el que se va a usar para los usuarios
    for(auto line : lines){
        usersTxt += line + "\n";
    }

    return usersTxt;
}

bool UsersData::trySetUsersTxt(const std::string &usersTxt)
{
    lines = MyStringUtil::tokenize(usersTxt, '\n');
    return true;
}

std::string UsersData::getGroupName(int id) const
{
    for(auto line : lines){
        std::vector<std::string> attributes = MyStringUtil::tokenize(line, ',');
        if(std::stoi(attributes[0]) == 0 ||
            attributes[1] == "U"){
            continue;
        }
        if(std::stoi(attributes[0]) == id){
            return attributes[2];
        }
    }
    return std::string();
}

std::string UsersData::getUsrName(int id) const
{
    for(auto line : lines){
        std::vector<std::string> attributes = MyStringUtil::tokenize(line, ',');
        if(std::stoi(attributes[0]) == 0 ||
            attributes[1] == "G"){
            continue;
        }
        if(std::stoi(attributes[0]) == id){
            return attributes[2];
        }
    }
    return std::string();
}

bool UsersData::hasGroup(const std::string &name) const //usr={id,type,name,grp,pwd} grp={id,type,name} const
{
    for(auto line : lines){
        std::vector<std::string> attributes = MyStringUtil::tokenize(line, ',');
        if(std::stoi(attributes[0]) == 0 ||
            attributes[1] == "U"){
            continue;
        }
        if(attributes[2] == name){
            return true;
        }
    }
    return false;
}

bool UsersData::tryRemoveGroup(const std::string &name)//usr={id,type,name,grp,pwd} grp={id,type,name}
{
    bool result = false;
    for(auto& line : lines){
        std::vector<std::string> attributes = MyStringUtil::tokenize(line, ',');
        if(std::stoi(attributes[0]) == 0){
            continue;
        }
        if(attributes[1] == "U"){
            if(attributes[3] == name){
                line.replace(0, attributes[0].length(), "0");
            }
            continue;
        }
        if(attributes[2] == name){
            line.replace(0, attributes[0].length(), "0");
            result = true;
        }
    }
    return result;
}

std::string UsersData::getGroupString(int id, const std::string &name) const
{
    return std::to_string(id) + "," + "G" + "," + name + "\n";
}

int UsersData::getGroupId(const std::string &name) const
{
    for(const auto& line : lines){
        std::vector<std::string> attributes = MyStringUtil::tokenize(line, ',');
        if(std::stoi(attributes[0]) == 0 ||
            attributes[1] == "U"){
            continue;
        }
        if(attributes[2] == name){
            return std::stoi(attributes[0]);
        }
    }
    return -1;
}

//Retorna -1 cuando no tiene ningun grupo en lines, esto nunca deberia de pasar porque todo fileSystem tiene un grupo root
int UsersData::getMaxGroupId()const
{
    for(int i = lines.size() - 1; i > -1; i--){//Talvez hay una manera mas c++ de recorrer un vector en orden inverso
        const std::string& line = lines[i];
        std::vector<std::string> attributes = MyStringUtil::tokenize(line, ',');
        if(std::stoi(attributes[0]) == 0 ||
            attributes[1] == "U"){
            continue;
        }
        return std::stoi(attributes[0]);
    }
    return -1;
}

bool UsersData::hasUser(const std::string &name) const
{
    for(const auto& line : lines){
        std::vector<std::string> attributes = MyStringUtil::tokenize(line, ',');
        if(std::stoi(attributes[0]) == 0 ||
            attributes[1] == "G"){
            continue;
        }
        if(attributes[2] == name){
            return true;
        }
    }
    return false;
}

bool UsersData::tryRemoveUser(const std::string &name)
{
    for(auto& line : lines){
        std::vector<std::string> attributes = MyStringUtil::tokenize(line, ',');
        if(std::stoi(attributes[0]) == 0 ||
            attributes[1] == "G"){
            continue;
        }
        if(attributes[2] == name){
            line.replace(0, attributes[0].length(), "0");
            return true;
        }
    }
    return false;
}

std::string UsersData::getUserString(int id, const std::string& name, const std::string& grp, const std::string& password) const
{
    return std::to_string(id) + "," + "U" + "," + name + "," + grp + "," + password + "\n";
}

int UsersData::getUserId(const std::string &name) const
{
    for(const auto& line : lines){
        std::vector<std::string> attributes = MyStringUtil::tokenize(line, ',');
        if(std::stoi(attributes[0]) == 0 ||
            attributes[1] == "G"){
            continue;
        }
        if(attributes[2] == name){
            return std::stoi(attributes[0]);
        }
    }
    return -1;
}

//Retorna -1 cuando no tiene ningun usuario en lines, esto nunca deberia de pasar porque todo fileSystem tiene un usuario root
int UsersData::getMaxUserId() const
{
    for(int i = lines.size() - 1; i > -1; i--){//Talvez hay una manera mas c++ de recorrer un vector en orden inverso
        const std::string& line = lines[i];
        std::vector<std::string> attributes = MyStringUtil::tokenize(line, ',');
        if(std::stoi(attributes[0]) == 0 ||
            attributes[1] == "G"){
            continue;
        }
        return std::stoi(attributes[0]);
    }
    return -1;
}

std::optional<User> UsersData::tryLogin(const std::string &name, const std::string &password) const
{
    using std::stoi;

    for(auto line : lines){
        std::vector<std::string> attributes = MyStringUtil::tokenize(line, ',');
        if(attributes[1] == "G" ||
            std::stoi(attributes[0]) == 0){
            continue;
        }
        if(attributes[2] == name && attributes[4] == password){
            return User(stoi(attributes[0]), attributes[2], attributes[3], attributes[4]);
        }
    }
    return {};
}
