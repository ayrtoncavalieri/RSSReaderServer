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
        commonOps::logMessage("emailConfirmation", "SSLException", Poco::Message::Priority::PRIO_ERROR);
        return false;
    }catch(Poco::Net::SMTPException &e){
        emailS.close();
        Poco::Net::uninitializeSSL();
        commonOps::logMessage("emailConfirmation", "SMTPException", Poco::Message::Priority::PRIO_ERROR);
        return false;
    }

    Poco::Net::uninitializeSSL();
    return true;
}

Poco::JSON::Object::Ptr emailConfirmation::eConf(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    Poco::JSON::Object::Ptr reqResp;

    try{
        
    }catch(...){
        throw;
    }

    return reqResp;
}