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

#include "logout.hpp"

Poco::JSON::Object::Ptr logout::_logout(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    Poco::JSON::Object::Ptr reqResp;
    std::string email;

    try{
        reqResp = silentLogin::login(op, req, session, salt);
        if(reqResp->has("error")){
            return reqResp;
        }
        std::string uuid = req->getValue<std::string>("uuid");
        session << "SELECT email FROM rssreader.navigators WHERE (`uuid` = ?)", into(email), use(uuid), now;
        session << "DELETE FROM `rssreader`.`navigators` WHERE (`email` = ?) and (`uuid` = ?)", use(email), use(uuid), now;
        reqResp = new Poco::JSON::Object;
        reqResp->set("status", "OK");
    }catch(...){
        throw;
    }

    return reqResp;
}