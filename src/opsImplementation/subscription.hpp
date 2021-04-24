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

#ifndef SUBSHPP
#define SUBSHPP

#include "../PocoInclude.hpp"
#include "../PocoData.hpp"
#include "../commonOps.hpp"
#include "../GoogleAuthException.hpp"
#include "../secretText.hpp"

#include "emailConfirmation.hpp"

#include <Poco/NumberParser.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Nullable.h>
#include <Poco/SharedPtr.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/Dynamic/Var.h>
#include <Poco/JWT/Token.h>
#include <Poco/JWT/Signer.h>
#include <Poco/JWT/JWTException.h>
#include <Poco/Net/HTTPSClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/MediaType.h>
#include <Poco/Net/StringPartSource.h>
#include <Poco/Crypto/RSAKey.h>
#include <Poco/Crypto/X509Certificate.h>
#include <Poco/UUIDGenerator.h>
#include <istream>
#include <time.h>

class subscription{
    public:
    static Poco::JSON::Object::Ptr subs(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt);
    private:
    static void composeConfirmationEmail(Poco::Net::MailMessage &mailMessage, std::string clientEmail, std::string clientAuthID);
};

#endif