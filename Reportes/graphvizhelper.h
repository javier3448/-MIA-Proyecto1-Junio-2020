#ifndef GRAPHVIZHELPER_H
#define GRAPHVIZHELPER_H

#include<string>

class GraphvizHelper
{
public:
    static int writeAndCompileDot(const std::string& path, const std::string& dotCode, bool open = true);
    static int writeDot(const std::string& path, const std::string& dotCode);
    static int compileDot(const std::string& path, bool open = true);
};

#endif // GRAPHVIZHELPER_H
