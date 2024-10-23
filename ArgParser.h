/*  Name: Sanket Makkar
    CaseID:         sxm1626
    File Name:      ArgParser.h
    Date Created:   10/17/2024
    Description:    This file contains function declarations for the ArgParser module. These functions
                    include parseArgs - which is the main function used to orchestrate parsing CLI
                    arguments
*/

#ifndef ARGSPARSER_H
#define ARGSPARSER_H

#include <string>
#include <vector>
using namespace std;

void usage(char *progname);
int parseArgs(int argc, char *argv[], void (*grabPortNumber)(string portNum), void (*grabRootDirectory)(string rootDir), void (*grabAuthToken)(string authenticationToken));
bool flagsContainBit(int cmdFlags, int bit);
void checkNonSetFlag(int cmdFlags, int bit, char arg);

#endif // ARGSPARSER_H
