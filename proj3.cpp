/*  Name: Sanket Makkar
    CaseID:         sxm1626
    File Name:      proj3.cpp
    Date Created:   10/17/2024
    Description:    This is the file that contains a main function for the simple CLI web-based client.
                    The point behind making this file is mostly to orchestrate functionality held within other
                    files of this project.
*/
#include <stdio.h>
#include "ArgParser.h"
#include "WebServer.h"   
#include <string>
#include <string.h>

using namespace std;

string portNum;
string rootDir;
string authToken;

void savePortNum(string portNumber){
    portNum = portNumber;
}

void saveRootDir(string rootDirectory){
    rootDir = rootDirectory;
}

void saveAuthToken(string authenticationToken){
    authToken = authenticationToken;
}

int main(int argc, char *argv[]){
    // execute parse args, passing in a callback that will grab file name, and returning/storing the args flag indicator
    int argLine = parseArgs(argc, argv, savePortNum, saveRootDir, saveAuthToken);

    printf("%s\n%s\n%s\n", portNum.c_str(), rootDir.c_str(), authToken.c_str());
    printf("argline = %d\n", argLine);
    WebServer server = WebServer(argLine, portNum, rootDir, authToken);

    // client.grabFromNetwork();

    return 0;
}
