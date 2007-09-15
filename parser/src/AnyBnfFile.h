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

/*
 *  TODO - better exceptions??
 *  
 * 
 */
#ifndef _ANYBNFFILE_
#define _ANYBNFFILE_

#include <fstream>
#include <string>
#include <stdexcept>

#include <cstdlib>

//!  It's used for iterative text-file editing.
/*  That means reading a line and inserting a set of lines
 *  in place of the read line. When entire file is processed,
 *  another processing can start over. It uses two temporary files and
 *  moves data between them. 
 *  \warning THE SOURCE FILE IS NOT MODIFIED UNLESS write_back() is called.
 *  \warning Only one instance of the class is allowed.
 */ 
class AnyBnfFile
{
private:
  std::fstream m_input;       //!<input file controller
  std::fstream m_output;      //!<output file controller
  static const int m_max_line_length = 4096;   //!<max input-file line length
  bool m_rw_state;        //!<false = temp1 - read, temp2 - write; true = v. v.
  
  std::string m_temp_dir; //!< The unique name of the temporary directory
  std::string m_temp1; //!< The name of the first temporary file
  std::string m_temp2; //!< The name of the second temporary file
  
  //! false = something has been already written to the file; true = the first entry will be written
  bool m_first_entry;     
  std::string m_filename; //!<name of the source file
  bool m_file_loaded;     //!<true if a file is loaded
  
  bool m_temp1_created; //!<True if the first temporary file has been created
  bool m_temp2_created; //!<True if the first temporary file has been created

public:
  //! prepares the specified file specified for using
  /** Actually it copies its contents to a temp file and
   *  then opens the temp for reading. It also opens another
   *  temp for writing.
   *
   *  argument - string containing a name of the file to be processed
   *  returns - nothing
   *  side effects - rewriting the temp files, opening them
   */      
  void load_file(const std::string& name);
     
  //!  Reads a line from the file and REMOVES IT. 
  /** argument - nothing
   *  returns - string containing read line
   *  side effects - changing the position in the file
   */
  std::string get_line(void);

  //!  Inserts a line into the file. 
  /*  argument - string containing data to be written
   *  returns - nothing
   *  side effects - adds a line to the file
   */
  void insert_line(const std::string& data);
  
  //! EOF checker
  /*  argument - nothing
   *  returns - 1 when EOF is reached, 0 otherwise
   *  side effects - none
   */   
  bool end_of_file(void);
  
  //!  Swaps read and write temp file.
  /*  argument - nothing
   *  returns - nothing 
   *  side effects - swaps read and write file
   */
  void swap(void);    

  //!  Writes the processed data back to the source file and ends processing the file.
  /*  argument - nothing
   *  returns - nothing
   *  side effects - closes all the files used
   */
  void write_back(void);

  //!constructor with no parameters
  AnyBnfFile(void):m_rw_state(false),m_first_entry(true),m_file_loaded(false),
  m_temp1_created(false), m_temp2_created(false)
  {
    char templ[] = P_tmpdir "/parser_XXXXXX";
    if(mkdtemp(templ) == NULL)
      throw std::runtime_error("Temporary directory not created");

    m_temp_dir = templ;
    m_temp1 = m_temp_dir + "/temp1";
    m_temp2 = m_temp_dir + "/temp2";
  }
  ~AnyBnfFile()
  {
    if(m_file_loaded)
    {
      m_input.close();
      m_output.close();
    }
    if(m_temp1_created)
      remove(m_temp1.c_str());
    if(m_temp2_created)
      remove(m_temp2.c_str());
    rmdir(m_temp_dir.c_str());
  }
};

#endif  //_ANYBNFFILE_

// end of file
