/*  Name: Sanket Makkar
    CaseID:         sxm1626
    File Name:      WebServer.h
    Date Created:   10/19/2024
    Description:    The purpose of this file is to define a header for the core functionality
                    as required by this assignment for the web-server.
*/
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

class WebServer{
    private:
        // passed to constructor
        string portNum;
        string rootDir;
        string authToken;

        // core methods used behind the scenes
        void setupListening(struct protoent* protoinfo, int &listenSD, struct sockaddr_in &sin);
        void awaitConnection(struct sockaddr* sdDataHolder, int &responseSD, int listenSD);
        bool respondToRequest(int &responseSD);
        
        // Helpers to the core methods which abstract away some of the complexity - essential components
        vector<string> readResponse(string &response, int &responseSD);
        unsigned char * getFile(string arg, string &response, int & size);

        // generic helpers
        int findOccurances(string toRead, string occurancesSubstring);
        void writeResponse(int socketDescriptor, string toWrite);
        void writeResponse(int socketDescriptor, unsigned char * toWrite, int length);
        unsigned char * grabFileContents(FILE * file, int & size);

    public:
        WebServer(int argLine, string portNumber, string rootDirectory, string authenticationToken); // basic constructor
        ~WebServer() = default; // nothing special for destructor

        void serverLive(int argc, char *argv []); // make the server live - core orchestrating method for functionality
};

#endif // WEB_SERVER_H