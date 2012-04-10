//                                               -*- C++ -*-
/*
 * BiiPS software is a set of libraries for
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

/*! \file GetLinearTransformVisitor.hpp
 * \brief 
 * 
 * \author  $LastChangedBy$
 * \date    $LastChangedDate$
 * \version $LastChangedRevision$
 * Id:      $Id$
 */

#ifndef BIIPS_GETLINEARTRANSFORMVISITOR_HPP_
#define BIIPS_GETLINEARTRANSFORMVISITOR_HPP_

#include "graph/NodeVisitor.hpp"

namespace Biips
{

  class Graph;
  class NodeSampler;

  class GetLinearTransformVisitor : public ConstNodeVisitor
  {
  protected:
    typedef GetLinearTransformVisitor SelfType;
    typedef Types<SelfType>::Ptr Ptr;

    const Graph & graph_;
    NodeId myId_;
    NodeSampler & nodeSampler_;
    Scalar a_;
    Scalar b_;

    virtual void visit(const ConstantNode & node)
    {
      throw LogicError("ConstantNode can not be visited by GetLinearTransformVisitor.");
    }

    virtual void visit(const StochasticNode & node);

    virtual void visit(const LogicalNode & node);

  public:
    Scalar GetA() const { return a_; }
    Scalar GetB() const { return b_; }

    GetLinearTransformVisitor(const Graph & graph, NodeId myId, NodeSampler & nodeSampler);

  };
}

#endif /* BIIPS_GETLINEARTRANSFORMVISITOR_HPP_ */
