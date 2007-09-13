/*
 * bnfparser2 - Generic BNF-adaptable parser
 * http://bnfparser2.sourceforge.net
 *
 *      This library is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License version 2.1, as published by the Free Software Foundation.
 *
 *      This library is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 * Copyright (c) 2007 ANF DATA spol. s r.o.
 * Copyright (c) 2007 Vaclav Vacek <vacek@ics.muni.cz>
 *
 * $Id$
 */

#include <getopt.h>
#include <string>
#include <iostream>
#include "BnfParser2.h"

#include "config.h"

int main(int argc, char  *argv[])
{
  int debug_level = 0;

  static struct option const long_options[] =
  {
    { "debug", required_argument, NULL, 'd' },
    { "help", no_argument, NULL, 'h' },
    { NULL, 0, NULL, 0 }
  };

  char optc;
  while(( optc = getopt_long( argc, argv, "d:h", long_options, NULL )) != -1 )
  {
    switch( optc )
    {
      case 'd':
        debug_level = atol(optarg);
        break;
      case 'h':
        printf(
"Usage: %s [OPTION]... GRAMMAR METASYNTAX START\n"
"Check input against a BNF syntax specification.\n"
"\n"
"  -d LEVEL, --debug=LEVEL   set debug to LEVEL (default '%i')\n"
"  --help                    display this help and exit\n"
"\n"
"Report bugs to <"PACKAGE_BUGREPORT">.\n",
          argv[0], debug_level
        );
        exit( 0 );

      default:
        printf( "Usage: %s [OPTION]... GRAMMAR METASYNTAX START\n", argv[0] );
        fprintf( stderr, "Try '%s --help' for more information.\n", argv[0] );
        exit( 1 );
    }
  }

  std::string grammar;
  std::string metasyntax;
  std::string start;
  std::string word;
  BnfParser2 test(debug_level);

  if(argc-optind < 3)
  {
    std::cerr << argv[0] << ": too few parameters" << std::endl;
    std::cerr << "Try `" << argv[0] << " --help' for more information." << std::endl;
    return 1;
  }

  grammar = argv[optind];
  metasyntax = argv[optind+1];
  start = argv[optind+2];

  test.set_start_nonterm(start, grammar);
  test.add_grammar(grammar, metasyntax);
  test.build_parser();

  std::cerr << "Enter the word: " << std::flush;
  while(std::cin >> word)
  {
    word = word + "\r\n";  //temporary edit!!!!
    word = "INVITE sip:bob@biloxi.com SIP/2.0\r\n\r\n";
    if(test.parse_word(word))
    {
      std::cout << "Word " << word << " accepted:"<< std::endl;
      std::cout << test.get_semantic_string() << std::endl;
    }
    else
    {
      std::cout << "Word " << word << " invalid - error at position " << test.get_error_position() + 1 << std::endl;
      std::cout <<word << std::endl;
      for(unsigned j = 0; j < test.get_error_position(); j++)
        std::cout << '-';
      std::cout  << '^' << std::endl;
    }
    std::cerr << "Enter the word: " << std::flush;
  }
  std::cout << std::endl;
  return 0;
}
