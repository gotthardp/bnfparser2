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

#ifndef _PARSER_
#define _PARSER_

#include <string>
#include <iostream>
#include <set>
#include <map>
#include <vector>

#include "AnyBnfLoad.h"
#include "GSS.h"
#include "LalrTable.h"


/** \brief This class contains the implementation of the parser proper.
 *  
 *
 */
class Parser
{
public:
  //! The class for storing pending shift actions
  /** Each shift action is represented by by the state node it is performed in
   *  and the number of the new state.  
   */
  class QMember
  {
  public:
    //! The identifier of the state
    GSS::StateIdent state_node;
    
    //! The number of the new state
    int new_state;
    
    //! The constructor always takes the two parameters
    QMember(GSS::StateIdent _state_node, int _new_state)
    :state_node(_state_node), new_state(_new_state)
    {}
  
    //! operator< is needed for the use of sets
    friend bool operator<(const QMember & first, const QMember & second);
  };

  //! The class for storing pending reductions
  /** Each reduction is represented by the state node it is performed in,
   *  the number of the grammar rule its performed by and its length.
   */
  class RMember
  {
  public:
    //! The identifier of the state
    GSS::StateIdent state_node;
    
    //! The number of the rule
    int rule_number;
    
    //! The length of the reduction
    int reduction_length;
    
    //! The identifier of the first symbol of the reduction
    GSS::SymbolIdent first_part;
    
    //! The constructor always takes these parameters
    RMember(GSS::StateIdent _state_node, int _rule_number, int _reduction_length, GSS::SymbolIdent _first_part)
    :state_node(_state_node), rule_number(_rule_number), reduction_length(_reduction_length),
    first_part(_first_part)
    {}
  
    //! operator< is needed for the use of sets
    friend bool operator<(const RMember & first, const RMember & second);
  };

  //! The parsing function. It takes a word and returns true iff it is accepted.
  /** \warning Must not be called before build_parser().
   */
  bool parse_word(const std::string& word);
  
  //! Sets the name of the starting nonterminal and the name of the file containing it.
  void set_start_nonterm(const std::string& start_name, const std::string& start_grammar_name)
  {
    m_grammar.set_start_nonterm(start_name,start_grammar_name);
  }

  //! Loads a grammar-file together with its configuration file (mutliple calls possible)
  void add_grammar(const std::string& g_name, const std::string& c_name)
  {
    m_grammar.add_grammar(g_name, c_name);
  }

  //! Processes the set of grammar files added, computes GLALR table.
  void build_parser(void)
  {
    m_grammar.remove_unreachable();
    process_grammar(m_grammar.get_grammar(), m_grammar.get_nonterm_count());
  }

  Parser(unsigned verbose = 0)
  :m_verbose_level(verbose), m_grammar(verbose)
  {}

  //! Returns the value of #m_verbose_level
  unsigned get_verbose_level(void)
  { 
    return m_verbose_level;
  }

  //! Sets the value of #m_verbose_level
  void set_verbose_level(unsigned vl)
  {
    m_verbose_level = vl;
  }

  //! Returns the result of the last parsing.
  bool get_parsing_result(void)
  {
    return m_last_accepted;
  }

  //! Returns the position of an error occuring during the last parsing.
  /** When the last parsing is successful, the return value is not defined.
   */
  unsigned get_error_position(void)
  {
    return m_error_position;
  }

  //! Returns the semantic string of the last parsing
  /** When the last parsing is unsuccessful, the return value is not defined.
   */
  std::string get_semantic_string(void)
  {
    return m_semantic_string;
  }

private:
  //! Stores the result of the last parsing. 
   bool m_last_accepted;

  //! Stores the position of an error occuring during the last parsing.
   unsigned m_error_position;

  //! Stores the semantic string of the last parsing.
   std::string m_semantic_string;

  //! The amount of information written to std::cerr
  unsigned m_verbose_level;
   
  //! The set of pending shift actions
  std::set<QMember> m_q;
  
  //! The set of pending reductions
  std::set<RMember> m_r;
  
  //! The pointer to the GLALR table
  /** Dynamic memory is used, because the size of the table may differ.
   */
  LalrTable *m_table;
  
  //! The gss used during the parsing
  GSS m_gss;
  
  //! Grammar-loading class
  AnyBnfLoad m_grammar;
  
  //! The subroutine of the parser, processes shift actions.
  /** The first parameter is the level in the gss it works in.
   *  The second and the third parameter are the following input symbols.
   */  
  void shifter(unsigned i, int a_i_plus_1, int a_i_plus_2);

  //! The subroutine of the parser, processes reductions.
  /** The first parameter is the level in the gss it works in.
   *  The second parameter is the following input symbol.
   */ 
  void reducer(unsigned i, int a_i_plus_1);
  
  //! Allocates memory, loads the grammar structure and creates the GLALR(1) table.
  /** The first parameter is the grammar structure - it is a multimap; the key
   *  is the LHS of the rule, the value is the RHS of the rule. The number of
   *  nonterminals is the second parameter. It is used for the memory to be properly
   *  allocated.     
   *  \warning Must be called before any parse_word() call.    
   *
   */
  void process_grammar(const std::multimap<int, std::vector<int> >& grammar, unsigned nonterm_count);
};

#endif

// end of file
