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

#ifndef DELUSRHPP
#define DELUSRHPP

#include "../PocoInclude.hpp"
#include "../PocoData.hpp"
#include "../commonOps.hpp"
#include "../secretText.hpp"
#include "login.hpp"
#include "silentLogin.hpp"

#include <Poco/NumberParser.h>
#include <Poco/NumberFormatter.h>
#include <Poco/Nullable.h>
#include <Poco/SharedPtr.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/Dynamic/Var.h>
#include <istream>
#include <time.h>

class delUSR{
    public:
    static Poco::JSON::Object::Ptr deleteUSR(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt);
};

#endif
