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

#ifndef COMMOPS_H
#define COMMOPS_H

#include "PocoInclude.hpp"
#include <cstdlib>
#include <Poco/NumberFormatter.h>
#include <Poco/JSON/Object.h>
#include <Poco/Crypto/DigestEngine.h>

using namespace Poco::JSON;

class commonOps{
    public:
        static std::string passwordCalc(std::string pass, std::string salt, std::string rSalt = "");
        static std::string genRsalt(unsigned int tam);
        static std::string genAuthID(unsigned int tam);
        static Object::Ptr erroOpJSON(unsigned int op, std::string erro);
        static void logMessage(std::string who, std::string what, Message::Priority howBad);
    private:
        static std::string SHA3Wrapper(std::string str);

};

#endif
