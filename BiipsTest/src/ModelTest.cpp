//                                               -*- C++ -*-
/*! \file ModelTest.cpp
* \brief
*
* $LastChangedBy$
* $LastChangedDate$
* $LastChangedRevision$
* $Id$
*/

#include "ModelTest.hpp"
#include "TestIO.hpp"
#include "common/cholesky.hpp"

namespace Biips
{

  void ModelTest::printValues(std::ostream & os, const String & name, const DataType::Array & dataArray, Size len, char separator) const
  {
    for (Size dim=0; dim<len; ++dim)
    {
      os << name;
      if (len > 1)
        os << "." << dim+1;
      os << " = ";
      printLine(os, dataArray, dim, separator);
    }
  }


  void ModelTest::printLine(std::ostream & os, const DataType::Array & dataArray, Size dim, char separator) const
  {
    for (Size k=0; k<dataArray.size(); ++k)
      os << dataArray[k].Value()[dim] << separator;
    os << std::endl;
  }


  void ModelTest::setObsValues()
  {
    NodeValues obs_values(pModelGraph_->GetSize());
    for (Size i_var=0; i_var<obsVarNames_.size(); ++i_var)
    {
      const String & var_name = obsVarNames_[i_var];
      DataType::Array & gen_val = dataValuesMap_[var_name];
      for (Size k=0; k<gen_val.size(); ++k)
        obs_values[modelNodeIdMap_[var_name][k]] = gen_val[k].ValuePtr();
    }
    pModelGraph_->SetObsValues(obs_values);
  }


//  void ModelTest::progressBar(Scalar progress)
//  {
//    static const Size N_CHAR = 40;
//    static Size current_pos;
//    using namespace std;
//    if ( progress == 0.0)
//    {
//      current_pos = 0;
//      os_ << String(N_CHAR, '-') << "|" << pSampler_->NIterations() << endl;
//    }
//    else
//    {
//      Size new_pos = floor(progress * N_CHAR);
//      if (new_pos > current_pos)
//      os_ << String(new_pos - current_pos, '*');
//      current_pos = new_pos;
//    }
//    if (progress == 1.0)
//    {
//      os_ << "100%" << endl;
//    }
//    os_.flush();
//  }


  void ModelTest::SetModelParam(const std::map<String, DataType> & model_param_map)
  {
    String var;

    std::map<String, DataType>::const_iterator it_param_map = model_param_map.begin();
    while(it_param_map != model_param_map.end())
    {
      String var = it_param_map->first;
      if (sizeParamMap_.count(var))
        sizeParamMap_[var] = floor(it_param_map->second.ScalarView());
      else if (scalarParamMap_.count(var))
        scalarParamMap_[var] = it_param_map->second.ScalarView();
      else
        dataTypeParamMap_[var].Alloc(it_param_map->second);
      ++it_param_map;
    }
  }

  void ModelTest::SetDimensions(const std::map<String, DimArray::Ptr> & dim_map)
  {
    dimArrayMap_ = dim_map;
  }


  void ModelTest::setDataArrayMap(const std::map<String, std::vector<DataType> > & from, std::map<String, DataType::Array> & to)
  {
    std::map<String, std::vector<DataType> >::const_iterator it_from = from.begin();
    while(it_from != from.end())
    {
      to[it_from->first].SetPtr(it_from->second.begin(), it_from->second.end());
      ++it_from;
    }
  }


  void ModelTest::BuildDataGraph()
  {
    BuildModelGraph();
    pDataGraph_ = pModelGraph_;
    dataNodeIdMap_ = modelNodeIdMap_;
  }


  void ModelTest::SampleData(Size rngSeed)
  {
    Rng my_rng(rngSeed);

    NodeValues gen_node_values = pDataGraph_->SampleValues(&my_rng);

    for (Size i_var=0; i_var<inDataVarNames_.size(); ++i_var)
    {
      const String & var_name = inDataVarNames_[i_var];
      Size var_len = dataNodeIdMap_[var_name].size();
      dataValuesMap_[var_name].SetPtr(DataType::Array(var_len));
      DataType::Array & gen_values = dataValuesMap_[var_name];
      for (Size k=0; k<gen_values.size(); ++k)
        gen_values[k] = DataType(dimArrayMap_[var_name], gen_node_values[dataNodeIdMap_[var_name][k]]);
    }
    if (verbose_>=2)
    {
      os_ << "Generated values: " << std::endl;

      for (Size i_var=0; i_var<inDataVarNames_.size(); ++i_var)
      {
        const String & var_name = inDataVarNames_[i_var];
        printValues(os_, var_name, dataValuesMap_[var_name], dimArrayMap_.at(var_name)->Length());
      }
      os_ << std::endl;
    }
  }


//  void ModelTest::OutputData(std::ostream & os) const
//  {
//    for (Size i_var=0; i_var<inDataVarNames_.size(); ++i_var)
//    {
//      const String & var_name = inDataVarNames_[i_var];
//      const DataType::Array & gen_values = dataValuesMap_.at(var_name);
//      printValues(os, var_name, gen_values, dimArrayMap_.at(var_name)->Length());
//    }
//  }


//  void ModelTest::InputData(std::istream & is)
//  {
//    for (Size i_var=0; i_var<inDataVarNames_.size(); ++i_var)
//    {
//      const String & var_name = inDataVarNames_[i_var];
//      Size var_len = dataNodeIdMap_[var_name].size();
//      inputValues(genValuesMap_[var_name], var_len, dimArrayMap_[var_name], is);
//    }
//
//    if (verbose_)
//    {
//      os_ << "Input values: " << std::endl;
//
//      for (Size i_var=0; i_var<inDataVarNames_.size(); ++i_var)
//      {
//        const String & var_name = inDataVarNames_[i_var];
//        printValues(os_, var_name, genValuesMap_[var_name], dimArrayMap_[var_name]->Length());
//      }
//    }
//  }


  void ModelTest::RunBench()
  {
    os_ << "WARNING: no bench implemented." << std::endl;
  };


//  void ModelTest::OutputBench(std::ostream & os) const
//  {
//    for (Size i_var=0; i_var<inBenchFilterVarNames_.size(); ++i_var)
//    {
//      const String & var_name = inBenchFilterVarNames_[i_var];
//      printValues(os, var_name, benchFilterValuesMap_.at(var_name), dimArrayMap_.at(var_name)->Length());
//    }
//    for (Size i_var=0; i_var<inBenchSmoothVarNames_.size(); ++i_var)
//    {
//      const String & var_name = inBenchSmoothVarNames_[i_var];
//      printValues(os, var_name, benchSmoothValuesMap_.at(var_name), dimArrayMap_.at(var_name)->Length());
//    }
//  }


//  void ModelTest::InputBench(std::istream & is)
//  {
//    for (Size i_var=0; i_var<inBenchFilterVarNames_.size(); ++i_var)
//    {
//      const String & var_name = inBenchFilterVarNames_[i_var];
//      Size var_len = modelNodeIdMap_[var_name].size();
//      inputValues(benchFilterValuesMap_[var_name], var_len, dimArrayMap_[var_name], is);
//    }
//    for (Size i_var=0; i_var<inBenchSmoothVarNames_.size(); ++i_var)
//    {
//      const String & var_name = inBenchSmoothVarNames_[i_var];
//      Size var_len = modelNodeIdMap_[var_name].size();
//      inputValues(benchSmoothValuesMap_[var_name], var_len, dimArrayMap_[var_name], is);
//    }
//
//    if (verbose_)
//    {
//      if (! inBenchFilterVarNames_.empty() )
//        os_ << "Input Filtering benchmark values: " << std::endl;
//
//      for (Size i_var=0; i_var<inBenchFilterVarNames_.size(); ++i_var)
//      {
//        const String & var_name = inBenchFilterVarNames_[i_var];
//        printValues(os_, var_name, benchFilterValuesMap_[var_name], dimArrayMap_[var_name]->Length());
//      }
//
//      if (! inBenchSmoothVarNames_.empty() )
//        os_ << "Input Smoothing benchmark values: " << std::endl;
//
//      for (Size i_var=0; i_var<inBenchSmoothVarNames_.size(); ++i_var)
//      {
//        const String & var_name = inBenchSmoothVarNames_[i_var];
//        printValues(os_, var_name, benchSmoothValuesMap_[var_name], dimArrayMap_[var_name]->Length());
//      }
//    }
//  }


  void ModelTest::ClearSMC()
  {
    smcFilterValuesMap_.clear();
    smcSmoothValuesMap_.clear();
  }

  void ModelTest::RunSMC(Size nParticles, Size rngSeed, Bool prior, Scalar essThreshold, ResampleType rsType, Bool showProgress)
  {
    Rng my_rng(rngSeed);

    // sampler
    //--------
    pSampler_ = SMCSampler::Ptr(new SMCSampler(nParticles, pModelGraph_.get(), &my_rng));
    pSampler_->SetResampleParams(rsType, essThreshold);

    if (! prior)
    {
      for (Size i=0; i<nodeSamplerFactoryInvOrder_.size(); ++i)
        pSampler_->PushNodeSamplerFactory(nodeSamplerFactoryInvOrder_[i]);
    }

    pSampler_->Initialize();
    if (verbose_>=2)
    {
      os_ << "Node sampler's sequence: " << std::endl;
      pSampler_->PrintSamplersSequence(os_);
      os_ << std::endl;
    }

    initFilterAccumulators();

    Size current_pos = 0;
    if (verbose_ == 1 && showProgress)
      current_pos = progressBar(0.0, current_pos, os_, toString(pSampler_->NIterations()));
    else if (verbose_>=2)
      os_ << "SMC sampler's progress: " << std::endl;

    Size t_max = pSampler_->NIterations();

    // filtering
    for (Size t=0; t<t_max; ++t)
    {
      pSampler_->Iterate();
      if (verbose_ == 1 && showProgress)
        current_pos = progressBar(Scalar(t+1)/t_max, current_pos, os_);
      else if (verbose_>=2)
        pSampler_->PrintSamplerState();

      filterAccumulate(t);
    }

    if (verbose_>=2)
    {
      os_ << std::endl;
      os_ << "Particle Filter estimates: " << std::endl;
      for (Size i_var=0; i_var<printResultsVarNames_.size(); ++i_var)
      {
        const String & var_name = printResultsVarNames_[i_var];
        printValues(os_, var_name, smcFilterValuesMap_[var_name], dimArrayMap_[var_name]->Length());
      }
      os_ << std::endl;
    }

    scalarAcc_.ClearQuantileProbs();
    scalarAcc_.ClearFeatures();
    discreteScalarAcc_.ClearFeatures();

    initSmoothAccumulators();

    // smoothing
    for (Size t=0; t<t_max; ++t)
    {
      smoothAccumulate(t);
    }

    if (verbose_>=2)
    {
      os_ << "Particle Smoother estimates: " << std::endl;
      for (Size i_var=0; i_var<printResultsVarNames_.size(); ++i_var)
      {
        const String & var_name = printResultsVarNames_[i_var];
        printValues(os_, var_name, smcSmoothValuesMap_[var_name], dimArrayMap_[var_name]->Length());
      }
      os_ << std::endl;
    }

  }


  Bool ModelTest::error(Scalar & error, const Types<String>::Array & varNames,
      const std::map<String, DataType::Array> & smcValuesMap,
      const std::map<String, DataType::Array> & benchValuesMap) const
  {
    static std::binder2nd<PowScalar> squared_scalar = std::bind2nd(PowScalar(), 2.0);

    Bool valid = false;
    error = 0.0;

    for (Size i_var=0; i_var<varNames.size(); ++i_var)
    {
      const String & var_name = varNames[i_var];

      if (smcValuesMap.count(var_name) && benchValuesMap.count(var_name) && benchValuesMap.count("var."+var_name))
      {
        const DataType::Array & smc_values = smcValuesMap.find(var_name)->second;
        const DataType::Array & bench_values = benchValuesMap.find(var_name)->second;
        const DataType::Array & bench_var_values = benchValuesMap.find("var."+var_name)->second;

        for (Size k=0; k<smc_values.size(); ++k)
        {
          Size dim = smc_values[k].Length(); // TODO check dim
          Vector diff_vec(dim, smc_values[k].Value() - bench_values[k].Value());
          Matrix var_chol(dim, dim, bench_var_values[k].Value());
          ublas::cholesky_factorize(var_chol);
          ublas::inplace_solve(var_chol, diff_vec, ublas::lower_tag());
          error += ublas::inner_prod(diff_vec, diff_vec);
        }
        valid = true;
      }
      else
      {
        valid = false;
        break;
      }
    }

    return valid;
  }


}
