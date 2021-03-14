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

#ifndef LOGINHPP
#define LOGINHPP

#include "../PocoInclude.hpp"
#include "../PocoData.hpp"
#include "../commonOps.hpp"
#include "../secretText.hpp"

#include "subscription.hpp"

class login{
    public:
        static Poco::JSON::Object::Ptr _login(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt);
};

#endif