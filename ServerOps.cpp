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

ServerOps::ServerOps(): salt("S417_3*ampl3")
{

}

ServerOps::~ServerOps()
{

}

std::string ServerOps::processReq(std::string &req)
{
    std::string respJSON; //32 bytes
    Poco::JSON::Object::Ptr reqJSON, procJSON; //16 bytes * 2  
    Poco::JSON::Parser p; //40 bytes
    unsigned int option = 0; //4 bytes
    //Process data
    try{
        reqJSON = p.parse(req.substr(3, std::string::npos)).extract<Poco::JSON::Object::Ptr>();
        option = Poco::NumberParser::parse(req.substr(0, 3));

        //Select option and do something.

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
    }catch(Poco::Exception &e){
        std::string reason = "Poco Exception: ";
        reason += e.className();
        reason += " -> ";
        reason += e.message();
        commonOps::logMessage("ServerOps::processReq", reason, Message::PRIO_CRITICAL);
        procJSON = commonOps::erroOpJSON(option, "server_exception");
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
