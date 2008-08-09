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

#include "BnfParser2.h"
#include "AnyBnfFile.h"

AnyBnfFile::AnyBnfFile(BnfParser2 *interface)
 : m_interface(interface)
{
  m_rw_state = false;
  m_first_entry = true;
  m_file_loaded = false;
  m_temp1_created = false;
  m_temp2_created = false;

  const char *tmpdir_s;
  if((tmpdir_s = getenv("TMPDIR")) == NULL &&
    (tmpdir_s = getenv("TMP")) == NULL &&
    (tmpdir_s = getenv("TEMP")) == NULL)
#ifdef _WIN32
    tmpdir_s = ".";
#else
    tmpdir_s = "/tmp";
#endif
  strncpy(m_temp1, tmpdir_s, sizeof(m_temp1)-1);
  strncpy(m_temp2, tmpdir_s, sizeof(m_temp2)-1);
  strncat(m_temp1, "/parser_XXXXXX", sizeof(m_temp1)-strlen(m_temp1)-1);
  strncat(m_temp2, "/parser_XXXXXX", sizeof(m_temp2)-strlen(m_temp2)-1);
#if !defined(_WIN32) && defined(HAVE_MKSTEMP)
  int fd = -1;
  // create and close temporary files
  if(((fd = mkstemp(m_temp1)) == -1 || close(fd) != 0) ||
    ((fd = mkstemp(m_temp2)) == -1 || close(fd) != 0))
#else
  // create temporary files
  if(mktemp(m_temp1) == NULL || mktemp(m_temp2) == NULL)
#endif
  {
    BnfReport report(m_interface->get_reporter(), BnfReporter::ErrorType_Fatal);
    report.text()
      << "Cannot create " << m_temp1 << " and " << m_temp2;
    throw std::runtime_error("Temporary files cannot be created.");
  }
}

AnyBnfFile::~AnyBnfFile()
{
  if(m_file_loaded)
  {
    m_input.close();
    m_output.close();
  }
  if(m_temp1_created)
    remove(m_temp1);
  if(m_temp2_created)
    remove(m_temp2);
}

void AnyBnfFile::load_file(const std::string& name)
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
  if(!m_input)
    throw std::runtime_error("Invalid input file");
    
  m_output.open(m_temp1, std::ios::out | std::ios::binary);
  if(!m_output)
    throw std::runtime_error("Temporary file not created");
  
  m_temp1_created = true;
  
  m_output << m_input.rdbuf();    //copies entire file into temp1
  
  m_input.close();
  m_output.close();

  m_input.open(m_temp1, std::ios::in);
  if(!m_input)
    throw std::runtime_error("File error");

  m_output.open(m_temp2, std::ios::out);
  if(!m_output)
    throw std::runtime_error("Temporary file not created");
  
  m_temp2_created = true;

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

void AnyBnfFile::insert_line(const std::string& data)
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
    m_input.open(m_temp1, std::ios::in);
    m_output.open(m_temp2, std::ios::out);
  }
  else
  {
    m_input.open(m_temp2, std::ios::in);
    m_output.open(m_temp1, std::ios::out);
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
    m_input.open(m_temp1, std::ios::in | std::ios::binary);
  else
    m_input.open(m_temp2, std::ios::in | std::ios::binary);

  if(!m_input || !m_output)
    throw std::runtime_error("File error");

  m_output.open(m_filename.c_str(), std::ios::out | std::ios::binary);
  
  m_output << m_input.rdbuf();    //copies entire file back into the source file
  
  m_input.close();
  m_output.close();
}

#ifdef _ANYBNFFILE_TEST_
int main(void)
{
  AnyBnfFile a;
  std::string line;
  
  try
  {
    a.load_file("AnyBnfFile.txt");
  }
  catch(std::runtime_error e)
  {
    std::cerr << "Exception: " << e.what() << std::endl;
    return 1;
  }

  while(!(a.end_of_file()))
  {
    line = a.get_line();
    a.insert_line(line);
    std::cerr << line << std::endl;
  }

  a.write_back();
  return 0;
}
#endif

// end of file
