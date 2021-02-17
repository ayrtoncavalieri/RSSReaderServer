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

#include "subscription.hpp"

Poco::JSON::Object::Ptr subscription::subs(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    // {nome, email, senha}
    //Com G - {jwt:string}
    Poco::JSON::Object::Ptr reqResp;
    std::string email, name;
    try{
        if(req->has("jwt")){ //Google login
            std::string jwt = req->getValue<std::string>("jwt");
            Poco::Net::initializeSSL();
            Poco::Net::Context::Ptr pContext = new Poco::Net::Context(
                    Poco::Net::Context::CLIENT_USE,
                    "",
                    "",
                    "",
                    Poco::Net::Context::VERIFY_RELAXED,
                    9,
                    true,
                    "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
            Poco::Net::HTTPSClientSession googleJWT("www.googleapis.com", 443, pContext);
            Poco::Net::HTTPRequest googleJWTReq(Poco::Net::HTTPRequest::HTTP_GET, "/oauth2/v1/certs", Poco::Net::HTTPMessage::HTTP_1_1);
            googleJWT.sendRequest(googleJWTReq);
            Poco::Net::HTTPResponse googleJWTResp;
            std::istream& resp = googleJWT.receiveResponse(googleJWTResp);
            std::string gResponse(std::istreambuf_iterator<char>(resp), {});
            Poco::Net::uninitializeSSL();

            Poco::JSON::Parser p; 
            Poco::JSON::Object::Ptr gJSON = p.parse(gResponse).extract<Poco::JSON::Object::Ptr>();

            Poco::JSON::Object::ConstIterator i;
            Poco::JWT::Token emBlanco;
            for(i = gJSON->begin(); i != gJSON->end(); ++i){
                std::istringstream sKeyPEM(i->second.convert<std::string>());
                Poco::Crypto::X509Certificate certGoogle(sKeyPEM);
                Poco::SharedPtr<Poco::Crypto::RSAKey> ptrKey = new Poco::Crypto::RSAKey(certGoogle);
                Poco::JWT::Signer signer(ptrKey);
                signer.addAllAlgorithms();
                if(signer.tryVerify(jwt, emBlanco)){
                    break;
                }
            }
            if(i == gJSON->end()){
                Poco::JWT::SignatureVerificationException e;
                throw e;   
            }
            Poco::JSON::Object jwtJSON = emBlanco.payload();

            std::string picLink = jwtJSON.getValue<std::string>("picture");
            bool emailVerified = jwtJSON.getValue<bool>("email_verified");
            std::string iss = jwtJSON.getValue<std::string>("iss");
            name = jwtJSON.getValue<std::string>("name");
            long expTime = jwtJSON.getValue<long>("exp");
            long timeNow = time(NULL);

            if((iss.compare("accounts.google.com") && iss.compare("https://accounts.google.com")) || expTime < timeNow){
                //uonoau
            }

            reqResp = new Poco::JSON::Object;
            reqResp->set("login", "true");
            Poco::UUIDGenerator uuidGen;
            reqResp->set("uuid", uuidGen.createRandom().toString());
            
        }else{

        }
    }catch(...){
        throw;
    }
    return reqResp;
}