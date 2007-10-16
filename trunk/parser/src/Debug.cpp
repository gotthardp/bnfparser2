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
 *
 * $Id$
 */

#include "Debug.h"

// portable ostream to /dev/null
// see http://www.thescripts.com/forum/thread428285.html

template <class cT, class traits = std::char_traits<cT> >
class basic_nullbuf: public std::basic_streambuf<cT, traits>
{
  typename traits::int_type overflow(typename traits::int_type c)
  { return traits::not_eof(c); } // indicate success
};

template <class cT, class traits = std::char_traits<cT> >
class basic_onullstream: public std::basic_ostream<cT, traits> {
public:
  basic_onullstream():
    std::basic_ios<cT, traits>(&m_sbuf),
	std::basic_ostream<cT, traits>(&m_sbuf)
  { init(&m_sbuf); }

private:
  basic_nullbuf<cT, traits> m_sbuf;
};

typedef basic_onullstream<char> onullstream;
typedef basic_onullstream<wchar_t> wonullstream;


// similar to std::cout, or std::cerr we define cnull
// portable ostream to /dev/null
static onullstream cnull;

int logCurrentLevel = LOG_NOTICE;

// end of file
