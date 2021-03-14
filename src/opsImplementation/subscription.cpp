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
            Poco::Net::uninitializeSSL();

            if(googleJWTResp.getStatus() != Poco::Net::HTTPResponse::HTTP_OK){
                Poco::JWT::SignatureVerificationException e;
                throw e; 
            }
            std::string gResponse(std::istreambuf_iterator<char>(resp), {});

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
            std::string iss = jwtJSON.getValue<std::string>("iss");
            long expTime = jwtJSON.getValue<long>("exp");
            long timeNow = time(NULL);
            std::string aud;
            aud = jwtJSON.getValue<std::string>("aud");

            if((iss.compare("accounts.google.com") && iss.compare("https://accounts.google.com")) || expTime < timeNow || aud.compare(secretText::audVal())){
                GoogleAuthenticationException e;
                throw e;
            }

            name = jwtJSON.getValue<std::string>("name");
            email = jwtJSON.getValue<std::string>("email");
            bool emailVerified = jwtJSON.getValue<bool>("email_verified");
            std::string picLink = jwtJSON.getValue<std::string>("picture");
            std::string sub = jwtJSON.getValue<std::string>("sub"); //Adicionar no idGoogle
            std::string idGoogle = "";
            session << "SELECT idGoogle FROM rssreader.users WHERE idGoogle=?", into(idGoogle), use(sub), now;
            unsigned int qtdUsers;
            session << "SELECT COUNT(*) FROM rssreader.users WHERE email=?", into(qtdUsers), use(email), now; 
            std::string userSettings = "";
            std::string loginType = "";
            if(idGoogle.empty() && qtdUsers == 0){
                session << "INSERT INTO `rssreader`.`users` (`email`, `emailConfirmed`, `userName`, `idGoogle`, `othersInfo`, `linkPhoto`) VALUES (?, ?, ?, ?, '{}', ?)", 
                            use(email), use(emailVerified), use(name), use(sub), use(picLink), now;
                if(!emailVerified){
                    //Send confirmation e-mail
                }
            }else{
                if(qtdUsers != 0){
                    session << "UPDATE `rssreader`.`users` SET `idGoogle` = ? WHERE (`email` = ?)", use(sub), use(email), now;
                    loginType = "server, ";
                }
                session << "SELECT email, linkPhoto, userName, settings FROM rssreader.users WHERE idGoogle=?", 
                            into(email), into(picLink), into(name), into(userSettings), use(sub), now;
            }

            Poco::UUIDGenerator uuidGen;
            std::string uuid = uuidGen.createRandom().toString();
            
            session << "INSERT INTO `rssreader`.`navigators` (`email`, `uuid`) VALUES (?, ?)", use(email), use(uuid), now;
            unsigned int qtdLinks;
            session << "SELECT COUNT(*) FROM rssreader.links WHERE email=?", into(qtdLinks), use(email), now;
            reqResp = new Poco::JSON::Object;
            loginType += "provider";
            reqResp->set("login", loginType);
            reqResp->set("email", email);
            reqResp->set("uuid", uuid);
            reqResp->set("feeds", qtdLinks);
            reqResp->set("pic", picLink);
            reqResp->set("name", name);
            if(!userSettings.empty()){
                reqResp->set("settings", userSettings);
            }
        }else{
            Poco::UUIDGenerator uuidGen;

            email = req->getValue<std::string>("email");
            unsigned int qtdUsers;
            session << "SELECT COUNT(*) FROM rssreader.users WHERE email=?", into(qtdUsers), use(email), now;
            if(qtdUsers != 0){
                return commonOps::erroOpJSON(op, "user_already_exists");
            }
            std::string uuid = uuidGen.createRandom().toString();
            name = req->getValue<std::string>("name");
            std::string password = req->getValue<std::string>("password");
            std::string rSalt = commonOps::genRsalt(16);
            password = commonOps::passwordCalc(password, salt, rSalt);
            session << "INSERT INTO `rssreader`.`users` (`email`, `userName`, `userPassword`, rSalt, `othersInfo`, `linkPhoto`) VALUES (?, ?, ?, ?, '{}', '')",
                        use(email), use(name), use(password), use(rSalt), now;
            session << "INSERT INTO `rssreader`.`navigators` (`email`, `uuid`) VALUES (?, ?)", use(email), use(uuid), now;
            unsigned int qtdLinks;

            reqResp = new Poco::JSON::Object;
            reqResp->set("login", "server");
            reqResp->set("email", email);
            reqResp->set("uuid", uuid);
            reqResp->set("feeds", 0);
            reqResp->set("pic", "");
            reqResp->set("name", name);
            //Enviar e-mail de confirmaçãos
        }
    }catch(...){
        throw;
    }
    return reqResp;
}