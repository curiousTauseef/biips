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

/*! \file DNormVar.hpp
 * \brief 
 * 
 * \author  $LastChangedBy$
 * \date    $LastChangedDate$
 * \version $LastChangedRevision$
 * Id:      $Id$
 */

#ifndef BIIPS_DNORMVAR_HPP_
#define BIIPS_DNORMVAR_HPP_

#include "distributions/BoostScalarDistribution.hpp"
#include <boost/random/normal_distribution.hpp>
#include <boost/math/distributions/normal.hpp>

namespace Biips
{
  typedef boost::math::normal_distribution<Scalar> NormalMathDistType;
  typedef boost::normal_distribution<Scalar> NormalRandomDistType;

  class DNormVar: public BoostScalarDistribution<NormalMathDistType,
      NormalRandomDistType>
  {
  public:
    typedef DNormVar SelfType;
    typedef BoostScalarDistribution<NormalMathDistType, NormalRandomDistType>
        BaseType;

  protected:
    DNormVar() :
      BaseType("dnormvar", 2, DIST_UNBOUNDED, false)
    {
    }
    virtual Bool checkParamValues(const NumArray::Array & paramValues) const;
    virtual void sample(ValArray & values,
                        const NumArray::Array & paramValues,
                        const NumArray::Pair & boundValues,
                        Rng & rng) const;
    virtual MathDistType mathDist(const NumArray::Array & paramValues) const;
    virtual RandomDistType
    randomDist(const NumArray::Array & paramValues) const;

  public:
    virtual Scalar d(Scalar x,
                     const NumArray::Array & paramValues,
                     Bool give_log) const;
    static Distribution::Ptr Instance()
    {
      static Distribution::Ptr p_instance(new SelfType());
      return p_instance;
    }
  };

}

#endif /* BIIPS_DNORMVAR_HPP_ */
