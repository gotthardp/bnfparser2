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

#include "Parser.h"

bool Parser::parse_word(const std::string& word)
{
  GSS::StateIdent initial_state;
  GSS::SymbolIdent not_an_ident;
  int first_character;
  unsigned i;
  int a_i_1, a_i_2;

  std::set<LalrTable::action>::iterator actions;
  
  std::vector<GSS::StateIdent> accepting_states;
  m_gss.reset(word.length());
  m_q.clear();
  m_r.clear();

  initial_state = m_gss.create_state(0, 0);
  if(word.length() > 0)
    first_character = static_cast<int>(word[0]);
  else 
    first_character = -LalrTable::end_of_input;

  for(actions = m_table->get_actions(0, first_character).begin();
      actions != m_table->get_actions(0, first_character).end();
      actions++)
  {
    if(actions->what == LalrTable::action::shift)
      m_q.insert(QMember(initial_state, actions->next_state));
    else if(actions->reduce_length == 0)
      m_r.insert(RMember(initial_state, actions->reduce_by, 0, not_an_ident));
  }

  for(i = 0; i <= word.size(); i++)
  {
    if(i < word.size())
      a_i_1 = -static_cast<int>(word[i]);
    else
      a_i_1 = LalrTable::end_of_input;
      
    if(i < word.size() - 1)
      a_i_2 = -static_cast<int>(word[i + 1]);
    else
      a_i_2 = LalrTable::end_of_input;
      
    
    if(m_gss.state_level_empty(i))
      break;

    while(!m_r.empty())
      reducer(i, a_i_1);

    if(i != word.size())
      if(m_q.empty())
      {
        m_last_accepted = false;
        m_error_position = i;
        return false;
      }
      else
        shifter(i, a_i_1, a_i_2);
  }

  accepting_states = m_gss.find_state(word.size(), m_table->get_accepting_state());


  if(accepting_states.size() > 0)
  {
    m_last_accepted = true;
    m_semantic_string = m_gss.get_semantic_string(m_gss.get_state_successors(accepting_states[0])[0]);
    return true;
  }
  else 
  {
    m_last_accepted = false;
    m_error_position = i - 1;
    return false;
  }
}

void Parser::shifter(unsigned i, int a_i_plus_1, int a_i_plus_2)
{
  std::set<LalrTable::action>::iterator actions;
  
  std::set<QMember> temp_q;
  std::set<QMember>::iterator q_iter;
  std::vector<GSS::StateIdent> state_with_label, successors_with_label, successors_of_symbol;
  std::vector<GSS::SymbolIdent> symbol_with_label;
  GSS::SymbolIdent temp_symbol;
  GSS::StateIdent temp_state;
  unsigned k, l, m;
  bool found_state_node;
  std::string helper;
  helper = static_cast<char>(-a_i_plus_1);

  for(q_iter = m_q.begin(); q_iter != m_q.end(); q_iter++)
  {
    if(m_verbose_level >= 2)
      std::cerr <<"(" << i <<  ") shift " << q_iter->new_state << std::endl;

    state_with_label = m_gss.find_state(i + 1, q_iter->new_state);

    if(!state_with_label.empty())
    {
      for(k = 0; k < state_with_label.size(); k++)  //always runs only once
      {
        symbol_with_label = m_gss.get_state_successors_with_label(state_with_label[k], a_i_plus_1);
        
        found_state_node = false;
        if(!symbol_with_label.empty())
        {
          for(m = 0; m < symbol_with_label.size(); m++)
          {
            successors_of_symbol = m_gss.get_symbol_successors(symbol_with_label[m]);
            for(l = 0; l < successors_of_symbol.size(); l++)
              if(successors_of_symbol[l] == q_iter->state_node)
              {
                found_state_node = true;
                m_gss.add_semantics_to_symbol(symbol_with_label[m], helper);
                temp_symbol = symbol_with_label[m];
                break;
              }
            if(found_state_node)
              break;
          }
          if(found_state_node == false)
          {
            for(m = 0; m < symbol_with_label.size(); m++)
            {
              successors_of_symbol = m_gss.get_symbol_successors(symbol_with_label[m]);
              for(l = 0; l < successors_of_symbol.size(); l++)
                if(m_gss.get_state_level(successors_of_symbol[l]) == m_gss.get_state_level(q_iter->state_node))
                {
                  found_state_node = true;
                  m_gss.add_successor_to_state(state_with_label[k], symbol_with_label[m]);
                  m_gss.add_successor_to_symbol(symbol_with_label[m], q_iter->state_node);
                  m_gss.add_semantics_to_symbol(symbol_with_label[m], helper);
                  temp_symbol = symbol_with_label[m];
                  break;
                }
              if(found_state_node)
                break;
            }
          }
        }
        if(found_state_node == false)
        {
          temp_symbol = m_gss.create_symbol(a_i_plus_1, helper);
          m_gss.add_successor_to_state(state_with_label[k], temp_symbol);
          m_gss.add_successor_to_symbol(temp_symbol, q_iter->state_node);
        }
      }
      
      
      for(actions = m_table->get_actions(q_iter->new_state, -a_i_plus_2).begin();
          actions != m_table->get_actions(q_iter->new_state, -a_i_plus_2).end();
          actions++)
      {
        if(actions->what == LalrTable::action::reduce && actions->reduce_length > 0)
          m_r.insert(RMember(q_iter->state_node, actions->reduce_by, actions->reduce_length, temp_symbol));
      }

    }
    else
    {
      temp_state = m_gss.create_state(i + 1, q_iter->new_state);
      temp_symbol = m_gss.create_symbol(a_i_plus_1, helper);
      m_gss.add_successor_to_state(temp_state, temp_symbol);
      m_gss.add_successor_to_symbol(temp_symbol, q_iter->state_node);
      
      for(actions = m_table->get_actions(q_iter->new_state, -a_i_plus_2).begin();
        actions != m_table->get_actions(q_iter->new_state, -a_i_plus_2).end();
        actions++)
      {
        if(actions->what == LalrTable::action::shift)
          temp_q.insert(QMember(temp_state, actions->next_state));
        else if(actions->reduce_length == 0)
          m_r.insert(RMember(temp_state, actions->reduce_by, 0, temp_symbol));
      }
    
      for(actions = m_table->get_actions(q_iter->new_state, -a_i_plus_2).begin();
        actions != m_table->get_actions(q_iter->new_state, -a_i_plus_2).end();
        actions++)
      {
        if(actions->what == LalrTable::action::reduce && actions->reduce_length > 0)
          m_r.insert(RMember(q_iter->state_node, actions->reduce_by, actions->reduce_length, temp_symbol));
      }
    }
  }

  m_q = temp_q;
}

void Parser::reducer(unsigned i, int a_i_plus_1)
{
  std::set<LalrTable::action>::iterator actions;

  RMember* now_processed;
  std::vector<std::pair<GSS::StateIdent, std::string > > chi;
  int state_to_go;
  unsigned k, l;
  
  std::string reduce_string;
  
  bool successor_added;
  
  std::vector<GSS::StateIdent> state_with_label;
  std::vector<GSS::SymbolIdent> symbol_with_label;
  
  GSS::StateIdent temp_state;
  GSS::SymbolIdent temp_symbol;
  
  now_processed = new RMember(*m_r.begin());
  m_r.erase(*now_processed);

  if(m_verbose_level >= 2)
  {
    std::cerr<<"(" << i << ") reduce by " << now_processed->rule_number<<", length is " 
        << now_processed->reduction_length << std::endl;
    std::cerr<<"  state: " << m_gss.get_state_label(now_processed->state_node)
             <<", level: " << m_gss.get_state_level(now_processed->state_node)<<std::endl;
  }
  
  if(now_processed->rule_number == 0)
  {
    delete now_processed;
    return;
  }

////////////////////////////////////////////////////////////////////////////////
//HERE the find_reachable function is rewritten, so that it sees the grammar
////////////////////////////////////////////////////////////////////////////////
  bool now_states = true;
  unsigned length = now_processed->reduction_length;
  
  std::set<std::pair<GSS::SymbolIdent, std::string> > symbols;
  std::set<std::pair<GSS::StateIdent, std::string> > states;
  
  std::set<std::pair<GSS::SymbolIdent, std::string> >::iterator symbols_iter;
  std::set<std::pair<GSS::StateIdent, std::string> >::iterator states_iter;
  
  std::string initial_value;

   if(length > 0)
    initial_value = m_gss.get_semantic_string(now_processed->first_part);

  if(length > 0 && m_table->symbol_is_marked(now_processed->rule_number, length - 1))
    initial_value = "<" + m_grammar.get_marked_name(m_table->get_symbol(now_processed->rule_number, length - 1)) + ">"
     + initial_value 
     + "</" + m_grammar.get_marked_name(m_table->get_symbol(now_processed->rule_number, length - 1)) + ">"; 
  
  for(k = now_processed->reduction_length; k < m_table->get_rule_length(now_processed->rule_number); k++)
    if(m_table->symbol_is_marked(now_processed->rule_number, k))
      initial_value = initial_value + "<" + m_grammar.get_marked_name(m_table->get_symbol(now_processed->rule_number, k))+">"
                    +  "</"+ m_grammar.get_marked_name(m_table->get_symbol(now_processed->rule_number, k))+">";

  if(length > 0)
  {
    states.insert(std::make_pair(now_processed->state_node, initial_value));
    length = 2*(length - 1);
  }
  else
    states.insert(std::make_pair(now_processed->state_node, ""));
    
    
    
  while(length > 0)
  {
    if(now_states)
    {
      symbols.clear();
      for(states_iter = states.begin(); states_iter != states.end(); states_iter++)
        for(k = 0;
            k < m_gss.get_state_successors(states_iter->first).size();
            k++)
          symbols.insert(std::make_pair(m_gss.get_state_successors(states_iter->first)[k],
                            states_iter->second));
    }
    else
    {
      states.clear();
      for(symbols_iter = symbols.begin(); symbols_iter != symbols.end(); symbols_iter++)
        for(k = 0;
            k < m_gss.get_symbol_successors(symbols_iter->first).size();
            k++)
        {
          if(m_table->symbol_is_marked(now_processed->rule_number, length / 2))
            states.insert(std::make_pair(m_gss.get_symbol_successors(symbols_iter->first)[k],
                                        + "<" + m_grammar.get_marked_name(m_table->get_symbol(now_processed->rule_number, length/2))+">" 
                                        + m_gss.get_semantic_string(symbols_iter->first)
                                        + "</"+ m_grammar.get_marked_name(m_table->get_symbol(now_processed->rule_number, length/2))+">"
                                        + symbols_iter->second));
           else
             states.insert(std::make_pair(m_gss.get_symbol_successors(symbols_iter->first)[k],
                                        m_gss.get_semantic_string(symbols_iter->first)
                                        + symbols_iter->second));
        }
    }
    length--;
    now_states = !now_states;
  }

  for(states_iter = states.begin(); states_iter != states.end(); states_iter++)
    chi.push_back(std::make_pair(states_iter->first, states_iter->second));

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
  
  for(k = 0; k < chi.size(); k++)
  {
    reduce_string = chi[k].second;
    state_to_go = m_table->get_go_to(m_gss.get_state_label(chi[k].first), 
                                     m_table->get_lhs(now_processed->rule_number) + 256);


                                     
    if(m_verbose_level >= 2)
    {
      std::cerr<<"label chi[k]: " << m_gss.get_state_label(chi[k].first) << std::endl;
      std::cerr<<"level chi[k]: " << m_gss.get_state_level(chi[k].first) << std::endl;
      std::cerr<<"LHS           " << m_table->get_lhs(now_processed->rule_number)<<std::endl;
      std::cerr<<"GOTO          " << state_to_go<< std::endl;
    }
         
    state_with_label = m_gss.find_state(i, state_to_go);
    if(state_with_label.empty())
    {
      temp_state = m_gss.create_state(i, state_to_go);
      temp_symbol = m_gss.create_symbol(m_table->get_lhs(now_processed->rule_number), reduce_string);
      
      m_gss.add_successor_to_state(temp_state, temp_symbol);
      m_gss.add_successor_to_symbol(temp_symbol, chi[k].first);
      for(actions = m_table->get_actions(state_to_go, -a_i_plus_1).begin();
          actions != m_table->get_actions(state_to_go, -a_i_plus_1).end();
          actions++)
      {
        if(actions->what == LalrTable::action::shift)
        {
          m_q.insert(QMember(temp_state, actions->next_state));
        }
        else if(actions->reduce_length == 0)
          m_r.insert(RMember(temp_state, actions->reduce_by, 0, temp_symbol));
      }
    
      if(now_processed->reduction_length != 0)
        for(actions = m_table->get_actions(state_to_go, -a_i_plus_1).begin();
            actions != m_table->get_actions(state_to_go, -a_i_plus_1).end();
            actions++)
        {
          if(actions->what == LalrTable::action::reduce && actions->reduce_length > 0)
          {
            m_r.insert(RMember(chi[k].first, actions->reduce_by, actions->reduce_length, temp_symbol));
          }
        }
    }
    else
    {
      symbol_with_label = m_gss.get_state_successors_with_label(state_with_label[0],
                                 m_table->get_lhs(now_processed->rule_number));

      successor_added = false;
      for(l = 0; l < symbol_with_label.size(); l++)
      {
        if(m_gss.has_state_successor(symbol_with_label[l], chi[k].first))
        {
          successor_added = true;
          m_gss.add_semantics_to_symbol(symbol_with_label[l], reduce_string);
          temp_symbol = symbol_with_label[l];
          break;
        }
      }
      if(!successor_added)
      {
        for(l = 0; l < symbol_with_label.size(); l++)
        {
          if(m_gss.get_state_level(m_gss.get_symbol_successors(symbol_with_label[l])[0]) == m_gss.get_state_level(chi[k].first))
          {
            successor_added = true;
            m_gss.add_semantics_to_symbol(symbol_with_label[l], reduce_string);
            m_gss.add_successor_to_symbol(symbol_with_label[l], chi[k].first);
            temp_symbol = symbol_with_label[l];
            break;
          }
        }
      }
      if(!successor_added)
      {
        temp_symbol = m_gss.create_symbol(m_table->get_lhs(now_processed->rule_number), reduce_string);
        m_gss.add_successor_to_state(state_with_label[0], temp_symbol);
        m_gss.add_successor_to_symbol(temp_symbol, chi[k].first);
      }
        
      if(now_processed->reduction_length != 0)
        for(actions = m_table->get_actions(state_to_go, -a_i_plus_1).begin();
            actions != m_table->get_actions(state_to_go, -a_i_plus_1).end();
            actions++)
        {
          if(actions->what == LalrTable::action::reduce && actions->reduce_length > 0)
            m_r.insert(RMember(chi[k].first, actions->reduce_by, actions->reduce_length, temp_symbol));
        }
    }
  }
  delete now_processed;
}

void Parser::process_grammar(const std::multimap<int, std::vector<int> >& grammar, unsigned nonterm_count)
{
  m_table = new LalrTable(nonterm_count, m_verbose_level);
  m_table->load(grammar);
  m_table->make_lalr_table();
}

bool operator<(const Parser::QMember & first, const Parser::QMember & second)
{
  return (first.state_node < second.state_node)
   || (first.state_node == second.state_node && first.new_state < second.new_state);
}

bool operator<(const Parser::RMember & first, const Parser::RMember & second)
{
  if (first.state_node < second.state_node)
    return true;

  if (first.state_node == second.state_node && first.rule_number < second.rule_number)
    return true;
  
  if (first.state_node == second.state_node && first.rule_number == second.rule_number
        && first.reduction_length < second.reduction_length)
    return true;
  
  if (first.state_node == second.state_node && first.rule_number == second.rule_number
        && first.reduction_length == second.reduction_length
        && (first.first_part < second.first_part))
    return true;

  return false;
}

// end of file
