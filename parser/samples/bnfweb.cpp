/*
 * bnfparser2 - Generic BNF-adaptable parser
 * http://bnfparser2.sourceforge.net
 *
 *      This program is free software; you can redistribute it and/or
 *      modify it under the terms of the GNU Lesser General Public
 *      License version 2.1, as published by the Free Software Foundation.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *      Lesser General Public License for more details.
 *
 * Copyright (c) 2007 ANF DATA spol. s r.o.
 *
 * $Id$
 */

#include <dirent.h>
#include <sys/time.h>
#include <set>
#include <stdexcept>

#include <cgicc/Cgicc.h>
#include <cgicc/HTTPHTMLHeader.h>
#include <cgicc/HTMLClasses.h>

#include "BnfParser2.h"

//! "HTML safe" character
struct schar
{
  schar(const char& ch) : m_ch(ch) {}
  const char& m_ch;
};

std::ostream& operator <<(std::ostream& out, schar text)
{
  switch(text.m_ch)
  {
  case '\t': return out << "<span style=\"margin-left:1.5em\"/>";
  case ' ': return out << "&nbsp;";
  case '&': return out << "&amp;";
  case '<': return out << "&lt;";

  case 0x0D: /* ignore */ return out;
  case 0x0A:
    return out << "<br/>" << std::endl;
  default:
    return out << text.m_ch;
  }
}

//! "HTML safe" string with special markers
struct sstring
{
  sstring(const std::string& str) : m_str(str) {}
  sstring(const std::string& str, int marker) : m_str(str) { m_markers.insert(marker); }
  sstring(const std::string& str, const std::set<int>& markers) : m_str(str), m_markers(markers) {}

  const std::string& m_str;
  std::set<int> m_markers;
};

std::ostream& operator <<(std::ostream& out, sstring text)
{
  for(std::string::const_iterator pos = text.m_str.begin();
    pos != text.m_str.end(); pos++)
  {
    if(text.m_markers.find(pos-text.m_str.begin()) != text.m_markers.end())
      out << "<span class=\"marker\">&rarr;</span>";

    out << schar(*pos);
  }
  // visualize markers at the end the text (and beyond)
  if(text.m_markers.lower_bound(text.m_str.size()) != text.m_markers.end())
    out << "<span class=\"marker\">&rarr;</span>";
  return out;
}

int main(int argc, char  *argv[])
{
  cgicc::Cgicc cgi;

  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
  std::cout << cgicc::HTTPHTMLHeader() << cgicc::HTMLDoctype(cgicc::HTMLDoctype::eStrict)
    << cgicc::head()
    << cgicc::title("Syntax verification report")
    << "<link rel=\"stylesheet\" href=\"bnfweb.css\" type=\"text/css\">"
    << cgicc::head()
    << cgicc::body() << "<h1>Syntax verification report</h1>" << std::endl;

  try
  {
    // instantiate the parser
    BnfParser2 test;
//    test.set_verbose_level(6);
    test.add_search_path("share");

    cgicc::const_form_iterator symbol = cgi.getElement("symbol");
    if(symbol == (*cgi).end() || symbol->getValue().empty())
      throw std::runtime_error("No symbol to check");

    std::cout
      << "<p class=\"query\">"
      << "Symbol: " << sstring(symbol->getValue()) << "<br/>" << std::endl;
    test.set_start_symbol(symbol->getValue().c_str());

    std::vector<cgicc::FormEntry> syntax_list;
    if(!cgi.getElement("syntax", syntax_list))
      throw std::runtime_error("No syntax to check");

    std::cout
      << "Specifications:";

    for(std::vector<cgicc::FormEntry>::const_iterator pos = syntax_list.begin();
      pos != syntax_list.end(); pos++)
    {
      std::cout << " " << sstring(pos->getValue());
      test.add_grammar(pos->getValue().c_str());
    }

    cgicc::const_file_iterator syntax_file = cgi.getFile("syntax-file");
    if(syntax_file != cgi.getFiles().end())
    {
      std::cout << " upload:" << sstring(syntax_file->getFilename());
      const char *tmpdir_s;
      if((tmpdir_s = getenv("TMPDIR")) == NULL &&
        (tmpdir_s = getenv("TMP")) == NULL &&
        (tmpdir_s = getenv("TEMP")) == NULL)
#ifdef _WIN32
        tmpdir_s = ".";
#else
        tmpdir_s = "/tmp";
#endif
      char tmpname[128];
      strncpy(tmpname, tmpdir_s, sizeof(tmpname)-1);
      strncat(tmpname, "/bnfweb_XXXXXX", sizeof(tmpname)-strlen(tmpname)-1);
#if !defined(_WIN32) && defined(HAVE_MKSTEMP)
      int fd = -1;
      // create and close the temporary file
      if((fd = mkstemp(tmpname)) == -1 || close(fd) != 0)
#else
      // create temporary the file
      if(mktemp(tmpname) == NULL)
#endif
        throw std::runtime_error("Internal error: Cannot create the temporary file");

      try
      {
        std::ofstream tmpfile(tmpname, std::ios::out | std::ios::trunc);
        syntax_file->writeToStream(tmpfile);
        tmpfile.close();

        test.add_grammar(tmpname);
      }
      catch(...)
      {
        // keep privacy of the users: make sure the temporary files are deleted
        remove(tmpname);
        throw;
      }
    }

    std::cout << "<br/></p>" << std::endl;
    test.add_referenced_grammars();
    test.build_parser();

    cgicc::const_form_iterator input_text = cgi.getElement("input-text");
    cgicc::const_file_iterator input_file = cgi.getFile("input-file");

    std::string word;
    // load the input word
    if(input_text != cgi.getElements().end() && !input_text->getValue().empty())
      word = input_text->getValue();
    else if(input_file != cgi.getFiles().end())
      word = input_file->getData();
    else
      throw std::runtime_error("No text to check");

    std::set<int> errorpos;
    if(test.parse_word(word))
    {
      std::cout
        << "Correct.<br/>"
        << "<table class=\"sample\"><tr><td>"
        << sstring(word)
        << "</td></tr></table>" << std::endl;
    }
    else
    {
      std::cout
        << "Syntax error at position " << test.get_error_position()+1 << ".<br/>" << std::endl
        << "<table class=\"sample\"><tr><td>"
        << sstring(word, test.get_error_position())
        << "</td></tr></table>" << std::endl;
    }
  }
  catch(std::exception &err)
  {
    std::cout
      << "<p>Error<br/>" << sstring(err.what()) << "</p>" << std::endl;
  }

  struct timeval end_time;
  gettimeofday(&end_time, NULL);
  // total time for request
  long querytime = ((end_time.tv_sec - start_time.tv_sec) * 1000000)
    + (end_time.tv_usec - start_time.tv_usec);

  struct tm now;
  localtime_r( (const time_t *) &(end_time.tv_sec), &now );

  char timestr[100];
  strftime(timestr, sizeof(timestr), "%F %T", &now);
  std::cout
    << "<p>Generated " << timestr << " (in " << static_cast<double>(querytime/1000000.0) << " sec)"
    << " by <a href=\"http://bnfparser2.sourceforge.net\" target=\"_blank\">BNF Parser&sup2;</a>.</p>";

  std::cout << cgicc::body() << std::endl;

  return 0;
}

// end of file
