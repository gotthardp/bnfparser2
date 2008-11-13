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

class LocalReporter : public BnfReporter
{
public:
  virtual void on_error(BnfReporter::ErrorTypes code, const std::string& text)
  {
    switch(code)
    {
      case BnfReporter::ErrorType_Fatal:
        std::cout << "<p class=\"error\">Fatal Error: ";
        break;
      case BnfReporter::ErrorType_Error:
      default:
        std::cout << "<p class=\"error\">Error: ";
        break;
      case BnfReporter::ErrorType_Warning:
        std::cout << "<p class=\"error\">Warning: ";
        break;
    }

    for(std::string::const_iterator pos = text.begin();
      pos != text.end(); pos++)
    {
      std::cout << schar(*pos);
    }

    std::cout << "</p>" << std::endl;
  }
};

static std::string getTempName()
{
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

  return tmpname;
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
    // instantiate the reporter
    LocalReporter reporter;
    test.set_reporter(&reporter);

//    test.set_verbose_level(6);
    test.add_search_path("share");

    cgicc::const_form_iterator symbol = cgi.getElement("symbol");
    if(symbol == (*cgi).end() || symbol->getValue().empty())
      throw std::runtime_error("No symbol to check");

    std::cout
      << "<p class=\"query\">"
      << "Symbol: " << sstring(symbol->getValue()) << "<br/>" << std::endl;
    test.set_start_symbol(symbol->getValue().c_str());

    std::vector<cgicc::FormFile> file_list = cgi.getFiles();

    std::vector<cgicc::const_file_iterator> syntax_files;
    for(cgicc::const_file_iterator pos = file_list.begin();
      pos != file_list.end(); pos++)
    {
      // extract all files with the given prefix
      if(strncmp(pos->getName().c_str(), "syntax-file", 11) == 0)
        syntax_files.push_back(pos);
    }

    std::string syntax_text;
    // get the input text
    cgicc::const_form_iterator __syntax_text = cgi.getElement("syntax-text");
    if(__syntax_text != cgi.getElements().end())
        syntax_text = __syntax_text->getValue();

    std::vector<cgicc::FormEntry> syntax_list;
    if(!cgi.getElement("syntax", syntax_list))
    {
      // no checkboxes selected, check for uploaded file
      if(syntax_files.empty() && syntax_text.empty())
        throw std::runtime_error("No syntax to check");
    }

    std::cout
      << "Specifications:";

    for(std::vector<cgicc::FormEntry>::const_iterator pos = syntax_list.begin();
      pos != syntax_list.end(); pos++)
    {
      std::cout << " " << sstring(pos->getValue());
      test.add_grammar(pos->getValue().c_str());
    }

    for(std::vector<cgicc::const_file_iterator>::const_iterator pos = syntax_files.begin();
      pos != syntax_files.end(); pos++)
    {
      std::cout << " upload:" << sstring((*pos)->getFilename());
      std::string tmpname = getTempName();

      try
      {
        std::ofstream tmpfile(tmpname.c_str(), std::ios::out | std::ios::trunc);
        (*pos)->writeToStream(tmpfile);
        tmpfile.close();

        test.add_grammar(tmpname.c_str());
        // keep privacy of the users: make sure the temporary files are deleted
        remove(tmpname.c_str());
      }
      catch(...)
      {
        remove(tmpname.c_str());
        throw;
      }
    }

    std::cout << "<br/>" << std::endl;

    if(!syntax_text.empty())
    {
      std::cout << "grammar:<br/>" << sstring(syntax_text);
      std::string tmpname = getTempName();

      try
      {
        std::ofstream tmpfile(tmpname.c_str(), std::ios::out | std::ios::trunc);
        tmpfile << syntax_text;
        tmpfile.close();

        test.add_grammar(tmpname.c_str());
        // keep privacy of the users: make sure the temporary files are deleted
        remove(tmpname.c_str());
      }
      catch(...)
      {
        remove(tmpname.c_str());
        throw;
      }
    }

    std::cout << "</p>" << std::endl;
    test.add_referenced_grammars();
    test.build_parser();

    std::vector<std::string> words;
    // get the input text
    cgicc::const_form_iterator input_text = cgi.getElement("input-text");
    if(input_text != cgi.getElements().end() && !input_text->getValue().empty())
      words.push_back(input_text->getValue());
    // get the input files
    for(cgicc::const_file_iterator pos = file_list.begin();
      pos != file_list.end(); pos++)
    {
      // extract all files with the given prefix
      if(strncmp(pos->getName().c_str(), "input-file", 10) == 0)
        words.push_back(pos->getData());
    }

    if(words.empty())
      throw std::runtime_error("No text to check");

    for(std::vector<std::string>::const_iterator pos = words.begin();
      pos != words.end(); pos++)
    {
      std::set<int> errorpos;
      if(test.parse_word(*pos))
      {
        std::cout
          << "<p class=\"result\">"
          << "Correct.<br/>"
          << "<table class=\"sample\"><tr><td>"
          << sstring(*pos)
          << "</td></tr></table>"
          << "</p>" << std::endl;
      }
      else
      {
        std::cout
          << "<p class=\"result\">"
          << "Syntax error at position " << test.get_error_position()+1 << ".<br/>" << std::endl
          << "<table class=\"sample\"><tr><td>"
          << sstring(*pos, test.get_error_position())
          << "</td></tr></table>"
          << "</p>" << std::endl;
      }
    }
  }
  catch(std::exception &err)
  {
    std::cout
      << "<p class=\"error\">Exception<br/>" << sstring(err.what()) << "</p>" << std::endl;
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
