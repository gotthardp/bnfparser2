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

#ifndef _ANYBNFLOAD_
#define _ANYBNFLOAD_

//#define _ANYBNFLOAD_TEST_

#include <vector> 
#include <list>
#include <queue> 
#include <set> 
#include <string>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <map>

#include <pcrecpp.h>

#include "AnyBnfFile.h"
#include "AnyBnfConf.h"

//!  This is the main grammar-file-loading class.
/*  First it loads the grammar and configuration file.
 *  Then it performs the preprocessing steps.
 *  Finally it adds the modified grammar into the table.
 */ 
class AnyBnfLoad
{
private:
  AnyBnfFile m_grammar;             //!<for manipulation with input file
  AnyBnfConf m_config;              //!<for manipulation with configuration file

  //!Contains the name of the starting nonterminal.
  std::string m_start_symbol;
  
  //!Contains the name of the grammar containing the starting nonterminal.  
  std::string m_start_grammar;
  
  //!Contains the global number of the starting nonterminal.
  int m_start_symbol_number;
  
  //! Is set to true when the starting nonterminal is set
  bool m_start_set;

  //!Encapsulation of int, default value is -1 instead of 0.
  class count 
  {
    int i;  //!< the encapsulated number
  public:
    count() : i(-1) {};  //!constructor, default value is -1
    void insert (int ins) {i=ins; }  //!<sets the number's value to ins.
    int val() const { return i; }  //!<returns the number
  };

  class GrammarInfo
  {
  public:
    //! Maps names of nonterminals to numbers, stored for each grammar
    std::map<std::string, count> m_nonterm_names;
    //! If grammar rulenames are case sensitive
    bool m_is_case_sensitive;

    int get_nonterm_id(const std::string name) const
    {
      std::string temp;
      if(!m_is_case_sensitive)
        std::transform(name.begin(), name.end(), std::back_inserter(temp), ::tolower);
      else
        temp = name;
      std::map<std::string, count>::const_iterator pos = m_nonterm_names.find(temp);
      return (pos != m_nonterm_names.end() ? pos->second.val() : -1);
    }
  };
  typedef std::map<std::string, GrammarInfo> GrammarMap;
  //! Stores all the grammars added
  GrammarMap m_grammars;

  //!Contains the name of the processed grammar file
  GrammarMap::iterator m_current_grammar;

  //!This structure stores the dependecies between grammar files.
  /** If the grammar file A contains nonterminal called a_n, which is
   *  defined int the grammar file B, the structure contains the name of
   *  the nonterminal, the names of the grammar files and the numbers of the
   *  nonterminal in both files. If the number is not known, value of -1 is used
   */   
  class dependency
  {
  public:
    std::string source_nonterm; //!<the name of the binded nonterminal
    std::string dest_nonterm; //!<the name of the nonterm in the pointed grammar
    std::string source_grammar;//!<the name of the source grammar
    std::string dest_grammar;  //!<the name of the destination grammar
    int source_num;   //!<the number of the nonterminal in the source grammar
    int dest_num;  //!<the number of the nonterminal in the destination grammar
    dependency() : source_num(-1), dest_num(-1){}
  };
  
  //!The list of the dependencies specified if the global configuration file
  std::list<dependency> m_dependencies;

  class NonterminalInfo
  {
  public:
    std::string m_name;
    GrammarMap::const_iterator m_grammar; //!< iterator to m_grammars
    // Note: iterators to map are not invalidated by insertion/removal of other entries.
  };
  //!Maps all the nonterminal numbers back to its original names.
  //!The information is not stored for newly created nonterminals
  std::map<int, NonterminalInfo> m_names;

  //!Counts nonterminals continuosly (is not reset to 0 after finishing one grammar).
  //!Actually it is one number higher than the real count of the nonterminals
  int m_nonterm_count;

  //!Stores the number of the rules.
  int m_rule_count;

  //!Global table of the rules from all the grammar files.
  /** The set of rules is represented as the multimap, the key is the number of the
   *  nonterminal on the left side of the rule, the value is the vector
   *  of the symbols on the right side of the rule. 
   */
  std::multimap<int, std::vector<int> > m_global_table;

  //!Gives a list of positions of beginings of terminal substrings.
  std::vector<int> find_term_pos(const std::string& data);

  //! A list of paths where grammar and syntax specifications are located.
  std::vector<std::string> m_search_paths;

  //!Gives a value of the first !syntax() parameter.
  std::string get_syntax(void);
    
  //!Removes empty lines from the processed file.    
  void remove_empty(void);   
  
  //!Removes all the comments from the processed file.             
  void remove_comments(void);

  //!Processes a line comment starting at the specified position of the given string (// in C++ style)
  /** The line without the comment is written directly to the file, if the comment
   *  contains a dependency directive, it is stored.  
   */       
  void process_line_comment(const std::string& m_line, int m_position);
  
  //!Removes group comment of the specified position of the given string
  /** It reads consequent lines until the end_of_comment symbol is found.
   *  Then the rest of the line (which is not a comment) is concatenated with
   *  the beginning of the first line and the result is returned.
   */   
  std::string remove_group_comment(std::string m_line, int m_position);
  
  //!Transforms all the strings in the processed file into sequences of decimal codes. Must not be called before remove_comments(). 
  /** "abc" is thus transformed into (%d97 %d98 %d99)
   */ 
  void transform_strings(void); 
  
  //!It condensates all the rules defined on multiple lines.
  /** For example the rule
   *  A = 
   *    B (C |
   *    D)
   *  is transformed into 
   *  A = B (C | D)    
   */     
  void condensate_rules(void);

  //!Reduces whitespace of any length to a single space. Must not be called before condensate_rules(). 
  void wipe_whitespace(void);       

  //!Transforms the grammar into ABNF. Must not be called before wipe_whitespace(). 
  void to_abnf(void);    
  
  //!Transforms the ABNF grammar into BNF. Must not be called before to_abnf().         
  void to_bnf(void);
  
  //!Transforms nonterminals to numbers. Must be called before to_abnf(). 
  /** As each grammar file receives its own interval of numbers, the procedure
   *  may be considered as namespacing as well. During this, the marked nonterminals
   *  are recognised   
   */               
  void transform_names(void);
  
  //!Adds the grammar into the global table. Must not be called before add_prefixes(). 
  void insert_into_table(void);

  //!Internal function used by remove_unreachable()
  std::set<int> process_nonterm(std::set<int>* pending,
                                std::set<int>* prev_nonterm);
                                
  //!Internal function used by remove_unreachable()
  std::queue<int> get_nonterm (int nonterminal);

public:
  //!Removes non-terminals and rules that cannot be reached from the starting rule.
  /** Before doing that, new rules (defined in the #m_dependencies strucure and
   *  the rule for the starting nonterminal) are added to the table.
   */ 
  void remove_unreachable (void);

  //! Adds new path where grammar and syntax specifications are located
  void add_search_path(const char *path)
  {
    m_search_paths.push_back(path);
  }

  //! Calls add_grammar() for unresolved references
  void add_referenced_grammars();

  //!The main processing procedure. Must not be called before load_global().
  /** It takes the name of the grammar and syntax config file, loads the configuration
   *  and performs the steps needed for adding the grammar into the global table.
   *  The sequence of step is: remove_comments(), transform_strings(),
   *  condensate_rules(), wipe_whitespace(), to_abnf(), to_bnf(), add_prefixes(),
   *  insert_into_table().
   */    
  void add_grammar(const char *grammar_name, const char *syntax_name = NULL);

  //!  Sets the name of the starting nonterminal and the name of the file containing it.
  void set_start_symbol(const char *symbol_name, const char *start_grammar_name = NULL);

  //! Returns entire grammar structure
  const std::multimap<int, std::vector<int> >& get_grammar(void) const
  {
    return m_global_table;
  }

  //! Returns the name of the specified nonterminal (if it is marked)
  std::string get_marked_name(int nonterm_number)
  {
    if(m_names.count(nonterm_number) > 0)
      return m_names[nonterm_number].m_name;
    else
      return "";
  }

  //!Returns the number of the nonterminals
  unsigned get_nonterm_count(void)
  {
    return m_nonterm_count;
  }

  //!Constructor takes the verbose level.
  AnyBnfLoad()
  : m_start_set(false), m_nonterm_count(2)
  {}

};

#endif  //_ANYBNFLOAD_

// end of file
