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
  int delimiter = '\n';

  static struct option const long_options[] =
  {
    { "debug", required_argument, NULL, 'd' },
    { "help", no_argument, NULL, 'h' },
    { "delimiter", required_argument, NULL, 'e' },
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
"  -d LEVEL, --debug=LEVEL   set debug to LEVEL (default %i)\n"
"  -e NUM, --delimiter=NUM   set word delimiter to NUM (default %i in ASCII)\n"
"  --help                    display this help and exit\n"
"\n"
"Report bugs to <"PACKAGE_BUGREPORT">.\n",
          argv[0], debug_level, delimiter
        );
        exit( 0 );
      case 'e':
        delimiter = atol(optarg);
        break;

      default:
        printf( "Usage: %s [OPTION]... GRAMMAR METASYNTAX START\n", argv[0] );
        fprintf( stderr, "Try '%s --help' for more information.\n", argv[0] );
        exit( 1 );
    }
  }

  std::string grammar;
  std::string metasyntax;
  std::string start;
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

  int errcount = 0;

  while(!feof(stdin))
  {
    std::string word;
    // read new word
    // note: words are separated by \0, program is terminated by EOF
    int ch;
    while((ch = fgetc(stdin)) != EOF && ch != delimiter)
      word += ch;

    if(test.parse_word(word))
    {
      // print accepted word
      std::cout << test.get_semantic_string() << std::endl;
    }
    else
    {
      errcount++;
      // locate the error
      std::cout << "Syntax error at position " << test.get_error_position() + 1 << std::endl;
      std::cout <<word << std::endl;
      for(unsigned j = 0; j < test.get_error_position(); j++)
        std::cout << '-';
      std::cout  << '^' << std::endl;
    }
  }

  return errcount;
}

// end of file
