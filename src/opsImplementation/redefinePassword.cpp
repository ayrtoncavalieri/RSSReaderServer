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

#include "redefinePassword.hpp"

Poco::JSON::Object::Ptr redefPassword::redefine(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    Poco::JSON::Object::Ptr reqResp;
    Poco::JSON::Object::Ptr authIDJSON;
    Poco::JSON::Parser p;
    std::string userEmail, authID;
    std::string othersInfo("");
    const std::string methodName("redefPassword::redefine");
    std::ostringstream streamg;
    unsigned int countEmail = 0;

    try{
        userEmail = req->getValue<std::string>("email");
        reqResp = new Poco::JSON::Object;
        reqResp->set("op", op);
        reqResp->set("status", "OK");
        if(req->has("authId")){
            authID = req->getValue<std::string>("authId");
            session << "SELECT COUNT(*) FROM rssreader.users WHERE (email = ?)", into(countEmail), use(userEmail), now;
            if(countEmail != 0){
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
                std::string password(req->getValue<std::string>("password"));
                std::string rSalt = commonOps::genRsalt(16);
                password = commonOps::passwordCalc(password, salt, rSalt);
                authIDJSON->stringify(streamg, 0, -1);
                othersInfo = streamg.str();
                session << "UPDATE `rssreader`.`users` SET `userPassword` = ?, `rSalt` = ?, `othersInfo` = ?, `emailConfirmed` = 1 WHERE (`email` = ?)",
                use(password), use(rSalt), use(othersInfo), use(userEmail), now;
            }
        }else{
            // Send redefine password email
            session << "SELECT COUNT(*) FROM rssreader.users WHERE (email = ?)", into(countEmail), use(userEmail), now;
            if(countEmail != 0){
                session << "SELECT othersInfo FROM rssreader.users WHERE (email = ?)", into(othersInfo), use(userEmail), now;
                if(othersInfo.empty()){
                    commonOps::logMessage(methodName, "othersInfo is empty!", Poco::Message::PRIO_ERROR);
                    othersInfo = "{}";
                }
                std::string emailBody("<html><head>    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\">    <link        href=\"https://fonts.googleapis.com/css2?family=Roboto:ital,wght@0,100;0,300;0,400;0,500;0,700;0,900;1,100;1,300;1,400;1,500;1,700;1,900&display=swap\"        rel=\"stylesheet\" />    <style>        body {            font-family: 'Roboto', sans-serif;            width: 60%;            margin: auto;        }        @media only screen and (max-width: 900px) {            body {                width: 100%;            }        }    </style></head><body>    <p style=\"text-align: center; padding: 16px;\">        <img src=\"https://rssreader.aplikoj.com/assets/images/app_icon.png\" width=\"40%\" alt=\"RSS Reader Logo\" />    </p>    <h2 style=\"text-align: center;\">A password reset was requested</h2>    <p style=\"text-align: center; padding: 16px;\">        A password reset was requested for the account registered with our service, linked to this email        address.<br />To proceed with the reset, please click the button below:    </p>    <p style=\"text-align: center; padding: 16px;\"><a            href=\"https://rssreader.aplikoj.com/#/redefinePassword/redefine?authId={auth}&email={email}\"><img                src=\"https://rssreader.aplikoj.com/assets/images/redefine_password.png\" width=\"50%\"                alt=\"Reset password button\" /></a>    </p>    <p style=\"text-align: center; padding: 16px;\">        If you did not request this reset, ignore this message.    </p></body></html>");
                authIDJSON = p.parse(othersInfo).extract<Poco::JSON::Object::Ptr>();
                authID = commonOps::genAuthID(128);
                authIDJSON->set("authID", authID);
                session << "UPDATE `rssreader`.`users` SET `othersInfo` = ? WHERE (`email` = ?)", use(othersInfo), use(userEmail), now;
                
                Poco::Net::MailMessage mailMessage;
                mailMessage.addRecipient(Poco::Net::MailRecipient(Poco::Net::MailRecipient::PRIMARY_RECIPIENT, userEmail));
                mailMessage.setSubject(Poco::Net::MailMessage::encodeWord("Password Redefinition", "UTF-8"));
                std::string senderName = Poco::Net::MailMessage::encodeWord("RSS Reader [Do Not Reply]", "UTF-8");
                senderName += "<no-reply@rssreader.aplikoj.com>";
                mailMessage.setSender(senderName);
                Poco::Net::MediaType mediaType("text/html");
                mailMessage.setContentType(mediaType);
                emailBody.replace(emailBody.find("{auth}"), 6, authID);
                emailBody.replace(emailBody.find("{email}"), 7, userEmail);
                mailMessage.addPart("", new Poco::Net::StringPartSource(emailBody, "text/html"), Poco::Net::MailMessage::CONTENT_INLINE, Poco::Net::MailMessage::ENCODING_QUOTED_PRINTABLE);

                if(!emailConfirmation::sendEmail(mailMessage, "no-reply@rssreader.aplikoj.com", secretText::noReplyEmailPassword()))
                    commonOps::logMessage(methodName, "Could not sent the Redefine Password e-mail", Poco::Message::PRIO_ERROR);
            }
        }
    }catch(...){
        throw;
    }

    return reqResp;
}