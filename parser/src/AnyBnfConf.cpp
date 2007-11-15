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

#include "Debug.h"
#include "AnyBnfConf.h"

void AnyBnfConf::reset(void)
{
  m_operators.clear();
  m_leftcomment.clear();
  m_rightcomment.clear();
  m_alternative.clear();
  m_comment.clear();
  m_concat.clear();
  m_defined.clear();
  m_leftgroup.clear();
  m_rightgroup.clear();
  m_rulename.clear();
  m_terminal.clear();
  m_allbrackets.clear();
  m_csstring = false;
  m_csname = false;
} 

char* AnyBnfConf::get_next_token(char*& buffer)
{
  // skip leading whitespace
  while(*buffer!=0 && isspace(*buffer)) buffer++;
  // begin of the token
  char* token = buffer;

  // find end of the token
  while(*buffer!=0)
  {
    if(isspace(*buffer))
    {
      // delimiter found, terminate token and exit
      *(buffer++) = 0;
      break;
    }
    else buffer++;
  }

  return token;
}

bool AnyBnfConf::parse_conf(const std::string& conf_name)
{
  reset();

  std::fstream conf_file;
  conf_file.open(conf_name.c_str(), std::ios::in | std::ios::binary); 
  if(!conf_file)
  {
    throw std::runtime_error("File error - cannot be opened");
  }

  std::string text;

  char line[m_max_line_length];
  bool operator_section = false;
  // read the configuration file
  while(conf_file.good())
  {
    conf_file.getline(line, m_max_line_length);

    // skip leading whitespace
    char *text = line;
    while(*text != 0 && isspace(*text)) text++;
    // skip empty lines
    if(*text == 0) continue;

    if(!operator_section)
    {
      // skip comments
      if(*text == '#') continue;

      // The line may contain several '=' characters. The first one is a separator.
      char* value = strchr(text, '=');
      if(value != NULL)
        *(value++) = 0;

      char *name = get_next_token(text);
      if(name == NULL || *name == 0) continue;

      if(strcasecmp(name, "OPERATORS") == 0)
      {
        operator_section = true;
        continue;
      }

      if(value == NULL || *value == 0) continue;

      if(strcasecmp(name, "rulename") == 0)
        m_rulename = value;
      else if(strcasecmp(name, "defined") == 0)
        m_defined = value;
      else if(strcasecmp(name, "terminal") == 0)
        m_terminal = value;
      else if(strcasecmp(name, "comment") == 0)
        m_comment = value;
      else if(strcasecmp(name, "concat") == 0)
        m_concat = value;
      else if(strcasecmp(name, "alternative") == 0)
        m_alternative = value;
      else if(strcasecmp(name, "leftgroup") == 0)
        m_leftgroup = value;
      else if(strcasecmp(name, "rightgroup") == 0)
        m_rightgroup = value;
      else if(strcasecmp(name, "leftcomment") == 0)
        m_leftcomment = value;
      else if(strcasecmp(name, "rightcomment") == 0)
        m_rightcomment = value;
      else if(strcasecmp(name, "allbrackets") == 0)
        m_allbrackets = value;
      else if(strcasecmp(name, "casesensitivestring") == 0)
        m_csstring = (strcasecmp(value, "true") == 0);
      else if(strcasecmp(name, "casesensitiverulename") == 0)
        m_csname = (strcasecmp(value, "true") == 0);

      else
        logTrace(LOG_WARNING, "unknown configuration parameter " << name);
    }
    else
    {
      m_operators.push_back(text);
    }
  }

  return(true);
}

std::string AnyBnfConf::backslash(const std::string& source, int doub)
{
  std::string result;
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

void AnyBnfConf::check_conf(std::ostream& output)
{
  output << "rulename='" << m_rulename << "'" << std::endl;
  output << "defined='" << m_defined << "'" << std::endl;
  output << "terminal='" << m_terminal << "'" << std::endl;
  output << "comment='" << m_comment << "'" << std::endl;
  output << "concat='" << m_concat << "'" << std::endl;
  output << "alternative='" << m_alternative << "'" << std::endl;
  output << "leftgroup='" << m_leftgroup << "'" << std::endl;
  output << "rightgroup='" << m_rightgroup << "'" << std::endl;
  output << "leftcomment='" << m_leftcomment << "'" << std::endl;
  output << "rightcomment='" << m_rightcomment << "'" << std::endl;
  output << "allbrackets='" << m_allbrackets << "'" << std::endl;
  output << "casesensitivestring=" << ( m_csstring ? "'true'" : "'false'" ) << std::endl;
  output << "casesensitiverulename=" << ( m_csname ? "'true'" : "'false'" ) << std::endl;

  output << "OPERATORS" << std::endl;
  for(TOperatorList::const_iterator pos = m_operators.begin();
    pos != m_operators.end(); pos++)
  {
    output << *pos << std::endl;
  }
}

#ifdef _ANYBNFCONF_TEST_
int main(void)
{
  AnyBnfConf hello;
  hello.parse_conf("test.txt");

  std::fstream output;
  output.open("conf_test.txt", std::ios::out | std::ios::binary);
  hello.check_conf(output);
  return(0);
}
#endif    

// end of file
