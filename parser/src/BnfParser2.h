/**
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

#ifndef BNFPARSER2_EXP_DEFN
#ifdef _WIN32
#define BNFPARSER2_EXP_DEFN __declspec(dllimport)
#else
#define BNFPARSER2_EXP_DEFN /* empty */
#endif
#endif

//! Generic BNF-adaptable parser.
/**
 * Implements a parser generated at run-time depending on given syntax
 * specification in a Backus-Naur Form (BNF).
 *
 * To use the parser
 * -# Call add_grammar() to load a syntax specification. May be called
 *    multiple times.
 * -# Call set_start_symbol() to set a start symbol.
 * -# Call build_parser() to process the specifications and build the parser.
 * -# Call parse_word() to parse a word. May be called multiple times.
 * -# Call get_error_position() or get_semantic_string() to obtain results of
 *    the parsing.
 */
class BNFPARSER2_EXP_DEFN BnfParser2
{
  //! The class containing the implementation of the parser
  Parser *m_core_parser;

public:
  //! A constructor.
  BnfParser2(void);

  //! A destructor.
  ~BnfParser2(void);

  //! Add new path searched for syntax and variant specifications.
  /**
   * Mutliple calls possible. Updates the list used by add_grammar() and
   * add_referenced_grammars() to search for syntax and variant specifications.
   *
   * \param[in] path Absolute or relative path.
   */
  void add_search_path(const char *path);

  //! Load a syntax specification file.
  /**
   * Mutliple calls possible. The list of directories specified using
   * add_search_path() and the specification is loaded from "path/syntax_name".
   * The variant definition is searched in "path/syntax/variant_name.conf".
   * If no path is specified, the specifications are searched in the working
   * directory only.
   *
   * \param[in] syntax_name Name of the syntax specification file.
   * \param[in] variant_name Variant of BNF used by this specification. When
   *   omited, the syntax is determined from the !syntax tag.
   */
  void add_grammar(const char *syntax_name, const char *variant_name = NULL);

  //! Load all referenced specifications.
  /**
   * Calls add_grammar() for all unresolved !include references.
   * Recursively processes all references, so it needs to be called only once.
   */
  void add_referenced_grammars(void);

  //! Set a starting symbol.
  /**
   * Sets name of starting nonterminal in the BNF grammar.
   *
   * \param[in] symbol_name Name of the start symbol.
   * \param[in] start_grammar_name Syntax specification containing the start symbol.
   *
   * \sa add_grammar(), add_referenced_grammars()
   */
  void set_start_symbol(const char *symbol_name, const char *start_grammar_name = NULL);

  //! Process the specifications and build the parser.
  void build_parser(void);

  //! Get a current verbose level.
  /**
   * \return Current verbose level.
   */
  int get_verbose_level(void);

  //! Set the verbose level.
  /**
   * \param[in] level New verbose level.
   */
  void set_verbose_level(unsigned level);

  //! Parse a word.
  /**
   * Takes a word and returns its (partial) syntax tree. Mutliple calls possible.
   *
   * \param[in] word The word to be parsed.
   * \return True if parsing was successfull.
   *
   * \sa get_parsing_result(), get_error_position(), get_semantic_string()
   * \warning Must not be called before build_parser().
   */
  bool parse_word(const std::string& word);

  //! Returns the result of the last parsing.
  /**
   * \return True if the last parse_word() was successfull.
   */
  bool get_parsing_result(void);

  //! Get the position of an error occured during the last parsing.
  /**
   * When the last parse_word() was successful, the return value is not defined.
   *
   * \return Index of the error position.
   */
  unsigned get_error_position(void);

  //! Get the semantic string of the last parsing.
  /**
   * When the last parse_word() was unsuccessful, the return value is not defined.
   *
   * \return Semantic string.
   */
  std::string get_semantic_string(void);
};

#endif

// end of file
