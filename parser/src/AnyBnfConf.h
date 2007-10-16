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
 * accessible by get_base() and get_operator() functions.
 */
class AnyBnfConf
{
private:
    std::vector<std::string> m_operators;       //!< list of operators syntax
    int m_operators_max;   //!< the number of user-defined operators

    std::string rulename;  //!< holds the regular expression describing rulename
    std::string defined;   //!< holds the regular expression describing the defined symbol (like =)
    std::string terminal;  //!<  holds the regular expression describing terminal symbols
    std::string nonterminal;  //!< holds the regular expression describing nonterminal symbols
    std::string comment;  //!<  holds the regular expression describing line-comment symbol (like //)
    std::string concat;   //!<  holds the regular expression describing concatenation
    std::string alternative; //!<  holds the regular expression describing alternative
    std::string leftgroup;   //!<  holds the regular expression describing left group-bracket
    std::string rightgroup;  //!<  holds the regular expression describing right group-bracket
    std::string leftcomment; //!<  holds the regular expression describing left group-comment symbol (like /*)
    std::string rightcomment;//!<  holds the regular expression describing right grou-comment symbol (like */)
    std::string allbrackets; //!<  holds the regular expression describing all the brackets used in the grammar
    std::string csstring;    //!<  contains "true" if the strings are case-sensitive
    std::string csname;      //!<  contains "true" if the nonterminals' names are case-sensitive
      // All of the above string variables are designated to hold 
      // representations of basic grammar symbols. Term "basic symbols"
      // therefore applies for this set of variables.
          
    void reset(void);   //!< Clears all the data stored in the class.
   
    static const int m_max_line_length = 256;   //!< max input-file line length
    bool m_operators_flag;  //!< true if all the base operators are already read
    bool m_conf_loaded;     //!< true if the config file is opened
    bool check_operator(void);  //!< returns true if the line just read contains text "OPERATORS"
    std::fstream m_conf_file;   //!<input configuration file   
    std::string extract(void);  //!<extracts information from one line 

    char read_buffer[m_max_line_length];//!<the variable for storing read data
    char output[m_max_line_length];     //!<the variables for storing returned data
    
    std::fstream m_output;  //!<  test output stream 
    
public:
    std::string get_operators(int num);  //!<returns regular expression describing the requested operator
    inline int get_ops_num(void){ return m_operators_max;}//!<returns the number of user-defined operators
    std::string get_base(int base_name); //!<returns regular expression describing the requested base symbol
    std::string get_base_re(int base_name, int doub=1);//!<returns RE describing the requested base sybmol, metachars are backslashed
    bool parse_conf(const std::string& m_conf_name); //!<parses the given configuration file
    void check_conf(void); //!< testing procedure, writes all the data to a textfile
    
    
    AnyBnfConf(void) //!<constructor with no parameters
    :m_operators_max(0),m_operators_flag(false), m_conf_loaded(false)
    {}
    ~AnyBnfConf() {}
};

#endif  //_ANYBNFCONF_

// end of file
