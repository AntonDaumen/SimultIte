/** 
 * \author Daumen Anton and Nicolas Derumigny
 * \file executable_options.h
 * \brief Definition of the tools for the command-line options.
 *
 */

#ifndef _EXECUTABLE_OPTION_H
#define _EXECUTABLE_OPTION_H

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/// \brief Structure of command line options for the executable
struct CommandLineOptions_t
{
	/// Size of the \a infilePath array
	unsigned int sizePath;
	/// Array of the path to the file descripting the matrix
	char         *infilePath;
};

typedef struct CommandLineOptions_t CommandLineOptions_t;

/// \brief Global variable of the parameters of the program
extern CommandLineOptions_t CommandLineOptions;

/// \brief Fill the \a CommandLineOption global variable with the given arguments
int parse_argument(
	int   argc,
	char  *argv[],
	char  *env[]);

#endif