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

#include "emailConfirmation.hpp"

bool emailConfirmation::sendEmail(Poco::Net::MailMessage &message, std::string senderEmail, std::string senderPass)
{
    Poco::Net::SecureSMTPClientSession emailS("rssreader.aplikoj.com", 587);
    emailS.open();
    Poco::Net::initializeSSL();

    Poco::SharedPtr<Poco::Net::InvalidCertificateHandler> ptrHandler = new Poco::Net::AcceptCertificateHandler(false);
    Poco::Net::Context::Ptr ptrContext;
    ptrContext = new Poco::Net::Context(Poco::Net::Context::CLIENT_USE, "", "", "", Poco::Net::Context::VERIFY_RELAXED, 9, true, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
    Poco::Net::SSLManager::instance().initializeClient(NULL, ptrHandler, ptrContext);

    try{
        emailS.login();
        if(emailS.startTLS(ptrContext)){
            emailS.login(Poco::Net::SecureSMTPClientSession::AUTH_LOGIN, senderEmail, senderPass);
            emailS.sendMessage(message);
            emailS.close();
        }else{
            emailS.close();
            Poco::Net::uninitializeSSL();
            commonOps::logMessage("emailConfirmation", "TLS_Failed", Poco::Message::Priority::PRIO_ERROR);
            return false;
        }
    }catch(Poco::Net::SSLException &e){
        emailS.close();
        Poco::Net::uninitializeSSL();
        commonOps::logMessage("emailConfirmation", "SSLException ->" + e.message(), Poco::Message::Priority::PRIO_ERROR);
        return false;
    }catch(Poco::Net::SMTPException &e){
        emailS.close();
        Poco::Net::uninitializeSSL();
        commonOps::logMessage("emailConfirmation", "SMTPException -> " + e.message(), Poco::Message::Priority::PRIO_ERROR);
        return false;
    }

    Poco::Net::uninitializeSSL();
    return true;
}

Poco::JSON::Object::Ptr emailConfirmation::eConf(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    Poco::JSON::Object::Ptr reqResp;
    const std::string methodName("emailConfirmation::eConf");
    const std::string noAuthIDJSON("no_authId_JSON");
    std::string othersInfo("");
    std::string userEmail, authID;
    std::ostringstream streamg;
    Poco::JSON::Object::Ptr authIDJSON;
    Poco::JSON::Parser p;

    try{
        if(req->has("authId")){
            authID = req->getValue<std::string>("authId");
            userEmail = req->getValue<std::string>("email");

            unsigned int emailCount = 0;
            session << "SELECT COUNT(*) FROM rssreader.users WHERE (email = ?)", into(emailCount), use(userEmail), now;
            if(emailCount == 0){
                return commonOps::erroOpJSON(op, "user_not_found");
            }
            session << "SELECT othersInfo FROM rssreader.users WHERE (email = ?)", into(othersInfo), use(userEmail), now;
            if(othersInfo.empty()){
                commonOps::logMessage(methodName, "othersInfo is empty!", Poco::Message::PRIO_ERROR);
                session << "UPDATE `rssreader`.`users` SET `othersInfo` = '{}' WHERE (`email` = ?)", use(userEmail), now;
                othersInfo = "{}";
            }
            authIDJSON = p.parse(othersInfo).extract<Poco::JSON::Object::Ptr>();
            if(!authIDJSON->has("authID") || authIDJSON->getValue<std::string>("authID").compare(authID)){
                return commonOps::erroOpJSON(op, "wrong_confid");
            }
            authIDJSON->remove("authID");

            authIDJSON->stringify(streamg, 0, -1);
            othersInfo = streamg.str();
        
            session << "UPDATE `rssreader`.`users` SET `emailConfirmed` = 1, `othersInfo` = ? WHERE (`email` = ?)", use(othersInfo), use(userEmail), now;

            reqResp = new Poco::JSON::Object;
            reqResp->set("status", "OK");
            reqResp->set("op", op);
            reqResp->set("verified", true);
        }else if(req->has("variable") && req->has("uuid")){
            reqResp = silentLogin::login(op, req, session, salt);
            if(!reqResp->has("error")){
                std::string uuid = req->getValue<std::string>("uuid");
                session << "SELECT email FROM rssreader.navigators WHERE (uuid = ?)", 
                        into(userEmail), use(uuid), now;
                session << "SELECT othersInfo FROM rssreader.users WHERE (email = ?)", 
                        into(othersInfo), use(userEmail), now;
                if(othersInfo.empty()){
                    commonOps::logMessage(methodName, "othersInfo is empty!", Poco::Message::PRIO_ERROR);
                    othersInfo = "{}";
                }
                authID = commonOps::genAuthID(128);
                authIDJSON = p.parse(othersInfo).extract<Poco::JSON::Object::Ptr>();
                authIDJSON->set("authID", authID);

                authIDJSON->stringify(streamg, 0, -1);
                othersInfo = streamg.str();
                
                Poco::Net::MailMessage message;
                subscription::composeConfirmationEmail(message, userEmail, authID);
                if(!emailConfirmation::sendEmail(message, "no-reply@rssreader.aplikoj.com", secretText::noReplyEmailPassword()))
                    commonOps::logMessage(methodName, "Failed to send e-mail confirmation", Poco::Message::PRIO_ERROR);
                
                session << "UPDATE `rssreader`.`users` SET `othersInfo` = ? WHERE (`email` = ?)", use(othersInfo), use(userEmail), now;
            }
        }else{
            reqResp = commonOps::erroOpJSON(op, "exception_empty_confid");
        }
    }catch(...){
        throw;
    }
    
    return reqResp;
}