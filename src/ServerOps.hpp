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

#ifndef SERVEROPS_H
#define SERVEROPS_H

#include <iostream>
#include <exception>
#include "PocoInclude.hpp"
#include "commonOps.hpp"
#include <Poco/NumberParser.h>
#include <Poco/NumberFormatter.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/Data/DataException.h>

class ServerOps
{
    public:
        ServerOps();
        ~ServerOps();
        std::string processReq(std::string &req);
    private:
        std::string salt;
};

#endif
