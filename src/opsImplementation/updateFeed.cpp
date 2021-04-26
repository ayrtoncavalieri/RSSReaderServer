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

#include "updateFeed.hpp"

Poco::JSON::Object::Ptr updFeed::updateFeed(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    Poco::JSON::Object::Ptr reqResp;
    std::string email, uuid, link, feedName, feedCategories;

    try{
        reqResp = silentLogin::login(op, req, session, salt);
        if(!reqResp->has("error")){
            uuid = req->getValue<std::string>("uuid");
            link = req->getValue<std::string>("feedAddress");
            feedName = req->getValue<std::string>("feedName");
            feedCategories = req->getValue<std::string>("feedCategories");
            session << "SELECT email FROM rssreader.navigators WHERE (uuid = ?)", into(email), use(uuid), now;
            session << "UPDATE `rssreader`.`links` SET `linkName` = ?, `category` = ? WHERE (`email` = ?) AND (`link` = ?)", use(feedName), use(feedCategories), use(email), use(link), now;
        }
    }catch(...){
        throw;
    }

    return reqResp;
}