#ifndef BIIPS_DUNIF_HPP_
#define BIIPS_DUNIF_HPP_

#include "distribution/Distribution.hpp"

namespace Biips
{

  class DUnif: public Distribution
  {
  protected:
    typedef DUnif SelfType;

    DUnif() :
      Distribution("dunif", 2)
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
    virtual void
    fixedUnboundedSupport(ValArray & lower,
                     ValArray & upper,
                     const NumArray::Array & paramValues) const;

  public:
    virtual Bool CheckParamValues(const NumArray::Array & paramValues) const;
    virtual Bool IsSupportFixed(const Flags & fixmask) const;
    static Distribution::Ptr Instance()
    {
      static Distribution::Ptr p_instance(new SelfType());
      return p_instance;
    }
  };

}

#endif /* BIIPS_DUNIF_HPP_ */
