/*
 * bnfparser2 - Generic BNF-adaptable parser
 * http://bnfparser2.sourceforge.net
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License version 2.1, as published by the Free Software Foundation.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 * Copyright (c) 2007 ANF DATA spol. s r.o.
 * Copyright (c) 2007 Vaclav Vacek <vacek@ics.muni.cz>
 *
 * $Id$
 */

#include <string>
#include <iostream>

#include <SimpleOpt.h>

#include "BnfParser2.h"
#include "config.h"

int main(int argc, char  *argv[])
{
  // instantiate the parser
  BnfParser2 test;

  int delimiter = '\n';
  bool automatic_includes = true;
#ifdef DATADIR
  test.add_search_path(DATADIR);
#endif

  enum
  {
    OPT_DIRECTORY,
    OPT_DELIMITER,
    OPT_MANUAL_INCLUDES,
    OPT_VERBOSE,
    OPT_HELP
  };

  static CSimpleOpt::SOption const long_options[] =
  {
    { OPT_DIRECTORY, "-d", SO_REQ_SEP },
    { OPT_DELIMITER, "-e", SO_REQ_SEP },
    { OPT_DELIMITER, "--delimiter", SO_REQ_CMB },
    { OPT_MANUAL_INCLUDES, "--manual-includes", SO_NONE },
    { OPT_VERBOSE, "-v", SO_REQ_SEP },
    { OPT_VERBOSE, "--verbose", SO_REQ_CMB },
    { OPT_HELP, "--help", SO_NONE },
    SO_END_OF_OPTIONS
  };

  int fileind = 0;
  CSimpleOpt args(argc, argv, long_options);

  while(args.Next())
  {
    if(args.LastError() != SO_SUCCESS)
    {
      printf( "Usage: %s [OPTION]... SYMBOL ([:[VARIANT]] SYNTAX)...\n", argv[0] );
      fprintf( stderr, "Try '%s --help' for more information.\n", argv[0] );
      exit(1);
    }

    switch(args.OptionId())
    {
      case OPT_DIRECTORY:
        test.add_search_path(args.OptionArg());
        break;
      case OPT_DELIMITER:
        delimiter = atol(args.OptionArg());
        break;
      case OPT_MANUAL_INCLUDES:
        automatic_includes = false;
        break;
      case OPT_VERBOSE:
        test.set_verbose_level( atol(args.OptionArg()) );
        break;

      case OPT_HELP:
        printf(
"Usage: %s [OPTION]... SYMBOL ([:[VARIANT]] SYNTAX)...\n"
"Check input against a BNF syntax specification.\n"
"\n"
"  -d DIR                    search specifications in the directory DIR\n"
"  -e NUM, --delimiter=NUM   set word delimiter to NUM (default %i in ASCII)\n"
"  --manual-includes         do not automatically load referenced grammars\n"
"  -v LEVEL, --verbose=LEVEL set verbosity to LEVEL (default %i)\n"
"  --help                    display this help and exit\n"
"\n"
"Report bugs to <"PACKAGE_BUGREPORT">.\n",
          argv[0], delimiter, test.get_verbose_level()
        );
        exit(0);
    }
  }

  if(fileind+2 > args.FileCount())
  {
    std::cerr << argv[0] << ": too few parameters" << std::endl;
    std::cerr << "Try `" << argv[0] << " --help' for more information." << std::endl;
    return 1;
  }

  const char* symbol = args.File(fileind++);

  const char* variant = NULL;
  const char* last_variant = NULL;
  bool use_last_variant = false;

  const char* param = args.File(fileind++);
  if(param[0] == ':')
  {
    // first VARIANT cannot say use_last_variant
    if(param[1] == '\0')
    {
      std::cerr << argv[0] << ": missing variant specification" << std::endl;
      exit(1);
    }
    else
      variant = param+1;

    if(fileind+1 > args.FileCount())
    {
      std::cerr << argv[0] << ": expecting specification name" << std::endl;
      std::cerr << "Try `" << argv[0] << " --help' for more information." << std::endl;
      return 1;
    }
    // load SYNTAX parameter
    param = args.File(fileind++);
  }

  // load start grammar
  test.set_start_nonterm(symbol, param);
  test.add_grammar(param, variant);
  // load next grammars
  while(fileind > args.FileCount())
  {
    if(*(param = args.File(fileind++)) == ':')
    {
      if(param[1] == '\0')
        use_last_variant = true;
      else
      {
        variant = param+1;
        use_last_variant = false;
      }
    }
    else
    {
      if(use_last_variant)
        variant = last_variant;
      else
        last_variant = variant; // store for further use

      test.add_grammar(param, variant);

      use_last_variant = false;
      variant = NULL;
    }
  }

  if(automatic_includes)
    test.add_referenced_grammars();

  test.build_parser();

  int errcount = 0;

  /* The std::cout contains machine readable information
   * [test number] passed
   * [test number] failed at position [position]
   */
  for(int caseno=1; !feof(stdin); caseno++)
  {
    std::string word;
    // read new word
    // note: words are separated by \0, program is terminated by EOF
    int ch;
    while((ch = fgetc(stdin)) != EOF && ch != delimiter)
      word += ch;

    std::cerr << "--------------------NEXT WORD--------------------" << std::endl;
    if(test.parse_word(word))
    {
      // print accepted word
      std::cerr << test.get_semantic_string() << std::endl;
      std::cout << "[" << caseno << "] passed" << std::endl;
    }
    else
    {
      errcount++;
      // print the word
      std::cerr <<word << std::endl;
      for(unsigned j = 0; j < test.get_error_position(); j++)
        std::cerr << '-';
      std::cerr  << '^' << std::endl;
      // locate the error
      std::cout << "[" << caseno << "] failed at position " << test.get_error_position() + 1 << std::endl;
    }
  }

  return errcount;
}

// end of file
