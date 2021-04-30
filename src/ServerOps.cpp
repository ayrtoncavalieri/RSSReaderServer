/*
    This file is part of Server_Shell.

    Server_Shell is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Server_Shell is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Server_Shell.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "ServerOps.hpp"

ServerOps::ServerOps(): salt(secretText::internSalt())
{

}

ServerOps::~ServerOps()
{

}

std::string ServerOps::processReq(std::string &req)
{
    std::string respJSON; //32 bytes
    Poco::JSON::Object::Ptr reqJSON; //16 bytes
    Poco::JSON::Object::Ptr procJSON; //16 bytes
    Poco::JSON::Parser p; //40 bytes
    unsigned int option = 0; //4 bytes
    std::string params = secretText::dbConnectionParams();
    const std::string methodName("ServerOps::processReq");
    //Process data
    try{
        reqJSON = p.parse(req.substr(3, std::string::npos)).extract<Poco::JSON::Object::Ptr>();
        option = Poco::NumberParser::parseUnsigned(req.substr(0, 3));
#ifdef DEBUG
        commonOps::logMessage(methodName, "option: " + req.substr(0, 3), Poco::Message::PRIO_DEBUG);
#endif
        Poco::Data::MySQL::Connector::registerConnector();
        Poco::Data::Session session("MySQL", params.c_str());
        //Select option and do something.
        switch(option){
            /*
                100 - User subscription
                101 - User Login
                102 - e-mail Confirmation
                103 - Change password
                104 - Change user data
                105 - Link Google account
                106 - User logout
                107 - Logout from other sessions
                150 - Delete user
                301 - Add feed
                302 - List feeds
                303 - Edit feed (name, category)
                304 - Retrieve feeds
                305 - Delete feed
            */
            case 100:
                procJSON = subscription::subs(option, reqJSON, session, salt);
                break;
            case 101:
                procJSON = login::_login(option, reqJSON, session, salt);
                break;
            case 102:
                procJSON = emailConfirmation::eConf(option, reqJSON, session, salt);
                break;
            case 103:
                procJSON = redefPassword::redefine(option, reqJSON, session, salt);
                break;
            case 104:
                break;
            case 105:
                break;
            case 106:
                procJSON = logout::_logout(option, reqJSON, session, salt);
                break;
            case 107:
                procJSON = remSessions::removeSes(option, reqJSON, session, salt);
                break;
            case 150:
                procJSON = delUSR::deleteUSR(option, reqJSON, session, salt);
                break;
            case 301:
                procJSON = addFeed::add(option, reqJSON, session, salt);
                break;
            case 302:
                procJSON = listFeed::_list(option, reqJSON, session, salt);
                break;
            case 303:
                procJSON = updFeed::updateFeed(option, reqJSON, session, salt);
                break;
            case 304:
                procJSON = recFeed::recover(option, reqJSON, session, salt);
                break;
            case 305:
                procJSON = delFeed::deleteFeed(option, reqJSON, session, salt);
                break;
            default:
                std::string reason = "Unknown Option";
                reason += " -> ";
                reason += req.substr(0, 3);
                commonOps::logMessage("ServerOps::processReq", reason, Message::PRIO_INFORMATION);
                procJSON = commonOps::erroOpJSON(option, "unknown_option");
        }

    }catch(Poco::SyntaxException &e){
        std::string reason = "Poco SyntaxException";
        reason += " -> ";
        reason += e.message();
        commonOps::logMessage("ServerOps::processReq", reason, Message::PRIO_WARNING);
        procJSON = commonOps::erroOpJSON(option, "json_with_error");
    }catch(Poco::InvalidAccessException &e){
        std::string reason = "Poco InvalidAccessException";
        reason += " -> ";
        reason += e.message();
        commonOps::logMessage("ServerOps::processReq", reason, Message::PRIO_WARNING);
        procJSON = commonOps::erroOpJSON(option, "json_with_error");
    }catch(Poco::RangeException &e){
        std::string reason = "Poco RangeException";
        reason += " -> ";
        reason += e.message();
        commonOps::logMessage("ServerOps::processReq", reason, Message::PRIO_WARNING);
        procJSON = commonOps::erroOpJSON(option, "json_with_error");
    }catch(Poco::JSON::JSONException &e){
        std::string reason = "Poco JSONException";
        reason += " -> ";
        reason += e.message();
        commonOps::logMessage("ServerOps::processReq", reason, Message::PRIO_WARNING);
        procJSON = commonOps::erroOpJSON(option, "json_with_error");
    }catch(Poco::Data::DataException &e){
        std::string reason = "Poco DataException";
        reason += " -> ";
        reason += e.message();
        commonOps::logMessage("ServerOps::processReq", reason, Message::PRIO_CRITICAL);
        procJSON = commonOps::erroOpJSON(option, "data_exception");
    }catch(Poco::JWT::SignatureVerificationException &e){
        std::string reason = "Poco JWT SignatureVerificationExceprion";
        reason += "->";
        reason += e.message();
        commonOps::logMessage("ServerOps::processReq", reason, Message::PRIO_WARNING);
        procJSON = commonOps::erroOpJSON(option, "google_crypto_exception");
    }catch(Poco::Exception &e){
        std::string reason = "Poco Exception: ";
        reason += e.className();
        reason += " -> ";
        reason += e.message();
        commonOps::logMessage("ServerOps::processReq", reason, Message::PRIO_CRITICAL);
        procJSON = commonOps::erroOpJSON(option, "server_exception");
    }catch(GoogleAuthenticationException &e){
        std::string reason = e.what();
        reason += "->";
        reason += e.message();
        commonOps::logMessage("ServerOps::processReq", reason, Message::PRIO_WARNING);
        procJSON = commonOps::erroOpJSON(option, "invalid_request");
    }catch(std::exception &e){
        std::string reason = "std::exception";
        reason += " -> ";
        reason += e.what();
        commonOps::logMessage("ServerOps::processReq", reason, Message::PRIO_CRITICAL);
        procJSON = commonOps::erroOpJSON(option, "std_exception");
    }catch( ... ){
        commonOps::logMessage("ServerOps::processReq", "Unknown exception!", Message::PRIO_CRITICAL);
        procJSON = commonOps::erroOpJSON(option, "unknown_exception");
    }
    
    std::ostringstream streamg;
    procJSON->stringify(streamg, 0, -1);
    respJSON = streamg.str();
    return respJSON;
}
