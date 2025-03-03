/*  Name: Sanket Makkar
    CaseID:         sxm1626
    File Name:      ArgParser.cpp
    Date Created:   10/17/2024
    Description:    This file contains implementations for functions described within ArgParser.h.
                    In general, this class is intended to provide a set of methods helpful to any
                    other file that may want to parse, or perform some parsing-related action, on
                    user input.
*/

#include "ArgParser.h"  
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <list>
#include <string>
#include <string.h>
#include <vector>

using namespace std;

// These are flags we will check for and set
#define ARG_PORT 0x1
#define ARG_ROOT 0x2
#define ARG_AUTH_TOKEN 0x4

// Basic counter related constants
#define COUNTER_INITIALIZER 0
#define OFF_BY_ONE_OFFSET 1

#define FIRST_VALUE 0
#define FULL_CLI_ARG_LINE (ARG_PORT + ARG_ROOT + ARG_AUTH_TOKEN)

// this is a local copy of the file name and a CLI flags holder (init to 0 - no flags)
string portNumber;
string rootDirectory;
string authenticationToken;
unsigned int cmdLineFlags = 0x000000;


// rather than writing exit(-1) every time I want to exit with an error, I wrote up this macro to make it easier
#define exitWithErr exit(-1)

// We take an input of argc, argv from the caller - arguments and number of arguments, as well as two callbacks to execute at the end of the function call
int parseArgs(int argc, char *argv[], void (*grabPortNumber)(string portNum), void (*grabRootDirectory)(string rootDir), void (*grabAuthToken)(string authenticationToken)){
    int opt;
    while ((opt = getopt(argc, argv, "p:r:t:")) != -1){
        // check for cases of prt
        switch (opt){
            case 'p': 
                checkNonSetFlag(cmdLineFlags, ARG_PORT, 'p'); 
                cmdLineFlags |= ARG_PORT;
                portNumber = optarg;
                break;
            case 'r':
                checkNonSetFlag(cmdLineFlags, ARG_ROOT, 'r'); 
                cmdLineFlags |= ARG_ROOT;
                rootDirectory = optarg;
                break;
            case 't': 
                checkNonSetFlag(cmdLineFlags, ARG_AUTH_TOKEN, 't'); 
                cmdLineFlags |= ARG_AUTH_TOKEN;
                authenticationToken = optarg;
                break;
            case '?': // if invalid option provided, react with an error message and exit
                usage(argv[FIRST_VALUE]);
                exitWithErr;
            default: // and if nothing else gets caught then just react with an error message and exit
                usage(argv[FIRST_VALUE]);
                exitWithErr;
        }
    }

    if (cmdLineFlags != FULL_CLI_ARG_LINE){ // no command-line flags set implies no CLI option was given up until now (the loop above would have caught it)
        fprintf(stderr, "Error: not all command line options (required) given\n");
        usage(argv[FIRST_VALUE]);
        exitWithErr;
    }

    grabPortNumber(portNumber);
    grabRootDirectory(rootDirectory);
    grabAuthToken(authenticationToken);
    return cmdLineFlags;
}

// more universal helper to ensure the flags contain a specified bit
bool flagsContainBit(int cmdFlags, int bit){ 
    if (cmdFlags & bit){
        return true;
    }
    return false;
}

// verify a flag was not set
void checkNonSetFlag(int cmdFlags, int bit, char arg){
    if (flagsContainBit(cmdFlags, bit)){
        fprintf(stderr, "Error: failed user entry (2 instances of -%c)\n", arg);
        exitWithErr;
    }
}

// inform the user of the flags available to use this program
void usage(char *progname){
    fprintf(stderr, "%s -p port_number -r document_directory -t auth_token\n", progname);
    fprintf(stderr, "   -p port_number          specifies port number to listen on as port_number (MUST BE GIVEN)\n");
    fprintf(stderr, "   -r document_directory   specifies a document directory to consider as the servers root directory (MUST BE GIVEN)\n");
    fprintf(stderr, "   -t auth_token           specifies an authentication token required to terminate the web server (MUST BE GIVEN)\n");
    exitWithErr;
}