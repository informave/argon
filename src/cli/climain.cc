

//ARGONCLIMP.001 argoncli(1)
//ARGONCLIMP.001 ===========
//ARGONCLIMP.001 :doctype: manpage
//ARGONCLIMP.001 
//ARGONCLIMP.001 NAME
//ARGONCLIMP.001 ----
//ARGONCLIMP.001 argoncli - an inptereted ETL programming language
//ARGONCLIMP.001 
//ARGONCLIMP.001 
//ARGONCLIMP.005 DESCRIPTION
//ARGONCLIMP.005 -----------
//ARGONCLIMP.005 Argon is an interpreter for the DTS language.
//ARGONCLIMP.005 
//ARGONCLIMP.005 
//ARGONCLIMP.010 SYNOPIS
//ARGONCLIMP.010 -------
//ARGONCLIMP.010 *argoncli* ['OPTION']... ['FILE']
//ARGONCLIMP.010 


//ARGONCLIMP.010 OPTIONS
//ARGONCLIMP.010 -------
//ARGONCLIMP.010 *-v, --verbose*::
//ARGONCLIMP.010     Verbose output
//ARGONCLIMP.010 
//ARGONCLIMP.010 *-p, --parse-only*::
//ARGONCLIMP.010     Only parse the input file, but does not executes anything.
//ARGONCLIMP.010 


//ARGONCLIMP.010 
//ARGONCLIMP.010 EXIT STATUS
//ARGONCLIMP.010 -----------
//ARGONCLIMP.010 *0*::
//ARGONCLIMP.010     Success
//ARGONCLIMP.010 
//ARGONCLIMP.010 *1*::
//ARGONCLIMP.010     Failure (syntax or usage error; configuration error; document
//ARGONCLIMP.010     processing failure; unexpected error).
//ARGONCLIMP.010 
//ARGONCLIMP.010 EXAMPLES
//ARGONCLIMP.010 --------
//ARGONCLIMP.010 *argoncli* myscript.dts
//ARGONCLIMP.010 
//ARGONCLIMP.010 
//ARGONCLIMP.010 *argoncli* --parse-only myscript.dts
//ARGONCLIMP.010 

//ARGONCLIMP.010 
//ARGONCLIMP.010 BUGS
//ARGONCLIMP.010 ----
//ARGONCLIMP.010 See the argonlib distribution BUGS file.
//ARGONCLIMP.010 
//ARGONCLIMP.010 
//ARGONCLIMP.010 AUTHOR
//ARGONCLIMP.010 ------
//ARGONCLIMP.010 Written by Daniel Vogelbacher, <daniel@vogelbacher.name>
//ARGONCLIMP.010 
//ARGONCLIMP.010 
//ARGONCLIMP.010 RESOURCES
//ARGONCLIMP.010 ---------
//ARGONCLIMP.010 Main web site: <http://www.informave.org/argon>
//ARGONCLIMP.010 
//ARGONCLIMP.010 
//ARGONCLIMP.010 COPYING
//ARGONCLIMP.010 -------
//ARGONCLIMP.010 Copyright \(C) 2010 Daniel Vogelbacher. Free use of this software is
//ARGONCLIMP.010 granted under the terms of the GNU General Public License (GPL).
//ARGONCLIMP.010 



#include <argon/dtsengine>

#include <iostream>

int main(void)
{
	std::cout << "Argon command line interface (c) 2010 Daniel Vogelbacher" << std::endl
		<< std::endl;

	//foo();

	return 0;

}

