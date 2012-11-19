//
// climain.cc - CLI Application
//
// Copyright (C)         informave.org
//   2010,               Daniel Vogelbacher <daniel@vogelbacher.name>
// 
// Lesser GPL 3.0 License
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.

/// @file
/// @brief CLI Application
/// @author Daniel Vogelbacher
/// @since 0.1


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
#include <argon/exceptions.hh>

#include <iostream>
#include <fstream>

int main(int argc, char **argv)
{
    using namespace informave::argon;

    std::locale::global(std::locale(""));

    std::cout.imbue(std::locale());
    std::cerr.imbue(std::locale());
    std::clog.imbue(std::locale());
    std::wcout.imbue(std::locale());
    std::wcerr.imbue(std::locale());
    std::wclog.imbue(std::locale());


    std::ios_base::sync_with_stdio(true); /// @todo check if this is correct
    std::cout.setf(std::ios::unitbuf);
    std::wcout.setf(std::ios::unitbuf);

	std::cout << "Argon command line interface (c) 2010 Daniel Vogelbacher" << std::endl
		<< std::endl;


    DTSEngine engine;



    if(argc != 2)
    {
        std::cout << "No file given" << std::endl;
        return -1;
    }
    

    try
    {
        if(String(argv[1]) == "-")
        {
            engine.load(std::istreambuf_iterator<wchar_t>(std::wcin.rdbuf()));
        }
        else
        {
            engine.load(argv[1]);
        }
        engine.exec();
	}
    catch(CompileError &err)
    {
        std::cout << "Compile error:" << std::endl
                  << err.what() << std::endl
                  << "No code was executed." << std::endl;
        return -1;
    }
    catch(RuntimeError &err)
    {
        std::cout << "Runtime error:" << std::endl
                  << err.what() << std::endl;
        return -1;
    }
	catch(std::exception &e)
	{
		std::cout << "UNHANDLED EXCEPTION:" << std::endl << e.what() << std::endl;
        return -1;
	}
    return 0;
}


//
// Local Variables:
// mode: C++
// c-file-style: "bsd"
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
//
