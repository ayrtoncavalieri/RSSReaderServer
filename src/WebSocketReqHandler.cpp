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

#include "WebSocketReqHandler.hpp"

void WebSocketRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    Application& app = Application::instance();
    Poco::Message msn;
    msn.setSource("WebSocketRequestHandler");
    Timespan timeOut(0, 30 * ONESEC);
    response.set("Sec-WebSocket-Protocol", "PDRAUM");
    WebSocket ws(request, response);
    std::string incomeBuf;
    try
    {
        ws.setReceiveTimeout(timeOut);
        ws.setSendTimeout(timeOut);
        msn.setText("WebSocket connection established");
        msn.setPriority(Poco::Message::PRIO_INFORMATION);
        app.logger().log(msn);
        char buffer[BUFSIZE + 1];
        int flags;
        int n;
        int frames;
        std::string respJSON;
        frames = 0;
        buffer[BUFSIZE] = '\0';
        Poco::Buffer<char> buff(0);
        //Receive frames
        ws.setMaxPayloadSize(BUFSIZE);
        unsigned int pingCount = 0;
        while(buff.size() <= MAXFRAMES * BUFSIZE){
            //memset(buffer, '\0', BUFSIZE);
            std::cout << ws.getReceiveBufferSize() << " bytes[1]\n";
            //n = ws.receiveFrame(buffer, BUFSIZE, flags);
            n = ws.receiveFrame(buff, flags);
            std::cout << "Received " << n << " bytes\n";
            std::cout << ws.getReceiveBufferSize() << " bytes[2]\n";
            if((flags & WebSocket::FRAME_OP_BITMASK) == WebSocket::FRAME_OP_PING){ // Process PINGS
                msn.setText(Poco::format("flags & WebSocket::FRAME_OP_BITMASK (flags=0x%x).", unsigned(flags & WebSocket::FRAME_OP_PING)));
                msn.setPriority(Poco::Message::PRIO_DEBUG);
                app.logger().log(msn);
                msn.setText("A PING");
                msn.setPriority(Poco::Message::PRIO_DEBUG);
                app.logger().log(msn);
                flags = (WebSocket::FRAME_FLAG_FIN | WebSocket::FRAME_OP_PONG);
                msn.setText(Poco::format("flags = 0x%x", unsigned(flags)));
                msn.setPriority(Poco::Message::PRIO_DEBUG);
                app.logger().log(msn);
                pingCount++;
                ws.sendFrame("", 0, flags);
            }else if(buff.begin()[buff.size() - 1] == 0x04 || n == 0){ // Received EOT and ending income
                buff.begin()[buff.size() - 1] = '\0';
                break;
            }else{ // Adding income to buffer
                frames += n;
            }
            if(pingCount == MAXPINGS){
                break;
            }
        }
        if(n != 0 && frames <= MAXFRAMES * BUFSIZE && pingCount < MAXPINGS){
            //Process requisitions
            ServerOps srv;
            incomeBuf = buff.begin();
            respJSON = srv.processReq(incomeBuf);

            if(respJSON.length() > BUFSIZE){ //Sending a big response
                int i;
                std::string _send;
                for(i = 0; i < respJSON.length(); i += BUFSIZE){
                    _send = respJSON.substr(i, BUFSIZE);
                    flags = WebSocket::FRAME_FLAG_FIN | WebSocket::FRAME_OP_TEXT;
                    ws.sendFrame(_send.c_str(), _send.length(), flags);
                }
            }else{ //Sending a small response
                flags = WebSocket::FRAME_FLAG_FIN | WebSocket::FRAME_OP_TEXT;
                ws.sendFrame(respJSON.c_str(), respJSON.length(), flags);
            }
            ws.shutdown(1000, "");
            n = ws.receiveFrame(buffer, BUFSIZE, flags);
            if((flags & WebSocket::FRAME_OP_BITMASK) != WebSocket::FRAME_OP_CLOSE){

            }
            msn.setText("WebSocket connection closed.");
            msn.setPriority(Poco::Message::PRIO_INFORMATION);
            app.logger().log(msn);
        }else if(n == 0 || pingCount == MAXPINGS){
            ws.close();
        }else{ //Warning sender that the payload is too big
            msn.setText("PAYLOAD too big![1]" + incomeBuf);
            msn.setPriority(Poco::Message::PRIO_INFORMATION);
            app.logger().log(msn);
            msn.setText("Content: " + incomeBuf);
            msn.setPriority(Poco::Message::PRIO_DEBUG);
            app.logger().log(msn);
            ws.shutdown(1009, "WS_PAYLOAD_TOO_BIG");
        }
        ws.close();
    }catch (WebSocketException& exc){
        std::string name = exc.what();
        msn.setSource("WebSocketRequestHandler, " + name);
        msn.setText(exc.message());
        msn.setPriority(Poco::Message::PRIO_WARNING);
        app.logger().log(msn);
        switch (exc.code())
        {
        case WebSocket::WS_ERR_HANDSHAKE_UNSUPPORTED_VERSION:
            response.set("Sec-WebSocket-Version", WebSocket::WEBSOCKET_VERSION);
            // fallthrough
        case WebSocket::WS_ERR_NO_HANDSHAKE:
        case WebSocket::WS_ERR_HANDSHAKE_NO_VERSION:
        case WebSocket::WS_ERR_HANDSHAKE_NO_KEY:
            response.setStatusAndReason(HTTPResponse::HTTP_BAD_REQUEST);
            response.setContentLength(0);
            response.send();
            break;
        case WebSocket::WS_ERR_PAYLOAD_TOO_BIG:
            msn.setText("PAYLOAD too big![2]");
            msn.setPriority(Poco::Message::PRIO_INFORMATION);
            app.logger().log(msn);
            msn.setText("Content: " + incomeBuf);
            msn.setPriority(Poco::Message::PRIO_DEBUG);
            app.logger().log(msn);
            ws.shutdown(1009, "WS_PAYLOAD_TOO_BIG");
            break;
        }
        ws.close();
    }catch (TimeoutException& e){
        std::string name = e.what();
        msn.setSource("WebSocketRequestHandler, " + name);
        msn.setText(e.message());
        msn.setPriority(Poco::Message::PRIO_WARNING);
        app.logger().log(msn);
    }catch(NetException &e){
        std::string name = e.what();
        msn.setSource("WebSocketRequestHandler, " + name);
        msn.setText(e.message());
        msn.setPriority(Poco::Message::PRIO_WARNING);
        app.logger().log(msn);
    }
}
