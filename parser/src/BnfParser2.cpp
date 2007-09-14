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
#include "BnfParser2.h"
#include "Parser.h"

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

void BnfParser2::set_start_nonterm(const std::string& start_name, const std::string& start_grammar_name)
{
  m_core_parser->set_start_nonterm(start_name, start_grammar_name);
}

void BnfParser2::add_grammar(const std::string& g_name, const std::string& c_name)
{
  m_core_parser->add_grammar(g_name, c_name);
}

void BnfParser2::build_parser(void)
{
  m_core_parser->build_parser();
}

BnfParser2::BnfParser2(unsigned verbose)
{
  m_core_parser = new Parser(verbose);
}

BnfParser2::~BnfParser2(void)
{
  delete m_core_parser;
}
