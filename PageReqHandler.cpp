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

#include "PageReqHandler.hpp"

void PageRequestHandler::handleRequest(HTTPServerRequest &request, HTTPServerResponse &response)
{
	//response.setStatusAndReason(response.HTTP_BAD_REQUEST);
    response.setChunkedTransferEncoding(true);
		response.setContentType("text/html");
		std::ostream& ostr = response.send();
		ostr << "<html>";
		ostr << "<head>";
		ostr << "<title>WebSocketServer</title>";
		ostr << "</head>";
		ostr << "<body onload=\"redirect()\">";
		ostr << "  <p></p>";
		ostr << "<script type=\"text/javascript\">";
		ostr << "function redirect(){";
		ostr << "window.location.replace(\"http://www.aplikoj.com/\");";
		ostr << "}";
		ostr << "</script>";
		ostr << "</body>";
		ostr << "</html>";
}
