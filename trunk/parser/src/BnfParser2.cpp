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

#include "Debug.h"
#include "Parser.h"

#ifndef BNFPARSER2_EXP_DEFN
#ifdef _WIN32
#define BNFPARSER2_EXP_DEFN __declspec(dllexport)
#else
#define BNFPARSER2_EXP_DEFN /* empty */
#endif
#endif

#include "BnfParser2.h"

BnfParser2::BnfParser2(void)
{
  m_core_parser = new Parser(this);
  m_reporter = NULL;
}

BnfParser2::~BnfParser2(void)
{
  delete m_core_parser;
}

void BnfParser2::add_search_path(const char *path)
{
  m_core_parser->add_search_path(path);
}

void BnfParser2::add_grammar(const char *syntax_name, const char *variant_name)
{
  m_core_parser->add_grammar(syntax_name, variant_name);
}

void BnfParser2::add_referenced_grammars()
{
  m_core_parser->add_referenced_grammars();
}

void BnfParser2::set_start_symbol(const char *symbol_name, const char *start_grammar_name)
{
  m_core_parser->set_start_symbol(symbol_name, start_grammar_name);
}

void BnfParser2::build_parser(void)
{
  m_core_parser->build_parser();
}

int BnfParser2::get_verbose_level(void)
{
  return logCurrentLevel;
}

void BnfParser2::set_verbose_level(unsigned level)
{
  logSetLevel(level);
}

bool BnfParser2::parse_word(const std::string& word)
{
  return m_core_parser->parse_word(word);
}

bool BnfParser2::get_parsing_result(void)
{
  return m_core_parser->get_parsing_result();
}

unsigned BnfParser2::get_error_position(void)
{
  return m_core_parser->get_error_position();
}

std::string BnfParser2::get_semantic_string(void)
{
  return m_core_parser->get_semantic_string();
}

// end of file
