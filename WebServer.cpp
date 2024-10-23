/*  Name: Sanket Makkar
    CaseID:         sxm1626
    File Name:      WebServer.cpp
    Date Created:   9/27/2024
    Description:    The purpose of this file is to implement the core functionality - i.e. the socket work and
                    arg-responses based on resulting socket work - as required by this assignment. This file in 
                    particular implements the methods intended to do this work as defined within the WebServer.h
                    header file.
*/
#include "WebServer.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

// protocol information
#define PROTOCOL "tcp"

// listening information
#define BACKUP_LISTENING_QUEUE_LENGTH 1

// Argument Flags
#define ARG_PORT 0x1
#define ARG_ROOT 0x2
#define ARG_AUTH_TOKEN 0x4

// Helpful variables for initialization of counters, handling off-by-one errors, and comparisons
#define COUNTER_INITIAL_VALUE 0
#define OFF_BY_ONE_OFFSET 1
#define DECREMENT -1
#define FUNCTION_ERROR_RETURN_VALUE -1
#define BUFLEN 1024

// exit options
#define exitWithErr exit(FUNCTION_ERROR_RETURN_VALUE)
#define exitWithNoErr exit(COUNTER_INITIAL_VALUE)

using namespace std;

WebServer::WebServer(int argLine, string portNumber, string rootDirectory, string authenticationToken) : portNum(portNumber), rootDir(rootDirectory), authToken(authenticationToken){}

void WebServer::serverLive(int argc, char *argv []){
    // data structures we will use
    struct sockaddr_in sin; // socket in address
    struct sockaddr addr; // socket address
    struct protoent *protoinfo; // holds protocol information
    int listenSD, responseSD; // one socket for listening, the other for responding

    // protocol info needs to be grabbed
    if ((protoinfo = getprotobyname (PROTOCOL)) == NULL){
        fprintf(stderr, "cannot find protocol information for %s", PROTOCOL);
        exitWithErr;
    }

    // lets get listening on the right port number
    setupListening(protoinfo, listenSD, sin);

    // and prepare for a new connection
    awaitConnection(&addr, responseSD, listenSD);

    respondToRequest(responseSD);
    
    // and when we are done lets wrap everything up
    close (listenSD);
    close (responseSD);
    exitWithNoErr;
}

void WebServer::setupListening(struct protoent* protoinfo, int &listenSD, struct sockaddr_in &sin){
    // socket in info -- note that we set up the socket to work on the given port number
    memset ((char *)&sin,0x0,sizeof (sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    sin.sin_port = htons ((u_short) atoi (portNum.c_str()));

    // allocate our socket (will operate on portNum)
    listenSD = socket(PF_INET, SOCK_STREAM, protoinfo->p_proto);
    if (listenSD < 0){
        fprintf(stderr, "Error: cannot create socket\n");
        exitWithErr;
    }

    // now bind (force socket to operate as specified by) the sin
    if (bind (listenSD, (struct sockaddr *)&sin, sizeof(sin)) < 0){
        fprintf(stderr, "cannot bind to port %s\n", portNum.c_str());
        exitWithErr;
    }
    
    // start listening
    if (listen(listenSD, BACKUP_LISTENING_QUEUE_LENGTH) < 0){
        fprintf(stderr, "cannot listen on port %s\n", portNum.c_str());
        exitWithErr;
    }
}

void WebServer::awaitConnection(struct sockaddr* sdDataHolder, int &responseSD, int listenSD){
    // block execution until a connection is made
    unsigned int addrlen = sizeof (sdDataHolder);
    responseSD = accept(listenSD,sdDataHolder,&addrlen);
    if (responseSD < 0){
        fprintf(stderr, "error accepting connection");
        exitWithErr;
    }
}

void WebServer::respondToRequest(int &responseSD){
    // We will, at some point, send a string response
    string response = "HTTP/1.1 400 Malformed Request\r\n\r\n";

    readResponse(response, responseSD);

    // send back the message
    if (write(responseSD, wholeRequest.data(), wholeRequest.size()) < 0) {
        exitWithErr;
    }
}

int WebServer::findOccurances(string toRead, string occurancesSubstring){
    string copyToRead = toRead;
    int found;
    int totalFound = 0;
    while (found = copyToRead.find(occurancesSubstring.c_str()) > -1){
        totalFound++;
        copyToRead = copyToRead.substr(found + OFF_BY_ONE_OFFSET);
    }
    return totalFound;
}

void WebServer::readResponse(string &response, int &responseSD){
    // We will only format the response if the entire response is not malformed

    // first we want to grab the whole request
    vector<unsigned char> wholeRequest;
    char buf[BUFLEN];
    size_t bytesRead;

    bool noDoubleNewline = false;
    while ((bytesRead = read(responseSD, buf, sizeof(buf)))){
        wholeRequest.insert(wholeRequest.end(), buf, buf + bytesRead); 
        string wholeStr(wholeRequest.begin(), wholeRequest.end());
        
        // if we don't find a "\r\n\r\n" within a reasonable frame
        if (wholeStr.length() > BUFLEN * BUFLEN){
            noDoubleNewline = true;
            break;
        }

        if (int(wholeStr.find("\r\n\r\n")) > -1)
            break;
    }
    
    /* First Check Malformed Request*/
    // No found "\r\n\r\n"
    if (!noDoubleNewline)
        break;

    // each line not terminated by \r\n
    string totalRequest = str(wholeRequest.begin(), wholeRequest.end());
    if ((findOccurances(totalRequest, ":") + OFF_BY_ONE_OFFSET) >= findOccurances(totalRequest, "\r\n")){
        break;
    }

    // the first line is not of form "METHOD ARGUMENT HTTP/VERSION\r\n"
    string firstLine = totalRequest.substr(COUNTER_INITIAL_VALUE, totalRequest.find("\r\n"));
    if (findOccurances(firstLine, " ") != 2) // this means we don't have two separators - so not matching form
        break;

    string versionString = firstLine.substr(firstLine.rfind(" "));
    if (versionString.find("/") < 0) // if we don't have a '/' in the version, it is not of the same form
        break;

    /* Second Check Protocol Not Implemented */
    if (versionString.substr(COUNTER_INITIAL_VALUE, strlen("HTTP/") + OFF_BY_ONE_OFFSET) != "HTTP/") // if we don't have the HTTP/
        response = "HTTP/1.1 501 Protocol Not Implemented\r\n\r\n";
        break;
    
    /* Third Check Unsupported Method */
    
}