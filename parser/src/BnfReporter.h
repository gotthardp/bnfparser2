/**
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
 * Copyright (c) 2008 Petr Gotthard <petr.gotthard@centrum.cz>
 *
 * $Id$
 */

#ifndef _BNFREPORTER_H_
#define _BNFREPORTER_H_

#include <string>
#include <sstream>

class BNFPARSER2_EXP_DEFN BnfReporter
{
public:
  enum ErrorTypes
  {
    ErrorType_Fatal,
    ErrorType_Error,
    ErrorType_Warning
  };

  virtual ~BnfReporter() { }

  //! Callback method invoked to report warnings
  virtual void on_error(ErrorTypes code, const std::string& text) = 0;
};

class BnfReport
{
public:
  BnfReport(BnfReporter *reporter, BnfReporter::ErrorTypes code)
   : m_reporter(reporter), m_code(code) { }

  std::ostringstream& text()
  { return m_text; }

  ~BnfReport()
  {
    if(m_reporter)
      m_reporter->on_error(m_code, m_text.str());
  }

private:
  BnfReporter *m_reporter;

  BnfReporter::ErrorTypes m_code;
  std::ostringstream m_text;
};

#endif // _BNFREPORTER_H_

// end of file
