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
 * Copyright (c) 2007 Petr Slovak <slovak@ics.muni.cz>
 *
 * $Id$
 */

//#define _ANYBNFCONF_TEST_

#ifndef _ANYBNFCONF_
#define _ANYBNFCONF_

#include <fstream>
#include <string>
#include <stdexcept>
#include <vector>

/** \brief This class is used for parsing the metaconfiguration 
 *         from a given file.
 *
 * The acquired information is stored 
 * in two separate structures - one is for basic symbols 
 * and syntax that has to be included in any grammar such 
 * as syntax of rulename, definition symbol, group symbols etc.. 
 * Second is for arbitrary operators that are defined using 
 * regular expressions in Perl syntax. These structures are 
 * accessible by get_xxx() and get_operator() functions.
 */
class AnyBnfConf
{
public:
  AnyBnfConf(void) {}

  const std::string& get_rulename() const { return m_rulename; }
  const std::string& get_defined() const { return m_defined; }
  const std::string& get_terminal() const { return m_terminal; }
  const std::string& get_comment() const { return m_comment; }
  const std::string& get_concat() const { return m_concat; }
  const std::string& get_alternative() const { return m_alternative; }
  const std::string& get_leftgroup() const { return m_leftgroup; }
  const std::string& get_rightgroup() const { return m_rightgroup; }
  const std::string& get_leftcomment() const { return m_leftcomment; }
  const std::string& get_rightcomment() const { return m_rightcomment; }
  const std::string& get_allbrackets() const { return m_allbrackets; }
  bool is_csstring() const { return m_csstring; }
  bool is_csname() const { return m_csname; }

  typedef std::vector<std::string> TOperatorList;
  const TOperatorList& get_operators() const { return m_operators; }

  void reset(void); //!< Clears all the data stored in the class.
  bool parse_conf(const std::string& m_conf_name); //!<parses the given configuration file

  //! returns RE describing the requested base sybmol, metachars are backslashed
  static std::string backslash(const std::string& source, int doub=1);

  void check_conf(std::ostream& output); //!< writes all the data to a stream

private:
  // All the following string variables are designated to hold
  // representations of basic grammar symbols. Term "basic symbols"
  // therefore applies for this set of variables.
  std::string m_rulename; //!< regular expression describing rulename
  std::string m_defined; //!< regular expression describing the defined symbol (like =)
  std::string m_terminal; //!< regular expression describing terminal symbols
  std::string m_comment; //!< regular expression describing line-comment symbol (like //)
  std::string m_concat; //!< regular expression describing concatenation
  std::string m_alternative; //!< regular expression describing alternative
  std::string m_leftgroup; //!< regular expression describing left group-bracket
  std::string m_rightgroup; //!< regular expression describing right group-bracket
  std::string m_leftcomment; //!< regular expression describing left group-comment symbol (like /*)
  std::string m_rightcomment; //!< regular expression describing right grou-comment symbol (like */)
  std::string m_allbrackets; //!< regular expression describing all the brackets used in the grammar
  bool m_csstring; //!< "true" if the strings are case-sensitive
  bool m_csname; //!< "true" if the nonterminals' names are case-sensitive

  TOperatorList m_operators; //!< list of operators syntax

  static const int m_max_line_length = 256; //!< max input-file line length
  char* get_next_token(char*& buffer);
};

#endif  //_ANYBNFCONF_

// end of file
