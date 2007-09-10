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

/////////////////////
//  AnyBnfConf.cpp
///////////////
/*
  Contains the AnyBnfConf class.
  This class is used for parsing the metaconfiguration 
  from a given file. The acquired information is stored 
  in two separate structures - one is for basic symbols 
  and syntax that has to be included in any grammar such 
  as syntax of rulename, definition symbol, group symbols etc.. 
  Second is for arbitrary operators that are defined using 
  regular expressions in Perl syntax. These structures are 
  accesible by get_base() and get_operator() functions.
*//////////////
//  Author: Petr Slovak
///////////////

#include <iostream>
#include "AnyBnfConf.h"


std::string AnyBnfConf::extract(void)
{
  if(!m_conf_loaded)
    throw std::runtime_error("File error - file not loaded");
  
  if(m_conf_file)
    m_conf_file.getline(read_buffer, m_max_line_length);
  
  
  if (m_operators_flag==false)
  {
    char* eq_pos=strchr(read_buffer,'=');
    if (!eq_pos)
    {
       throw std::runtime_error("File error - bad file format");
    }
    else
    strcpy(output,++eq_pos);
    return(output);   
  }
  else
  {
    return(read_buffer);
  }
}       

bool AnyBnfConf::check_operator(void)
{
  if(m_conf_file)
    m_conf_file.getline(read_buffer, m_max_line_length);
  if (std::string(read_buffer).find("OPERATORS",0)==0) 
  {
    return(true);
  }
  else
    return(false);
} 


void AnyBnfConf::reset(void)
{
  m_operators.clear();
  m_operators_max = 0;
  leftcomment="";
  rightcomment="";
  alternative="";
  comment="";
  concat="";
  defined="";
  leftgroup="";
  nonterminal="";
  rightgroup="";
  rulename="";
  terminal="";
  allbrackets="";
  csstring="";
  csname="";
  m_conf_file.clear();

} 


bool AnyBnfConf::parse_conf(std::string conf_name)
{
  reset();
  m_operators_flag=false;
  m_conf_loaded=false;
   
  m_conf_file.open(conf_name.c_str(), std::ios::in | std::ios::binary); 
  if(!m_conf_file)
  {
    throw std::runtime_error("File error - cannot be opened");
  }
  else m_conf_loaded=true;
   
  std::string text;
  for (int i=0; i<=13; i++)
  {
    if(m_conf_file.eof())  
    {
      throw std::runtime_error("File error - missing base lines");
    }
    else
    {
      text=extract();
      if (m_operators_flag==false)
      {
        switch(i)
        {
          case 0 : rulename=text; break;
          case 1 : defined=text; break;
          case 2 : terminal=text; break;
          case 3 : nonterminal=text; break;
          case 4 : comment=text; break;
          case 5 : concat=text; break;
          case 6 : alternative=text; break;
          case 7 : leftgroup=text; break;
          case 8 : rightgroup=text; break;
          case 9 : leftcomment=text; break;
          case 10: rightcomment=text; break;
          case 11: allbrackets=text; break;
          case 12: csstring=text; break;
          case 13: csname=text; break;
        }
      }
    }
  }
  m_operators_max=0;
  if (check_operator())
    m_operators_flag=true;
  else
  {
    throw std::runtime_error("File error - missing OPERATORS word"); 
  }
  while (!m_conf_file.eof())   
  {
    text=extract();
    if(text.size() > 0)
    {
      m_operators.push_back(text);
      m_operators_max++;
    }
  }
  m_conf_file.close();
  
  return(true);
}

std::string AnyBnfConf::get_operators(int num)
{
  return(m_operators[num]);
}



std::string AnyBnfConf::get_base(int base_name)
{
  switch(base_name)
  {
    case 0 : return(rulename);
    case 1 : return(defined);
    case 2 : return(terminal);   
    case 3 : return(nonterminal);       
    case 4 : return(comment);   
    case 5 : return(concat);
    case 6 : return(alternative);       
    case 7 : return(leftgroup);
    case 8 : return(rightgroup);  
    case 9 : return(leftcomment);
    case 10: return(rightcomment);
    case 11: return(allbrackets);
    case 12: return(csstring);
    case 13: return(csname);
    default: return("");
  }
}


std::string AnyBnfConf::get_base_re(int base_name, int doub)
{
  std::string source, result;
  source = get_base(base_name);
  std::string::size_type position, pos_before = 0;

  position = source.find_first_of("?|.[]^$()*+{}\\");
  while(position != std::string::npos)
  {
    if(doub == 2)
      result += source.substr(pos_before, position - pos_before) + "\\\\" + source.at(position);
    else
      result += source.substr(pos_before, position - pos_before) + "\\" + source.at(position);
    pos_before = position + 1;
    position = source.find_first_of(".[]^$()*+{}\\", pos_before);
  }

  result += source.substr(pos_before);
  return result;
}


void AnyBnfConf::check_conf(void)
{
  m_output.open("conf_test.txt", std::ios::out | std::ios::binary);
  for (int i=0; i<=10; i++)
  {
    switch(i)
    {
      case 0 : m_output << (rulename)<<std::endl; break;
      case 1 : m_output << (defined)<<std::endl; break;
      case 2 : m_output << (terminal)<<std::endl; break;
      case 3 : m_output << (nonterminal)<<std::endl; break;
      case 4 : m_output << (comment)<<std::endl; break;
      case 5 : m_output << (concat)<<std::endl; break;
      case 6 : m_output << (alternative)<<std::endl; break;
      case 7 : m_output << (leftgroup)<<std::endl; break;
      case 8 : m_output << (rightgroup)<<std::endl; break;
      case 9 : m_output << (leftcomment)<<std::endl; break;
      case 10 : m_output << (rightcomment)<<std::endl; break;
      case 11 : m_output << (allbrackets)<<std::endl; break;
      case 12 : m_output << (csstring)<<std::endl; break;
      case 13 : m_output << (csname)<<std::endl; break;
    }   
  }
  int i=0;
  if (!m_operators.empty())
    while (i<m_operators_max)
    {
      m_output << m_operators[i]<<std::endl; i++;
    }
  m_output.close(); 
}



//#ifdef _ANYBNFCONF_TEST_
/*
int main(void)
{
    AnyBnfConf hello;
    hello.parse_conf("test.txt");
    hello.check_conf();
    return(0);
}
*/
//#endif    
