#include "subscription.hpp"

Poco::JSON::Object::Ptr subscription::subs(unsigned int op, Poco::JSON::Object::Ptr req, Poco::Data::Session &session, std::string salt)
{
    // {nome, email, senha}
    //Com G - {jwt:string}
    Poco::JSON::Object::Ptr reqResp;
    try{
        if(req->has("jwt")){ //Google login
            std::string jwt = req->getValue<std::string>("jwt");
            Poco::Net::initializeSSL();
            Poco::Net::Context::Ptr pContext = new Poco::Net::Context(
                    Poco::Net::Context::CLIENT_USE,
                    "",
                    "",
                    "",
                    Poco::Net::Context::VERIFY_RELAXED,
                    9,
                    true,
                    "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
            Poco::Net::HTTPSClientSession googleJWT("www.googleapis.com", 443, pContext);
            Poco::Net::HTTPRequest googleJWTReq(Poco::Net::HTTPRequest::HTTP_GET, "/oauth2/v1/certs", Poco::Net::HTTPMessage::HTTP_1_1);
            googleJWT.sendRequest(googleJWTReq);
            Poco::Net::HTTPResponse googleJWTResp;
            std::istream& resp = googleJWT.receiveResponse(googleJWTResp);
            std::string gResponse(std::istreambuf_iterator<char>(resp), {});
            Poco::Net::uninitializeSSL();

            Poco::JSON::Parser p; 
            Poco::JSON::Object::Ptr gJSON = p.parse(gResponse).extract<Poco::JSON::Object::Ptr>();

            Poco::JSON::Object::ConstIterator i;
            Poco::JWT::Token emBlanco;
            for(i = gJSON->begin(); i != gJSON->end(); ++i){
                std::istringstream sKeyPEM(i->second.convert<std::string>());
                Poco::Crypto::X509Certificate certGoogle(sKeyPEM);
                Poco::SharedPtr<Poco::Crypto::RSAKey> ptrKey = new Poco::Crypto::RSAKey(certGoogle);
                Poco::JWT::Signer signer(ptrKey);
                signer.addAllAlgorithms();
                if(signer.tryVerify(jwt, emBlanco)){
                    break;
                }
            }
            if(i == gJSON->end()){
                Poco::JWT::SignatureVerificationException e;
                throw e;   
            }
            Poco::JSON::Object jwtJSON = emBlanco.payload();
            
            reqResp = new Poco::JSON::Object;
            reqResp->set("login", "true");
            Poco::UUIDGenerator uuidGen;
            reqResp->set("uuid", uuidGen.createRandom().toString());
            
            
        }else{

        }
    }catch(...){
        throw;
    }
    return reqResp;
}