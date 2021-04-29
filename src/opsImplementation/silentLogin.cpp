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

#include "silentLogin.hpp"

Poco::JSON::Object::Ptr silentLogin::login(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    Poco::JSON::Object::Ptr reqResp;
    std::string uuid;
    try{
        uuid = req->getValue<std::string>("uuid");
        std::string hashNavigator, hashNavigatorJSON;
        hashNavigator = "";
        hashNavigatorJSON = req->getValue<std::string>("variable");
        session << "SELECT hashNavigator FROM rssreader.navigators WHERE (`uuid` = ?)", into(hashNavigator), use(uuid), now;
        if(hashNavigator.empty() || hashNavigatorJSON.compare(hashNavigator)){
            reqResp = commonOps::erroOpJSON(op, "wrong_credentials");
        }else{
            session << "UPDATE `rssreader`.`navigators` SET `lastAccess` = CURRENT_TIMESTAMP WHERE (`uuid` = ?)", use(uuid), now;
            reqResp = new Poco::JSON::Object;
            reqResp->set("op", op);
            reqResp->set("status", "OK");
        }
    }catch(...){
        throw;
    }

    return reqResp;
}