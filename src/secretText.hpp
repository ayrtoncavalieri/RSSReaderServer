#ifndef SCRTTXT
#define SCRTTXT

#include <string>

class secretText{
    public:
    static std::string internSalt();
    static std::string audVal();
    static std::string dbConnectionParams();
};

#endif