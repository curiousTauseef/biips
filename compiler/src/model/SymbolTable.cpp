//                                               -*- C++ -*-
/*
 * BiiPS software is a set of C++ libraries for
 * Bayesian inference with interacting Particle Systems.
 * Copyright (C) Inria, 2012
 * Contributors: Adrien Todeschini, Francois Caron
 *
 * BiiPS is derived software based on:
 * JAGS, Copyright (C) Martyn Plummer, 2002-2010
 * SMCTC, Copyright (C) Adam M. Johansen, 2008-2009
 *
 * This file is part of BiiPS.
 *
 * BiiPS is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*! \file SymbolTable.cpp
 * \brief 
 * 
 * \author  $LastChangedBy$
 * \date    $LastChangedDate$
 * \version $LastChangedRevision$
 * Id:      $Id$
 *
 * COPY: Adapted from JAGS SymTab class
 */

#include "model/SymbolTable.hpp"
#include "model/Model.hpp"
#include "iostream/outStream.hpp"
#include "model/DeparseVisitor.hpp"

namespace Biips
{

  SymbolTable::SymbolTable(Model & model) :
    model_(model)
  {
  }

  void SymbolTable::AddVariable(const String & name, const DimArray & dim)
  {
    if (nodeArraysMap_.count(name))
      throw RuntimeError(String("Name ") + name
          + " already in use in symbol table");

    NodeArray::Ptr p_node_array(new NodeArray(name, *(model_.GraphPtr()), dim));
    nodeArraysMap_[name] = p_node_array;
  }

  void SymbolTable::InsertNode(NodeId nodeId,
                               const String & name,
                               const IndexRange & range)
  {
    // check that name is an existing variable
    if (!nodeArraysMap_.count(name))
      throw LogicError(String("Name ") + name
          + " does not exist in the symbol table.");

    //    // check that nodeId is not already in an existing NodeArray
    //    for (std::map<String, NodeArray::Ptr>::const_iterator it_table =
    //        nodeArraysMap_.begin(); it_table != nodeArraysMap_.end(); ++it_table)
    //    {
    //      if (it_table->second->Contains(nodeId))
    //        throw LogicError(String("Node ") + print(nodeId)
    //            + " already in use in variable " + it_table->first + ".");
    //    }

    nodeArraysMap_.at(name)->Insert(nodeId, range);
  }

  Bool SymbolTable::Contains(NodeId nodeId) const
  {
    std::map<String, NodeArray::Ptr>::const_iterator it_table =
        nodeArraysMap_.begin();
    for (; it_table != nodeArraysMap_.end(); ++it_table)
    {
      if (it_table->second->Contains(nodeId))
        return true;
    }

    return false;
  }

  const String & SymbolTable::GetVariableName(NodeId nodeId) const
  {
    std::map<String, NodeArray::Ptr>::const_iterator it_table =
        nodeArraysMap_.begin();
    for (; it_table != nodeArraysMap_.end(); ++it_table)
    {
      const NodeArray::Ptr & p_array = it_table->second;
      if (p_array->Contains(nodeId))
      {
        IndexRange node_range = p_array->GetRange(nodeId);
        return it_table->first;
      }
    }

    throw RuntimeError(String("Unable to find Node ") + print(nodeId)
        + " in the symbol table.");
  }

  String SymbolTable::GetName(NodeId nodeId) const
  {
    std::map<String, NodeArray::Ptr>::const_iterator it_table =
        nodeArraysMap_.begin();
    for (; it_table != nodeArraysMap_.end(); ++it_table)
    {
      const NodeArray::Ptr & p_array = it_table->second;
      if (p_array->Contains(nodeId))
      {
        IndexRange node_range = p_array->GetRange(nodeId);
        if (node_range == p_array->Range())
          return it_table->first;
        else
          return it_table->first + print(node_range);
      }
    }

    //Name not in symbol table: calculate name from parents
    typedef GraphTypes::ParentIterator DirectParentNodeIdIterator;
    DirectParentNodeIdIterator it_parents, it_parents_end;

    boost::tie(it_parents, it_parents_end)
        = model_.GraphPtr()->GetParents(nodeId);
    Size n_par = std::distance(it_parents, it_parents_end);
    Types<String>::Array par_names(n_par);
    for (Size i = 0; it_parents != it_parents_end; ++it_parents, ++i)
      par_names[i] = GetName(*it_parents);

    return deparse(nodeId, *(model_.GraphPtr()), par_names);
  }

  void SymbolTable::WriteData(std::map<String, MultiArray> const & dataMap)
  {
    for (std::map<String, MultiArray>::const_iterator p(dataMap.begin()); p
        != dataMap.end(); ++p)
    {
      const String & name = p->first;
      if (Contains(name))
      {
        NodeArray & array = getNodeArray(name);
        array.SetData(p->second);
      }
    }
  }

  void SymbolTable::update_children(const std::map<Size, NodeId> & logicChildrenByRank,
                                    std::map<Size, NodeId> & stoChildrenByRank)
  {
    // update logical children
    for (std::map<Size, NodeId>::const_iterator it(logicChildrenByRank.begin()); it
        != logicChildrenByRank.end(); ++it)
    {
      NodeId id = it->second;
      model_.GraphPtr()->SampleValue(id);
      model_.GraphPtr()->UpdateDiscreteness(id, stoChildrenByRank);
    }

    // update stochastic children discreteness
    for (Size rank = 0; rank < model_.GraphPtr()->GetRanks().back(); ++rank)
    {
      if (!stoChildrenByRank.count(rank))
        continue;

      NodeId id = stoChildrenByRank.at(rank);
      model_.GraphPtr()->UpdateDiscreteness(id, stoChildrenByRank);
    }
  }

  Bool SymbolTable::ChangeData(const std::map<String, MultiArray> & dataMap)
  {
    Bool set_observed_nodes = false;

    std::map<Size, NodeId> logic_children_by_rank;
    std::map<Size, NodeId> sto_children_by_rank;
    for (std::map<String, MultiArray>::const_iterator p(dataMap.begin()); p
        != dataMap.end(); ++p)
    {
      const String & name = p->first;
      if (!Contains(name))
        throw RuntimeError("Can't change data: variable is not defined in the model.");

      NodeArray & array = getNodeArray(name);
      // change data, get all observed logical children
      // update discreteness and get stochastic children to be updated
      set_observed_nodes = array.ChangeData(p->second,
                                            logic_children_by_rank,
                                            sto_children_by_rank);
    }

    // update children
    update_children(logic_children_by_rank, sto_children_by_rank);

    return set_observed_nodes;
  }

  Bool SymbolTable::SampleData(const Types<String>::Array & variableNames,
                               Rng * pRng,
                               std::map<String, MultiArray> & dataMap)
  {
    Bool set_observed_nodes = false;

    std::map<Size, NodeId> logic_children_by_rank;
    std::map<Size, NodeId> sto_children_by_rank;
    for (Types<String>::ConstIterator it(variableNames.begin()); it
        != variableNames.end(); ++it)
    {
      const String & name = *it;
      if (!Contains(name))
        throw RuntimeError("Can't sample data: variable is not defined in the model.");

      NodeArray & array = getNodeArray(name);
      // sample data, get all observed logical children
      // update discreteness and get stochastic children to be updated
      set_observed_nodes = array.SampleData(logic_children_by_rank,
                                            sto_children_by_rank,
                                            pRng);
      dataMap.insert(std::make_pair(name, array.GetData()));
    }

    // update children
    update_children(logic_children_by_rank, sto_children_by_rank);

    return set_observed_nodes;
  }

  void SymbolTable::ReadData(std::map<String, MultiArray> & dataMap) const
  {
    std::map<String, NodeArray::Ptr>::const_iterator it_table;
    for (it_table = nodeArraysMap_.begin(); it_table != nodeArraysMap_.end(); ++it_table)
    {
      /* Create a new MultiArray to hold the values from the symbol table */
      MultiArray read_values = it_table->second->GetData();
      /* Only write to the data table if we can find at least one
       non-missing value */
      if (!allMissing(read_values))
      {
        const String & name = it_table->first;
        if (dataMap.find(name) != dataMap.end())
        {
          //Replace any existing entry
          dataMap.erase(name);
        }
        dataMap.insert(std::make_pair(name, read_values));
      }
    }
  }
}
