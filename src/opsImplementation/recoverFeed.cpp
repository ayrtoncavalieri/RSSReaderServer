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

#include "recoverFeed.hpp"

Poco::JSON::Object::Ptr recFeed::recover(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    Poco::JSON::Object::Ptr reqResp;
    std::string email, uuid, feedLink, feedContent("");
    const std::string maxAgeStr("max-age=");
    const std::string defaultTimeToLive("7200");
    unsigned int hasLink = 0;
    bool updateCache = false, sslInitialized = false;

    try{
        reqResp = silentLogin::login(op, req, session, salt);
        if(!reqResp->has("error")){
            uuid = req->getValue<std::string>("uuid");
            feedLink = req->getValue<std::string>("feedAddress");
            session << "SELECT email FROM rssreader.navigators WHERE (uuid  = ?)", into(email), use(uuid), now;
            session << "SELECT COUNT(*) FROM rssreader.links WHERE (email = ?) AND (link = ?)", into(hasLink), use(email), use(feedLink), now;
            if(!hasLink){
                return commonOps::erroOpJSON(op, "feed_not_found");
            }
            Poco::DateTime expirationDate, _now;
            _now.makeLocal(-10800); //-3 * 3600
            session << "SELECT content, expirationDate FROM rssreader.linkCache WHERE (link = ?)", into(feedContent), use(feedLink), into(expirationDate), now;
            if(feedContent.empty() || _now > expirationDate){
                std::string cacheControl;
                updateCache = !feedContent.empty(); /*If the feedContent is empty after reading from DB, it means the event removed it
                  If updateCache is true, it means the link is in the DB and content must be updated,
                  Otherwise, the link is not present on the DB, and the entry should be recreated */
                Poco::URI uri(feedLink);
                feedContent = "";
                if(!uri.getScheme().compare("https")){
                    // HTTPS connection
                    Poco::Net::initializeSSL();
                    sslInitialized = true;
                    Poco::Net::Context::Ptr pContext = new Poco::Net::Context(
                            Poco::Net::Context::CLIENT_USE,
                            "",
                            "",
                            "",
                            Poco::Net::Context::VERIFY_RELAXED,
                            9,
                            true,
                            "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
                    Poco::Net::HTTPSClientSession pFeed(uri.getHost(), Poco::Net::HTTPSClientSession::HTTPS_PORT, pContext);
                    Poco::Net::HTTPRequest pFeedReq(Poco::Net::HTTPRequest::HTTP_GET, uri.getPathEtc(), Poco::Net::HTTPMessage::HTTP_1_1);
                    pFeed.sendRequest(pFeedReq);
                    Poco::Net::HTTPResponse pFeedResp;
                    std::istream& resp = pFeed.receiveResponse(pFeedResp);
                    commonOps::logMessage("recoverFeed", "HTTP status = " + Poco::NumberFormatter::format(pFeedResp.getStatus()), Poco::Message::PRIO_DEBUG);
                    if(pFeedResp.getStatus() != Poco::Net::HTTPResponse::HTTP_OK || pFeed.networkException() != NULL){
                        Poco::Net::uninitializeSSL();
                        if(pFeed.networkException() != NULL){
                            commonOps::logMessage("recoverFeed", "networkException: " + pFeed.networkException()->displayText(), Poco::Message::PRIO_DEBUG);
                        }
                        commonOps::logMessage("recoverFeed", "HTTP not OK, or networkException", Poco::Message::PRIO_DEBUG);
                        return commonOps::erroOpJSON(op, "not_reachable");                        
                    }
                    cacheControl = pFeedResp.get("cache-control", maxAgeStr + defaultTimeToLive);
                    if(cacheControl.find(maxAgeStr, 0) == std::string::npos){
                        cacheControl = maxAgeStr + defaultTimeToLive;
                    }
                    std::string temporary(std::istreambuf_iterator<char>(resp), {});
                    feedContent = temporary;
                    Poco::Net::uninitializeSSL();
                    sslInitialized = false;
                }else{
                    // HTTP connection
                }
                std::string numberToParse;
                for(unsigned int i = maxAgeStr.length() + cacheControl.find(maxAgeStr, 0); i < cacheControl.length() && cacheControl[i] != ','; ++i){
                    numberToParse += cacheControl[i];
                }
                unsigned int maxAgeTime;
                if(Poco::NumberParser::tryParseUnsigned(numberToParse, maxAgeTime) == false){
                    return commonOps::erroOpJSON(op, "parse_error");
                }
                Poco::Timespan timeToLive((long)maxAgeTime, (long)0);
                _now += timeToLive;
                std::string expDateString = Poco::DateTimeFormatter::format(_now, "%Y-%m-%d %H:%M:%S");
                if(updateCache){
                    //Here we update the link that is already in cache
                    session << "UPDATE `rssreader`.`linkCache` SET `content` = ?, `expirationDate` = ? WHERE (`link` = ?)",
                        use(feedContent), use(expDateString), use(feedLink), now;
                }else{
                    //Here, the link does not exist in the cache anymore
                    session << "INSERT INTO `rssreader`.`linkCache` (`link`, `content`, `expirationDate`) VALUES (?, ?, ?);", 
                        use(feedLink), use(feedContent), use(expDateString), now;
                }
            }
            reqResp->set("feedContent", feedContent);
        }
    }catch(Poco::Net::HostNotFoundException &e){
        if(sslInitialized == true){
            Poco::Net::uninitializeSSL();
        }
        commonOps::logMessage("recoverFeed", "HostNotFoundException " + e.message(), Poco::Message::PRIO_DEBUG);
        return commonOps::erroOpJSON(op, "not_reachable");
    }catch(...){
        throw;
    }

    return reqResp;
}