#ifndef BIIPS_DBERN_HPP_
#define BIIPS_DBERN_HPP_

#include "distribution/Distribution.hpp"

namespace Biips
{

  class DBern: public Distribution
  {
  protected:
    typedef DBern SelfType;

    DBern() :
      Distribution("dbern", 1)
    {
    }

    virtual Bool
    checkParamDims(const Types<DimArray::Ptr>::Array & paramDims) const;
    virtual DimArray dim(const Types<DimArray::Ptr>::Array & paramDims) const;
    virtual void sample(ValArray & values,
                        const NumArray::Array & paramValues,
                        const NumArray::Pair & boundValues,
                        Rng & rng) const;
    virtual Scalar logDensity(const NumArray & x,
                              const NumArray::Array & paramValues,
                              const NumArray::Pair & boundValues) const;
    virtual void fixedUnboundedSupport(ValArray & lower,
                                  ValArray & upper,
                                  const NumArray::Array & paramValues) const;

  public:
    virtual Bool CheckParamValues(const NumArray::Array & paramValues) const;
    virtual Bool IsDiscreteValued(const Flags & mask) const
    {
      return true;
    }
    virtual Bool IsSupportFixed(const Flags & fixmask) const
    {
      return true;
    }
    static Distribution::Ptr Instance()
    {
      static Distribution::Ptr p_instance(new SelfType());
      return p_instance;
    }
  };

}

#endif /* BIIPS_DBERN_HPP_ */
