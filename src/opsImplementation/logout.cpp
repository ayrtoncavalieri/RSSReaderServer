#include "logout.hpp"

Poco::JSON::Object::Ptr logout::_logout(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    Poco::JSON::Object::Ptr reqResp;
    std::string email;

    try{
        email = req->getValue<std::string>("email");
        std::string uuid = req->getValue<std::string>("uuid");
        session << "DELETE FROM `rssreader`.`navigators` WHERE (`email` = ?) and (`uuid` = ?)", use(email), use(uuid), now;
        reqResp = new Poco::JSON::Object;
        reqResp->set("status", "OK");
    }catch(...){
        throw;
    }

    return reqResp;
}