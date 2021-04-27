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

#include "listFeeds.hpp"

Poco::JSON::Object::Ptr listFeed::_list(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    Poco::JSON::Object::Ptr reqResp;
    std::string email, uuid;

    try{
        reqResp = silentLogin::login(op, req, session, salt);
        if(!reqResp->has("error")){
            uuid = req->getValue<std::string>("uuid");
            session << "SELECT email FROM rssreader.navigators WHERE (uuid = ?)", into(email), use(uuid), now;
            typedef Poco::Tuple<std::string, std::string, std::string, std::string, Poco::DateTime> LinkData;
            /*
                0 - email (std::string)
                1 - link (std::string)
                2 - linkName (std::string)
                3 - category (std::string)
                4 - dateAdded (Poco::DateTime)
            */
            std::vector<LinkData> retrievedLinks;
            session << "SELECT * FROM rssreader.links WHERE (email = ?)", into(retrievedLinks), use(email), now;
            Poco::JSON::Object::Ptr linkInformation;
            Poco::JSON::Array::Ptr linkVector = new Poco::JSON::Array;
            Poco::JSON::Array::Ptr feedCategories;
            Poco::JSON::Parser p;
            for(std::vector<LinkData>::iterator item = retrievedLinks.begin(); item < retrievedLinks.end(); item++){
                linkInformation = new Poco::JSON::Object;
                linkInformation->set("feedAddress", item->get<1>());
                linkInformation->set("feedName", item->get<2>());
                feedCategories = p.parse(item->get<3>()).extract<Poco::JSON::Array::Ptr>();
                linkInformation->set("feedCategories", feedCategories);
                p.reset();
                linkInformation->set("dateAdded", Poco::DateTimeFormatter::format(item->get<4>(), "%Y-%m-%d %H:%M:%S"));
                linkVector->add(linkInformation);
            }
            reqResp->set("feeds", linkVector);
        }
    }catch(...){
        throw;
    }

    return reqResp;
}