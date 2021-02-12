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

#include "ReqHandlerFactory.hpp"

HTTPRequestHandler* RequestHandlerFactory::createRequestHandler(const HTTPServerRequest &request)
{
    Application& app = Application::instance();
    Message msn("ReqHandlerFactory", "Request from "
        + request.clientAddress().toString()
        + ": "
        + request.getMethod()
        + " "
        + request.getURI()
        + " "
        + request.getVersion(), Message::PRIO_INFORMATION);
    app.logger().log(msn);
#ifdef TRACE
    for (HTTPServerRequest::ConstIterator it = request.begin(); it != request.end(); ++it)
    {
        msn.setSource("ReqHandlerFactory");
        msn.setText(it->first + ": " + it->second);
        msn.setPriority(Poco::Message::PRIO_DEBUG);
        app.logger().log(msn);
    }
#endif
    if(request.find("Upgrade") != request.end() && Poco::icompare(request["Upgrade"], "websocket") == 0
    && request.find("sec-websocket-protocol") != request.end() && Poco::icompare(request["sec-websocket-protocol"], "PDRAUM") == 0)
        return new WebSocketRequestHandler;
    else
        return new PageRequestHandler;
}
