extern "C" {
  void BiipsBase_is_present(void) {}
}

#include "BiipsBase.hpp"
#include "sampler/ForwardSampler.hpp"

#include "functions/LogicalOperators.hpp"
#include "functions/ComparisonOperators.hpp"
#include "functions/ArithmeticOperators.hpp"
#include "functions/MatMult.hpp"
#include "functions/Pow.hpp"
#include "functions/IfElse.hpp"
#include "functions/TrigonometricFunctions.hpp"
#include "functions/UsualFunctions.hpp"
#include "functions/Step.hpp"
#include "functions/Transpose.hpp"
#include "functions/Sum.hpp"
#include "functions/Inverse.hpp"
#include "functions/Max.hpp"
#include "functions/Min.hpp"
#include "functions/Prod.hpp"
#include "functions/Equals.hpp"
#include "functions/Sort.hpp"
#include "functions/Logit.hpp"
#include "functions/Probit.hpp"
#include "functions/CLogLog.hpp"

#include "distributions/DNorm.hpp"
#include "distributions/DNormVar.hpp"
#include "distributions/DMNorm.hpp"
#include "distributions/DMNormVar.hpp"
#include "distributions/DBeta.hpp"
#include "distributions/DBin.hpp"
#include "distributions/DCat.hpp"
#include "distributions/DUnif.hpp"
#include "distributions/DBern.hpp"
#include "distributions/DExp.hpp"
#include "distributions/DLnorm.hpp"
#include "distributions/DChisqr.hpp"
#include "distributions/DGamma.hpp"
#include "distributions/DT.hpp"
#include "distributions/DWeib.hpp"
//#include "distributions/DPar.hpp"
#include "distributions/DPois.hpp"
#include "distributions/DF.hpp"
//#include "distributions/DDexp.hpp"
//#include "distributions/DGenGamma.hpp"
#include "distributions/DMulti.hpp"
#include "distributions/DInterval.hpp"

#include "samplers/FiniteSampler.hpp"
#include "samplers/ConjugateNormalVar.hpp"
#include "samplers/ConjugateNormal.hpp"
#include "samplers/ConjugateBeta.hpp"
#include "samplers/ConjugateMNormalCov.hpp"
#include "samplers/ConjugateMNormal.hpp"
#include "samplers/ConjugateNormalVarLinear.hpp"
#include "samplers/ConjugateNormalLinear.hpp"
#include "samplers/ConjugateMNormalCovLinear.hpp"
#include "samplers/ConjugateMNormalLinear.hpp"

namespace Biips
{

  void loadBaseModule(FunctionTable & funcTab, DistributionTable & distTab)
  {
    // TODO check returned boolean
    funcTab.Insert(And::Instance(), true);
    funcTab.Insert(Or::Instance(), true);
    funcTab.Insert(Not::Instance(), true);

    funcTab.Insert(GreaterThan::Instance(), true);
    funcTab.Insert(GreaterOrEqual::Instance(), true);
    funcTab.Insert(LessThan::Instance(), true);
    funcTab.Insert(LessOrEqual::Instance(), true);
    funcTab.Insert(Equal::Instance(), true);
    funcTab.Insert(NotEqual::Instance(), true);

    funcTab.Insert(Add::Instance(), true);
    funcTab.Insert(Subtract::Instance(), true);
    funcTab.Insert(Multiply::Instance(), true);
    funcTab.Insert(Divide::Instance(), true);
    funcTab.Insert(MatMult::Instance(), true);
    funcTab.Insert(Neg::Instance(), true);

    funcTab.Insert(PowInfix::Instance(), true);
    funcTab.Insert(Pow::Instance(), true);

    funcTab.Insert(Equals::Instance(), true);
    funcTab.Insert(IfElse::Instance(), true);

    funcTab.Insert(Cos::Instance(), true);
    funcTab.Insert(Sin::Instance(), true);
    funcTab.Insert(Tan::Instance(), true);
    funcTab.Insert(ArcCos::Instance(), true);
    funcTab.Insert(ArcSin::Instance(), true);
    funcTab.Insert(ArcTan::Instance(), true);
    funcTab.Insert(Cosh::Instance(), true);
    funcTab.Insert(Sinh::Instance(), true);
    funcTab.Insert(Tanh::Instance(), true);
    funcTab.Insert(ArcCosh::Instance(), true);
    funcTab.Insert(ArcSinh::Instance(), true);
    funcTab.Insert(ArcTanh::Instance(), true);

    funcTab.Insert(Abs::Instance(), true);
    funcTab.Insert(Sqrt::Instance(), true);
    funcTab.Insert(Exp::Instance(), true);
    funcTab.Insert(Log::Instance(), true);
    funcTab.Insert(Round::Instance(), true);
    funcTab.Insert(Trunc::Instance(), true);

    funcTab.Insert(Logit::Instance(), true);
    funcTab.Insert(ILogit::Instance(), true);
    funcTab.Insert(Probit::Instance(), true);
    funcTab.Insert(Phi::Instance(), true);
    funcTab.Insert(CLogLog::Instance(), true);
    funcTab.Insert(ICLogLog::Instance(), true);

    funcTab.Insert(Step::Instance(), true);

    funcTab.Insert(Transpose::Instance(), true);
    funcTab.Insert(Sum::Instance(), true);
    funcTab.Insert(Inverse::Instance(), true);
    funcTab.Insert(Max::Instance(), true);
    funcTab.Insert(Min::Instance(), true);
    funcTab.Insert(Prod::Instance(), true);
    funcTab.Insert(Sort::Instance(), true);

    // TODO check returned boolean
    distTab.Insert(DNorm::Instance(), true);
    distTab.Insert(DNormVar::Instance(), true);
    distTab.Insert(DMNorm::Instance(), true);
    distTab.Insert(DMNormVar::Instance(), true);
    distTab.Insert(DBeta::Instance(), true);
    distTab.Insert(DBin::Instance(), true);
    distTab.Insert(DCat::Instance(), true);
    distTab.Insert(DUnif::Instance(), true);
    distTab.Insert(DBern::Instance(), true);
    distTab.Insert(DExp::Instance(), true);
    distTab.Insert(DLnorm::Instance(), true);
    distTab.Insert(DChisqr::Instance(), true);
    distTab.Insert(DGamma::Instance(), true);
    distTab.Insert(DT::Instance(), true);
    distTab.Insert(DWeib::Instance(), true);
//    distTab.Insert(DPar::Instance(), true);
    distTab.Insert(DPois::Instance(), true);
    distTab.Insert(DF::Instance(), true);
//    distTab.Insert(DDexp::Instance(), true);
//    distTab.Insert(DGenGamma::Instance(), true);
    distTab.Insert(DMulti::Instance(), true);
    distTab.Insert(DInterval::Instance(), true);

    ForwardSampler::NodeSamplerFactories().push_front(std::make_pair(ConjugateMNormalLinearFactory::Instance(),
                                                                     true));
    ForwardSampler::NodeSamplerFactories().push_front(std::make_pair(ConjugateMNormalCovLinearFactory::Instance(),
                                                                     true));
    ForwardSampler::NodeSamplerFactories().push_front(std::make_pair(ConjugateNormalLinearFactory::Instance(),
                                                                     true));
    ForwardSampler::NodeSamplerFactories().push_front(std::make_pair(ConjugateNormalVarLinearFactory::Instance(),
                                                                     true));

    ForwardSampler::NodeSamplerFactories().push_front(std::make_pair(ConjugateMNormalFactory::Instance(),
                                                                     true));
    ForwardSampler::NodeSamplerFactories().push_front(std::make_pair(ConjugateMNormalCovFactory::Instance(),
                                                                     true));
    ForwardSampler::NodeSamplerFactories().push_front(std::make_pair(ConjugateBetaFactory::Instance(),
                                                                     true));
    ForwardSampler::NodeSamplerFactories().push_front(std::make_pair(ConjugateNormalFactory::Instance(),
                                                                     true));
    ForwardSampler::NodeSamplerFactories().push_front(std::make_pair(ConjugateNormalVarFactory::Instance(),
                                                                     true));

    ForwardSampler::NodeSamplerFactories().push_front(std::make_pair(FiniteSamplerFactory::Instance(),
                                                                     true));
  }

}
