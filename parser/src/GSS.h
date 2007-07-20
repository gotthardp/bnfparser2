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

#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <set>

/** \brief This class contains the implementation of GSS (Graph Structured Stack)
 *
 */
 
class GSS
{
public:
  //!This class is used for identifying state nodes.
  class StateIdent
  {
  public:
    unsigned level; //!< The level the node is stored in
    unsigned id;  //!< The number the node has within the level
    StateIdent(unsigned _level, unsigned _id)
    :level(_level), id(_id){}
    StateIdent(){}   
  };
  
  //!This class is used for identifying state nodes.
  class SymbolIdent
  {
  public:
    unsigned id;  //!< The number of the symbol node
    SymbolIdent(unsigned _id)
    :id(_id){}
    SymbolIdent(){}
  };


  friend bool operator<(const StateIdent & first, const StateIdent & second);
  
  friend bool operator==(const StateIdent & first, const StateIdent & second);

  friend bool operator<(const SymbolIdent & first, const SymbolIdent & second);

private:
  //!The class representing state nodes in the GSS
  class StateNode
  {
  public:
    int label; //!< The number of the state 
    std::vector<SymbolIdent> successors; //!< Identifiers of the succeeding symbol nodes
    StateNode(int _label) //!< Constructor takes the number of the state
    :label(_label){}
  };
  
  //!The class representing symbol nodes in the GSS
  class SymbolNode
  {
  public:
    int symbol; //!< Grammar symbol
    std::set<std::string> semantic_value; //!< The portion of input generated by the symbol
    std::vector<StateIdent> successors; //!< Identifiers of the succeeding state nodes
    SymbolNode(int _symbol, std::string _semantic) //!< Constructor takes the symbol and its semantic value
    :symbol(_symbol)
    {
      semantic_value.insert(_semantic);
    }
  
    //! Adds a new semantic value to the symbol node
    void add_value(std::string val)
    {
      semantic_value.insert(val);
    }
  };
  
  
  //! Stores all the symbol nodes
  std::vector<SymbolNode> m_symbol_nodes;

  //! Each state level is a vector of state nodes
  std::vector<std::vector<StateNode> > m_state_levels;

  //! The length of the word
  unsigned m_length;
  
public:
  //! Constructor creates an empty GSS, must be initialised before use!
  GSS(void)
  :m_length(0)
  {}
  
  //! Resets the GSS, sets the new length of the word
  void reset(unsigned length)
  {
    m_symbol_nodes.clear();
    m_state_levels.clear();
    m_length = length + 1; //0 <= index <= length
    m_state_levels.resize(m_length);
  }


  //! Creates a symbol node with the specified label
  SymbolIdent create_symbol(int _symbol, std::string _sem_val)
  {
    m_symbol_nodes.push_back(SymbolNode(_symbol, _sem_val));
    return SymbolIdent(m_symbol_nodes.size() - 1);
  }
  
  //! Creates a symbol node with the specified label within the specified level
  StateIdent create_state(unsigned _level, int _label)
  {
    if(_level >= m_length)
      throw std::out_of_range("Invalid level");
      
    m_state_levels.at(_level).push_back(StateNode(_label));
    return StateIdent(_level, m_state_levels.at(_level).size() - 1);
  }

  //! Makes symbol node which successor of state node whose
  void add_successor_to_state(StateIdent whose, SymbolIdent which)
  {
    if(whose.level >= m_length || m_state_levels.at(whose.level).size() < whose.id)
      throw std::out_of_range("Invalid level or id (StateIdent)");
    if(which.id > m_symbol_nodes.size())
      throw std::out_of_range("Invalid symbol node");
      
    m_state_levels.at(whose.level).at(whose.id).successors.push_back(which);
  }

  //! Makes state node which successor of symbol node whose
  void add_successor_to_symbol(SymbolIdent whose, StateIdent which)
  {
    if(which.level >= m_length || m_state_levels.at(which.level).size() < which.id)
      throw std::out_of_range("Invalid level or id (StateIdent)");
    if(whose.id > m_symbol_nodes.size())
      throw std::out_of_range("Invalid symbol node");
      
    m_symbol_nodes.at(whose.id).successors.push_back(which);
  }

  //! Returns the label of the specified state node
  int get_state_label(GSS::StateIdent which)
  {
    return m_state_levels[which.level][which.id].label;
  }

  //! Returns the level of the specified state node
  unsigned get_state_level(GSS::StateIdent which)
  {
    return which.level;
  }

  //! Returns the label of the specified symbol node
  int get_symbol_label(GSS::SymbolIdent which)
  {
    return m_symbol_nodes[which.id].symbol;
  }

  //! Returns the vector of state nodes reachable from the from state node reducing by certain rule length
  std::vector<std::pair<GSS::StateIdent, std::string> > find_reachable(GSS::StateIdent from, unsigned length, GSS::SymbolIdent first_part);

  //! Returns the vector of state nodes with the specified label within the specified level 
  std::vector<GSS::StateIdent> find_state(unsigned level, int label);
  
  //! Checks if the state level specified by the argument is empty
  bool state_level_empty(unsigned level)
  {
    return m_state_levels.at(level).empty();
  }

  //! Returns the successors of the state node with the label "label"
  std::vector<SymbolIdent> get_state_successors_with_label(StateIdent node, int label)
  {
    std::vector<SymbolIdent> retval;
    unsigned k;
    for(k = 0; k < m_state_levels[node.level][node.id].successors.size(); k++)
    {
      if(m_symbol_nodes.at(m_state_levels[node.level][node.id].successors[k].id).symbol
          == label)
        retval.push_back(m_state_levels[node.level][node.id].successors[k]);
    }
    return retval;
  }

  //! Returns the successors of the state node
  std::vector<SymbolIdent> get_state_successors(StateIdent node)
  {
    return m_state_levels[node.level][node.id].successors;
  }

  //! Returns the successors of the symbol node
  std::vector<StateIdent> get_symbol_successors(SymbolIdent node)
  {
    return m_symbol_nodes[node.id].successors;
  }

  //! Returns the successors of the symbol node
  unsigned get_successor_level(SymbolIdent node)
  {
    return m_symbol_nodes[node.id].successors.at(0).level;
  }
  //! Checks if the specified symbol node has the specified state node as a successor
  bool has_state_successor(SymbolIdent symbol, StateIdent state)
  {
    unsigned i;
    for(i = 0; i < m_symbol_nodes[symbol.id].successors.size(); i++)
    {
      if(m_symbol_nodes[symbol.id].successors[i] == state)
        return true;
    }
    return false;
  }


  //! Adds a new semantic value to the specified symbol node
  void add_semantics_to_symbol(SymbolIdent symbol, std::string value)
  {
    m_symbol_nodes[symbol.id].add_value(value);
  }

  //! Returns the XML string describing the set of semantic values stored in the specified symbol node
  std::string get_semantic_string(SymbolIdent symbol)
  {
    std::string retval;
    std::set<std::string>::iterator s_i;
    
    if (m_symbol_nodes[symbol.id].semantic_value.size() == 0)
      return "";
    else if (m_symbol_nodes[symbol.id].semantic_value.size() == 1)
      return *(m_symbol_nodes[symbol.id].semantic_value.begin());
     
    retval = "<ambiguity>"; 
    for(s_i = m_symbol_nodes[symbol.id].semantic_value.begin();
        s_i != m_symbol_nodes[symbol.id].semantic_value.end();
        s_i++)
      retval += "<way>" + *s_i + "</way>";
    
    retval += "</ambiguity>";
    
    return retval;
  }

};
