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

///////////////
//  AnyBnfFile.cpp
///////////////
//  Contains the implementation of the AnyBnfFile's methods.
//  The class provides a simple line-based
//  read-write access to a text file.
//  The main purpose is modifying contents of the file.
///////////////
//  Author: Vaclav Vacek
///////////////

#include <iostream>
#include "AnyBnfFile.h"


void AnyBnfFile::load_file(std::string name)
{
  std::string read_line;

  if(m_file_loaded)
  {
    m_input.close();
    m_output.close();
  }
  m_input.clear();
  m_output.clear();
  m_rw_state = false;
  m_first_entry = true;
  m_file_loaded = false;

  m_input.open(name.c_str(), std::ios::in | std::ios::binary);
  m_output.open("temp1", std::ios::out | std::ios::binary);

  if(!m_input || !m_output)
    throw std::runtime_error("File error");
  
  m_output << m_input.rdbuf();    //copies entire file into temp1
  
  m_input.close();
  m_output.close();

  m_input.open("temp1", std::ios::in);
  m_output.open("temp2", std::ios::out);

  if(!m_input || !m_output)
    throw std::runtime_error("File error");

  m_filename = name;
  m_file_loaded = true;

}


std::string AnyBnfFile::get_line(void)
{
  if(!m_file_loaded)
    throw std::runtime_error("File error");
  char read_buffer[m_max_line_length];
  read_buffer[0] = '\0';

  if(!m_input.eof())
    m_input.getline(read_buffer, m_max_line_length);

  std::string return_line(read_buffer);
  return return_line;
}



void AnyBnfFile::insert_line(std::string data)
{
  if(!m_file_loaded)
    throw std::runtime_error("File error");

  if(m_first_entry)
  {
    m_output << data;
    m_first_entry = false;
  }
  else
    m_output << '\n' << data;
}


bool AnyBnfFile::end_of_file(void)
{
  return m_input.eof();
}


void AnyBnfFile::swap(void)
{
  m_input.close();
  m_output.close();

  m_input.clear();
  m_output.clear();
    
  if(m_rw_state)
  {
    m_input.open("temp1", std::ios::in);
    m_output.open("temp2", std::ios::out);
  }
  else
  {
    m_input.open("temp2", std::ios::in);
    m_output.open("temp1", std::ios::out);
  }
  if(!m_input || !m_output)
    throw std::runtime_error("File error");

  m_rw_state = !m_rw_state;
  m_first_entry = true;

}


void AnyBnfFile::write_back(void)
{
  if(!m_file_loaded)
    throw std::runtime_error("File error");
  m_input.close();
  m_output.close();

  m_input.clear();

  m_file_loaded = false;


  if(m_rw_state)
    m_input.open("temp1", std::ios::in | std::ios::binary);
  else
    m_input.open("temp2", std::ios::in | std::ios::binary);

  if(!m_input || !m_output)
    throw std::runtime_error("File error");

  m_output.open(m_filename.c_str(), std::ios::out | std::ios::binary);
  
  m_output << m_input.rdbuf();    //copies entire file into temp1
  
  m_input.close();
  m_output.close();

}

#ifdef _ANYBNFFILE_TEST_
int main(void)
{
  AnyBnfFile a;
  char x;
  
  try{
  a.load_file("AnyBnfFile.txt");
  }
  catch(std::runtime_error)
  {
    std::cerr << "The file cannot be opened." << std::endl;
    std::cin >>x;
  }

  while(!(a.end_of_file()))
  {
    a.insert_line(a.get_line());
  }

  a.swap();
  std::cin >> x;
  a.write_back();
  return 0;
}
#endif
