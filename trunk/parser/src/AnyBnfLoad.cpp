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

#include <iostream>
#include <sys/stat.h>

#include "Debug.h"
#include "AnyBnfLoad.h"

std::string AnyBnfLoad::get_syntax(void)
{
  std::string syntax;
  pcrecpp::RE re_syntax("!syntax\\(\\s*\"([^\"]+)\"\\s*\\)");
  bool syntax_found = false;

  std::string line;
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();

    if(!syntax_found && re_syntax.PartialMatch(line, &syntax))
      syntax_found = true;

    m_grammar.insert_line(line);
  }

  if(syntax.empty())
    logTrace(LOG_INFO, "  no syntax variant indicated");
  else
    logTrace(LOG_INFO, "  indicated syntax variant: " << syntax);

  return syntax;
}

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
  bool get_line=true;
  bool comment_flag=false;
  int cur_pos=0;
  std::string::size_type com_pos=std::string::npos; 
      //line_comment symbol position
  std::string::size_type lcom_pos=std::string::npos;
      //left_group_comment symbol position

  // go through the whole grammar file
  while(!m_grammar.end_of_file()||!get_line)
  {
    com_pos=std::string::npos;
    lcom_pos=std::string::npos;
    comment_flag=false;

    // we need to keep checking the line, until no valid comment symbols are left 
    if (get_line)
    {
      current=m_grammar.get_line();
    }
    get_line=false;

    std::vector<int> terminals;
    cur_pos=0;
    std::string::size_type found;
    // get a list of positions of beginings of terminal substrings.
    if(m_config.get_terminal().size() > 0)
    {
      while((found=current.find(m_config.get_terminal().c_str(),cur_pos)) != std::string::npos)
      {
        terminals.push_back(found);
        cur_pos=int(found) + m_config.get_terminal().size();
      }
    }
    // terminals is now filed with positions of all occurences of
    // starting terminal symbols (eg. " in ABNF or C++)

    cur_pos=0;
  // after the next two while cycles the value of com_pos resp. lcom_pos
  // will be equal to the position of first valid
  // comment_line symbol resp. left_group_comment symbol
    while (!comment_flag)
    {
      com_pos=current.find(m_config.get_comment().c_str(),cur_pos);
     
      if (com_pos==std::string::npos)
        break;

      std::string::size_type i = 0;
      if (!terminals.empty()) 
      {
        while ((terminals[i] < int(com_pos)) && i < terminals.size())
        {
          i++;
        }
      }

      if (!(i%2))
      {
        comment_flag=true;
      }
      else
        cur_pos=com_pos + m_config.get_comment().size();
    }
    
    comment_flag=false;
    cur_pos=0;
    while (!comment_flag)
    {
      lcom_pos=current.find(m_config.get_leftcomment().c_str(),cur_pos);
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

      if (!(i%2))
      {
        comment_flag=true;
      }
      else
        cur_pos=lcom_pos + m_config.get_leftcomment().size();
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
        process_line_comment(current,com_pos);
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
        process_line_comment(current,com_pos);
        get_line=true; 
      }
      else
        current=remove_group_comment(current, lcom_pos);
    }
  }
}

void AnyBnfLoad::process_line_comment(const std::string& line, int position)
{
  std::string comment;
  std::string help_string;
  // reference = "!import(" 1*( destination-nonterminal [ "as" source-nonterminal ] "," ) filename ")"
  pcrecpp::RE re_dep("!import\\(\\s*((\"[^\"]+\"\\s*(?:\\s*as\\s*\"[^\"]+\"\\s*)?\\s*,\\s*)+)\"([^\"]+)\"\\s*\\)");
  pcrecpp::RE re_nonterm("^\"([^\"]+)\"\\s*(?:\\s*as\\s*\"([^\"]+)\"\\s*)?\\s*,\\s*");
  dependency curr_dep;

  //take just the substring before line_comment symbol 
  //and insert it back into the file
  comment = line.substr(position);
  m_grammar.insert_line(line.substr(0,position));

  std::string ignored; // this parameter is not used (for now)
  if(re_dep.PartialMatch(comment, &help_string, &ignored, &(curr_dep.dest_grammar)))
  {
    while(re_nonterm.PartialMatch(help_string, &(curr_dep.dest_nonterm), &(curr_dep.source_nonterm)))
    {
      re_nonterm.Replace("", &help_string);

      curr_dep.source_grammar = m_current_grammar->first;
      // by default, source and destination nonterminal are identical
      if(curr_dep.source_nonterm.empty())
      {
        logTrace(LOG_INFO, "    Import: "
          << curr_dep.dest_nonterm << " in " << curr_dep.dest_grammar << " to " << curr_dep.source_grammar);

        curr_dep.source_nonterm = curr_dep.dest_nonterm;
      }
      else
      {
        logTrace(LOG_INFO, "    Import: "
          << curr_dep.dest_nonterm << " in " << curr_dep.dest_grammar << " to "
          << curr_dep.source_nonterm << " in " << curr_dep.source_grammar);
      }

      m_dependencies.push_back(curr_dep);
    }
  } 
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
    // avoid infinite loop when group comment is not closed
    if (m_grammar.end_of_file())
      throw std::runtime_error("Syntax error: Group comment not closed");

    switch(state)
    {
      case 0: //The starting state
      {
        rcom_pos=line.find(m_config.get_rightcomment().c_str(),position);
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
        rcom_pos=line.find(m_config.get_rightcomment().c_str(),position);
        if (rcom_pos==std::string::npos)
          state=4;
        else
          state=3;
        break;
      }
      case 3:  //Final state
      {
        int i=rcom_pos-position+m_config.get_rightcomment().size();
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
  
  pcrecpp::RE re_rule_wsp_def("^(\\s)*"+m_config.get_rulename()+"@?(\\s)*"+m_config.get_defined());
  //line in format <wsp><rulename><wsp><defined><anything else>
  
  pcrecpp::RE re_def(m_config.get_defined());
  //is defined symbol on the line?
  
  pcrecpp::RE re_wsp_def("^(\\s)*"+ (m_config.get_defined()));
  //line in format <wsp><defined><anything else>
  
  pcrecpp::RE re_rule_wsp("^(\\s)*"+(m_config.get_rulename())+"@?(\\s)*");
  //line in format <rulename><wsp>

  if (m_grammar.end_of_file())
  {
    throw std::runtime_error("Syntax error: Unexpected end of file (1)");
  }
  current=m_grammar.get_line();

  //skip first blank lines
  while (re_blank.FullMatch(current))
  {
    if (m_grammar.end_of_file())
    {
     throw std::runtime_error("Syntax error: Unexpected end of file (2)");
    }
    current=m_grammar.get_line();
  }
 
  //after this if block, string in 'current' variable will have following format:
  //<rulename><?wsp><defined><?anything else>
  if (!re_rule_wsp_def.PartialMatch(current))
  {
    logTrace(LOG_DEBUG, "  Not a definiton: "<<current);
    //find first none blank line
    if (m_grammar.end_of_file())
    {
      throw std::runtime_error("Syntax error: Unexpected end of file (3)");
    }
    next=m_grammar.get_line();
    while (re_blank.FullMatch(next))
    {
      if (m_grammar.end_of_file())
      {
        throw std::runtime_error("Syntax error: Unexpected end of file (4)");
      }
      next=m_grammar.get_line();
    }

    //it has to have the required format
    if (!re_wsp_def.PartialMatch(next))
    {
      logTrace(LOG_ERR, "unexpected format '" << next << "'");
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

  while (!finished)
  {
    //FIRST automaton - checks, if the string in 'current' is a complete rule.

    //find first non blank line
    if (!no_next_flag)
    {
      next=m_grammar.get_line();
    }
    no_next_flag=false;

    while ((re_blank.FullMatch(next))&&(!finished))
    {
      if (m_grammar.end_of_file())
      {
        m_grammar.insert_line(current);
        finished=true;
        continue;
      }
      next=m_grammar.get_line();
    }
    
    if (finished)
      continue;

    if (re_rule_wsp_def.PartialMatch(next))
    {
      //both, rulename and definition, are in 'next' line ==> the rule in 'current' is complete
      //so we will write it back and continue
      m_grammar.insert_line(current);

      current.assign(next);
      continue;
    }

    //SECOND automaton - tries to find beginning of the next rule by looking for
    //defined sequence and then backtracking (one line at most). Therefore we
    //need just three variables - current, next, lookout.
    
    //first non-blank line into lookout
    if (m_grammar.end_of_file())
    {
      current.append(next);
      m_grammar.insert_line(current);
      finished=true;
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
        continue;
      }

      lookout=m_grammar.get_line();
    }

    if (re_def.PartialMatch(lookout))
    { 
      // if there the defined sequence is in 'lookout' string two options are open:
      if (re_rule_wsp_def.PartialMatch(lookout))
      {
        //rulename and definition are both in the 'lookout' line
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
      no_next_flag=true;
      current.append(next);
      next.assign(lookout);
    }
  }
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

void AnyBnfLoad::transform_strings(void)
{
  std::ostringstream stream_helper;
  std::string line;
  bool in_string=false;
  std::string::size_type position;
  char read_char;

  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    position = 0; 
    
    while(position < line.size())
    {
      if(m_config.get_terminal().size() > 0 &&
        line.substr(position, m_config.get_terminal().size()) == m_config.get_terminal())
      {
        position += m_config.get_terminal().size();
        in_string = !in_string;
        if(in_string)
          stream_helper << m_config.get_leftgroup();
        else stream_helper << m_config.get_rightgroup();
      }
      else if(!in_string) stream_helper << line.at(position++);
      else
      {
        read_char = line.at(position++);
        if(isalpha(read_char) && !m_config.is_csstring())
        {
            read_char = tolower(read_char);
            stream_helper << m_config.get_leftgroup() <<
              "%d" << static_cast<int>(static_cast<unsigned char>(read_char)) << m_config.get_alternative() <<
              "%d" << static_cast<int>(static_cast<unsigned char>(toupper(read_char))) << \
              m_config.get_rightgroup();
        }
        else
          stream_helper << "%d" << static_cast<int>(static_cast<unsigned char>(read_char)) << ' ';
      }
    }
    m_grammar.insert_line(stream_helper.str());
    logTrace(LOG_DEBUG, stream_helper.str());
    stream_helper.str("");
  }
  
  if(in_string)
    throw(std::runtime_error("BNF syntax error: nonterminated string"));
}

void AnyBnfLoad::transform_names(void)
{
///////////////////////////////////////
  std::string line, rule, new_name;
  std::ostringstream stream_helper;

  //Removing %d50.60.70
  //example %d50.60.70 -> %d50 %d60.70 -> %d50 %d60 %d70
  pcrecpp::RE term_concat("%([bdx])([0-9A-F]+)\\.([0-9A-F]+)");
  pcrecpp::RE term_in_progress("%([bdx])([0-9A-F]+)\024\\.([0-9A-F]+)");
  pcrecpp::RE term_end("\024");
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    while(term_concat.Replace(m_config.get_leftgroup() + "%\\1\\2 %\\1\\3\024", &line))
    {
      while(term_in_progress.Replace("%\\1\\2 %\\1\\3\024", &line))
        ;
      term_end.Replace(m_config.get_rightgroup(), &line);
    }
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
      stream_helper << m_config.get_leftgroup() << ' ';
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
          break;
        default:
          // this will never happen, see term_range
          throw std::runtime_error("Unexpected base indicator");
      }
      leftb = strtol(leftbound.c_str(), &err, base);
      rightb = strtol(rightbound.c_str(), &err, base);
      for(int i = leftb; i <= rightb; i++)
      {
        stream_helper << "%d" << i << ' ';
        if(i < rightb)
           stream_helper << m_config.get_alternative();
      }
      stream_helper << m_config.get_rightgroup() << ' ';
      term_range.Replace(stream_helper.str(),  &line);
      stream_helper.str("");
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
////////////////////////////////////////


  std::string current;
  std::string found_word, found_word_lc;
  std::string at_char;
  //either terminal or nonterminal
  pcrecpp::RE word("(%?" + m_config.get_rulename() + ")" + "(@?)");
  pcrecpp::RE term("%d(\\d+)");
  pcrecpp::RE concat(m_config.get_concat());
  pcrecpp::RE def(AnyBnfConf::backslash(m_config.get_defined()));

  pcrecpp::RE alternative(m_config.get_alternative());
  
  if(m_config.is_csname())
    m_current_grammar->second.m_is_case_sensitive = true;

  while (!m_grammar.end_of_file())
  {
    current=m_grammar.get_line();
    def.Replace(" ", &current);
    concat.GlobalReplace(" ", &current);
    alternative.GlobalReplace("\035", &current);
    //we need a clean copy of the original line - 
    //we can't make changes to the string the StringPiece points to
    std::string current_h=current;
    pcrecpp::StringPiece input(current_h);

    //find tokens that meet the regulation for nonterminals and change them to
    //numbers
    while ((word.FindAndConsume(&input,&found_word, &at_char)))
    {
      if(found_word.at(0) == '%')
        continue;
      std::stringstream value;

      found_word_lc = found_word;
      if(!m_config.is_csname())
      {
          for(unsigned i = 0; i < found_word.size(); i++)
          found_word_lc.at(i) = tolower(found_word_lc.at(i));
      }

      if (m_current_grammar->second.m_nonterm_names[found_word_lc].val()==-1)
      {
        //the found word is found for the first time
        //m_nonterm_count now contains numeric identifier of the nonterminal
        m_current_grammar->second.m_nonterm_names[found_word_lc].insert(m_nonterm_count);

        m_names[m_nonterm_count].m_name = found_word_lc;
        m_names[m_nonterm_count].m_grammar = m_current_grammar;

        logTrace(LOG_DEBUG, found_word_lc << " => " << m_nonterm_count);
        m_nonterm_count++;
      }
      if(at_char == "") //normal non-terminal
        value << '\036'
              << m_current_grammar->second.m_nonterm_names[found_word_lc].val()
              << '\037';
        
      else   //marked non-terminal
      {
        value << '\036' 
              << INT_MAX - m_current_grammar->second.m_nonterm_names[found_word_lc].val()
              << '\037';
      }
      //replace the previously found occurence of the found word with the appropriate number on
      //the current line
      pcrecpp::RE(found_word + at_char).Replace(value.str(),&current);      
    }
    term.GlobalReplace("\036\034\\1\037", &current);
    m_grammar.insert_line(current);
  } 
/*
  //looking for the starting non-terminal
  if(m_start_grammar == m_current_grammar)
  {
    found_word = m_start_symbol;
    if(!m_config.is_csname())
      for(unsigned i = 0; i < found_word.size(); i++)
        found_word.at(i) = tolower(found_word.at(i));
    m_start_symbol_number = test_table[found_word].val();
    if(m_start_symbol_number == -1)
      throw (std::runtime_error("File error - missing starting nonterminal"));
  }
  
  //Here we look for the numbers of the nonterminals
  //used for connecting grammars and store it in m_dependencies 
  //structure
  for(unsigned i = 0; i < m_dependencies.size(); i++)
  {
    if(m_dependencies.at(i).source_grammar == m_current_grammar)
    {
      logTrace(LOG_DEBUG, "Got left side " << m_dependencies.at(i).nonterm);
      m_dependencies.at(i).source_num = test_table[m_dependencies.at(i).nonterm].val();
    }
    if(m_dependencies.at(i).dest_grammar == m_current_grammar)
    {
      logTrace(LOG_DEBUG, "Got right side " << m_dependencies.at(i).nonterm);
      m_dependencies.at(i).dest_num = test_table[m_dependencies.at(i).nonterm].val();
    }
  }*/
}

void AnyBnfLoad::to_abnf(void)
{
  std::string pattern, definition;
  std::vector<pcrecpp::RE *> releft;
  pcrecpp::RE *construct;
  std::vector<std::string> right;
  std::string::size_type position;

//pcrecpp::RE special_chars("([\\?\\|\\.\\[\\]\\^\\$\\(\\)\\*\\+\\{\\}\\\\])");
  pcrecpp::RE escaped_seq("\\\\[1-9]");
  pcrecpp::RE escaped_nums("\\\\n[1-9]");
  pcrecpp::RE escaped_elem("\\\\e[1-9]");
  std::string non_brack_word("(\\\\s*(\036[0-9\034]+\037\\\\s*)+)");
  std::string single_element("\\\\s*(\036[0-9\034]+\037)\\\\s*");
  std::string numbers("\\\\s*([0-9]+)\\\\s*");
  
  for(AnyBnfConf::TOperatorList::const_iterator oper = m_config.get_operators().begin();
    oper != m_config.get_operators().end(); oper++)
  {
    //First we split the rule into pattern and its definition
    position = (*oper).find("=");
    if(position == std::string::npos)
    {
      logTrace(LOG_WARNING, "Skipping invalid operator line");
      continue;
    }
    pattern = (*oper).substr(0, position);
    definition = (*oper).substr(position + 1);

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
    {   
        std::string source;
        source = pattern;
        std::string::size_type pos_before = 0;
                   
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
    escaped_seq.GlobalReplace(non_brack_word, &pattern);
    escaped_nums.GlobalReplace(numbers, &pattern);
    escaped_elem.GlobalReplace(single_element, &pattern);

    construct = new pcrecpp::RE(pattern);
    releft.push_back(construct);
    right.push_back(definition);
  }
  construct = new pcrecpp::RE(AnyBnfConf::backslash(m_config.get_leftgroup()) +
      "(\\s*(\036[0-9\034]+\037\\s*)+)" +
      AnyBnfConf::backslash(m_config.get_rightgroup()));
  releft.push_back(construct);
  right.push_back("\\1");

  std::string line, result;
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
        stream_helper << '\036' << m_nonterm_count++ << "\037 ";
        releft.at(priority)->Replace(stream_helper.str(), &line);
        priority = 0;
        m_grammar.insert_line("\a" + stream_helper.str() + result);
        stream_helper.str("");
      }
      else if(priority == right.size() - 1)
        was_change = false;
      else
        priority++;
    }
    m_grammar.insert_line(line);
    logTrace(LOG_DEBUG, line);
  }

  m_grammar.swap();
  
  pcrecpp::RE term("%d(\\d+)");
  pcrecpp::RE alt("/");
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    if(line.at(0) == '\a')
    {
      line = line.substr(1);
      term.GlobalReplace("\036\034\\1\037", &line);
      alt.GlobalReplace("\035", &line);
    }
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

/*  
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
*/
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

  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    while(bracket_take.PartialMatch(line, &rule))
    {
      stream_helper << '\036' << m_nonterm_count++ << '\037';
      new_name = stream_helper.str();
      bracket_take.Replace(' ' + new_name, &line);
      
      m_grammar.insert_line(new_name + ' ' + rule);
      stream_helper.str("");
    }
    m_grammar.insert_line(line);
  }
  m_grammar.swap();

  //Transforming rules with *
  pcrecpp::RE num_only("\\s+(\\d+)\\s*(\036[0-9\034]+\037@?)");
  pcrecpp::RE aster_only("\\s+\\*\\s*(\036[0-9\034]+\037@?)");
  pcrecpp::RE fixed_range("\\s+(\\d+)\\*(\\d+)\\s*(\036[0-9\034]+\037@?)");
  pcrecpp::RE at_most("\\s+\\*(\\d+)\\s*(\036[0-9\034]+\037@?)");
  pcrecpp::RE at_least("\\s+(\\d+)\\*\\s*(\036[0-9\034]+\037@?)");
  int m, n;

  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    //Removing <n> <elem> - like rules
    while(num_only.PartialMatch(line, &n, &rule))
    {
      stream_helper << '\036' << m_nonterm_count++ << '\037';
      new_name = stream_helper.str();
      num_only.Replace(' ' + new_name, &line);
      stream_helper.str("");
      for(int i = 0; i < n; i++)
        stream_helper << rule << ' ';
      m_grammar.insert_line(new_name + ' ' + stream_helper.str());
      stream_helper.str("");
    }

    //Removing *<elem> - like rules
    while(aster_only.PartialMatch(line, &rule))
    {
      stream_helper << '\036' << m_nonterm_count++ << '\037';
      new_name = stream_helper.str();
      aster_only.Replace(' ' + new_name, &line);
      stream_helper.str("");
      m_grammar.insert_line(new_name);
      m_grammar.insert_line(new_name + ' ' + rule + ' ' + new_name);
    }

    //Removing <n>*<m> <elem> - like rules
    while(fixed_range.PartialMatch(line, &m, &n, &rule))
    {
      if(m > n)
      {
        logTrace(LOG_WARNING, "Warning: interval " << m << " - " << n << " is empty.");
        logTrace(LOG_WARNING, line);
      }
      stream_helper << '\036' << m_nonterm_count++ << '\037';
      new_name = stream_helper.str();
      fixed_range.Replace(' ' + new_name, &line);
      stream_helper.str("");
      for(int i = 0; i < m ; i++)
        stream_helper << rule << ' ';
      for(int i = m; i <= n ; i++)
      {
        m_grammar.insert_line(new_name + ' ' + stream_helper.str());
        stream_helper << rule << ' ';
      }
      stream_helper.str("");
    }

    //Removing *<m> <elem> - like rules
    while(at_most.PartialMatch(line, &m, &rule))
    {
      stream_helper << '\036' << m_nonterm_count++ << '\037';
      new_name = stream_helper.str();
      at_most.Replace(' ' + new_name, &line);
      stream_helper.str("");
      for(int i = 0; i <= m ; i++)
      {
        m_grammar.insert_line(new_name + ' ' + stream_helper.str());
        stream_helper << rule << ' ';
      }
    }

    //Removing <m>* <elem> - like rules
    while(at_least.PartialMatch(line, &m, &rule))
    {
      stream_helper << '\036' << m_nonterm_count++ << '\037';
      new_name = stream_helper.str();
      at_least.Replace(' ' + new_name, &line);
      stream_helper.str("");
      stream_helper << new_name << ' ';
      for(int i = 0; i < m; i++)
        stream_helper << rule << ' ';
      stream_helper << '\036' << m_nonterm_count << '\037';
      m_grammar.insert_line(stream_helper.str());
      stream_helper.str("");
      stream_helper << '\036' << m_nonterm_count<< '\037' << ' ';
      m_grammar.insert_line(stream_helper.str());
      stream_helper << rule << ' ' << '\036' << m_nonterm_count++ << '\037';
      m_grammar.insert_line(stream_helper.str());
      stream_helper.str("");
    }

    logTrace(LOG_DEBUG, line);
    m_grammar.insert_line(line);
  }
  m_grammar.swap();

  //Splitting rules
  //Now there are no brackets, each "/" means rule-split.
  std::string::size_type position;
  while(!m_grammar.end_of_file())
  {
    line = m_grammar.get_line();
    position = line.find(" ") + 1;
    new_name = line.substr(0, position);
    line = line.substr(position);
    position = line.find("\035");
    while(position != std::string::npos)
    {
      rule = line.substr(0, position);
      line = line.substr(position + 1);
      position = line.find("\035");
      m_grammar.insert_line(new_name + rule);
    }
    m_grammar.insert_line(new_name + line);
  }
}

void AnyBnfLoad::insert_into_table(void)
{
  std::string current;
  std::string found_word;
  std::string at;
  pcrecpp::RE word("\036(-?\\d+)\037");   
  pcrecpp::RE number("\034");   
  while (!m_grammar.end_of_file())
  {
    current=m_grammar.get_line();
   // change the \034 for minus sign
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

  if(!m_start_set)
    throw std::runtime_error("Start symbol not set");

  if(!m_start_grammar.empty())
  {
    logTrace(LOG_INFO, "  using start symbol " << m_start_symbol << " from " << m_start_grammar);
    // find start symbol in given start grammar
    m_start_symbol_number = m_grammars[m_start_grammar].get_nonterm_id(m_start_symbol);

    if(m_start_symbol_number == -1)
    {
      throw std::runtime_error("Symbol \"" + m_start_symbol
        + "\" not found in \"" + m_start_grammar + "\" specification");
    }
  }
  else
  {
    m_start_symbol_number = -1;
    // search start symbol in grammar files
    for(std::map<std::string, GrammarInfo>::const_iterator pos = m_grammars.begin();
      pos != m_grammars.end(); pos++)
    {
      logTrace(LOG_INFO, "  looking for symbol " << m_start_symbol << " in " << pos->first);
      int idpos;
      if((idpos = pos->second.get_nonterm_id(m_start_symbol)) != -1)
      {
        logTrace(LOG_INFO, "  symbol " << m_start_symbol << " found in " << pos->first);
        if(m_start_symbol_number != -1)
          logTrace(LOG_ERR, "  duplicate start symbol " << m_start_symbol << " in " << pos->first);
        else
          m_start_symbol_number = idpos;
      }
    }

    if(m_start_symbol_number == -1)
    {
      throw std::runtime_error("Symbol \"" + m_start_symbol
        + "\" not found in any specification");
    }
  }

  //inserting starting nonterminal
  right_side.push_back(m_start_symbol_number);
  m_global_table.insert(std::make_pair(1, right_side));
  right_side.clear();
  
  for(std::list<dependency>::iterator pos = m_dependencies.begin();
    pos != m_dependencies.end(); pos++)
  {
    if(m_grammars.find(pos->dest_grammar) == m_grammars.end())
    {
      std::cerr << "Warning: grammar file " << pos->dest_grammar << " referenced from " 
                << pos->source_grammar << " not loaded." << std::endl;
    }
    else
    {
      pos->source_num = m_grammars[pos->source_grammar].get_nonterm_id(pos->source_nonterm);
      pos->dest_num = m_grammars[pos->dest_grammar].get_nonterm_id(pos->dest_nonterm);

      if(pos->source_num == -1)
      {
        std::cerr << "Warning: referenced nonterminal " << pos->source_nonterm << " not present in " 
                  << pos->source_grammar << "." << std::endl;
      }
      else if(pos->dest_num == -1)
      {
        std::cerr << "Warning: referenced nonterminal " << pos->dest_nonterm << " not present in " 
                  << pos->dest_grammar << "." << std::endl;
      }
      else
      {
        right_side.push_back(pos->dest_num);
        m_global_table.insert(std::make_pair(pos->source_num, right_side));
        right_side.clear();
      }
    }
  }

  
  for(std::list<dependency>::const_iterator pos = m_dependencies.begin();
    pos != m_dependencies.end(); pos++)
  {
    if(pos->source_num != -1 && pos->dest_num != -1)
    {
      right_side.push_back(pos->dest_num);
      m_global_table.insert(std::make_pair(pos->source_num, right_side));
      right_side.clear();
    }
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
  next_set.insert(1); 

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
  bool nonterm_present;
  for (int i=1; i!=m_nonterm_count; i++)//going through all the non-terminals
  {
    //if a nonterminal is reachable
    if (processed_nonterm.find(i) != processed_nonterm.end())
    {
      nonterm_present = true;
      range_iter=m_global_table.equal_range(i);
      if(range_iter.first == range_iter.second)
        nonterm_present = false;
        
      else if(logIsEnabledFor(LOG_DEBUG))
        for (CIT rip=range_iter.first; rip!=range_iter.second; ++rip)
        {
          std::cerr << i << " = ";
          for(unsigned j = 0; j < (rip->second).size(); j++)
            std::cerr << (rip->second).at(j) << ' ';
          std::cerr << std::endl;
        }
      
      range_iter=m_global_table.equal_range(INT_MAX - i);
      if(range_iter.first == range_iter.second && !nonterm_present)
      {
        std::cerr << "Warning: nonterminal " << m_names[i].m_name << " is not defined in "
                  << m_names[i].m_grammar->first << "." << std::endl;
      }
      else if(logIsEnabledFor(LOG_DEBUG))
        for (CIT rip=range_iter.first; rip!=range_iter.second; ++rip)
        {
          std::cerr << INT_MAX - i << " = ";
          for(unsigned j = 0; j < (rip->second).size(); j++)
            std::cerr << (rip->second).at(j) << ' ';
          std::cerr << std::endl;
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

void AnyBnfLoad::add_referenced_grammars()
{
  for(std::list<AnyBnfLoad::dependency>::const_iterator pos = m_dependencies.begin();
    pos != m_dependencies.end(); pos++)
  {
    if(m_grammars[pos->dest_grammar].m_nonterm_names.empty())
    {
      logTrace(LOG_INFO, "  load " << pos->dest_grammar << " referenced from " << pos->source_grammar);
      // load referenced grammar
      // this may cause new entries appended to m_dependencies, the *pos iterator is not invalidated
      add_grammar(pos->dest_grammar.c_str());
    }
  }
}

void AnyBnfLoad::add_grammar(const char *grammar_name, const char *syntax_name)
{
  std::string line_test;

  // insert new grammar to the list, get iterator to the entry
  m_current_grammar = m_grammars.insert(GrammarMap::value_type(grammar_name, GrammarInfo())).first;

  logTrace(LOG_INFO, "## File processing start ##");

  std::string grammar_filename = grammar_name;
  // search for grammar file
  std::vector<std::string>::const_iterator gpos = m_search_paths.begin();
  while(1)
  {
    logTrace(LOG_INFO, "  searching for grammar: " << grammar_filename);

    // search for the given file
    struct stat fileinfo;
    if(stat(grammar_filename.c_str(), &fileinfo) == 0)
      break;
    if(gpos == m_search_paths.end())
      throw std::runtime_error("File not found");

    grammar_filename = *(gpos++) + "/" + grammar_name;
  }

  // load grammar file
  logTrace(LOG_INFO, "  loading grammar: " << grammar_filename);
  m_grammar.load_file(grammar_filename);

  std::string syntax_name_s;
  if(syntax_name != NULL )
  {
    logTrace(LOG_INFO, "  using given syntax: " << syntax_name);
    syntax_name_s = syntax_name;
  }
  else
  {
    // obtain syntax from the grammar file
    syntax_name_s = get_syntax();
    m_grammar.swap();

    if(syntax_name_s.empty())
      throw std::runtime_error("Syntax specification has unknown syntax");
  }

  std::string syntax_filename = "syntax/" + syntax_name_s + ".conf";
  // search for syntax configuration file
  std::vector<std::string>::const_iterator spos = m_search_paths.begin();
  while(1)
  {
    logTrace(LOG_INFO, "  searching for syntax: " << syntax_filename);

    // search for the given file
    struct stat fileinfo;
    if(stat(syntax_filename.c_str(), &fileinfo) == 0)
      break;
    if(spos == m_search_paths.end())
      throw std::runtime_error("File not found");

    syntax_filename = *(spos++) + "/syntax/" + syntax_name_s + ".conf";
  }

  // load grammar syntax confinguration
  logTrace(LOG_INFO, "  loading configuration: " << syntax_filename);
  m_config.parse_conf(syntax_filename);
//  m_config.check_conf(std::cout); // debugging output

  logTrace(LOG_INFO, "  processing comments");
  remove_comments();
  m_grammar.swap();
  
  logTrace(LOG_INFO, "  transforming strings");
  transform_strings();
  m_grammar.swap();

  logTrace(LOG_INFO, "  condensating rules");
  condensate_rules();
  m_grammar.swap();
  
  logTrace(LOG_INFO, "  reducing whitespace");
  wipe_whitespace();
  m_grammar.swap();
  
  logTrace(LOG_INFO, "  nonterminals to numbers");
  transform_names();
  m_grammar.swap();
  
  logTrace(LOG_INFO, "  making ABNF");
  to_abnf();
  m_grammar.swap();
  
  logTrace(LOG_INFO, "  making BNF");
  to_bnf();
  m_grammar.swap();
  
  logTrace(LOG_INFO, "  inserting into global table");
  insert_into_table();
  m_grammar.swap();
  
  logTrace(LOG_INFO, "## File processing end ##");
}

void AnyBnfLoad::set_start_symbol(const char *symbol_name, const char *start_grammar_name)
{
  m_start_set = true;
  m_start_symbol = symbol_name;
  m_start_grammar = start_grammar_name ? start_grammar_name : "";
}

#ifdef _ANYBNFLOAD_TEST_
int main(void)
{
  AnyBnfLoad first_grammar(1);
  try
  {
    std::cerr << ">>> Loading the grammar" << std::endl;
    first_grammar.add_grammar("conf/druha.gram", "conf/abnf.conf");
    first_grammar.set_start_nonterm("<S>", "conf/druha.gram");
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

// end of file
