#include "RFunction.h"
#include <algorithm>
#include "Rbiips_utils.h"

namespace Biips 
{

    void RFunction::eval(ValArray & output,
                        const NumArray::Array & params) const {
   
        int nrhs  = params.size();
        std::vector<Rcpp::NumericVector> vecParams(nrhs);
        for(int i = 0; i < nrhs ; ++i ){
            vecParams[i] = convArrayVector(params[i]);
        }
        
        Rcpp::NumericVector outvec = apply(vecParams, fun_eval_, nrhs); 
        output.assign(outvec.begin(), outvec.end()); 
    }

    DimArray RFunction::dim(const std::vector<DimArray::Ptr> & paramDims) const {
        int nrhs = paramDims.size();
        std::vector<Rcpp::IntegerVector> paramvec(nrhs);
        for(int i = 0; i < nrhs; ++i) {
            paramvec[i].assign(paramDims[i]->begin(), paramDims[i]->end());

        }
        Rcpp::IntegerVector outvec = apply(paramvec, fun_dim_, nrhs);
       
        return DimArray(outvec.begin(), outvec.end());
    }

    Bool RFunction::CheckParamValues(const NumArray::Array & paramValues) const {

       int nrhs = paramValues.size();
       std::vector<Rcpp::NumericVector> vecParamValues(nrhs);
       for(int i = 0; i < nrhs; ++i) {
        vecParamValues[i] = convArrayVector(paramValues[i]);
       }
       Rcpp::NumericVector res = apply(vecParamValues, fun_check_param_, nrhs);
       
       int mybool = static_cast<int>(res[0]);
       return mybool;
    }

    Bool RFunction::IsDiscreteValued(const std::vector<bool> & mask) const {
       
         bool res = apply(mask, fun_is_discrete_, mask.size());
         return static_cast<bool>(res); 
    }
      
}
