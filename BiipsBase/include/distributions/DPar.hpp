//                                               -*- C++ -*-
/*! \file DPar.hpp
 * \brief 
 * 
 * $LastChangedBy$
 * $LastChangedDate$
 * $LastChangedRevision$
 * $Id$
 */

#ifndef BIIPS_DPAR_HPP_
#define BIIPS_DPAR_HPP_

#include "distributions/BoostScalarDistribution.hpp"
#include <boost/random/pareto_distribution.hpp>
#include <boost/math/distributions/pareto.hpp>

namespace Biips
{

  typedef boost::math::pareto_distribution<Scalar> ParMathDistType;
  typedef boost::pareto_distribution<Scalar> ParRandomDistType;

  class DPar : public BoostScalarDistribution<ParMathDistType, ParRandomDistType>
  {
  public:
    typedef DPar SelfType;
    typedef BoostScalarDistribution<ParMathDistType, ParRandomDistType> BaseType;

  protected:
    DPar() : BaseType("dpar", 2, DIST_SPECIAL, false) {}
    virtual Bool checkParamValues(const MultiArray::Array & paramValues) const;
    virtual Scalar unboundedLower(const MultiArray::Array & paramValues) const;
    virtual Scalar unboundedUpper(const MultiArray::Array & paramValues) const;

    virtual MathDistType mathDist(const MultiArray::Array & paramValues) const;
    virtual RandomDistType randomDist(const MultiArray::Array & paramValues) const;

  public:
    virtual String Alias() const { return "dbinom"; }
    virtual Bool IsSupportFixed(const Flags & fixmask) const;
    virtual Scalar d(Scalar x, const MultiArray::Array & paramValues,
        Bool give_log) const;
    static Distribution::Ptr Instance() { static Distribution::Ptr p_instance(new SelfType()); return p_instance; }
  };

}

#endif /* BIIPS_DPAR_HPP_ */
