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

////////////////////////////
//   LalrTable.cpp
////////////////////////////

////////////////////////////
//   Author: Vaclav Vacek
////////////////////////////

#include "LalrTable.h"

void LalrTable::print_item(std::pair<int, int> item)
{
  char nonterm_names[] = {'X', 'C', 'A', 'B', 'S', 'D', 'E'};
  unsigned iter;
  std::cerr << nonterm_names[m_rules[item.first].first] << " -> ";
  for(iter = 0; iter < (m_rules[item.first].second).size(); iter++)
  {
    if(item.second == static_cast<int>(iter))
      std::cerr << '.';
    if((m_rules[item.first].second).is_nonterminal(iter))
      std::cerr << nonterm_names[(m_rules[item.first].second)[iter]] << ' ';
    else 
      std::cerr << static_cast<char>(-((m_rules[item.first].second)[iter])) << ' ';
  }
  if(item.second == static_cast<int>(iter))
      std::cerr << '.';
  std::cerr << std::endl;
}

void LalrTable::compute_first(void)
{
  bool change = true;
  unsigned i, j;
  std::set<int>::iterator k, end_iter;
  
  //Computing first
  //epsilon is represented by number epsilon
  while(change)
  {
    change = false;
    for(i = 0; i != m_rules.size(); i++)
    {
      for(j = 0; j < (m_rules[i].second).size(); j++)
      {
        //if the token is a terminal symbol, we add it to the set and stop
        //processing the rule
        if((m_rules[i].second).is_terminal(j))
        {
          if(m_firsts[(m_rules[i].first)].find((m_rules[i].second).at(j)) == m_firsts[(m_rules[i].first)].end())
          { 
            m_firsts[(m_rules[i].first)].insert((m_rules[i].second).at(j));
            change = true;
            if(m_verbose_level >= 2)
              std::cerr << (m_rules[i].first) << " <--term-- " << (m_rules[i].second).at(j)<<std::endl;
          }
          break;   //after this we do not have to continue with this rule
        }
        //if the token is a nonterm, we add its first set (except epsilon)
        //if its first contains epsilon, we continue with the rest of the rule
  
        //epsilon delimiter
        end_iter = m_firsts[(m_rules[i].second).at(j)].find(epsilon);
        //for all the current nonterm's first members except epsilon
        for(k = m_firsts[(m_rules[i].second).at(j)].begin(); k != end_iter; k++)
        {
          if(m_firsts[(m_rules[i].first)].find(*k) == m_firsts[(m_rules[i].first)].end())
          { 
            m_firsts[(m_rules[i].first)].insert(*k);
            change = true;
            if(m_verbose_level >= 2)
              std::cerr << (m_rules[i].first) << " <--nont-- " << *k <<std::endl;
          }
        }
        //if epsilon was not present in the current nonterm's first set,
        //we cannot continue processing the rule
        if(end_iter == m_firsts[(m_rules[i].second).at(j)].end())
         break;
      }
      
      //the rule is now processed
      //if all of the for cycles above were performed, epsilon must be added
      if(j == (m_rules[i].second).size())
      {
        if(m_firsts[(m_rules[i].first)].find(epsilon) == m_firsts[(m_rules[i].first)].end())
        { 
          m_firsts[(m_rules[i].first)].insert(epsilon);
          change = true;
          if(m_verbose_level >= 2)
            std::cerr << (m_rules[i].first) << " <--epsi-- " << epsilon <<std::endl;
        }
      }
    }
  }
}

void LalrTable::compute_nont_first(void)
{
  bool change = true;
  unsigned i, j;
  std::set<int>::iterator k;
  
  for(j = 0; j < m_nonterm_count; j++)
    m_nont_firsts[j].insert(j);
    
  while(change)
  {
    change = false;
    for(i = 0; i != m_rules.size(); i++)
    {
      for(j = 0; j < (m_rules[i].second).size(); j++)
      {
     
        //if the token is a terminal symbol, stop processing the rule
        if((m_rules[i].second).is_terminal(j))
          break;
        //if the token is a nonterm, we add its nont_first set

       
        //for all the current nonterm's nont_first members
        for(k = m_nont_firsts[(m_rules[i].second).at(j)].begin(); 
            k != m_nont_firsts[(m_rules[i].second).at(j)].end();
            k++)
        {
          if(m_nont_firsts[(m_rules[i].first)].find(*k) == m_nont_firsts[(m_rules[i].first)].end())
          { 
            m_nont_firsts[(m_rules[i].first)].insert(*k);
            change = true;
            if(m_verbose_level >= 2)
              std::cerr << (m_rules[i].first) << " <---- " << *k <<std::endl;
          }
        }
        //as the rightmost derivation is
        //required, break comes ALWAYS.
        break;
      }
      
      //the rule is now processed
    }
  }
}

void LalrTable::compute_ext_nont_first(void)
{
  bool change = true;
  bool was_epsilon;
  unsigned i, j;
  std::map<int, std::set<int> >::iterator k, h;
  std::set<int>::iterator l;
  std::set<int> help_set;
  
  help_set.insert(epsilon);
  for(j = 0; j < m_nonterm_count; j++)
    m_ext_nont_firsts[j].insert(std::make_pair(j, help_set));
    
  while(change)
  {
    change = false;
    for(i = 0; i != m_rules.size(); i++)
    {
      //not interested in epsilon-rules
      if(m_rules[i].second.size() == 0)
        break;
        
      //if the token is a terminal symbol, stop processing the rule
      if((m_rules[i].second).is_terminal(0))
        break;
      
      //if the rule looks like A -> By, first(y) is computed here
      help_set.clear();
      help_set.insert(epsilon);
      for(j = 1; j < (m_rules[i].second.size()); j++)
      {
        if(help_set.find(epsilon) == help_set.end())
          break;
        help_set.erase(epsilon);
        if((m_rules[i].second).is_terminal(j))
        {
          help_set.insert((m_rules[i].second)[j]);
          break;
        }
        for(l = m_firsts[(m_rules[i].second)[j]].begin();
              l != m_firsts[(m_rules[i].second)[j]].end();
              l++)
          help_set.insert(*l);
      }
      //help_set now contains first(y)
      
      //k iterates through B's ext_nont_firsts set
      for(k = m_ext_nont_firsts[(m_rules[i].second).at(0)].begin(); 
          k != m_ext_nont_firsts[(m_rules[i].second).at(0)].end();
          k++)
      {
        //h points to the A's ext member with the first part identical with
        //the k's one
        h = m_ext_nont_firsts[(m_rules[i].first)].find(k->first);
        
        //if there is not such member in the A's set, it is added
        if(h == m_ext_nont_firsts[(m_rules[i].first)].end())
        { 
          m_ext_nont_firsts[(m_rules[i].first)].insert(*k);
          
          //if the second part of the k's member contains epsilon,
          //first(y) has to be also added
          if(k->second.find(epsilon) != k->second.end())
          {
            for(l = help_set.begin(); l != help_set.end(); l++)
              m_ext_nont_firsts[(m_rules[i].first)][k->first].insert(*l);
              
            //epsilon remains here only if it is in first(y)
            if(help_set.find(epsilon) == help_set.end())
              m_ext_nont_firsts[(m_rules[i].first)][k->first].erase(epsilon);
          }
          change = true;
        }
      
        //there is a member in A's ext nonterm set with the first part
        //identical with the k's one
        else
        {
          was_epsilon = (h->second.find(epsilon) != h->second.end());
          //the k's second part is added to the A's second part
          for(l = k->second.begin(); l != k->second.end(); l++)
          {
            if(h->second.find(*l) == h->second.end())
            {
              h->second.insert(*l);
              
              //if epsilon is added, no change is made actually, because it
              //will be erased later
              if(*l != epsilon)
                change = true;
            }
          }
        
          //if B's ext nonterm set contains epsilon, first(y) is added
          if(k->second.find(epsilon) != k->second.end())
          {
            if(!was_epsilon)
              h->second.erase(epsilon);
            for(l = help_set.begin(); l != help_set.end(); l++)
              if(h->second.find(*l) == h->second.end())
              {
                if(!was_epsilon || *l != epsilon)
                  change = true;
                h->second.insert(*l);
              }
          }
          //epsilon remains here only if it's in the first(y)
        }
      }
    }
  }


  
  if(m_verbose_level >= 2)
  {
    for(i = 0; i < m_nonterm_count; i++)
    {
      std::cerr << "----ext_nont_first for " << i << std::endl;
      for(k = m_ext_nont_firsts[i].begin(); k != m_ext_nont_firsts[i].end(); k++)
      {
        std::cerr << k->first << ": ";
        for(l = k->second.begin(); l != k->second.end(); l++)
          std::cerr << (*l)<< " ";
        std::cerr<< std::endl;
      }
    }
  }
}

void LalrTable::compute_neps_first(void)
{
  bool change = true;
  unsigned i, j;
  std::set<int>::iterator k, end_iter;
  
  while(change)
  {
    change = false;
    for(i = 0; i != m_rules.size(); i++)
    {
      for(j = 0; j < (m_rules[i].second).size(); j++)
      {
        //if the token is a terminal symbol, we add it to the set and stop
        //processing the rule
        if((m_rules[i].second).is_terminal(j))
        {
          if(m_neps_firsts[(m_rules[i].first)].find((m_rules[i].second).at(j)) == m_neps_firsts[(m_rules[i].first)].end())
          { 
            m_neps_firsts[(m_rules[i].first)].insert((m_rules[i].second).at(j));
            change = true;
            if(m_verbose_level >= 2)
              std::cerr << (m_rules[i].first) << " <--term-- " << (m_rules[i].second).at(j)<<std::endl;
          }
          break;   //after this we do not have to continue with this rule
        }
  
        //if the token is a nonterm, we add its neps_first set (except epsilon)
        //we do NOT continue with the rest of the rule
        end_iter = m_neps_firsts[(m_rules[i].second).at(j)].find(epsilon);
        //for all the current nonterm's first members except epsilon
        for(k = m_neps_firsts[(m_rules[i].second).at(j)].begin(); k != end_iter; k++)
        {
          if(m_neps_firsts[(m_rules[i].first)].find(*k) == m_neps_firsts[(m_rules[i].first)].end())
          { 
            m_neps_firsts[(m_rules[i].first)].insert(*k);
            change = true;
            if(m_verbose_level >= 2)
              std::cerr << (m_rules[i].first) << " <--nont-- " << *k <<std::endl;
          }
        }
         //always break - no other item can be added without epsilon rule       
         break;
      }
      
      //the rule is now processed
    }
  }
}

void LalrTable::compute_lr0_items(void)
{
  std::set<std::pair<int, int> > member;
  unsigned state_count = 1, current_state = 0;
  std::set<std::pair<int, int> >::iterator l;
  std::set<int>::iterator k;
  
  //Here the initial rule is added
  member.insert(std::make_pair(0, 0));
  m_items.push_back(member);
  
  m_go_to.push_back(std::vector<int>(m_nonterm_count + 256, -1)); //-1 is the default value
  
  while(current_state != state_count)
  {
    if(m_verbose_level >= 2)
    {
      std::cerr << "current_state: "<< current_state << std::endl;
      std::cerr << "state_count:   "<< state_count << std::endl;
    }
  
    for(int h = -255; h < static_cast<int>(m_nonterm_count); h++) //for each symbol
    {
      member.clear();
      for(l = m_items[current_state].begin(); l != m_items[current_state].end(); l++)
      //for each item from the kernel
      {
        //the dot cannot be on the right end of the rule
        if(static_cast<int>(((m_rules[(*l).first]).second).size()) > (*l).second)
        //if the rule from the l item has nonterm h on the l item's dot position..
        if(((m_rules[(*l).first]).second)[(*l).second] == h)
        {
          member.insert(std::make_pair((*l).first, (*l).second + 1));
          if(m_verbose_level >= 2)
            std::cerr << "INSERT: " << (*l).first << " - " << (*l).second + 1 << std::endl;
        }
        //for all the nonterms Q such that A -> x.Cy is in the current state and C ->* Q
        //dot cannot be in the last position
        if(static_cast<int>(((m_rules[(*l).first]).second).size()) > (*l).second)
         if(((m_rules[(*l).first]).second).is_nonterminal((*l).second))//if the dot is before a nonterm
          for(k = m_nont_firsts[((m_rules[(*l).first]).second)[(*l).second]].begin();
            k != m_nont_firsts[((m_rules[(*l).first]).second)[(*l).second]].end();
            k++)
        {
  
          //for all the rules
          for(unsigned m = 0; m < m_rules.size(); m++)
          {
            if(((m_rules[m].first) == *k) && (m_rules[m].second.size() > 0))
              if((m_rules[m].second)[0] == h)
              {
                member.insert(std::make_pair(m, 1));
                if(m_verbose_level >= 2)
                  std::cerr << "2NSERT: " << m << " - " << 1 << std::endl; 
              }
          }
        }
      }
      //"go_to(current_state, h)" now computed
  
      bool found = false;
      if(member.empty())
        continue;
  
      
      for(unsigned m = 0; m < m_items.size(); m++)
      {
        if(m_items[m] == member)
        {
          if(h < 0)
            m_go_to[current_state][-h] = m; //0-255 - terminals
          else
            m_go_to[current_state][h + 256] = m; //255+ - nonterminals
          if(m_verbose_level >= 2)
            std::cerr << "goto1(" << current_state<<", " << h << ") = " << m << std::endl;
                 
          found = true;
          break;
        }
      }
      
      if(!found && !member.empty())
      {
        if(m_verbose_level >= 2)
          std::cerr << "goto2(" << current_state<<", " << h << ") = " << state_count << std::endl;
      
        if(h < 0)
            m_go_to[current_state][-h] = state_count; //0-255 - terminals
          else
            m_go_to[current_state][h + 256] = state_count; //255+ - nonterminals
        state_count++;
        
        m_go_to.push_back(std::vector<int>(m_nonterm_count + 256, -1)); //-1 as the default value
        m_items.push_back(member);
      }
  
    } //for each nonterminal
    current_state++;
  } 
}

void LalrTable::compute_lookaheads()
{
  int rulenumber, dotpos;
  bool found;
  bool change;
  std::vector<std::pair<std::pair<int, int>, std::set<int> > > closure;
  std::set<int> first_beta_a;
  std::stack<int, std::list<int> > process_stack;
  unsigned now_processed;
  
////////////////////////
  for(unsigned state = 0; state < m_ext_items.size(); state++)
  {
   for(unsigned var2 = 0; var2 < m_ext_items[state].size(); var2++)
   {
    //For each item in each state
    
    rulenumber = m_ext_items[state][var2].rule_number;
    dotpos = m_ext_items[state][var2].dot_position;
  
//////Here the LR(1) closure is computed
    while(!(process_stack.empty()))
      process_stack.pop();
  
    //initialisation
    closure.clear();
    process_stack.push(0);
    first_beta_a.clear();
    first_beta_a.insert(cross_char);
    closure.push_back(std::make_pair(std::make_pair(rulenumber, dotpos), first_beta_a));
    first_beta_a.clear();
    
    
    while(!(process_stack.empty()))
    {
      now_processed = process_stack.top();
      process_stack.pop();
  
      //computing "first(<beta>a)"
      first_beta_a.clear();
      first_beta_a.insert(epsilon); //starting with epsilon alone
      for(unsigned x = closure[now_processed].first.second + 1;
          x < ((m_rules[closure[now_processed].first.first]).second).size();
          x++)
      {
        if(first_beta_a.find(epsilon) == first_beta_a.end())
          break;
    
        first_beta_a.erase(epsilon);
    
        if(((m_rules[closure[now_processed].first.first]).second).is_terminal(x))
        {
          first_beta_a.insert(((m_rules[closure[now_processed].first.first]).second)[x]);
          break;
        }
        
        for(std::set<int>::iterator
              is = m_firsts[((m_rules[closure[now_processed].first.first]).second)[x]].begin();
              is != m_firsts[((m_rules[closure[now_processed].first.first]).second)[x]].end();
              is++)
          first_beta_a.insert(*is);
      }
      if(first_beta_a.find(epsilon) != first_beta_a.end())
        for(std::set<int>::iterator
              is = closure[now_processed].second.begin();
              is != closure[now_processed].second.end();
              is++)
        first_beta_a.insert(*is);
    
      first_beta_a.erase(epsilon);
  
  
      //If the dot is not in the last position
      if(static_cast<int>(((m_rules[closure[now_processed].first.first]).second).size()) >
                            closure[now_processed].first.second)
      //and if the dot is before a nonterm                  
      if(((m_rules[closure[now_processed].first.first]).second).is_nonterminal\
       (closure[now_processed].first.second))
      {
        for(unsigned x = 0; x < m_rules.size(); x++)
        {
          //looking for all the rules with the nonterm before the dot on the left side
          if(m_rules[x].first == ((m_rules[closure[now_processed].first.first]).second)\
             [closure[now_processed].first.second])
          {
            found = false;
            for(unsigned y = 0; y < closure.size(); y++)
            {
              //if the processed item is already in the closure, only the lookaheads are added
              if((static_cast<unsigned>(closure[y].first.first) == x) &&
                (closure[y].first.second == 0))
                 //0 is present here because only nonkernel items can be got by the closure operation
              {
                found = true;
  
                change = false;
                for(std::set<int>::iterator
                   is = first_beta_a.begin();
                   is != first_beta_a.end();
                   is++)
                {
                  if(closure[y].second.find(*is) == closure[y].second.end())
                  {
                    closure[y].second.insert(*is);
                    change = true;
                  }
                }
                if(change)  //if there was a change in an existing item's lookaheads
                  process_stack.push(y);  //the item has to be processed again
                break; //once the item is found, we do not have to continue
              }
            }
            if(!found)
            {
              closure.push_back(std::make_pair(std::make_pair(x, 0), first_beta_a));
              process_stack.push(closure.size() - 1); //setting the item just inserted to be processed
            }
          
          }
        }
      }
    }//while cycle
  
  ////LR(1) closure for one kernel item now computed
  
  ////Here the spontaneous lookaheads/propagation of the lookaheads is computed
    for(unsigned x = 0; x < closure.size(); x++)
    {
      int symbol;
      int gotoitem_first, gotoitem_second;
      unsigned goto_index1, goto_index2;
      if(static_cast<int>((m_rules[closure[x].first.first].second).size()) <= 
          (closure[x].first.second)) continue;
      //not looking for the rules with the dot on the right end
  
      gotoitem_first = closure[x].first.first;
      gotoitem_second = closure[x].first.second + 1;
      symbol = (m_rules[closure[x].first.first].second)[gotoitem_second - 1];
      
      goto_index1 = m_go_to[state][(symbol < 0)?-symbol:symbol + 256];
  
      for(unsigned y = 0; y < m_ext_items[goto_index1].size(); y++)
        if(m_ext_items[goto_index1][y].rule_number == gotoitem_first &&
           m_ext_items[goto_index1][y].dot_position == gotoitem_second)
        {
          goto_index2 = y;
          break;
        }
  
  
      for(std::set<int>::iterator
            is = closure[x].second.begin();
            is != closure[x].second.end();
            is++)
      {
        if((*is) != cross_char) m_ext_items[goto_index1][goto_index2].lookaheads.insert(*is);
        else m_ext_items[state][var2].propagate_to.insert(std::make_pair(goto_index1, goto_index2)); 
      }
  
       
  
      
    }
   }//all rules cycle
  }//all items cycle
////////////////////////
////finished computing spontaneous lookaheads/propagation

//now propagating the lookaheads
//The EOF lookahead is present in the 0, 0 item
  m_ext_items[0][0].lookaheads.insert(end_of_input);
  
  change = true;
  while(change)
  {
    change = false;
    for(unsigned x = 0; x < m_ext_items.size(); x++)
    {
      for(unsigned y = 0; y < m_ext_items[x].size(); y++)
      {
        for(std::set<int>::iterator z = m_ext_items[x][y].lookaheads.begin();
            z != m_ext_items[x][y].lookaheads.end();
            z++)
          for(std::set<std::pair<int, int> >::iterator zz = m_ext_items[x][y].propagate_to.begin();
              zz != m_ext_items[x][y].propagate_to.end();
              zz++)
        {
          if(m_ext_items[zz->first][zz->second].lookaheads.find(*z) == 
             m_ext_items[zz->first][zz->second].lookaheads.end())
          {
            m_ext_items[zz->first][zz->second].lookaheads.insert(*z);
            change = true;
          }
        }
      }
    }
  }
////Propagating finished
}

void LalrTable::build_table(void)
{
  unsigned i, j, g, h;
  int dot_symbol;
  bool dot_in_the_end;
  std::set<int> help_set;
  std::set<int>::iterator k;
  action help_action;
  m_table.resize(m_ext_items.size());
  
  std::vector<unsigned> epsilon_productions;
  std::map<int, std::set<int> >::iterator m;
  
  for(i = 0; i < m_rules.size(); i++)
    if(m_rules[i].second.size() == 0)
      epsilon_productions.push_back(i);
  
  //for each state
  for(i = 0; i < m_ext_items.size(); i++)
  {
    //256 chars + end_of_input
    m_table[i].resize(1 + 256);
    
    //for each item in the state
    for(j = 0; j < m_ext_items[i].size(); j++)
    {
      //shift action
      if(static_cast<int>(m_rules[m_ext_items[i][j].rule_number].second.size()) > m_ext_items[i][j].dot_position)
      {
        dot_symbol = m_rules[m_ext_items[i][j].rule_number].second.at(m_ext_items[i][j].dot_position);
        if(m_rules[m_ext_items[i][j].rule_number].second.is_terminal(m_ext_items[i][j].dot_position))
        {
          help_action.what = action::shift;
          help_action.next_state = m_go_to[i][- dot_symbol];
          help_action.reduce_by = -1;
          help_action.reduce_by = -1;
          m_table[i][- dot_symbol].insert(help_action);
          if(m_verbose_level >= 2)
          {
            std::cerr << "State " << i << ", symbol "
             << static_cast<char>(- dot_symbol)
             << ", shift " << help_action.next_state << std::endl;
          }
        }
        else
        {
          help_action.what = action::shift;
          for(k = m_neps_firsts[dot_symbol].begin(); k != m_neps_firsts[dot_symbol].end(); k++)
          {
            help_action.next_state = m_go_to[i][- (*k)];
            help_action.reduce_by = -1;
            help_action.reduce_by = -1;
            m_table[i][-(*k)].insert(help_action);
            if(m_verbose_level >= 2)
            {
              std::cerr << "State " << i << ", symbol "
               << static_cast<char>(- (*k))
               << ", shift " << help_action.next_state << std::endl;
            }
          }
        }
      }
      //reduce action
      dot_in_the_end = true;
      for(h = m_ext_items[i][j].dot_position;
          h < (m_rules[m_ext_items[i][j].rule_number].second.size());
          h++)
      {
        if(m_rules[m_ext_items[i][j].rule_number].second.is_terminal(h))
        {
          dot_in_the_end = false;
          break;
        }
        if(m_firsts[m_rules[m_ext_items[i][j].rule_number].second.at(h)].find(epsilon)
           == m_firsts[m_rules[m_ext_items[i][j].rule_number].second.at(h)].end())
        {
          dot_in_the_end = false;
          break;
        }
      }
    
      if(dot_in_the_end)
      {
        help_action.what = action::reduce;
        help_action.next_state = -1;
        help_action.reduce_by = m_ext_items[i][j].rule_number;
        help_action.reduce_length = m_ext_items[i][j].dot_position;
        
        if(help_action.reduce_by == 0)
          m_accepting_state = i;
        for(k = m_ext_items[i][j].lookaheads.begin();
            k != m_ext_items[i][j].lookaheads.end();
            k++)
        {
          m_table[i][-(*k)].insert(help_action);
          if(m_verbose_level >= 2)
          {
            std::cerr << "State " << i << ", symbol "
             << static_cast<char>(- (*k))
             << ", reduce by " << help_action.reduce_by << std::endl;
          }
          
        }
      }
    
    
      //reduce by an epsilon-production action
      if(static_cast<int>(m_rules[m_ext_items[i][j].rule_number].second.size()) > m_ext_items[i][j].dot_position)
      {
        if(m_rules[m_ext_items[i][j].rule_number].second.is_nonterminal(m_ext_items[i][j].dot_position))
        {
          dot_symbol = m_rules[m_ext_items[i][j].rule_number].second.at(m_ext_items[i][j].dot_position);
          for(h = 0; h < epsilon_productions.size(); h++)
          {
            m = m_ext_nont_firsts[dot_symbol].find(m_rules[epsilon_productions[h]].first);
            if(m != m_ext_nont_firsts[dot_symbol].end())
            {
              help_set = m->second;
              for(g = m_ext_items[i][j].dot_position + 1;
                  g < (m_rules[m_ext_items[i][j].rule_number].second.size());
                  g++)
              {
                if(help_set.find(epsilon) == help_set.end())
                  break;
                  
                help_set.erase(epsilon);
                if(m_rules[m_ext_items[i][j].rule_number].second.is_terminal(g))
                {
                  help_set.insert(m_rules[m_ext_items[i][j].rule_number].second.at(g));
                  break;
                }
                else
                {
                  for(k = m_firsts[m_rules[m_ext_items[i][j].rule_number].second.at(g)].begin();
                      k != m_firsts[m_rules[m_ext_items[i][j].rule_number].second.at(g)].end();
                      k++)
                  {
                    help_set.insert(*k);
                  }
                }
              } 
              if(help_set.find(epsilon) != help_set.end())
              {
                help_set.erase(epsilon);
                for(k = m_ext_items[i][j].lookaheads.begin();
                    k != m_ext_items[i][j].lookaheads.end();
                    k++)
                  help_set.insert(*k);

              }
            //first computed
            help_action.what = action::reduce;
            help_action.next_state = -1;
            help_action.reduce_by = epsilon_productions[h];
            help_action.reduce_length = 0;
            for(k = help_set.begin();
                k != help_set.end();
                k++)
            {
              m_table[i][-(*k)].insert(help_action);
              if(m_verbose_level >= 2)
              {
                std::cerr << "State " << i << ", symbol "
                 << static_cast<char>(- (*k))
                 << ", reduce by epsilon " << help_action.reduce_by << std::endl;
              }
              
            }

            
            ///////
            }
          }
        }
      }
    }
  }
}

bool operator<(const LalrTable::action& a1, const LalrTable::action& a2)
{
  if(a1.what < a2.what)
    return true;
  if(a1.next_state < a2.next_state)
    return true;
  if(a1.reduce_by < a2.reduce_by)
    return true;
  if(a1.reduce_length < a2.reduce_length)
    return true;
    
  return false;
}

void LalrTable::make_lalr_table(void)
{
  if(m_verbose_level >= 1)
    std::cerr << ">>>>COMPUTING first<<<<" << std::endl;
  compute_first();
  
  if(m_verbose_level >= 1)
    std::cerr << ">>>>COMPUTING nont_first<<<<" << std::endl;
  compute_nont_first();
  
  if(m_verbose_level >= 1)
    std::cerr << ">>>>COMPUTING neps_first<<<<" << std::endl;
  compute_neps_first();
  
  if(m_verbose_level >= 1)
    std::cerr << ">>>>COMPUTING ext_nont_first<<<<" << std::endl;
  compute_ext_nont_first();
  
  if(m_verbose_level >= 1)
    std::cerr << ">>>>COMPUTING LR(0) items<<<<" << std::endl;
  compute_lr0_items();
  
  
  
  std::set<std::pair<int, int> >::iterator l;
  //listing LR(0) items
  if(m_verbose_level >= 2)
  {
    
    for(unsigned var = 0; var < m_items.size(); var++)
    {
      std::cerr << "----ITEM " << var << "----" << std::endl;
      for(l = m_items[var].begin(); l != m_items[var].end(); l++)
      {
        print_item(*l);
      }
    }
  }

  //copying the data into another structure - m_ext_items
  lr1_ext_item help_item;
  m_ext_items.resize(m_items.size());
  for(unsigned var = 0; var < m_items.size(); var++)
  {
    for(l = m_items[var].begin(); l != m_items[var].end(); l++)
    {
      help_item.rule_number = (*l).first;
      help_item.dot_position = (*l).second;
      m_ext_items[var].push_back(help_item);
    }
  }
  //freeing the previous structure
  m_items.clear();
  
  if(m_verbose_level >= 2)
    std::cerr << ">>>>COMPUTING lookaheads<<<<" << std::endl;
  compute_lookaheads();
  
  
  ////Listing the result
  if(m_verbose_level >= 2)
  {
    for(unsigned x = 0; x < m_ext_items.size(); x++)
    {
      std::cerr << "------ STATE " << x <<" ------" << std::endl;
      for(unsigned y = 0; y < m_ext_items[x].size(); y++)
      {
        print_item(std::make_pair(m_ext_items[x][y].rule_number, m_ext_items[x][y].dot_position));
        std::cerr << "Lookaheads: ";
        for(std::set<int>::iterator z = m_ext_items[x][y].lookaheads.begin();
            z != m_ext_items[x][y].lookaheads.end();
            z++)
          std::cerr << (((*z) == end_of_input)?'$':(static_cast<char>(-(*z)))) << " ";
        std::cerr << '\n' << std::endl;
      }
    }
  }

  if(m_verbose_level >= 1)
    std::cerr << ">>>>BUILDING GLALR table<<<<" << std::endl;
  build_table();
  
}

void LalrTable::load(std::multimap<int, std::vector<int> >& input)
{
  std::multimap<int, std::vector<int> >::iterator i;

  for(i = input.begin(); i != input.end(); i++)
  {
    m_rules.push_back(*i);
  }
}

void LalrTable::print_table(std::string file_name)
{
  std::set<int> used_terminals;
  std::set<int>::iterator set_iter;
  std::set<action>::iterator action_iter;
  std::fstream out_file;
  unsigned i, j;
  
  for(i = 0; i < m_table.size(); i++)
    for(j = 1; j <= 256; j++)
      if(!m_table[i][j].empty())
        used_terminals.insert(j);
        
  out_file.open(file_name.c_str(), std::ios::out);
  if(!out_file)
    throw(std::runtime_error("File cannot be opened."));
    
  out_file << ">>>> PARSING TABLE <<<<" << std::endl;
  for(i = 0; i < m_table.size(); i++)
  {
    out_file << "#### state " << i << " ####" << std::endl;
    for(set_iter = used_terminals.begin();
        set_iter != used_terminals.end();
        set_iter++)
    {
      if(-(*set_iter) == end_of_input)
        out_file << "$: |";
      else
        out_file << static_cast<char>(*set_iter) << ": |";
          
      for(action_iter = m_table[i][*set_iter].begin();
          action_iter != m_table[i][*set_iter].end();
          action_iter++)
      {
        if(action_iter->what == action::shift)
          out_file << "shift " << action_iter->next_state << "| ";
        else
          out_file << "reduce " << action_iter->reduce_by << ", " 
            << action_iter->reduce_length << "| ";
        
      }
      out_file << std::endl;
    
    }
    
  }
  out_file << ">>>> GOTO TABLE <<<<" << std::endl;
  for(i = 0; i < m_table.size(); i++)
  {
    out_file << "#### state " << i << " ####" << std::endl;
    for(j = 256; j < 256 + m_nonterm_count; j++)
      out_file << j - 256 << ": " << m_go_to[i][j] << std::endl;
  }
    
  
}
///////////////////////
///////////////////////
///////////////////////

#ifdef LALRTABLE_TEST
int main(void)
{
  const unsigned nonterm_count = 4;
  LalrTable table(nonterm_count);
  
  std::multimap<int, std::vector<int> > grammar;
  std::vector<int> line;
  /*
  test grammar:
  S'-> S
  S -> bA
  A -> aAB
  A ->
  B -> 
  */
  /*
  //S'-> S
  line.push_back(1);
  grammar.insert(std::pair<int, std::vector<int> >(0, line));
  line.clear();

  //S -> bA
  line.push_back(-98);
  line.push_back(2);
  grammar.insert(std::pair<int, std::vector<int> >(1, line));
  line.clear();

  //A -> aAB
  line.push_back(-97);
  line.push_back(2);
  line.push_back(3);
  grammar.insert(std::pair<int, std::vector<int> >(2, line));
  line.clear();

  //A -> 
  grammar.insert(std::pair<int, std::vector<int> >(2, line));
  line.clear();
  
  //B ->
  grammar.insert(std::pair<int, std::vector<int> >(3, line));
  line.clear();
  */
/*
  test grammar:
  S'-> S
  S -> L = R
  S -> R
  L -> *R
  L -> id
  R -> L
*/
  //Now we have to fill in the test data structure

  //S'-> S
  line.push_back(1);
  grammar.insert(std::pair<int, std::vector<int> >(0, line));
  line.clear();

  //S -> L = R
  line.push_back(2);
  line.push_back(-61);
  line.push_back(3);
  grammar.insert(std::pair<int, std::vector<int> >(1, line));
  line.clear();

  //S -> R
  line.push_back(3);
  grammar.insert(std::pair<int, std::vector<int> >(1, line));
  line.clear();

  //L -> *R
  line.push_back(-42);
  line.push_back(3);
  grammar.insert(std::pair<int, std::vector<int> >(2, line));
  line.clear();

  //L -> id
  line.push_back(-105); 
  grammar.insert(std::pair<int, std::vector<int> >(2, line));
  line.clear();


  //R -> L
  line.push_back(2);
  grammar.insert(std::pair<int, std::vector<int> >(3, line));
  line.clear();


  //Grammar is now in the grammar structure
  
  table.load(grammar);
  table.make_lalr_table();
  table.print_table("table.txt");
  return 0;
}
#endif
  
