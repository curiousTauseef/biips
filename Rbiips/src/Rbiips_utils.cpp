//                                               -*- C++ -*-
/*
 * Rbiips package for GNU R is an interface to Biips C++ libraries for
 * Bayesian inference with interacting Particle Systems.
 * Copyright (C) Inria, 2012
 * Authors: Adrien Todeschini, Francois Caron
 *
 * Rbiips is derived software based on:
 * Biips, Copyright (C) Inria, 2012
 * rjags, Copyright (C) Martyn Plummer, 2002-2010
 * Rcpp, Copyright (C) Dirk Eddelbuettel and Romain Francois, 2009-2011
 *
 * This file is part of Rbiips.
 *
 * Rbiips is free software: you can redistribute it and/or modify
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

/*! \file Rbiips_utils.cpp
 * \brief 
 * 
 * \author  $LastChangedBy$
 * \date    $LastChangedDate$
 * \version $LastChangedRevision$
 * Id:      $Id$
 */

#include "Rbiips_utils.h"
#include <compiler/Compiler.hpp>
#include <BiipsBase.hpp>


Size VERBOSITY = 1;


void load_base_module()
{
  using namespace Biips;
  BEGIN_RBIIPS
  loadBaseModule(Compiler::FuncTab(), Compiler::DistTab());
  VOID_END_RBIIPS
}


template<>
std::map<String, MultiArray> writeDataTable<ColumnMajorOrder>(SEXP data)
{
  std::map<String, MultiArray> data_map;

  if (VERBOSITY>1)
    rbiips_cout << PROMPT_STRING << "Writing data table" << endl;

  Rcpp::List data_list(data);
  if (!data_list.hasAttribute("names"))
  {
    rbiips_cerr << "Warning: Missing variable names" << endl;
    return data_map;
  }

  if (VERBOSITY>1)
    rbiips_cout << INDENT_STRING << "Variables:";

  Rcpp::CharacterVector names = data_list.attr("names");
  for (int i=0; i<names.size(); ++i)
  {
    String var_name(names[i]);
    if (VERBOSITY>1)
      rbiips_cout << " " << var_name;

    Rcpp::NumericVector r_vec = data_list[var_name];
    MultiArray marray;

    if (!r_vec.hasAttribute("dim"))
    {
      DimArray::Ptr p_dim(new DimArray(1, r_vec.size()));
      ValArray::Ptr p_val(new ValArray(r_vec.size()));
      std::replace_copy(r_vec.begin(), r_vec.end(), p_val->begin(), NA_REAL, BIIPS_REALNA);
      marray.SetPtr(p_dim, p_val);
    }
    else
    {
      Rcpp::IntegerVector r_dim = r_vec.attr("dim");
      DimArray::Ptr p_dim(new DimArray(r_dim.begin(), r_dim.end()));
      ValArray::Ptr p_val(new ValArray(r_vec.size()));
      std::replace_copy(r_vec.begin(), r_vec.end(), p_val->begin(), NA_REAL, BIIPS_REALNA);
      marray.SetPtr(p_dim, p_val);
    }

    data_map[var_name] = marray;
  }
  if (VERBOSITY>1)
    rbiips_cout << endl;

  return data_map;
}


template<>
SEXP readDataTable<ColumnMajorOrder>(const std::map<String, MultiArray> & dataMap)
{
  if (VERBOSITY>1)
    rbiips_cout << PROMPT_STRING << "Reading data table" << endl;

  Rcpp::List data_list;

  if (VERBOSITY>1)
    rbiips_cout << INDENT_STRING << "Variables:";

  Rcpp::CharacterVector names;
  std::map<String, MultiArray>::const_iterator it_table = dataMap.begin();
  for (; it_table!=dataMap.end(); ++it_table)
  {
    const String & var_name = it_table->first;
    const MultiArray & values_array = it_table->second;

    // dim
    Rcpp::IntegerVector dim(values_array.Dim().begin(), values_array.Dim().end());

    Size len = values_array.Dim().Length();
    Rcpp::NumericVector values(len);

    std::replace_copy(values_array.Values().begin(), values_array.Values().end(), values.begin(), BIIPS_REALNA, NA_REAL);

    values.attr("dim") = dim;

    data_list[var_name] = values;

    if (VERBOSITY>1)
      rbiips_cout << " " << var_name;
  }
  if (VERBOSITY>1)
    rbiips_cout << endl;

  return data_list;
}


IndexRange makeRange(const Rcpp::RObject & lower,
                            const Rcpp::RObject & upper)
{
  if (lower.isNULL() || upper.isNULL())
    return IndexRange();

  Rcpp::IntegerVector il(lower);
  Rcpp::IntegerVector iu(upper);
  if (il.size() != iu.size())
    throw LogicError("length mismatch between lower and upper limits");

  IndexRange::Indices lind(il.begin(), il.end());
  IndexRange::Indices uind(iu.begin(), iu.end());

  IndexRange r = IndexRange(lind, uind);
  return r;
}


template<>
SEXP getMonitors<ColumnMajorOrder>(const std::map<String, NodeArrayMonitor> & monitorsMap, const String & type)
{
  Rcpp::List smcarray_list;

  std::map<String, NodeArrayMonitor>::const_iterator it_map;
  for (it_map = monitorsMap.begin(); it_map != monitorsMap.end(); ++it_map)
  {
    const String & name = it_map->first;
    const NodeArrayMonitor & monitor = it_map->second;

    // dim
    Rcpp::IntegerVector dim_particles(monitor.GetValues().Dim().begin(), monitor.GetValues().Dim().end());
    Rcpp::IntegerVector dim_array(monitor.GetRange().Dim().begin(), monitor.GetRange().Dim().end());

    // names(dim)
    Rcpp::CharacterVector dim_names(dim_particles.size(), "");
    dim_names[dim_names.size()-1] = "particle";

    dim_particles.attr("names") = dim_names;

    Size len_part = monitor.GetValues().Dim().Length();
    Rcpp::NumericVector values(len_part);
    const ValArray & values_val = monitor.GetValues().Values();
    std::replace_copy(values_val.begin(), values_val.end(), values.begin(), BIIPS_REALNA, NA_REAL);
    values.attr("dim") = dim_particles;

    const ValArray & weight_val = monitor.GetWeights().Values();
    Rcpp::NumericVector weights(weight_val.begin(), weight_val.end());
    weights.attr("dim") = dim_particles;

    const ValArray & ess_val(monitor.GetESS().Values());
    Rcpp::NumericVector ess(ess_val.begin(), ess_val.end());
    ess.attr("dim") = dim_array;

    const ValArray & discrete_val(monitor.GetDiscrete().Values());
    Rcpp::LogicalVector discrete(discrete_val.begin(), discrete_val.end());
    discrete.attr("dim") = dim_array;

    const ValArray & iter_val(monitor.GetIterations().Values()+1);
    Rcpp::NumericVector iterations(iter_val.begin(), iter_val.end());
    iterations.attr("dim") = dim_array;

    const Types<Types<String>::Array>::Array & cond = monitor.GetConditionalNodeNames();
    Size len = monitor.GetRange().Length();
    Rcpp::List cond_list(len);
    Rcpp::CharacterVector cond_vec;
    if (cond.size() == len) {
      for (Size i=0; i < len; ++i)
      {
        cond_list[i] = Rcpp::CharacterVector(cond[i].begin(), cond[i].end());
      }
      cond_list.attr("dim") = dim_array;
    }
    else if (cond.size() == 1) {
      cond_vec.assign(cond[0].begin(), cond[0].end());
    }
    else {
      throw LogicError("conditionals must either be of the same size as the node array or of size 1.");
    }


    const IndexRange::Indices & lower_ind = monitor.GetRange().Lower();
    Rcpp::IntegerVector lower(lower_ind.begin(), lower_ind.end());

    const IndexRange::Indices & upper_ind = monitor.GetRange().Upper();
    Rcpp::IntegerVector upper(upper_ind.begin(), upper_ind.end());

    Rcpp::List smcarray;
    smcarray["values"] = values;
    smcarray["weights"] = weights;
    smcarray["ess"] = ess;
    smcarray["discrete"] = discrete;
    smcarray["iterations"] = iterations;
    if (cond.size() == len)
      smcarray["conditionals"] = cond_list;
    else
      smcarray["conditionals"] = cond_vec;
    smcarray["name"] = Rcpp::wrap(monitor.GetName());
    smcarray["lower"] = lower;
    smcarray["upper"] = upper;
    smcarray["type"] = Rcpp::wrap(type);

    smcarray.attr("class") = "smcarray";

    smcarray_list[name] = smcarray;
  }

  return smcarray_list;
}


Rcpp::NumericVector convArrayVector(const Biips::NumArray & array ) {
  const Biips::ValArray & values = array.Values();
  const Biips::DimArray & dims = array.Dim();
  const int ndim = dims.size();
  Rcpp::Dimension * pdim;
  switch (ndim) {
    case 1: pdim = new Rcpp::Dimension(dims[0]); break;
    case 2: pdim = new Rcpp::Dimension(dims[0], dims[1]); break;
    case 3: pdim = new Rcpp::Dimension(dims[0], dims[1], dims[2]); break;
    default : throw Biips::RuntimeError("Array limited to 3 dims max in RDistribution"); break;
  }
  Rcpp::NumericVector vec(*pdim);
  vec.assign(values.begin(), values.end());
  delete pdim;
  return vec;
}
