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

#include "GSS.h"

std::vector<std::pair<GSS::StateIdent, std::string> > GSS::find_reachable(const GSS::StateIdent& from, unsigned length, const GSS::SymbolIdent& first_part)
{
  unsigned i;
  std::set<std::pair<unsigned, std::string> > symbols;
  std::set<std::pair<std::pair<unsigned, unsigned>, std::string> > states;
  
  std::set<std::pair<std::pair<unsigned, unsigned>, std::string> >::iterator states_iter;
  std::set<std::pair<unsigned, std::string> >::iterator symbols_iter;
  
  std::vector<std::pair<GSS::StateIdent, std::string> > retval;
  std::string reduce_string;
  
  bool now_states = true;
  if(length > 0)
  {
    states.insert(std::make_pair(std::make_pair(from.level, from.id), get_semantic_string(first_part)));
    length = 2*(length - 1);
  }
  else
    states.insert(std::make_pair(std::make_pair(from.level, from.id), ""));
  
  while(length > 0)
  {
    if(now_states)
    {
      symbols.clear();
      for(states_iter = states.begin(); states_iter != states.end(); states_iter++)
        for(i = 0;
            i < m_state_levels.at(states_iter->first.first).at(states_iter->first.second).successors.size();
            i++)
          symbols.insert(std::make_pair(m_state_levels.at(states_iter->first.first)
                            .at(states_iter->first.second).successors.at(i).id,
                            states_iter->second));
    }
    else
    {
      states.clear();
      for(symbols_iter = symbols.begin(); symbols_iter != symbols.end(); symbols_iter++)
        for(i = 0;
            i < m_symbol_nodes.at(symbols_iter->first).successors.size();
            i++)
          states.insert(std::make_pair(std::make_pair(m_symbol_nodes.at(symbols_iter->first).successors.at(i).level,
                                       m_symbol_nodes.at(symbols_iter->first).successors.at(i).id),
                                       get_semantic_string(SymbolIdent(symbols_iter->first)) + symbols_iter->second));
    }
    length--;
    now_states = !now_states;
  }

  for(states_iter = states.begin(); states_iter != states.end(); states_iter++)
    retval.push_back(std::make_pair(StateIdent(states_iter->first.first, states_iter->first.second), states_iter->second));
 
  return retval;
}

std::vector<GSS::StateIdent> GSS::find_state(size_t level, int label)
{
  unsigned i;
  
  std::vector<GSS::StateIdent> retval;
  
  for(i = 0; i < m_state_levels.at(level).size(); i++)
    if(m_state_levels.at(level).at(i).label == label)
    {
      retval.push_back(StateIdent(level, i));
      return retval;
    }
  
  return retval;
}


bool operator<(const GSS::StateIdent & first, const GSS::StateIdent & second)
{
  return (first.level < second.level) || (first.level == second.level && first.id < second.id);
}

bool operator<(const GSS::SymbolIdent & first, const GSS::SymbolIdent & second)
{
  return first.id < second.id;
}

bool operator==(const GSS::StateIdent & first, const GSS::StateIdent & second)
{
  return (first.level == second.level && first.id == second.id);
}

#ifdef GSS_TEST
int main()
{
  GSS graph;
  GSS::StateIdent st, st1;
  GSS::SymbolIdent sy;
  
  std::vector<GSS::StateIdent> test;

  graph.reset(5);
  
  st = graph.create_state(0, 64);
  
  sy = graph.create_symbol(-64, "test");
  
  graph.add_successor_to_symbol(sy, st);
  
 
  st1 = graph.create_state(1, 256);

  graph.add_successor_to_state(st1, sy);
  
  
  std::cout << graph.exists_2_path(st1, st) << std::endl;
  
  return 0;
}
#endif

// end of file
