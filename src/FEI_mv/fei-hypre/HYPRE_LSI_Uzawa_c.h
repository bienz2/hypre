/*BHEADER**********************************************************************
 * Copyright (c) 2006   The Regents of the University of California.
 * Produced at the Lawrence Livermore National Laboratory.
 * Written by the HYPRE team. UCRL-CODE-222953.
 * All rights reserved.
 *
 * This file is part of HYPRE (see http://www.llnl.gov/CASC/hypre/).
 * Please see the COPYRIGHT_and_LICENSE file for the copyright notice, 
 * disclaimer, contact information and the GNU Lesser General Public License.
 *
 * HYPRE is free software; you can redistribute it and/or modify it under the 
 * terms of the GNU General Public License (as published by the Free Software
 * Foundation) version 2.1 dated February 1999.
 *
 * HYPRE is distributed in the hope that it will be useful, but WITHOUT ANY 
 * WARRANTY; without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS 
 * FOR A PARTICULAR PURPOSE.  See the terms and conditions of the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * $Revision: 2.3 $
 ***********************************************************************EHEADER*/



/******************************************************************************
 *
 * HYPRE_LSI_Uzawa interface
 *
 *****************************************************************************/

#ifndef __HYPRE_UZAWA__
#define __HYPRE_UZAWA__

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <math.h>

#include "HYPRE.h"
#include "utilities/utilities.h"
#include "parcsr_ls/parcsr_ls.h"
#include "parcsr_mv/parcsr_mv.h"
#include "HYPRE_LSI_UZAWA.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern int HYPRE_LSI_UzawaCreate(MPI_Comm comm, HYPRE_Solver *solver);
extern int HYPRE_LSI_UzawaDestroy(HYPRE_Solver solver);
extern int HYPRE_LSI_UzawaSetMaxIterations(HYPRE_Solver solver, int iter);
extern int HYPRE_LSI_UzawaSetTolerance(HYPRE_Solver solver, double tol);
extern int HYPRE_LSI_UzawaSetParams(HYPRE_Solver solver, char *params);
extern int HYPRE_LSI_UzawaGetNumIterations(HYPRE_Solver solver, int *iter);
extern int HYPRE_LSI_UzawaSetup(HYPRE_Solver solver,HYPRE_ParCSRMatrix A,
                                HYPRE_ParVector b,HYPRE_ParVector x);
extern int HYPRE_LSI_UzawaSolve(HYPRE_Solver solver,HYPRE_ParCSRMatrix A,
                                HYPRE_ParVector b, HYPRE_ParVector x);

#ifdef __cplusplus
}
#endif

#endif
