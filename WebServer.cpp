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

// methods available for user
#define VALID_GET_METHOD "GET"
#define VALID_SHUTDOWN_METHOD "SHUTDOWN"

// possible headers
#define MALFORMED "HTTP/1.1 400 Malformed Request\r\n\r\n"
#define UNIMPLEMENTED_PROTOCOL "HTTP/1.1 501 Protocol Not Implemented\r\n\r\n"
#define UNSUPPORTED_METHOD "HTTP/1.1 405 Unsupported Method\r\n\r\n"
#define INVALID_FILE_NAME "HTTP/1.1 406 Invalid Filename\r\n\r\n"
#define FILE_NOT_FOUND "HTTP/1.1 404 File Not Found\r\n\r\n"
#define OK_HEADER "HTTP/1.1 200 OK\r\n\r\n"

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
    string response = MALFORMED;

    // consider response formatting requirements
    bool goodFormatting = true;
    vector<string> methodAndArg = readResponse(response, responseSD, goodFormatting);
    string method = methodAndArg.at(COUNTER_INITIAL_VALUE);
    string arg = methodAndArg.at(COUNTER_INITIAL_VALUE + OFF_BY_ONE_OFFSET);

    /* Handle GET and SHUTDOWN cases */
    if (method == VALID_GET_METHOD){
        
        char * fileBuff;
        fileBuff = getFile(arg, response, goodFormatting);

        // send back the header
        if (write(responseSD, response.c_str(), response.size()) < 0) {
            exitWithErr;
        }
        // and don't forget the file
        if (fileBuff != NULL){
            string strFileBuff = fileBuff;
            if (write(responseSD, fileBuff, strFileBuff.length()) < 0){
                exitWithErr;
            }
        }
    }
    
}

int WebServer::findOccurances(string toRead, string occurancesSubstring){
    string copyToRead = toRead;
    int found;
    int totalFound = 0;
    while ((found = copyToRead.find(occurancesSubstring.c_str())) > -1){
        totalFound++;
        copyToRead = copyToRead.substr(found + OFF_BY_ONE_OFFSET);
    }
    return totalFound;
}

vector<string> WebServer::readResponse(string &response, int &responseSD, bool &goodFormatting){
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
    if (!noDoubleNewline){
        goodFormatting = false;
        
    }
    // each line not terminated by \r\n
    string totalRequest = str(wholeRequest.begin(), wholeRequest.end());
    if ((findOccurances(totalRequest, ":") + OFF_BY_ONE_OFFSET) >= findOccurances(totalRequest, "\r\n")){
        goodFormatting = false;
        break;
    }

    // the first line is not of form "METHOD ARGUMENT HTTP/VERSION\r\n"
    string firstLine = totalRequest.substr(COUNTER_INITIAL_VALUE, totalRequest.find("\r\n"));
    if (findOccurances(firstLine, " ") != 2){ // this means we don't have two separators - so not matching form
        goodFormatting = false;
        break;
    }

    string versionString = firstLine.substr(firstLine.rfind(" "));
    if (versionString.find("/") < 0){ // if we don't have a '/' in the version, it is not of the same form
        goodFormatting = false;
        break;
    }

    /* Second Check Protocol Not Implemented */
    if (versionString.substr(COUNTER_INITIAL_VALUE, strlen("HTTP/") + OFF_BY_ONE_OFFSET) != "HTTP/"){ // if we don't have the HTTP/
        response = UNIMPLEMENTED_PROTOCOL;
        goodFormatting = false;
        break;
    }
    
    /* Prepare for Third Check Unsupported Method */
    string methodString = firstLine.substr(COUNTER_INITIAL_VALUE, firstLine.find(" "));
    // not get or shutdown - inform user of problem
    if (methodString != VALID_GET_METHOD && methodString != VALID_SHUTDOWN_METHOD){ 
        response = UNSUPPORTED_METHOD;
        goodFormatting = false;
        break;
    }

    // now figure out what the argument is
    string postMethodString = firstLine.substr(firstLine.find(" ") + OFF_BY_ONE_OFFSET);
    string argumentString = postMethodString.substr(COUNTER_INITIAL_VALUE, postMethodString.find(" ")); // from after "METHOD " to before " HTTP/VERSION"
    
    vector<string> methodAndArgumentBucket = {methodString, argumentString};
}

char * WebServer::getFile(string arg, string &response, bool &goodFormatting){
    // does it start with a '/'
        if (arg.at(COUNTER_INITIAL_VALUE) != "/"){
            response = INVALID_FILE_NAME;
            break;
        }
        // create full path
        string toOpen = rootDir + arg;
        
        // open requested file
        FILE * requestedFile = fopen(toOpen.c_str(), "r");
        if (requestedFile == NULL){
            response = FILE_NOT_FOUND;
            return NULL;
        }
        
        // now we want the contents, so we can use a fseek trick to get the whole file without much work.
        fseek(requestedFile, COUNTER_INITIAL_VALUE, SEEK_END); // pull the file pointer to the end
        int requestedFileSize = ftell(requestedFile); // now find the position
        fseek(requestedFile, COUNTER_INITIAL_VALUE, SEEK_SET); // now pull the file pointer back to the beginning

        char * fileBuff = new char[requestedFileSize];
        fileBuff = fread(fileBuff, OFF_BY_ONE_OFFSET, requestedFileSize, requestedFile);

        // at this point, if we have made it here
        if (goodFormatting){ // we know that there was no problem - so lets format the response appropriately
            response = OK_HEADER;
        }

        return fileBuff;
}