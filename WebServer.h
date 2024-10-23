/*  Name: Sanket Makkar
    CaseID:         sxm1626
    File Name:      WebServer.h
    Date Created:   10/20/2024
    Description:    The purpose of this file is to define a header for the core functionality - i.e. the socket
                    work and arg-responses based on resulting socket work - as required by this assignment for
                    the server.
*/
#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <string>
#include <vector>
using namespace std;

class WebServer{
    private:
        // passed to constructor
        string portNum;
        string rootDir;
        string authToken;

        // core methods used behind the scenes
        void setupListening();
    public:
        WebServer(int argLine, string portNumber, string rootDirectory, string authenticationToken); // basic constructor
        ~WebServer() = default; // nothing special for destructor

        void grabFromNetwork(); // method used to orchestrate core functionality (grabbing something from a url, reacting to CLI args, and saving it to a file)
};

#endif // WEB_SERVER_H