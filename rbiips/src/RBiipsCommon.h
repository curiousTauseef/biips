//                                               -*- C++ -*-
/*
 * RBiips package for GNU R is an interface to BiiPS C++ libraries for
 * Bayesian inference with interacting Particle Systems.
 * Copyright (C) Inria, 2012
 * Authors: Adrien Todeschini, Francois Caron
 *
 * RBiips is derived software based on:
 * BiiPS, Copyright (C) Inria, 2012
 * rjags, Copyright (C) Martyn Plummer, 2002-2010
 * Rcpp, Copyright (C) Dirk Eddelbuettel and Romain Francois, 2009-2011
 *
 * This file is part of RBiips.
 *
 * RBiips is free software: you can redistribute it and/or modify
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
/*! \file RBiipsCommon.h
 * \brief 
 * 
 * \author  $LastChangedBy$
 * \date    $LastChangedDate$
 * \version $LastChangedRevision$
 * Id:      $Id$
 */

#ifndef RBIIPSCOMMON_H_
#define RBIIPSCOMMON_H_

#include <Rcpp.h>
#include <vector>
#include "Rostream.h"
#include  "common/Error.hpp"

#ifndef BEGIN_RBIIPS
#define BEGIN_RBIIPS BEGIN_RCPP
#endif

#ifndef CATCH_RBIIPS
#define CATCH_RBIIPS                                                    \
  }                                                                     \
  catch (Biips::RuntimeError & except)                                  \
  {                                                                     \
    ::Rf_error(except.what());                                          \
  }                                                                     \
  catch (Biips::LogicError & except)                                    \
  {                                                                     \
    ::Rf_error(except.what());
#endif

#ifndef VOID_END_RBIIPS
#define VOID_END_RBIIPS CATCH_RBIIPS VOID_END_RCPP
#endif

#ifndef END_RBIIPS
#define END_RBIIPS CATCH_RBIIPS END_RCPP
#endif

void load_base_module();


template <class InType> 
InType apply(const std::vector<InType> & invec, const Rcpp::Function & fun, int nrhs) {
     
      InType outvec;
      switch(nrhs) {

       case 1: outvec = fun(invec[0]); 
           break;
       case 2: outvec = fun(invec[0], invec[1]);
           break;
       case 3: outvec = fun(invec[0], invec[1], invec[2]);
           break;                        
       case 4: outvec = fun(invec[0], invec[1], invec[2], invec[3]);
           break;                        
       case 5: outvec = fun(invec[0], invec[1], invec[2], invec[3], invec[4]);
           break;                              
       default: throw Biips::LogicError("Too much arguments in RFunction must be <= 5");
                break;
      }
    return outvec;
}





#endif /* RBIIPSCOMMON_H_ */
