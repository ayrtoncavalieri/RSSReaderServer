#ifndef LOGOUTHPP
#define LOGOUTHPP

#include "../PocoInclude.hpp"
#include "../PocoData.hpp"
#include "../commonOps.hpp"

#include <Poco/UTF8String.h>

class logout{
    public:
        static Poco::JSON::Object::Ptr _logout(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt);
};

#endif