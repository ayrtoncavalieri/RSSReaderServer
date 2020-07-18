#include "WebSocketReqHandler.hpp"

void WebSocketRequestHandler::handleRequest(HTTPServerRequest& request, HTTPServerResponse& response)
{
    Application& app = Application::instance();
    Poco::Message msn;
    msn.setSource("WebSocketRequestHandler");
    Timespan timeOut(0, 30 * ONESEC);
    response.set("Sec-WebSocket-Protocol", "PDRAUM");
    WebSocket ws(request, response);
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
        std::string incomeBuf;
        std::string respJSON;
        frames = 0;
        buffer[BUFSIZE] = '\0';
        //Receive frames
        ws.setMaxPayloadSize(BUFSIZE);
        while(frames <= MAXFRAMES){
            memset(buffer, '\0', BUFSIZE);
            n = ws.receiveFrame(buffer, BUFSIZE, flags);
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
                n = 1;
                ws.sendFrame(buffer, 0, flags);
            }else if(buffer[0] == 0x04){ // Received EOT and ending income
                break;
            }else{ // Adding income to buffer
                ++frames;
                incomeBuf += buffer;
            }
        }
        if(frames <= MAXFRAMES){
            //Process requisitions
            ServerOps srv;
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
        }else{ //Warning sender that the payload is too big
            msn.setText("PAYLOAD too big!");
            msn.setPriority(Poco::Message::PRIO_INFORMATION);
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
            msn.setText("PAYLOAD too big!");
            msn.setPriority(Poco::Message::PRIO_INFORMATION);
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