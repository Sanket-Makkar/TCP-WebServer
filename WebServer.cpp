/*  Name: Sanket Makkar
    CaseID:         sxm1626
    File Name:      WebServer.cpp
    Date Created:   10/19/2024
    Description:    The purpose of this file is to implement the core functionality for the web-server as required 
                    by this assignment. This file in particular implements the methods intended to do this work as 
                    defined within the WebServer.h header file.
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
#define OK_SERVER_SHUTDOWN "HTTP/1.1 200 Server Shutting Down\r\n\r\n"
#define BAD_SERVER_NOT_SHUTDOWN "HTTP/1.1 403 Operation Forbidden\r\n\r\n"

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
        fprintf(stderr, "Error: cannot find protocol information for %s\n", PROTOCOL);
        exitWithErr;
    }

    // lets get listening on the right port number
    setupListening(protoinfo, listenSD, sin);
    
    bool listenLoop = true;
    while (listenLoop){
        // and prepare for a new connection
        awaitConnection(&addr, responseSD, listenSD);

        // and respond to requests, which inform us if we should continue to keep the server alive (listening)
        listenLoop = respondToRequest(responseSD);

        close(responseSD);
    }

    // and when we are done lets wrap everything up
    close (listenSD);
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
        fprintf(stderr, "Error: error accepting connection\n");
        exitWithErr;
    }
}

bool WebServer::respondToRequest(int &responseSD){
    // We will, at some point, send a string response - assume the string is malformed and we will try to prove it is not
    string response = MALFORMED;

    // consider response formatting requirements
    vector<string> methodAndArg = readResponse(response, responseSD);
    if (methodAndArg.empty()){ // if we failed within readResponse, it at least updated our response
        writeResponse(responseSD, response);
        return true;
    }

    // else lets continue as normal and deal with the method
    string method = methodAndArg.at(COUNTER_INITIAL_VALUE);
    string arg = methodAndArg.at(COUNTER_INITIAL_VALUE + OFF_BY_ONE_OFFSET);

    /* Handle GET and SHUTDOWN cases */
    if (method == VALID_GET_METHOD){
        unsigned char * fileBuff;
        int fileSize;
        fileBuff = getFile(arg, response, fileSize);
        if (fileBuff == NULL){
            writeResponse(responseSD, response);
            return true;
        }

        // send back the header
        writeResponse(responseSD, response);

        // and don't forget the file
        if (fileBuff != NULL){
            writeResponse(responseSD, fileBuff, fileSize);
            return true;
        }
        delete[] fileBuff; // memory management
    }
    else if (method == VALID_SHUTDOWN_METHOD){
        // now if we want to shut down, verify the auth token and tell the server what to do
        bool continueResponding = (arg != authToken);
        response = continueResponding? BAD_SERVER_NOT_SHUTDOWN : OK_SERVER_SHUTDOWN;
        writeResponse(responseSD, response);
        return continueResponding;
    }
    return false;
}

// simple helper, counts occurances of a substring within a string
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

// this does the job of reading through the incoming http request, and then allows us to setup a response to that request
vector<string> WebServer::readResponse(string &response, int &responseSD){
    // first we want to grab the whole request
    vector<unsigned char> wholeRequest;
    char buf[BUFLEN];
    vector<string> methodAndArgumentBucket = {};

    FILE * responseSP = fdopen(responseSD, "r");
    if (responseSP == NULL){
        fprintf(stderr, "Error: failed to open response pointer");
        exitWithErr;
    }
    bool noNewLine = false;
    while ((fgets(buf, BUFLEN, responseSP)) != NULL){ // grab from the socket line by line, put it in buf
        wholeRequest.insert(wholeRequest.end(), buf, buf + strlen(buf)); // we want to keep track of all data, so slap it on the end of the vector
        string wholeStr = string(buf); // it can be useful to view the incoming line as a string
        if (wholeStr.find("\r\n") < 0){ 
            noNewLine = false;
            break;
        }
        if (wholeStr == "\n" || wholeStr == "\r\n"){ // even if we have a line with only \n, we will still pick that up later - so break if the line is just a newline
            break;
        }
        memset(buf, '\0', BUFLEN); // lets get rid of random values and make all values null-terminated by default
    }
    
    /* Check Malformed Request*/
    // No found "\r\n" for a particular line
    if (noNewLine){
        return methodAndArgumentBucket;    
    }

    // \r\n\r\n does not end the input
    string totalRequest = string(wholeRequest.begin(), wholeRequest.end());
    if (totalRequest.find("\r\n\r\n") + strlen("\r\n\r\n") != totalRequest.length()){
        return methodAndArgumentBucket;    
    }

    // the first line is not of form "METHOD ARGUMENT HTTP/VERSION\r\n"
    string firstLine = totalRequest.substr(COUNTER_INITIAL_VALUE, totalRequest.find("\r\n"));
    if (findOccurances(firstLine, " ") != 2){ // this means we don't have two separators - so not matching form
        return methodAndArgumentBucket;
    }

    string versionString = firstLine.substr(firstLine.rfind(" ") + OFF_BY_ONE_OFFSET);
    if (versionString.find("/") < 0){ // if we don't have a '/' in the version, it is not of the same form
        return methodAndArgumentBucket;
    }

    /* Check Protocol Not Implemented */
    if (versionString.substr(COUNTER_INITIAL_VALUE, strlen("HTTP/")) != "HTTP/"){ // if we don't have the HTTP/
        response = UNIMPLEMENTED_PROTOCOL;
        return methodAndArgumentBucket;
    }
    
    /* Prepare for Check Unsupported Method */
    string methodString = firstLine.substr(COUNTER_INITIAL_VALUE, firstLine.find(" "));
    // not get or shutdown - inform user of problem
    if (methodString != VALID_GET_METHOD && methodString != VALID_SHUTDOWN_METHOD){ 
        response = UNSUPPORTED_METHOD;
        return methodAndArgumentBucket;
    }

    // now figure out what the argument is
    string postMethodString = firstLine.substr(firstLine.find(" ") + OFF_BY_ONE_OFFSET);
    string argumentString = postMethodString.substr(COUNTER_INITIAL_VALUE, postMethodString.find(" ")); // from after "METHOD " to before " HTTP/VERSION"
    
    methodAndArgumentBucket = {methodString, argumentString};
    return methodAndArgumentBucket;
}

// this gets the file we have been told to retrieve from the server
unsigned char * WebServer::getFile(string arg, string &response, int & size){
    // does it start with a '/'
    if (arg.at(COUNTER_INITIAL_VALUE) != '/'){
        response = INVALID_FILE_NAME;
        return NULL;
    }
    else if (arg == "/"){
        arg = "/index.html";
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
    unsigned char * fileBuff;
    fileBuff = grabFileContents(requestedFile, size);

    fclose(requestedFile);

    // at this point, if we have made it here the formatting and checks must have been correct
    response = OK_HEADER;

    return fileBuff;
}

// this helps us write responses without repeating code
void WebServer::writeResponse(int socketDescriptor, string toWrite){
    if (write(socketDescriptor, toWrite.c_str(), toWrite.length()) < 0) { // so send what we have
        fprintf(stderr, "Error: unable to write response\n");
        exitWithErr;
    }
}

// an overload for char * (as opposed to string) that we want to write to the socket
void WebServer::writeResponse(int socketDescriptor, unsigned char * toWrite, int length){
    if (write(socketDescriptor, toWrite, length) < 0) { // so send what we have
        fprintf(stderr, "Error: unable to write response\n");
        exitWithErr;
    }
}

// this generically grabs file contents given a file - also informs of size using passed param.
unsigned char * WebServer::grabFileContents(FILE * file, int & size){
    fseek(file, COUNTER_INITIAL_VALUE, SEEK_END); // file pointer at end
    int fileSize = ftell(file); // tell position of pointer (gives size indirectly)
    fseek(file, COUNTER_INITIAL_VALUE, SEEK_SET); // file pointer at start
    size = fileSize;

    unsigned char * fileBuff = new unsigned char[fileSize]; // now we will create a perfectly sized buffer
    fread(fileBuff, OFF_BY_ONE_OFFSET, fileSize, file); // and fread into that buffer the file
    return fileBuff; // and return the buffer - closing the file is on the person calling this
}