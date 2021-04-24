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

void subscription::composeConfirmationEmail(Poco::Net::MailMessage &mailMessage, std::string clientEmail, std::string clientAuthID)
{
    std::string emailMessage = "<html>  <head>     <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">     <link         href=\"https://fonts.googleapis.com/css2?family=Roboto:ital,wght@0,100;0,300;0,400;0,500;0,700;0,900;1,100;1,300;1,400;1,500;1,700;1,900&display=swap\"         rel=\"stylesheet\" />     <style>         body {             font-family: 'Roboto', sans-serif;             width: 60%;             margin: auto;         }          @media only screen and (max-width: 900px) {             body {                 width: 100%;             }         }     </style> </head>  <body>     <p style=\"text-align: center; padding: 16px;\"><img src=\"https://rssreader.aplikoj.com/assets/images/app_icon.png\"             width=\"40%\" alt=\"RSS Reader Logo\" />     </p>     <h2 style=\"text-align: center;\">Confirm your registration</h2>     <p style=\"text-align: center; padding: 16px;\">         Thank you for signing up for our service, we love your interest and ask you to confirm your email address so we can get in touch when necessary (we promise not to send any unsolicited email).<br />To confirm your email address, please click the button below:     </p>     <p style=\"text-align: center; padding: 16px;\"><a             href=\"https://rssreader.aplikoj.com/#/confirmEmail/confirm?authId={auth}&email={email}\"><img                 src=\"https://rssreader.aplikoj.com/assets/images/confirm_email.png\" width=\"50%\"                 alt=\"Confirm email button\" /></a>     </p>     <p style=\"text-align: center; padding: 16px;\">         If you have not completed this registration, please contact us through <a href=\"mailto:contato@aplikoj.com\">contato@aplikoj.com</a> so that we can solve.     </p> </body>  </html>";

    mailMessage.addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, clientEmail));
    mailMessage.setSubject(Poco::Net::MailMessage::encodeWord("e-mail Confirmation", "UTF-8"));
    std::string senderName = Poco::Net::MailMessage::encodeWord("RSS Reader [Do Not Reply]", "UTF-8");
    senderName += "<no-reply@rssreader.aplikoj.com>";
    mailMessage.setSender(senderName);
    Poco::Net::MediaType mediaType("text/html");
    mailMessage.setContentType(mediaType);
    emailMessage.replace(emailMessage.find("{auth}"), 6, clientAuthID);
    emailMessage.replace(emailMessage.find("{email}"), 7, clientEmail);
    mailMessage.addPart("", new Poco::Net::StringPartSource(emailMessage, "text/html"), Poco::Net::MailMessage::CONTENT_INLINE, Poco::Net::MailMessage::ENCODING_QUOTED_PRINTABLE);
}

Poco::JSON::Object::Ptr subscription::subs(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    // {nome, email, senha}
    //Com G - {jwt:string}
    Poco::JSON::Object::Ptr reqResp;
    std::string email, name, confirmationID;
    std::string othersInfo;
    Poco::Net::MailMessage mailMessage;
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
            Poco::Net::HTTPSClientSession googleJWT("www.googleapis.com", Poco::Net::HTTPSClientSession::HTTPS_PORT, pContext);
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
                session << "INSERT INTO `rssreader`.`users` (`email`, `emailConfirmed`, `userName`, `idGoogle`, `othersInfo`, `linkPhoto`) VALUES (?, ?, ?, ?, ?, ?)", 
                            use(email), use(emailVerified), use(name), use(sub), use(othersInfo), use(picLink), now;
                if(emailVerified == false){
                    confirmationID = commonOps::genAuthID(128);
                    emailConfirmation::sendEmail(mailMessage, "no-reply@rssreader.aplikoj.com", secretText::noReplyEmailPassword());
                    othersInfo = "{\"authID\":";
                    othersInfo += "\"" + confirmationID + "\"}";
                }else{
                    othersInfo = "{}";
                }
            }else{
                if(qtdUsers != 0){
                    session << "UPDATE `rssreader`.`users` SET `idGoogle` = ? WHERE (`email` = ?)", use(sub), use(email), now;
                    loginType = "server, ";
                }
                session << "SELECT email, linkPhoto, userName, settings, emailConfirmed FROM rssreader.users WHERE idGoogle=?", 
                            into(email), into(picLink), into(name), into(userSettings), into(emailVerified), use(sub), now;
            }

            Poco::UUIDGenerator uuidGen;
            std::string uuid = uuidGen.createRandom().toString();
            std::string hashNavigator = commonOps::passwordCalc(email, uuid, "");
            session << "INSERT INTO `rssreader`.`navigators` (`email`, `uuid`, `hashNavigator`) VALUES (?, ?, ?)", use(email), use(uuid), use(hashNavigator), now;
            unsigned int qtdLinks;
            session << "SELECT COUNT(*) FROM rssreader.links WHERE email=?", into(qtdLinks), use(email), now;
            reqResp = new Poco::JSON::Object;
            loginType += "provider";
            reqResp->set("login", loginType);
            reqResp->set("variable", hashNavigator);
            reqResp->set("uuid", uuid);
            reqResp->set("feeds", qtdLinks);
            reqResp->set("pic", picLink);
            reqResp->set("name", name);
            reqResp->set("verified", emailVerified);
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
            confirmationID = commonOps::genAuthID(128);
            othersInfo = "{\"authID\":";
            othersInfo += "\"" + confirmationID + "\"}";

            session << "INSERT INTO `rssreader`.`users` (`email`, `userName`, `userPassword`, rSalt, `othersInfo`, `linkPhoto`) VALUES (?, ?, ?, ?, ?, '')",
                        use(email), use(name), use(password), use(rSalt), use(othersInfo),now;
            std::string hashNavigator = commonOps::passwordCalc(email, uuid, "");
            session << "INSERT INTO `rssreader`.`navigators` (`email`, `uuid`, `hashNavigator`) VALUES (?, ?, ?)", use(email), use(uuid), use(hashNavigator), now;
            unsigned int qtdLinks;

            reqResp = new Poco::JSON::Object;
            reqResp->set("login", "server");
            reqResp->set("variable", hashNavigator);
            reqResp->set("uuid", uuid);
            reqResp->set("feeds", 0);
            reqResp->set("pic", "");
            reqResp->set("name", name);
            reqResp->set("verified", false);

            emailConfirmation::sendEmail(mailMessage, "no-reply@rssreader.aplikoj.com", secretText::noReplyEmailPassword());
            
        }
    }catch(...){
        throw;
    }
    return reqResp;
}