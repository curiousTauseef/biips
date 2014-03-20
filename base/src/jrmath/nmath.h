/*
 *  Mathlib : A C Library of Special Functions
 *  Copyright (C) 1998-2004  The R Development Core Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, a copy is available at
 *  http://www.r-project.org/Licenses/
 */

/* Private header file for use during compilation of Mathlib */

#ifndef BIIPS_MATHLIB_PRIVATE_H
#define BIIPS_MATHLIB_PRIVATE_H

	/* Gamma and Related Functions */

double  stirlerr(double);  /* Stirling expansion "error" */

double  bd0(double, double);

double  dbinom_raw(double, double, double, double);

#endif /* BIIPS_MATHLIB_PRIVATE_H */
