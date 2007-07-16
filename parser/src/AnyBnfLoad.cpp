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

///////////////
//  AnyBnfLoad.cpp
///////////////
//  Contains the AnyBnfLoad's methods.
//  This is the main file-loading class.
//  First it loads the grammar and configuration file.
//  Then it performs the preprocessing steps.
//  Finally it adds the modified grammar into the table.
//  
///////////////
//   Author: Petr Slovak and Vaclav Vacek 
///////////////

#include <iostream>
#include "AnyBnfLoad.h"

void AnyBnfLoad::remove_empty(void)
{
  std::string line;
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    if(!line.empty())
      m_grammar.insert_line(line);
  }

}



void AnyBnfLoad::remove_comments(void)  
{
// there are two types of comments - 
// a) line comment (// in C++) 
//       states anything from this symbol on is a comment
// b) group comment ( /* ....  */ in C++) 
//       anything between the left and right group comment
//         symbols is a comment

  std::string current;
  //current line fetched from the file
// bool in_terminal_flag=false;
  bool get_line=true;
  bool comment_flag=false;
  int cur_pos=0;
  std::string::size_type com_pos=std::string::npos; 
      //line_comment symbol position
  std::string::size_type lcom_pos=std::string::npos;
      //left_group_comment symbol position
  std::vector<int> terminals;

  // go through the whole grammar file
  while(!m_grammar.end_of_file()||!get_line)
  {
    com_pos=std::string::npos;
    lcom_pos=std::string::npos;
    comment_flag=false;
    cur_pos=0;
  

// we need to keep checking the line, until no valid comment symbols are left 
    if (get_line)
    {
      current=m_grammar.get_line();
    }
    get_line=false;

    terminals=find_term_pos(current);
      // terminals is now filed with positions of all occurences of
  // starting terminal symbols (eg. " in ABNF or C++)
    

  // after the next two while cycles the value of com_pos resp. lcom_pos
  // will be equal to the position of first valid
  // comment_line symbol resp. left_group_comment symbol
    while (!comment_flag)
    {
      com_pos=current.find(m_config.get_base(4).c_str(),cur_pos);
      std::string::size_type i=0;
     
      if (com_pos==std::string::npos)
      {
        break;
      }
      if (!terminals.empty()) 
      {
        while ((terminals[i] < int(com_pos)) && (i+1<=terminals.size()))
        {
          i++;
        }
      }

//      if (in_terminal_flag)
//  i++;
      if (!(i%2))
      {
        comment_flag=true;
      }
      else
        cur_pos=com_pos+2; //enit na size right comment
    }
    
    comment_flag=false;
    cur_pos=0;
    while (!comment_flag)
    {
      lcom_pos=current.find(m_config.get_base(9).c_str(),cur_pos);
      std::string::size_type i=0;
    
      if (lcom_pos==std::string::npos)
      {
        break;
      }

      if (!terminals.empty())
      {
        while ((terminals[i] < int(lcom_pos)) && (i+1<=terminals.size()))
        {
          i++;
        }
      }
//        if (in_terminal_flag)
//      i++;
      if (!(i%2))
      {
        comment_flag=true;
      }
      else
        cur_pos=lcom_pos+2; //zmenit na size right comment
    }

    // we need to find out, what is first on the line - 
    // - line_comment symbol or left_group_comment symbol

    if ((com_pos==std::string::npos) && (lcom_pos==std::string::npos))
    {
      get_line=true; 
      m_grammar.insert_line(current);
      continue;
    }
    if ((com_pos!=std::string::npos) && (lcom_pos!=std::string::npos))
    {
      if (int(com_pos)<int(lcom_pos))
      {
        remove_line_comment(current,com_pos);
        get_line=true; 
      }
      else
      {
        current=remove_group_comment(current, lcom_pos);
      }
    }
    else
    {
      if (com_pos!=std::string::npos)
      {
        remove_line_comment(current,com_pos);
        get_line=true; 
      }
      else
        current=remove_group_comment(current, lcom_pos);
    }
  }
}


std::vector<int> AnyBnfLoad::find_term_pos(std::string data)
{
  bool flag=false;
  int cur_pos=0;
  std::string::size_type found=std::string::npos;
  std::vector<int> terminal_pos;
  while(!flag)
  {
    found=data.find(m_config.get_base(2).c_str(),cur_pos);
    if (found==std::string::npos)
    {
      flag=true;
    }
    else
    {
      terminal_pos.push_back(found);
      cur_pos=int(found)+1;   //m_config.get_base(2).size();
    }
  }
  return(terminal_pos);
}
 

void AnyBnfLoad::remove_line_comment(std::string line, int position)
{
  //take just the substring before line_comment symbol 
  //and insert it back into the file
  line = line.substr(0,position);
  m_grammar.insert_line(line);
}

std::string AnyBnfLoad::remove_group_comment(std::string line, int position)
{
  int state=0;
  std::string::size_type rcom_pos=std::string::npos;
  //The problem of finding right_group_comment symbols
  //that could be located a lot of lines after the left one
  //is solved using a kind of Finite State Automaton. 
  //The cycle is repeated until right_group comment 
  //symbol is found (case 3).
  
  while(1)
  {
//  if (m_grammar.end_of_file())
//    throw std::runtime_error("File error");
    switch(state)
    {
      case 0: //The starting state
      {
        rcom_pos=line.find(m_config.get_base(10).c_str(),position);
        if (rcom_pos==std::string::npos)
          state=1;
        else
          state=3;
        break;
      }
      case 1:
      {
        line=line.substr(0, position);//position,rcom_pos);
        m_grammar.insert_line(line);
        position=0;
        state=4;//2;
        break;
      }
      case 2:
      {
        rcom_pos=line.find(m_config.get_base(10).c_str(),position);
        if (rcom_pos==std::string::npos)
          state=4;
        else
          state=3;
        break;
      }
      case 3:  //Final state
      {
        int i=rcom_pos-position+m_config.get_base(10).size();
        line.erase(position,i);
        return(line);
      }
      case 4:
      {
        line=m_grammar.get_line();
        state=2;
        break;
      }
    }
  }
}

void AnyBnfLoad::condensate_rules(void)
{
  std::string current;
  std::string next;
  std::string lookout;
  //initialization of patters used
  pcrecpp::RE re_blank("^(\\s)*");
  //blank line (just whitespace (==wsp))
  pcrecpp::RE re_rule_wsp_def("^(\\s)*?"+m_config.get_base(0)+"(\\s)*"+m_config.get_base(1));
  //line in format <wsp><rulename><wsp><defined><anything else>
  pcrecpp::RE re_def(m_config.get_base(1)); 
  //is defined symbol on the line?
  pcrecpp::RE re_wsp_def("^(\\s)*"+ (m_config.get_base(1)));  
  //line in format <wsp><defined><anything else>
  pcrecpp::RE re_rule_wsp("^"+(m_config.get_base(0))+"(\\s)*");  
  //line in format <rulename><wsp>



  if(m_verbose_level >= 2)
    std::cerr<<"ENTER Condensate"<<std::endl;
  if (m_grammar.end_of_file())
  {
    throw std::runtime_error("Condensate error 1");
  }
  current=m_grammar.get_line();

  if(m_verbose_level >= 2)
    std::cerr<<"ENTER Condensate"<<std::endl;
  //skip first blank lines
  while (re_blank.FullMatch(current))
    {
      if (m_grammar.end_of_file())
      {
       throw std::runtime_error("Condensate error 2");
      }
      if(m_verbose_level >= 2)
        std::cerr<<"Blank skipped"<<std::endl;
      current=m_grammar.get_line();
    }
 
  if(m_verbose_level >= 2)
    std::cerr<<"ENTER Condensate   "<<current<<std::endl;
  //the first non_blank line has to start with <rulename>
  std::string help="^"+ m_config.get_base(0);
  if(m_verbose_level >= 2)
  {
    std::cerr<<"^"+ m_config.get_base(0)<<"  help "<<help<<std::endl;
    std::cerr <<current<<std::endl;
  }
//  if (!pcrecpp::RE("^"+ (m_config.get_base(0))).PartialMatch(current))
  if (!pcrecpp::RE("^\\w+").PartialMatch(current))
  { 
    throw std::runtime_error("Condensate error 3");
  }


  if(m_verbose_level >= 2)
    std::cerr<<"^^ENTER Condensate   "<<current<<std::endl;
  //after this if block, string in 'current' variable will have following format:
  //<rulename><?wsp><defined><?anything else>
  if (!re_rule_wsp_def.PartialMatch(current))
  {
    
    //find first none blank line
    if (m_grammar.end_of_file())
    {
      throw std::runtime_error("Condensate error 4");
    }
    next=m_grammar.get_line();
    while (re_blank.FullMatch(next))
    {
      if (m_grammar.end_of_file())
      {
        throw std::runtime_error("Condensate error 5");
      }
      if(m_verbose_level >= 2)
        std::cerr<<"getting new line because of the previous being blank"<<std::endl;
      next=m_grammar.get_line();
    }
    std::cerr<<next<<std::endl;

    //it has to have the required format
    if (!re_wsp_def.PartialMatch(next))
    {
      throw std::runtime_error("Condensate error 6");
    }

    //paste the two lines together
    current.append(next);
  }

  //for the rest of the file - all the time, the value of 'current' is in format
  //<rulename><?wsp><defined><?anything else> although it might not be, at some
  //points, completed (some pieces of the rule are still not read). 
  //At the end, each rule is exactly on one line. 
  
  bool no_next_flag=false;
  bool finished=false;

  if(m_verbose_level >= 2)
    std::cerr<<"After inicialization"<<std::endl;
  while (!finished)
  {
    //FIRST automaton - checks, if the string in 'current' is a complete rule.

    //find first non blank line
    if(m_verbose_level >= 2)
      std::cerr<<"Enter first Automaton"<<std::endl;
    if (!no_next_flag)
    {
      if(m_verbose_level >= 2)
        std::cerr<<"fetch new line ... FA"<<std::endl;
      next=m_grammar.get_line();
    }
    no_next_flag=false;

    if(m_verbose_level >= 2)
      std::cerr<<current<<"  <- current next ->  "<<next<<std::endl;
    while ((re_blank.FullMatch(next))&&(!finished))
    {
      if (m_grammar.end_of_file())
      {
        m_grammar.insert_line(current);
        finished=true;
        if(m_verbose_level >= 2)
          std::cerr<<"FINISHED HERE"<<std::endl;
        continue;
      }
      next=m_grammar.get_line();
      if(m_verbose_level >= 2)
        std::cerr<<"Enter first Automaton BLANK MATCH"<<std::endl;
    }
    
    if (finished)
    {
      if(m_verbose_level >= 2)
        std::cerr<<"CONTINUE in the 1st automaton"<<std::endl;
      continue;
    }

    if (re_rule_wsp_def.PartialMatch(next))
    {
      //both, rulename and definition, are in 'next' line ==> the rule in 'current' is complete
      //so we will write it back and continue
      m_grammar.insert_line(current);
      if(m_verbose_level >= 2)
        std::cerr<<"Enter first Automaton RULE MATCH"<<std::endl;

      current.assign(next);
      continue;
    }

    //SECOND automaton - tries to find beginning of the next rule by looking for
    //defined sequence and then backtracking (one line at most). Therefore we
    //need just three variables - current, next, lookout.
    
    if(m_verbose_level >= 2)
    {
      std::cerr<<"Enter second Automaton"<<std::endl;
      std::cerr<<current<<"  <- current next ->  "<<next<<std::endl;
    }
    //first non-blank line into lookout
    if (m_grammar.end_of_file())
    {
      current.append(next);
      m_grammar.insert_line(current);
      finished=true;
      if(m_verbose_level >= 2)
        std::cerr<<"FINISHED HERE - blank line SA"<<std::endl;
      continue;
    }
    lookout=m_grammar.get_line();
    while ((re_blank.FullMatch(lookout))&&(!finished))
    {
      if (m_grammar.end_of_file())
      {
        current.append(next);
        m_grammar.insert_line(current);
        finished=true;
        if(m_verbose_level >= 2)
          std::cerr<<"FINISHED HERE - FULL MATCH SA"<<std::endl;
        continue;
      }

      if(m_verbose_level >= 2)
        std::cerr<<"Load blank into lookahead in SA"<<std::endl;
      lookout=m_grammar.get_line();
    }

    if (re_def.PartialMatch(lookout))
    { 
      if(m_verbose_level >= 2)
        std::cerr << "re_def.PartialMatch" << std::endl;
    // if there the defined sequence is in 'lookout' string two options are open:
      if (re_rule_wsp_def.PartialMatch(lookout))
      {
        //rulename and definition are both in the 'lookout' line
        if(m_verbose_level >= 2)
          std::cerr<<"rulename and definition are both in the 'lookout' line"<<std::endl;
        current.append(next);
        m_grammar.insert_line(current);
        current.assign(lookout);
        continue;
      }

      if (re_wsp_def.PartialMatch(lookout))
      {
  //the defined substring is the first non-blank sequence in 'lookout'
  //line ==> rulename has to be the string in 'next'.
        if (!re_rule_wsp.PartialMatch(next))
        {
          throw std::runtime_error("Condensate error 10");
        }
        if(m_verbose_level >= 2)
          std::cerr<<"re_wsp_def"<<std::endl;
        m_grammar.insert_line(current);
        next.append(lookout);
        current.assign(next);
        continue;
      }
    }
    else
    {
      // defined is not on lookout line ==> the string in 'next' still belongs
      // to rule in 'current'. 
      if(m_verbose_level >= 2)
        std::cerr<<"else Second Automaton"<<std::endl;
      no_next_flag=true;
      current.append(next);
      next.assign(lookout);
    }
  }
  if(m_verbose_level >= 2)
    std::cerr<<"PASS SUCCESFULL"<<std::endl;
}


void AnyBnfLoad::wipe_whitespace(void)
{
  std::string line;
  pcrecpp::RE re_white("\\s+");
  pcrecpp::RE re_start_white("^\\s+");

  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    re_start_white.Replace("", &line);
    re_white.GlobalReplace(" ", &line);
    m_grammar.insert_line(line);
  }
}
  
/*
//It will be never used propably
void AnyBnfLoad::split_rules(void)
{
  unsigned position, start_pos;
  int brackets;
  std::string line, left;
  while(!(m_grammar.end_of_file()))
  {
    line = m_grammar.get_line();
    position = line.find(m_config.get_base(1));  //finds the definition symbol
    left = line.substr(0, position);             //stores left side of the rule     
    position += m_config.get_base(1).size();     //skips the symbol

    brackets = 0;
    start_pos = position;
    while(position < line.size())
    {
      //left bracket
      if(line.substr(position, m_config.get_base(7).size())==(m_config.get_base(7)))
      {
        position += m_config.get_base(7).size();
        brackets++;
      }

      //right bracket
      else if(line.substr(position, m_config.get_base(8).size())==(m_config.get_base(8)))  
      {
        position += m_config.get_base(8).size();
        brackets--;
      }

      //alternative
      else if((line.substr(position, m_config.get_base(6).size())==(m_config.get_base(6))) && (brackets==0))
      {
        m_grammar.insert_line(left + m_config.get_base(1) + line.substr(start_pos, position - start_pos));
        position += m_config.get_base(6).size();
        start_pos = position;

      }
   
      else position++;
      
    }
    
    m_grammar.insert_line(left + m_config.get_base(1) + line.substr(start_pos, \
    position - start_pos));

    
    
  }

}
*/


void AnyBnfLoad::transform_strings(void)
{
  std::ostringstream stream_helper;
  std::string line;
  bool in_string=false;
  bool case_insensitive=true;
  unsigned position;
  char read_char;


  if(m_config.get_base(12) == "true")
    case_insensitive = false;
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    position = 0; 
    
    while(position < line.size())
    {
      if(line.substr(position, m_config.get_base(2).size())==(m_config.get_base(2)))
      {
        position += m_config.get_base(2).size();
        in_string = !in_string;
        if(in_string)
          stream_helper << m_config.get_base(7);
        else stream_helper << m_config.get_base(8);
      }
      else if(!in_string) stream_helper << line.at(position++);
      else
      {
        read_char = line.at(position++);
        if(isalpha(read_char) && case_insensitive)
        {
            read_char = tolower(read_char);
            stream_helper << m_config.get_base(7) << \
              "%d" << static_cast<int>(read_char) << m_config.get_base(6) << \
              "%d" << static_cast<int>(toupper(read_char)) << \
              m_config.get_base(8);
        }
        else
          stream_helper << "%d" << static_cast<int>(read_char) << ' ';
      }
    }
    m_grammar.insert_line(stream_helper.str());
    if(m_verbose_level >= 2)
      std::cerr << stream_helper.str()<<std::endl;
    stream_helper.str("");
  }
  
  if(in_string)
    throw(std::runtime_error("File error"));
}

/*
//It will be never used propably.
void AnyBnfLoad::remove_brackets(void)
{
  std::string line, rule, new_name;
  std::ostringstream stream_helper;
  unsigned rulecounter = 0;
  
  pcrecpp::RE bracket_take(m_config.get_base_re(7) + "([^" + \
      m_config.get_base_re(7) +  m_config.get_base_re(8)+"]*)" + \
      m_config.get_base_re(8));
  
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    while(bracket_take.PartialMatch(line, &rule))
    {
      stream_helper << "brack" << rulecounter++;
      new_name = stream_helper.str();
      bracket_take.Replace(new_name, &line);
      m_grammar.insert_line(new_name +  " = " + rule);
      stream_helper.str("");
    }
    m_grammar.insert_line(line);
  }
}     
*/
void AnyBnfLoad::to_abnf(void)
{
  //Will be modified
  std::string oper, pattern, definition;
  std::vector<pcrecpp::RE *> releft;
  pcrecpp::RE *construct;
  std::vector<std::string> right;
  std::string::size_type position;

//pcrecpp::RE special_chars("([\\?\\|\\.\\[\\]\\^\\$\\(\\)\\*\\+\\{\\}\\\\])");
  pcrecpp::RE escaped_ids("\\\\[1-9]");
  pcrecpp::RE escaped_nums("\\\\\\^[1-9]");
  std::string non_brack_word("\\\\s*([^" + m_config.get_base_re(11, 2) + "]*)\\\\s*");

  std::string numbers("\\\\s*([0-9]+)\\\\s*");
  
  for(int i = 0; i < m_config.get_ops_num(); i++)
  {
    //First we split the rule into pattern and its definition
    oper = m_config.get_operators(i);
    position = oper.find("=");
    if(position == std::string::npos)
    {
      if(m_verbose_level >= 2)
        std::cerr << "Invalid operator line - skipping..." << std::endl;
      continue;
    }
    pattern = oper.substr(0, position);
    definition = oper.substr(position + 1);

    //Then outer whitespace must be deleted
    position = pattern.find_first_not_of("\n\t\r ");
    if(position != std::string::npos)
      pattern = pattern.substr(position);
    position = pattern.find_last_not_of("\n\t\r ");
    if(position != std::string::npos)
      pattern = pattern.substr(0, position + 1);
    
    position = definition.find_first_not_of("\n\t\r ");
    if(position != std::string::npos)
      definition = definition.substr(position);
    position = definition.find_last_not_of("\n\t\r ");
    if(position != std::string::npos)
      definition = definition.substr(0, position + 1);
    
    //regexp special chars must be backslashed
    //Will be replaced with find and consume
    {   
        std::string source;
        source = pattern;
        unsigned pos_before = 0;
                   
        pattern = "";
        position = source.find_first_of("?|.[]$()*+{}");
        while(position != std::string::npos)
        {    
          pattern += source.substr(pos_before, position - pos_before) +\
           "\\" + source.at(position);
          pos_before = position + 1;
          position = source.find_first_of(".[]$()*+{}", pos_before);
        }    
                       
       pattern += source.substr(pos_before);
    }
    
    //Metachars \1 ... \n must be replaced by regexps.
    escaped_ids.GlobalReplace(non_brack_word, &pattern);
    escaped_nums.GlobalReplace(numbers, &pattern);
    if(m_verbose_level >= 2)
      std::cerr << pattern << "->" << definition << std::endl;
    construct = new pcrecpp::RE(pattern);
    releft.push_back(construct);
    right.push_back(definition);
  }
  construct = new pcrecpp::RE(m_config.get_base_re(7) + \
      "\\s*([^" + m_config.get_base_re(11) + "]*)\\s*" + \
      m_config.get_base_re(8));
  releft.push_back(construct);
  right.push_back("\\1");

  std::string line, result;
  int rulecounter = 0;
  unsigned priority;
  bool was_change;
  std::ostringstream stream_helper("");
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    was_change = true;
    priority = 0;
    while(was_change)
    {
      if(releft.at(priority)->Extract(right.at(priority), line, &result))
      {
        stream_helper << "toabnf" << rulecounter++ << ' ';
        releft.at(priority)->Replace(stream_helper.str(), &line);
        priority = 0;
        m_grammar.insert_line("\a" + stream_helper.str() + "= " + result);
        if(m_verbose_level >= 2)
          std::cerr << stream_helper.str() + "= " + result << std::endl;
        stream_helper.str("");
      }
      else if(priority == right.size() - 1)
        was_change = false;
      else
        priority++;
    }
    m_grammar.insert_line(line);
    if(m_verbose_level >= 2)
      std::cerr << line << std::endl;
  }
  m_grammar.swap();

  pcrecpp::RE def(m_config.get_base_re(1));
  pcrecpp::RE con(m_config.get_base_re(5));
  pcrecpp::RE alt(m_config.get_base_re(6));
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    if(line.at(0) == '\a')
    {
      line = line.substr(1);
      alt.GlobalReplace("/", &line);
      con.GlobalReplace(" ", &line);
    }
    else
    {
      def.Replace("=", &line);
      con.GlobalReplace(" ", &line);
      alt.GlobalReplace("/", &line);
    }
    if(m_verbose_level >= 2)
      std::cerr << line <<std::endl;
    m_grammar.insert_line(line);
  }

  //Freeing the dynamic memory
  for(unsigned i = 0; i < releft.size(); i++)
    delete releft.at(i);
}

//Transforms abnf grammar to bnf
void AnyBnfLoad::to_bnf(void)
{
  std::string line, rule, new_name;
  std::ostringstream stream_helper;
  int rulecounter = 0;

  
  //Removing %d50.60.70
  //example %d50.60.70 -> %d50 %d60.70 -> %d50 %d60 %d70
  pcrecpp::RE term_concat("%([bdx])([0-9A-F]+)\\.([0-9A-F]+)");
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    while(term_concat.Replace("%\\1\\2 %\\1\\3", &line))
        ;
    m_grammar.insert_line(line);
  }
  m_grammar.swap();

  
  //Removing %d50-70
  pcrecpp::RE term_range("%([bdx])([0-9A-F]+)-([0-9A-F]+)");
  std::string leftbound, rightbound;
  long leftb, rightb;
  int base;
  char *err;
  char bdx;
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    while(term_range.PartialMatch(line, &bdx, &leftbound, &rightbound))
    {
      stream_helper << "range" << rulecounter++;
      new_name = stream_helper.str();
      term_range.Replace(new_name,  &line);
      stream_helper.str("");
      switch(bdx)
      {
        case 'x':
          base = 16;
          break;
        case 'b':
          base = 2;
          break;
        case 'd':
          base = 10;
      }
      leftb = strtol(leftbound.c_str(), &err, base);
      rightb = strtol(rightbound.c_str(), &err, base);
      //listing the range
      for(int i = leftb; i <= rightb; i++)
      {
        stream_helper << new_name << '=' << "%d"<< i;
        m_grammar.insert_line(stream_helper.str());
        stream_helper.str("");
      }
        
    }
    m_grammar.insert_line(line);
  }
  m_grammar.swap();

 
  //Removing %x and %b by transforming it into a decimal number
  pcrecpp::RE term_binhex("%([bx])([0-9A-F]+)");
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    while(term_binhex.PartialMatch(line, &bdx, &leftbound))
    {
      if(bdx == 'x')
        base = 16;
      else base = 2;
      leftb = strtol(leftbound.c_str(), &err, base);
      stream_helper << "%d" << leftb;
      term_binhex.Replace(stream_helper.str(), &line);
      stream_helper.str("");
    }
    m_grammar.insert_line(line);
      
  }
  m_grammar.swap();


  //Transforming [] into 0*1()
  pcrecpp::RE optional("\\[([^\\[\\]]+)\\]");
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    while(optional.Replace("0*1(\\1)", &line))
      ;
    m_grammar.insert_line(line);
  }
  m_grammar.swap();


  //Removing all the brackets (now there are only round brackets)
  //The content of each bracket is moved to a new rule
  pcrecpp::RE bracket_take("\\(([^\\(\\)]*)\\)");
  rulecounter = 0;
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    while(bracket_take.PartialMatch(line, &rule))
    {
      
      stream_helper << "abrack" << rulecounter++;
      new_name = stream_helper.str();
      bracket_take.Replace(' ' + new_name, &line);
      
      m_grammar.insert_line(new_name +  " = " + rule);
      stream_helper.str("");
    }
    m_grammar.insert_line(line);
  }
  m_grammar.swap();

  //Transforming rules with *
  pcrecpp::RE num_only("\\s+(\\d+)\\s*([a-zA-Z][a-zA-Z0-9\\-]*@?)");
  pcrecpp::RE aster_only("\\s+\\*\\s*([a-zA-Z][a-zA-Z0-9\\-]*@?)");
  pcrecpp::RE fixed_range("\\s+(\\d+)\\*(\\d+)\\s*([a-zA-Z][a-zA-Z0-9\\-]*@?)");
  pcrecpp::RE at_most("\\s+\\*(\\d+)\\s*([a-zA-Z][a-zA-Z0-9\\-]*@?)");
  pcrecpp::RE at_least("\\s+(\\d+)\\*\\s*([a-zA-Z][a-zA-Z0-9\\-]*@?)");
  int m, n;
  rulecounter = 0;
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();

    //Removing <n> <elem> - like rules
    while(num_only.PartialMatch(line, &n, &rule))
    {
      stream_helper << "numonly" << rulecounter++;
      new_name = stream_helper.str();
      num_only.Replace(' ' + new_name, &line);
      stream_helper.str("");
      for(int i = 0; i < n; i++)
        stream_helper << rule << ' ';
      m_grammar.insert_line(new_name + " = " + stream_helper.str());
      if(m_verbose_level >= 2)
        std::cerr << new_name + " = " + stream_helper.str() <<std::endl;
      stream_helper.str("");
    }

    //Removing *<elem> - like rules
    while(aster_only.PartialMatch(line, &rule))
    {
      stream_helper << "astonly" << rulecounter++;
      new_name = stream_helper.str();
      aster_only.Replace(' ' + new_name, &line);
      stream_helper.str("");
      m_grammar.insert_line(new_name + " = ");
      if(m_verbose_level >= 2)
        std::cerr << new_name << " = "<<std::endl;
      m_grammar.insert_line(new_name + " = " + rule + ' ' + new_name);
      if(m_verbose_level >= 2)
        std::cerr << new_name + " = " + rule + ' ' + new_name <<std::endl;
    }

    //Removing <n>*<m> <elem> - like rules
    while(fixed_range.PartialMatch(line, &m, &n, &rule))
    {
      if(m > n && m_verbose_level >= 1)
      {
        std::cerr << "VAROVANI: interval " << m << " - " << n << " je prazdny." << std::endl;
        std::cerr << line << std::endl;
      }
      stream_helper << "range" << rulecounter++;
      new_name = stream_helper.str();
      fixed_range.Replace(' ' + new_name, &line);
      stream_helper.str("");
      for(int i = 0; i < m ; i++)
        stream_helper << rule << ' ';
      for(int i = m; i <= n ; i++)
      {
        m_grammar.insert_line(new_name + " = " + stream_helper.str());
        if(m_verbose_level >= 2)
          std::cerr << new_name + " = " + stream_helper.str()<<std::endl;
        stream_helper << rule << ' ';
      }
      stream_helper.str("");
    }

    //Removing *<m> <elem> - like rules
    while(at_most.PartialMatch(line, &m, &rule))
    {
      stream_helper << "atmost" << rulecounter++;
      new_name = stream_helper.str();
      at_most.Replace(' ' + new_name, &line);
      stream_helper.str("");
      for(int i = 0; i <= m ; i++)
      {
        m_grammar.insert_line(new_name + " = " + stream_helper.str());
        if(m_verbose_level >= 2)
          std::cerr << new_name << " = " << stream_helper.str() << std::endl;
        stream_helper << rule << ' ';
      }
    }

    //Removing <m>* <elem> - like rules
    while(at_least.PartialMatch(line, &m, &rule))
    {
      stream_helper << "atleast" << rulecounter;
      new_name = stream_helper.str();
      at_least.Replace(' ' + new_name, &line);
      stream_helper.str("");
      stream_helper << new_name << " = ";
      for(int i = 0; i < m; i++)
        stream_helper << rule << ' ';
      stream_helper << "iterator" << rulecounter;
      m_grammar.insert_line(stream_helper.str());
      if(m_verbose_level >= 2)
        std::cerr << stream_helper.str() << std::endl;
      stream_helper.str("");
      stream_helper << "iterator" << rulecounter << " = ";
      m_grammar.insert_line(stream_helper.str());
      if(m_verbose_level >= 2)
        std::cerr << stream_helper.str() << std::endl;
      stream_helper << rule << ' ' << "iterator" << rulecounter++;
      m_grammar.insert_line(stream_helper.str());
      if(m_verbose_level >= 2)
        std::cerr << stream_helper.str() << std::endl;
      stream_helper.str("");
    }
    m_grammar.insert_line(line);
    if(m_verbose_level >= 2)
      std::cerr << line <<std::endl;
  }
  m_grammar.swap();

  //Splitting rules
  //Now there are no brackets, each "/" means rule-split.
  std::string::size_type position;
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    position = line.find("=") + 1;
    new_name = line.substr(0, position);
    line = line.substr(position);
    position = line.find("/");
    while(position != std::string::npos)
    {
      rule = line.substr(0, position);
      line = line.substr(position + 1);
      position = line.find("/");
      m_grammar.insert_line(new_name + rule);
      if(m_verbose_level >= 2)
        std::cerr << new_name + rule << std::endl;
    }
    m_grammar.insert_line(new_name + line);
    if(m_verbose_level >= 2)
      std::cerr << new_name + line<< std::endl;
  }
    
}

void AnyBnfLoad::add_prefixes(void)
{
  std::string current;
  std::string found_word, found_word_lc;
  std::string at_char;
  //either terminal or nonterminal
  pcrecpp::RE word("(\\%?[a-zA-Z][a-zA-Z0-9\\-]*)(@?)"); 
  while (!m_grammar.end_of_file())
  {
    current=m_grammar.get_line();
    //we need a clean copy of the original line - 
    //we can't make changes to the string the StringPiece points to
    std::string current_h=current;
    pcrecpp::StringPiece input(current_h);

   //find tokens that meet the regulation for nonterminals and change them to
   //numbers
   while ((word.FindAndConsume(&input,&found_word, &at_char)))
    {
      //if the found word is a terminal, do nothing
      if(found_word.at(0) == '%') continue;
      std::stringstream value;
      if(m_verbose_level >= 2)
        std::cerr<<"value of nontermcount   =>"<<nonterm_count<<std::endl;

      found_word_lc = found_word;
      if(m_config.get_base(13) != "true")
      {
        for(unsigned i = 0; i < found_word.size(); i++)
          found_word_lc.at(i) = tolower(found_word_lc.at(i));
      }


      if(m_verbose_level >= 2)
        std::cerr << found_word_lc << std::endl;
      if (test_table[found_word_lc].val()==-1)
      {
        //the found word is found for the first time
        test_table[found_word_lc].insert(nonterm_count);
        nonterm_count++;
      }
      if(at_char == "") //normal non-terminal
        value<<test_table[found_word_lc].val();
      else   //marked non-terminal
      {
        value<< INT_MAX - test_table[found_word_lc].val();
        m_marked_names.insert(std::make_pair(test_table[found_word_lc].val(), found_word_lc));
      }
    //replace all occurences of the found word with the appropriate number on
    //the current line
      pcrecpp::RE(found_word + at_char).Replace(value.str(),&current);      
    }
    m_grammar.insert_line(current);
  } 

  //looking for the starting non-terminal
  if(m_start_grammar == m_current_grammar)
  {
    found_word = m_start_nonterm;
    if(m_config.get_base(13) != "true")
      for(unsigned i = 0; i < found_word.size(); i++)
        found_word.at(i) = tolower(found_word.at(i));
    m_start_nonterm_number = test_table[found_word].val();
    if(m_start_nonterm_number == -1)
      throw (std::runtime_error("File error - missing starting nonterminal"));
  }
  
  //Here we look for the numbers of the nonterminals
  //used for connecting grammars and store it in m_dependencies 
  //structure
  for(unsigned i = 0; i < m_dependencies.size(); i++)
  {
    if(m_dependencies.at(i).source_grammar == m_current_grammar)
    {
      if(m_verbose_level >= 2)
        std::cerr << "Got left side " << m_dependencies.at(i).nonterm << std::endl;
      m_dependencies.at(i).source_num = test_table[m_dependencies.at(i).nonterm].val();
    }
    if(m_dependencies.at(i).dest_grammar == m_current_grammar)
    {
      if(m_verbose_level >= 2)
        std::cerr << "Got right side " << m_dependencies.at(i).nonterm << std::endl;
      m_dependencies.at(i).dest_num = test_table[m_dependencies.at(i).nonterm].val();
    }
  }
}

void AnyBnfLoad::insert_into_table(void)
{
  std::string current;
  std::string found_word;
  std::string at;
  pcrecpp::RE word("(-?\\d+)");   
  pcrecpp::RE number("(%d)");   
  while (!m_grammar.end_of_file())
  {
    current=m_grammar.get_line();
   // change the %d for minus sign
   // ==> terminals have negative and nonterminals positive values
    number.GlobalReplace("-",&current);
    pcrecpp::StringPiece input(current);

  bool first_flag=true;
  int first;
  std::vector<int> cur_rule;

  while ((word.FindAndConsume(&input,&found_word)))
  {
    int value;
    std::stringstream h_value(found_word);

    //the first number is the nonterminal on the left side of the rule
    if (first_flag)
    {
      h_value>>first;
      first_flag=false;
    }
    else
    {
      h_value>>value;
      cur_rule.push_back(value);
    }
  }
  
  //insert into the global table
  m_global_table.insert(std::make_pair(first,cur_rule));
  m_grammar.insert_line(current);
  } 
}

std::queue<int> AnyBnfLoad::get_nonterm (int nonterminal)
{
  std::queue<int> rule_value;
  typedef std::multimap<int, std::vector<int> >::iterator CIT;
  std::pair<CIT,CIT> range_iter;

  //for all rules that have the sought nonterminal on the left side
  //take all nonterminals on the right side and put them into a queue.
  range_iter=m_global_table.equal_range(nonterminal);
  for (CIT i=range_iter.first; i!=range_iter.second; ++i)
  {
    for (unsigned vect=0; vect!=i->second.size(); vect++)
    {
      if (i->second[vect] > 0)
      {
        if(i->second[vect] > INT_MAX / 2)
          rule_value.push(INT_MAX - i->second[vect]);
        else
          rule_value.push(i->second[vect]);
      }
          
    }
  }

  range_iter=m_global_table.equal_range(INT_MAX - nonterminal);
  for (CIT i=range_iter.first; i!=range_iter.second; ++i)
  {
    for (unsigned vect=0; vect!=i->second.size(); vect++)
    {
      if (i->second[vect] > 0)
      {
        if(i->second[vect] > INT_MAX / 2)
          rule_value.push(INT_MAX - i->second[vect]);
        else
          rule_value.push(i->second[vect]);
      }
    }
  }
  return(rule_value);
}
  
std::set<int> AnyBnfLoad::process_nonterm 
          (std::set<int>* pending, std::set<int>* prev_nonterm)
{
  std::set<int>::iterator iter;
  std::set<int> next;
  std::queue<int> rule_queue;

  //For each nonterminal in the pending queue find all rules in which it
  //appears on the left side. Add into a queue all nonterminals that are
  //included in any of these rules (this is done by the get_nonterm function)
  for( iter = pending->begin(); iter != pending->end(); iter++ )
  {
    rule_queue=get_nonterm(*iter);
    int cur_nonterm;

    // for all nonterminals in the current rule run a check and insert them into
    // the 'next' set only if haven't been processed (or are not waiting for
    // processing) already
    while (!rule_queue.empty())
    {
      cur_nonterm=rule_queue.front();
      if( 
        (next.find(cur_nonterm)==next.end()) &&
        (pending->find(cur_nonterm)==pending->end()) &&
        (prev_nonterm->find(cur_nonterm)==prev_nonterm->end()) 
        )
      {
        next.insert(cur_nonterm);
      }

      rule_queue.pop();
    }
  }
  return(next);
}

void AnyBnfLoad::remove_unreachable (void)
{
  //Before removing the unusable nonterminals, connections among
  //the grammars are made
  std::vector<int> right_side;

  //inserting starting nonterminal
  right_side.push_back(m_start_nonterm_number);
  m_global_table.insert(std::make_pair(0, right_side));
  right_side.clear();

  
  for(unsigned k = 0; k < m_dependencies.size(); k++)
    if(m_dependencies.at(k).source_num != -1 &&
      m_dependencies.at(k).dest_num != -1)
    {
      right_side.push_back(m_dependencies.at(k).dest_num);
      m_global_table.insert(std::make_pair(m_dependencies.at(k).source_num, \
            right_side));
      right_side.clear();
    }
  
      

  typedef std::multimap<int, std::vector<int> >::iterator CIT;
  std::pair<CIT,CIT> range_iter;
  std::set<int> processed_nonterm;
    //set of nonterminals that were already processed
  std::set<int> pending_nonterm;
    //set of nonterminals to be processed in the current cycle
  std::set<int> next_set;
    //set of nonterminals to be processed in the next cycle
  std::set<int>::iterator set_iter;


  //inicialization - insert start nonterm into pending
  next_set.insert(0); 

  //repeat until no new unprocessed nonterminals appear after one cycle

  while (!next_set.empty())
  {
    pending_nonterm.swap(next_set);
    next_set.clear();
    next_set=process_nonterm(&pending_nonterm, &processed_nonterm);
    for( 
      set_iter = pending_nonterm.begin(); 
      set_iter != pending_nonterm.end(); 
      set_iter++ 
       )
    {
      processed_nonterm.insert(*set_iter);
    }
  }


  //Now all the nonreachable rules are deleted
  for (int i=0; i!=nonterm_count; i++)//going through all the non-terminals
  {
    //if a nonterminal is reachable
    if (processed_nonterm.find(i)!=processed_nonterm.end())
    {
      if(m_verbose_level >= 2)
      {
        range_iter=m_global_table.equal_range(i);
        for (CIT rip=range_iter.first; rip!=range_iter.second; ++rip)
        {
          std::cerr << i << " = ";
          for(unsigned j = 0; j < (rip->second).size(); j++)
            std::cerr << (rip->second).at(j) << ' ';
          std::cerr << std::endl;
        }
      
        range_iter=m_global_table.equal_range(INT_MAX - i);
        for (CIT rip=range_iter.first; rip!=range_iter.second; ++rip)
        {
          std::cerr << INT_MAX - i << " = ";
          for(unsigned j = 0; j < (rip->second).size(); j++)
            std::cerr << (rip->second).at(j) << ' ';
          std::cerr << std::endl;
        }
      }
    }
    //All the rules with a nonreachable nonterminal on the left side are
    //erased
    else
    {
      m_global_table.erase(i);
      m_global_table.erase(INT_MAX - i);
    }
  }
}


void AnyBnfLoad::add_grammar(std::string grammar, std::string config)
{
  test_table.clear();
  m_current_grammar = grammar;
  
  // load grammar file
  if(m_verbose_level >= 1)
    std::cerr<<"Loading grammar"<<std::endl;
  m_grammar.load_file(grammar);

  //load grammar conf
  if(m_verbose_level >= 1)
    std::cerr<<"Loading configuration" <<std::endl;
  m_config.parse_conf(config);

//  std::cerr << "Removing empty lines" <<std::endl;
//  remove_empty();
//  m_grammar.swap();
  
  if(m_verbose_level >= 1)
    std::cerr<<"Removing comments"<<std::endl;
  remove_comments();
  m_grammar.swap();
  
  if(m_verbose_level >= 1)
    std::cerr<<"Transforming strings"<<std::endl;
  transform_strings();
  m_grammar.swap();

  if(m_verbose_level >= 1)
    std::cerr<<"Condensating rules"<<std::endl;
  condensate_rules();
  m_grammar.swap();

  if(m_verbose_level >= 1)
    std::cerr << "Reducing whitespace" << std::endl;
  wipe_whitespace();
  m_grammar.swap();

  if(m_verbose_level >= 1)
    std::cerr<<"Making ABNF"<<std::endl;
  to_abnf();
  m_grammar.swap();

  if(m_verbose_level >= 1)
    std::cerr<<"Making BNF"<<std::endl;
  to_bnf();
  m_grammar.swap();

  if(m_verbose_level >= 1)
    std::cerr<<"Nonterminals to numbers"<<std::endl;
  add_prefixes();
  m_grammar.swap();

  if(m_verbose_level >= 1)
    std::cerr<<"Inserting into global table"<<std::endl;
  insert_into_table();
  m_grammar.swap();
  
}

//Reads the global configuration file
void AnyBnfLoad::load_global(std::string cfg_name)
{
  bool first_line = true;
  pcrecpp::RE start_line("\\(\\s*\"([^\"]+)\"\\s*,\\s*\"([^\"]+)\"\\s*\\)");
  pcrecpp::RE   dep_line("\\(\\s*\"([^\"]+)\"\\s*,\\s*\"([^\"]+)\"\\s*,\\s*\"([^\"]+)\"\\s*\\)"); 
  std::fstream file;
  dependency curr_line;
  char read_buffer[256];
  file.open(cfg_name.c_str(), std::ios::in | std::ios::binary);
  if(!file)
    throw std::runtime_error("Global CFG error");
  while(!file.eof())
  {
    file.getline(read_buffer, 256);
    if(strlen(read_buffer) == 0) continue;
    if(first_line)
    {
      first_line = false;
      if(!start_line.FullMatch(read_buffer, &m_start_nonterm, &m_start_grammar))
        throw(std::runtime_error("Global CFG error - bad format"));
    }
    else
    {
      if(!dep_line.FullMatch(read_buffer, &(curr_line.nonterm),\
        &(curr_line.source_grammar), &(curr_line.dest_grammar)))
      throw(std::runtime_error("Global CFG error - bad format"));
      m_dependencies.push_back(curr_line);
    }
  }
}



#ifdef _ANYBNFLOAD_TEST_
int main(void)
{
  AnyBnfLoad first_grammar;
  try
  {
    std::cerr << ">>> Loading global config" <<std::endl;
    first_grammar.load_global("conf/global.gconf");
    std::cerr << ">>> Loading the SIP grammar" << std::endl;
    first_grammar.add_grammar("conf/druha.gram", "conf/abnf.conf");
    std::cerr << "Grammars added, removing unreachable nonterminals"<<std::endl;
    first_grammar.remove_unreachable();
  }
  catch(std::runtime_error e)
  {
    std::cerr << e.what()<< std::endl;
    return 1;
  }

  return 0;
}
#endif