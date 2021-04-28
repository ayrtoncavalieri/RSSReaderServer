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

#include "addFeed.hpp"

Poco::JSON::Object::Ptr addFeed::add(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    Poco::JSON::Object::Ptr reqResp;
    std::string receivedFeed, cacheControl;
    const std::string maxAgeStr("max-age=");
    const std::string defaultTimeToLive("7200");
    bool sslInitialized = false;
    unsigned int linksFound = 0;
    std::string feedName, feedCategory, email, uuid;
    try{
        reqResp = silentLogin::login(op, req, session, salt);
        if(reqResp->has("error")){
            return reqResp;
        }
        uuid = req->getValue<std::string>("uuid");
        std::string address(req->getValue<std::string>("feedAddress"));
        if(address.find("://", 0) == std::string::npos){
            address = "://" + address;
        }
        Poco::URI uri(address);
        if(uri.getScheme().empty()){
            uri.setScheme("https");
        }
        std::string endereco = uri.toString();
        commonOps::logMessage("addFeed", "endereco = " + endereco, Poco::Message::PRIO_DEBUG);
        session << "SELECT COUNT(*) FROM rssreader.linkCache WHERE (link = ?)", into(linksFound), use(endereco), now;
        if(linksFound == 0){
            if(!uri.getScheme().compare("https")){
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
                commonOps::logMessage("addFeed", "HTTP status = " + Poco::NumberFormatter::format(pFeedResp.getStatus()), Poco::Message::PRIO_DEBUG);
                if(pFeedResp.getStatus() != Poco::Net::HTTPResponse::HTTP_OK || pFeed.networkException() != NULL){
                    Poco::Net::uninitializeSSL();
                    if(pFeed.networkException() != NULL){
                        commonOps::logMessage("addFeed", "networkException: " + pFeed.networkException()->displayText(), Poco::Message::PRIO_DEBUG);
                    }
                    commonOps::logMessage("addFeed", "HTTP not OK, or networkException", Poco::Message::PRIO_DEBUG);
                    return commonOps::erroOpJSON(op, "not_reachable");
                }
                if(pFeedResp.getContentType().find("xml") == std::string::npos){
                    pFeed.abort();
                    Poco::Net::uninitializeSSL();
                    return commonOps::erroOpJSON(op, "invalid_address");
                }
                cacheControl = pFeedResp.get("cache-control", maxAgeStr + defaultTimeToLive);
                if(cacheControl.find(maxAgeStr, 0) == std::string::npos){
                    cacheControl = maxAgeStr + defaultTimeToLive;
                }
                std::string temporary(std::istreambuf_iterator<char>(resp), {});
                receivedFeed = temporary;
                Poco::Net::uninitializeSSL();
                sslInitialized = false;
            }else if(!uri.getScheme().compare("http")){
                Poco::Net::HTTPClientSession pFeed(uri.getHost(), Poco::Net::HTTPClientSession::HTTP_PORT);
                Poco::Net::HTTPRequest pFeedReq(Poco::Net::HTTPRequest::HTTP_GET, uri.getPathEtc(), Poco::Net::HTTPMessage::HTTP_1_1);
                pFeed.sendRequest(pFeedReq);
                Poco::Net::HTTPResponse pFeedResp;
                std::istream& resp = pFeed.receiveResponse(pFeedResp);
                if(pFeedResp.getStatus() != Poco::Net::HTTPResponse::HTTP_OK || pFeed.networkException() != NULL){
                    Poco::Net::uninitializeSSL();
                    commonOps::logMessage("addFeed", "HTTP not OK, or networkException", Poco::Message::PRIO_DEBUG);
                    return commonOps::erroOpJSON(op, "not_reachable");
                }
                if(pFeedResp.getContentType().find("xml") == std::string::npos){
                    pFeed.abort();
                    return commonOps::erroOpJSON(op, "invalid_address");
                }
                cacheControl = pFeedResp.get("cache-control", maxAgeStr + defaultTimeToLive);
                if(cacheControl.find(maxAgeStr, 0) == std::string::npos){
                    cacheControl = maxAgeStr + defaultTimeToLive;
                }
                std::string temporary(std::istreambuf_iterator<char>(resp), {});
                receivedFeed = temporary;
            }else{
                return commonOps::erroOpJSON(op, "invalid_address");
            }
            
            Poco::XML::DOMParser parser;
            Poco::AutoPtr<Poco::XML::Document> feed = parser.parseString(receivedFeed);
            Poco::XML::ElementsByTagNameList *elements = (Poco::XML::ElementsByTagNameList*)feed->getElementsByTagName("rss");
            if(elements->length() != 1){
                return commonOps::erroOpJSON(op, "not_rss");
            }
            std::string numberToParse;
            for(unsigned int i = maxAgeStr.length() + cacheControl.find(maxAgeStr, 0); i < cacheControl.length() && cacheControl[i] != ','; ++i){
                numberToParse += cacheControl[i];
            }
#ifdef DEBUG
            commonOps::logMessage("addFeed", "URI to String: " + uri.toString(), Poco::Message::PRIO_DEBUG);
            commonOps::logMessage("addFeed", "Number parsed(max-age): " + numberToParse, Poco::Message::PRIO_DEBUG);
            commonOps::logMessage("addFeed", "String to parse(cacheControl): " + cacheControl, Poco::Message::PRIO_DEBUG);
#endif
            unsigned int maxAgeTime;
            if(Poco::NumberParser::tryParseUnsigned(numberToParse, maxAgeTime) == false){
                return commonOps::erroOpJSON(op, "parse_error");
            }
            Poco::Timespan timeToLive((long)maxAgeTime, (long)0);
            Poco::DateTime expirationDate;
            expirationDate.makeLocal(-10800); //-3 * 3600 
            commonOps::logMessage("addFeed", "expirationDate made local: " + Poco::DateTimeFormatter::format(expirationDate, "%dd %H:%M:%S.%i"), Poco::Message::PRIO_DEBUG);
            expirationDate += timeToLive;
            commonOps::logMessage("addFeed", "expirationDate + timeToLive: " + Poco::DateTimeFormatter::format(expirationDate, "%dd %H:%M:%S.%i"), Poco::Message::PRIO_DEBUG);
            const std::string completeURI(uri.toString());
            std::string val(completeURI);
            std::string expDateString = Poco::DateTimeFormatter::format(expirationDate, "%Y-%m-%d %H:%M:%S");
            session << "INSERT INTO `rssreader`.`linkCache` (`link`, `content`, `expirationDate`) VALUES (?, ?, ?);", 
            use(val), use(receivedFeed), use(expDateString), now;
        }
        session << "SELECT email FROM rssreader.navigators WHERE (uuid = ?)", into(email), use(uuid), now;
        const std::string completeURI(uri.toString());
        std::string val(completeURI);
        unsigned int linkAssociated = 0;
        session << "SELECT COUNT(*) FROM rssreader.links WHERE (link = ?) AND (email = ?)",
        into(linkAssociated), use(val), use(email), now;
        if(linkAssociated != 0){
            return commonOps::erroOpJSON(op, "duplicate_feed");
        }
        feedName = req->getValue<std::string>("feedName");
        feedCategory = req->getValue<std::string>("feedCategories");
        session << "INSERT INTO `rssreader`.`links` (`email`, `link`, `linkName`, `category`) VALUES (?, ?, ?, ?)", 
        use(email), use(val), use(feedName), use(feedCategory), now;
        reqResp = new Poco::JSON::Object;
        reqResp->set("status", "OK");
    }catch(Poco::URISyntaxException &e){
        return commonOps::erroOpJSON(op, "invalid_address");
    }catch(Poco::Net::HostNotFoundException &e){
        if(sslInitialized == true){
            Poco::Net::uninitializeSSL();
        }
        commonOps::logMessage("addFeed", "HostNotFoundException " + e.message(), Poco::Message::PRIO_DEBUG);
        return commonOps::erroOpJSON(op, "not_reachable");
    }catch(Poco::XML::SAXException &e){
        return commonOps::erroOpJSON(op, "not_valid_feed");
    }catch(...){
        if(sslInitialized){
            Poco::Net::uninitializeSSL();
        }
        throw;
    }

    return reqResp;
}