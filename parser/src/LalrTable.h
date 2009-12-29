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

#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <list>
#include <stdexcept>
#include <fstream>

#include <limits.h>

/** \brief This is the main class for building GLALR table.
 *
 *  It takes an augemented grammar and performs various actions.
 */  
class LalrTable
{
  /** \brief The structure used for storing LR(1) items together with the information
   *         about propagating the lookaheads.
   *   
   *   The 4-tuple containing the number of the rule, the position of the dot, the
   *   set of the lookaheads and the set of items the lookaheads propagate to.
   *   If the rule A -> x.YZ has number 5, then the item [A -> x.YZ, a/b/c]
   *   is stored as [5, 1, {a, b, c}, {set of items the lookaheads propagate to}]  
   */
  class lr1_ext_item
  {
  public:
    int rule_number;    //!<the number of the rule
    int dot_position;  //!<the position of the dot
    std::set<int> lookaheads;  //!< the set of lookaheads
    /** The pair contains the state and the number of the item in that state. */
    std::set<std::pair<int, int> > propagate_to; //!<the set of items the lookaheads propagate to
  };

 
public:
  //! All the constants used in the class
  enum constants {end_of_input = -256, //!<special terminal symbol for the end of input, usually $
                  cross_char = 300,    //!<symbol used during discovering propagating lookaheads, usually #
                  epsilon = 10000      //!<symbol denoting empty word
                  };

  /**
   *  \brief The structure used for storing parsing actions.
   *  
   *  The action proper is stored in the variable what. Is is either shift or
   *  reduce. If shift is the parsing action, next_state stores the state the DFA
   *  should go to after shifting the symbol. If reduce is the action, reduce_by
   *  stores the number of the rule  the reduction is to be done by, reduce_length
   *  stores the length of the reduction.    
   */ 
  class action
  {
  public:
    enum{shift = 0, reduce = 1} what;  //!<the action proper
    int next_state; //!<the number of the state the DFA goes to after shifting
    int reduce_by;  //!<the number of the rule the reduction is done by
    int reduce_length;  //!<the length of the reduction
  };
private: 
  /** \brief Encapsulation of a standard std::vector<int> for the purposes of 
   *         storing right sides of the rules
   *  
   *   It does not support assingning new values to the members, but it provides
   *   the information about whether the member is a marked nonterminal. When used
   *   as usually, it returns the real number of the nonterminal (removes marking).
   *   The maximum value it can store is INT_MAX/2 - 1. If a number x is greater
   *   than INT_MAX/2 - 1, it is said to be marked. The real value of x can
   *   be got as INT_MAX - x. 
   *  
   *  \warning Does not support assigning values.  
   */ 
  class marked_vector
  {
    std::vector<int> m_data; //!<the encapsulated vector
  public:
  
    //! returns the value on the position [index], removes marking of the value
    int operator[](unsigned index)
    {
      return (m_data[index] < INT_MAX /2) ? m_data[index]:(INT_MAX - m_data[index]);
    }
  
    //! returns the value on the position [index], removes marking of the value
    int at(unsigned index)
    {
      return (m_data.at(index) < INT_MAX /2) ? m_data[index]:(INT_MAX - m_data[index]);
    }

    //! returns the size of the encapsulated vector
    size_t size(void)
    {
      return m_data.size();
    }
    
    //! returns true if the symbol on the position [index] is marked
    bool marked(unsigned index)
    {
      return m_data.at(index) >= INT_MAX /2;
    }
  
    //! returns true if the symbol on the position [index] is a terminal symbol
    bool is_terminal(unsigned index)
    {
      return (m_data.at(index) <= 0);
    }
    //! returns true if the symbol on the position [index] is a nonterminal symbol
    bool is_nonterminal(unsigned index)
    {
      return (m_data.at(index) > 0);
    }
  
    //! constructor with one parameter - std::vector<int>
    marked_vector(std::vector<int> input)
    {
      m_data = input;
    }
  };
    
  //! The variable stores the number of the nonterminals used in the grammar
  unsigned m_nonterm_count;  

  //! The structure used for storing the grammar
  /** Each rule is represented as the pair containing the nonterminal on the
   *  left side and the vector of the right side.
   */
  std::vector<std::pair<int, marked_vector> > m_rules;

  //! This structure stores grammar rules, used for fast searching.
  /** The number of the rule is stored in the last position of the
   *  right side of the rule.
   */
  std::multimap<int, marked_vector> m_rules_map;
  
  
  
  //! The structure used for storing LR(0) items
  /** The vector contains the sets of items (states of the DFA), each state
   *  is though a set of LR(0) items, LR(0) item is stored as a pair consisting
   *  of the number of the rule and the position of the dot.
   *  [X -> AB.Cd] is an LR(0) item for example. If the number of the rule
   *  X -> ABCd is 8, the item is represented as (8, 2).   
   */
  std::vector<std::set<std::pair<int, int> > > m_items;

  //! The structure stores the same information as #m_items, better for searching.
  /** As the numeric identifier of each item (state) is needed, a map is used
   *  in place of a set. The value is the identifier.
   */
  std::map<std::set<std::pair<int, int> >, unsigned> m_items_map;
  
  
  //! The structure used for storing go_to information for each state and grammar symbol
  /** m_go_to[s][g] contains the number of the state the DFA goes to after
   *  reading symbol g in state s. The way the indexing of the symbols is done is
   *  a bit confusing. Terminal symbols (ASCII characters from -1 to -255 in the program)
   *  have its entries on the indexes from 1 to 255, nonterminal symbols 
   *  (from 0 to #m_nonterm_count) have its entries on the indexes starting with 256.
   *  The index of the entry for a nonterminal x is computed as 256 + x.   
   *  
   *  \warning The way the symbols are indexed is a bit confusing.   
   */  
  std::vector<std::vector<int> > m_go_to;
  
  //! The structure used for storing LR(1) items
  /** m_ext_items[s][n] contains the n-th item in the state s. The vector is now
   *  used instead of the set, becase the number of the item is needed.
   */
  std::vector<std::vector<lr1_ext_item> > m_ext_items;
  
  //! The structure for storing the GLALR table proper.
  /** m_table[s][g] contains the set of actions possible in the state s when there
   *  is the symbol g in the input.
   */   
  std::vector<std::vector<std::set<action> > > m_table;
  
  //! the set of terminal symbols (includint epsilon) x such that A ->* x<anything> stored for each nonterminal A
  std::vector<std::set<int> > m_firsts;
  
  //! the set of nonterminal symbols B such that A ->* B<anything> stored for each nonterminal A, ->* is the rightmost derivation.
  std::vector<std::set<int> > m_nont_firsts;
  
  //! the set of terminal symbols (includint epsilon) x such that A ->* x<anything> stored for each nonterminal A, ->* is the rightmost derivation.
  std::vector<std::set<int> > m_neps_firsts;
  
  /** \brief the set of nonterminal symbols B such that A ->* B<anything> stored
   *         for each nonterminal A (->* is the rightmost derivation) together
   *         with with the symbols that could appear after B. 
   *  
   *  If A ->* B<beta>, then m_ext_nont_firsts[A] contains the pair (B, S) and
   *  first(<beta>) is a subset of S.
   */    
  std::vector<std::map<int, std::set<int> > > m_ext_nont_firsts;
  
  
  //! Holds the number of the accepting state
  unsigned m_accepting_state;

  
  //! Prints the LR(0) item on cerr.
  void print_item(const std::pair<int, int>& item);
  
  //! Fills the #m_firsts structure for all the nonterminals.
  void compute_first(void);
  
  //! Fills the #m_nont_firsts structure for all the nonterminals. Must not be called before compute_first()!
  void compute_nont_first(void);
  
  //! Fills the #m_neps_firsts structure for all the nonterminals. Must not be called before compute_first()!
  void compute_neps_first(void);

  //! Fills the #m_ext_nont_firsts structure for all the nonterminals. Must not be called before compute_first()!
  void compute_ext_nont_first(void);

  //! Fills the #m_items structure.  Must not be called before compute_nont_first()!
  void compute_lr0_items(void);
  
  //! Fills the #m_ext_items structure. Must not be called before compute_lr0_items()!
  void compute_lookaheads(void);
  
  //! Fills the #m_table structure. Must not be called before compute_lookaheads()!
  void build_table(void);
  
  
public:
  //! \brief The constructor takes the number of the nonterminals and allocates memory 
  //!        needed for processing the grammar
  LalrTable(unsigned _nonterm_count)
  : m_nonterm_count(_nonterm_count)
  {
    m_firsts.resize(_nonterm_count);
    m_nont_firsts.resize(_nonterm_count);
    m_neps_firsts.resize(_nonterm_count);
    m_ext_nont_firsts.resize(_nonterm_count);
  }
  
  //! Takes the multimap and reads the data
  void load(const std::multimap<int, std::vector<int> >& input);
  
  //! The main processing procedure
  void make_lalr_table(void);
  
  //! Prints the table to the specified file. Must not be called before make_lalr_table()!
  void print_table(const std::string& file_name);

  //! Returns the set of possible action in specified state and lookahead.
  const std::set<action> & get_actions(int state, int lookahead)
  {
    return m_table.at(state).at(lookahead);
  }

  //! Returns the m_go_to[state][symbol] entry
  int get_go_to(int state, int symbol)
  {
    return m_go_to.at(state).at(symbol);
  }

  //! Returns the left side of the specified rule
  int get_lhs(int rule)
  {
    return m_rules.at(rule).first;
  }

  //! Returns the number of the accepting state
  unsigned get_accepting_state(void)
  {
    return m_accepting_state;
  }

  //! Returns the length of the rule
  size_t get_rule_length(unsigned rulenumber)
  {
    return m_rules.at(rulenumber).second.size();
  }

  //! Checks if the specified symbol of the specified rule is marked
  bool symbol_is_marked(unsigned rulenumber, unsigned token)
  {
    return m_rules.at(rulenumber).second.marked(token);
  }

  //! Returns the symbol on the specified position of the specified rule
  int get_symbol(unsigned rulenumber, unsigned token)
  {
    return m_rules.at(rulenumber).second.at(token);
  }
};

// end of file
