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

//
///////////////
//  AnyBnfLoad.h
///////////////
//  Contains the AnyBnfLoad class.
//  This is the main grammar-file-loading class.
//  First it loads the grammar and configuration file.
//  Then it performs the preprocessing steps.
//  Finally it adds the modified grammar into the table.
//  
///////////////
//  Author: Vaclav Vacek
///////////////


/*
TODO: free parameters in to_abnf
*/
#ifndef _ANYBNFLOAD_
#define _ANYBNFLOAD_

//#define _ANYBNFLOAD_TEST_

#include <vector> 
#include <queue> 
#include <set> 
#include <string>
#include <stdexcept>
#include <sstream>
#include <cstdlib>
#include <cctype>
#include <map>

#include "pcrecpp.h"
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
  //! The amount of information written to std::cerr
  unsigned m_verbose_level;
  
  AnyBnfFile m_grammar;             //!<for manipulation with input file
  AnyBnfConf m_config;              //!<for manipulation with configuration file

  //!Contains the name of the starting nonterminal specified in the global configuration file.
  std::string m_start_nonterm;
  
  //!Contains the name of the grammar containing the starting nonterminal.  
  std::string m_start_grammar;
  
  //!Contains the global number of the starting nonterminal.
  int m_start_nonterm_number;
  
  //! Is set to true when the starting nonterminal is found
  bool m_start_present;
  
  //!Contains the name of the processed grammar file
  std::string m_current_grammar;
  
  //!This structure stores the dependecies between grammar files.
  /** If the grammar file A contains nonterminal called a_n, which is
   *  defined int the grammar file B, the structure contains the name of
   *  the nonterminal, the names of the grammar files and the numbers of the
   *  nonterminal in both files. If the number is not known, value of -1 is used
   */   
  class dependency
  {
  public:
    std::string nonterm; //!<the name of the binded nonterminal
    std::string source_grammar;//!<the name of the source grammar
    std::string dest_grammar;  //!<the name of the destination grammar
    int source_num;   //!<the number of the nonterminal in the source grammar
    int dest_num;  //!<the number of the nonterminal in the destination grammar
    dependency() : source_num(-1), dest_num(-1){}
  };
  
  //!The vector of the dependencies specified if the global configuration file
  std::vector<dependency> m_dependencies;

  //!Encapsulation of int, default value is -1 instead of 0.
  class count 
  {
    int i;  //!< the encapsulated number
  public:
    count() : i(-1) {};  //!constructor, default value is -1
    void insert (int ins) {i=ins; }  //!<sets the number's value to ins.
    int& val() { return i; }  //!<returns the number
  };

  //!Maps names of nonterminals to numbers, is cleared for each grammar
  std::map<std::string, count> test_table;
  
  std::map<int, std::string> m_marked_names;

  //!Counts nonterminals continuosly (is not reset to 0 after finishing one grammar).
  int nonterm_count;

  //!Stores the number of the rules.
  int m_rule_count;

  //!Global table of the rules from all the grammar files.
  /** The set of rules is represented as the multimap, the key is the number of the
   *  nonterminal on the left side of the rule, the value is the vector
   *  of the symbols on the right side of the rule. 
   */
  std::multimap<int, std::vector<int> > m_global_table;

  //!Gives a list of positions of beginings of terminal substrings.
  std::vector<int> find_term_pos(std::string data);
    
  //!Removes empty lines from the processed file.    
  void remove_empty(void);   
  
  //!Removes all the comments from the processed file.             
  void remove_comments(void);

  //!Removes line comment of the specified position of the given string (// in C++ style)
  /** The result is written directly to the file.
   */       
  void remove_line_comment(std::string m_line, int m_position);
  
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
  //  void split_rules(void);           //1 line = 1 rule
  //  void remove_brackets(void);       //removes brackets

  //!Transforms the grammar into ABNF. Must not be called before wipe_whitespace(). 
  void to_abnf(void);    
  
  //!Transforms the ABNF grammar into BNF. Must not be called before to_abnf().         
  void to_bnf(void);
  
  //!Transforms nonterminals to numbers. Must not be called before to_bnf(). 
  /** As each grammar file receives its own interval of numbers, the procedure
   *  may be considered as namespacing as well. During this, the number of the
   *  start nonterminal and the numbers in the #m_dependencies structure
   *  are discovered
   */               
  void add_prefixes(void);
  
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
  
  //!The main processing procedure. Must not be called before load_global().
  /** It takes the name of the grammar and config file, loads the configuration
   *  and performs the steps needed for adding the grammar into the global table.
   *  The sequence of step is: remove_comments(), transform_strings(),
   *  condensate_rules(), wipe_whitespace(), to_abnf(), to_bnf(), add_prefixes(),
   *  insert_into_table().
   */    
  void add_grammar(std::string grammar, std::string config);
  
  //!Loads the global configuration file (containing information about the starting nonterminal and the dependencies).
  /** It takes the name of the global configuration file and reads its contents.
   *  The format of the file is:
   *  <first line>("starting_nonterm_name", "file_with_the_grammar_containing_the_nonterm")
   *  <following lines>("nonterm_name", "file_with_the_source_grammar", "file_with_the_destination_grammar")  
   */   
  void load_global(std::string cfg_name);
  
  //! Returns entire grammar structure
  std::multimap<int, std::vector<int> > get_grammar(void)
  {
    return m_global_table;
  }

  //! Returns the name of the specified nonterminal (if it is marked)
  std::string get_marked_name(int nonterm_number)
  {
    if(m_marked_names.count(nonterm_number) > 0)
      return m_marked_names[nonterm_number];
    else
      return "";
  }

  //!Returns the number of the nonterminals
  unsigned get_nonterm_count(void)
  {
    return nonterm_count;
  }

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

  //!Constructor takes the verbose level.
  AnyBnfLoad(unsigned verbose = 0)
  :m_verbose_level(verbose), m_start_present(false), nonterm_count(1)
  {}

};


#endif  //_ANYBNFLOAD_

