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
 * $Revision: 1.3 $
 ***********************************************************************EHEADER*/



/* ******************************************************************** */
/* See the file COPYRIGHT for a complete copyright notice, contact      */
/* person and disclaimer.                                               */        
/* ******************************************************************** */

/* ******************************************************************** */
/* data structure to hold grid information in the form of a graph       */
/* ******************************************************************** */
/* Author        : Charles Tong (LLNL)                                  */
/* Date          : September, 1998                                      */
/* ******************************************************************** */

#ifndef __MLGRIDG__
#define __MLGRIDG__

#include <stdio.h>
/* #include <stdlib.h> */

#include "ml_common.h"
#include "ml_defs.h"
#include "ml_comm.h"
#include "ml_gridfunc.h"
#include "ml_memory.h"
#include "ml_comminfoop.h"
#include "ml_operator.h"
#include "ml_mat_formats.h"

/* ******************************************************************** */
/* definition of the grid graph structure                               */
/* -------------------------------------------------------------------- */

typedef struct ML_GGraph_Struct
{
   int  ML_id;
   int  Npoints, Nselected;
   int  ML_rank;
   int  *row_ptr, *col_ptr;
   int  send_cnt, *send_leng, *send_proc, **send_list;
   int  recv_cnt, *recv_leng, *recv_proc, **recv_list;
   char *bdry_type;
   char *vertex_state;

} ML_GGraph;

/* ******************************************************************** */
/* functions to manipulate the grid graph data structure                */
/* -------------------------------------------------------------------- */

#ifndef ML_CPP
#ifdef __cplusplus
extern "C" 
{
#endif
#endif

extern int  ML_GGraph_Create( ML_GGraph ** );
extern int  ML_GGraph_Destroy( ML_GGraph ** );
extern int  ML_GGraph_Print( ML_GGraph * );
extern int  ML_GGraph_Load_BdryTypes( ML_GGraph *, int , char *);
extern int  ML_GGraph_Coarsen(ML_GGraph*, ML_Comm *);
extern int  ML_GGraph_Gen_NodeGraph(ML_GGraph*,void*,void (*func),ML_Comm *);
extern int  ML_GGraph_Get_NodeStates(ML_GGraph*, int *, char **);
extern int  ML_GGraph_Gen_ElementGraph(ML_GGraph*,void*,void (*gf),ML_Comm*);
extern int  ML_GGraph_Gen_Restrictor(ML_GGraph*);
extern int ML_GGraph_CheckMIS( ML_GGraph *ml_gg, ML_Comm *comm );
extern int ML_GGraph_Find_NeighborElements(int leng1, int *list1, int leng2,
					   int *list2, int *vlist3);
extern int ML_GGraph_LabelVertices(int, int *, int, char *, char *, int,
                     int *, int *, int, int **, int, int **, int *, int *,
                     int, int **, int *, int *, int **, int, ML_Comm *);

#ifndef ML_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif
