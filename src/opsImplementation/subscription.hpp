#ifndef SUBSHPP
#define SUBSHPP

#include "../PocoInclude.hpp"
#include "../commonOps.hpp"

#include <Poco/NumberParser.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Nullable.h>
#include <Poco/SharedPtr.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/Data/Session.h>
#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/JWT/Token.h>
#include <Poco/JWT/Signer.h>
#include <Poco/JWT/JWTException.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Crypto/RSAKey.h>
#include <Poco/Crypto/X509Certificate.h>
#include <Poco/UUIDGenerator.h>
#include <istream>
#include <time.h>

class subscription{
    public:
    static Poco::JSON::Object::Ptr subs(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt);
};

#endif