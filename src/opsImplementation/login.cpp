/*
    This file is part of RSSReaderServer.

    RSSReaderServer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    RSSReaderServer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with RSSReaderServer.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "login.hpp"

Poco::JSON::Object::Ptr login::_login(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    Poco::JSON::Object::Ptr reqResp;
    std::string email;
    try{
        if(req->has("jwt")){
            return subscription::subs(op, req, session, salt);
        }
        email = req->getValue<std::string>("email");
        /*
            0 - email (std::string)
            1 - userName (std::string)
            2 - userPassword (std::string)
            3 - rSalt (std::string)
            4 - settings (std::string)
            5 - othersInfo (std::string)
            6 - linkPhoto (std::string)
            7 - idGoogle (std::string)
            8 - emailConfirmed (bool)
        */
        typedef Poco::Tuple<std::string, std::string, std::string, std::string, std::string, std::string, std::string, std::string, bool> QUser;
        QUser _user;
        session << "SELECT email, userName, userPassword, rSalt, settings, othersInfo, linkPhoto, idGoogle, emailConfirmed FROM rssreader.users WHERE email=?", into(_user), use(email), now;
        if(_user.get<0>().empty()){
                return commonOps::erroOpJSON(op, "wrong_credentials");
        }
        std::string password = req->getValue<std::string>("password");
        password = commonOps::passwordCalc(password, salt, _user.get<3>());
        if(_user.get<2>().compare(password) != 0){
            return commonOps::erroOpJSON(op, "wrong_credentials");
        }
        Poco::UUIDGenerator uuidGen;
        std::string uuid = uuidGen.createRandom().toString();
        session << "INSERT INTO `rssreader`.`navigators` (`email`, `uuid`) VALUES (?, ?)", use(email), use(uuid), now;
        unsigned int qtdLinks;
        session << "SELECT COUNT(*) FROM rssreader.links WHERE email=?", into(qtdLinks), use(email), now;
        reqResp = new Poco::JSON::Object;
        std::string loginType = "server";
        if(!_user.get<7>().empty()){
            loginType += ", provider";
        }
        reqResp->set("login", loginType);
        reqResp->set("email", email);
        reqResp->set("uuid", uuid);
        reqResp->set("feeds", qtdLinks);
        reqResp->set("pic", _user.get<6>());
        reqResp->set("name", _user.get<1>());
        reqResp->set("verified", _user.get<8>());
        if(!_user.get<4>().empty()){
            reqResp->set("settings", _user.get<4>());
        }
    }catch(...){
        throw;
    }
    return reqResp;
}