/*  Name: Sanket Makkar
    CaseID:         sxm1626
    File Name:      ArgParser.h
    Date Created:   9/25/2024
    Description:    This file contains function declarations for the ArgParser module. These functions
                    include parseArgs - which is the main function used to orchestrate parsing CLI
                    arguments - and also helper functions other files may find useful such as 
                    "usage", "flagsContainBit".
*/

#ifndef ARGSPARSER_H
#define ARGSPARSER_H

#include <string>
#include <vector>
using namespace std;

void usage(char *progname);
int parseArgs(int argc, char *argv[], void (*grabPortNumber)(int portNum), void (*grabRootDirecotry)(string rootDir), void (*grabAuthToken)(string authToken));
bool flagsContainBit(int cmdFlags, int bit);
void checkNonSetFlag(int cmdFlags, int bit, char arg);

#endif // ARGSPARSER_H
