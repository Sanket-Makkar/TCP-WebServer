/*  Name: Sanket Makkar
    CaseID:         sxm1626
    File Name:      proj2.cpp
    Date Created:   9/25/2024
    Description:    This is the file that contains a main function for the simple CLI web-based client.
                    The point behind making this file is mostly to orchestrate functionality held within other
                    files of this project.
*/
#include <stdio.h>
#include "ArgParser.h"
#include "WebClient.h"
#include <string>
#include <string.h>

using namespace std;

// we will store the file name we grab from parseArgs here
string URL;
string savePath;

// this callback will grab the url from parseArgs
void urlStorage(char url[]){
    URL = url;
}
// this callback will grab the savePath from parseArgs
void saveStorage(char path[]){
    savePath = path;
}

int main(int argc, char *argv[])
{
    // execute parse args, passing in a callback that will grab file name, and returning/storing the args flag indicator
    int argLine = parseArgs(argc, argv, urlStorage, saveStorage);

    WebClient client = WebClient(argLine, URL, savePath);

    client.grabFromNetwork();

    return 0;
}
