/*! \file Counter.cpp
 * COPY: Adapted from JAGS Counter class
 */

#include "common/Counter.hpp"
#include "common/Error.hpp"

namespace Biips
{

  Counter::Counter(const IndexRange & range)
    : BaseType(range)
  {
    if (range.NDim(false) != 1)
      throw LogicError("Attempt to construct Counter from non-scalar Range");
  }
}
