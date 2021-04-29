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

#ifndef WEBSOCKREQHAND_H
#define WEBSOCKREQHAND_H

#include <iostream>

#include "PocoInclude.hpp"
#include "ServerOps.hpp"

#include <Poco/Buffer.h>

#define BUFSIZE 65536
#define MAXFRAMES 160
#define ONESEC 1000000
#define MAXPINGS 6

class WebSocketRequestHandler: public HTTPRequestHandler
{
    public: 
        void handleRequest(HTTPServerRequest& request, HTTPServerResponse& response);
};

#endif
