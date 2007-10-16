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

#ifndef DEBUG_H
#define DEBUG_H

#include<iostream>

// Minimalist implementation of log4j-like logging functions
// See also http://logging.apache.org

#define LOG_EMERG    0 // system is unusable
#define LOG_ALERT    1 // action must be taken immediately
#define LOG_CRIT     2 // critical conditions
#define LOG_ERR      3 // error conditions
#define LOG_WARNING  4 // warning conditions
#define LOG_NOTICE   5 // normal but significant condition
#define LOG_INFO     6 // informational
#define LOG_DEBUG    7 // debug-level messages
#define LOG_TRACE    8 // function call tracing

extern int logCurrentLevel;

inline void logSetLevel( const int level )
{ logCurrentLevel = level; }

#define logIsEnabledFor(level) level <= logCurrentLevel

#define logTrace(level, params) \
  if(level <= logCurrentLevel) std::cerr << params << std::endl; else (void)0

#endif /* DEBUG_H */

// end of file
