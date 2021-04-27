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

#ifndef SERVER_H
#define SERVER_H

#include <iostream>
#include <vector>
#include "PocoInclude.hpp"
#include "ReqHandlerFactory.hpp"
#include <Poco/AutoPtr.h>
#include <Poco/FileChannel.h>
#include <Poco/AsyncChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/FormattingChannel.h>
#include <stdlib.h>
#include <time.h>


class WebSocketServer: public Poco::Util::ServerApplication
{
    public:
        WebSocketServer();
        const char* name();
        ~WebSocketServer();

    protected:
       void initialize(Application &self);
       void uninitialize();
       void defineOptions(OptionSet &options);
       void handleOption(const std::string &name, const std::string &value);
       void displayHelp();
       int main(const std::vector<std::string>& args);

    private:
        bool _helpRequested;
};

#endif 
//SERVER_H
