#!/bin/sh

g++ -g -O2 -DDEBUG -DSECSRV '-I src/' '-I src/opsImplementation' src/opsImplementation/*.cpp src/*.cpp -o RSSReaderServer.out -lPocoFoundation -lPocoNet -lPocoUtil -lPocoNetSSL -lPocoCrypto -lPocoData -lPocoDataMySQL -lPocoJSON -lPocoJWT