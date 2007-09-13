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

#ifndef _BNFPARSER2_
#define _BNFPARSER2_
#include <string>

class Parser;

/** \brief The class providing the interface to the parser.
 *  The parser is encapsulated in order that it may be used
 *  as a library.
 *
 */
class BnfParser2
{
  //! The class containing the implementation of the parser
  Parser *m_core_parser;

public:
  //! The parsing function. It takes a word and returns its (partial) syntax tree.
  /** \warning Must not be called before process().
   */
  bool parse_word(std::string word);

 //! Returns the result of the last parsing.
  bool get_parsing_result(void);

  //! Returns the position of an error occuring during the last parsing.
  /** When the last parsing is successful, the return value is not defined.
   */
  unsigned get_error_position(void);

  //! Returns the semantic string of the last parsing
  /** When the last parsing is unsuccessful, the return value is not defined.
   */
  std::string get_semantic_string(void);

  //! Sets the name of the starting nonterminal and the name of the file containing it.
  void set_start_nonterm(std::string start_name, std::string start_grammar_name);

  //! Loads a grammar-file together with its configuration file (mutliple calls possible)
  void add_grammar(std::string g_name, std::string c_name);

  //! Processes the set of grammar files added, computes GLALR table.
  void build_parser(void);

  //! The parameter determines the verbose level.
  BnfParser2(unsigned verbose = 0);

  ~BnfParser2(void);
};
#endif
