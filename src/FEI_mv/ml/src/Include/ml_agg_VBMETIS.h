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



/********************************************************************* */
/* See the file COPYRIGHT for a complete copyright notice, contact      */
/* person and disclaimer.                                               */   
/* ******************************************************************** */

/********************************************************************* */
/*          Decomposition with METIS                                  */
/********************************************************************* */

#ifndef __MLAGGVBMETIS__
#define __MLAGGVBMETIS__


/* ------------------------------------------------------------------------ */
/*!
 \brief structure for  variable block METIS 
 <pre>
    used to implement support for variable block sizes  
    used in ML_Aggregate_CoarsenVBMETIS.
    It holds user supplied block data and space to temporarily store values
    which otherwise would be lost when applying 
    ML_Operator_AmalgamateAndDropWeak_VBlocks                           
    data is set applying ML_Aggregate_Set_Vblocks_CoarsenScheme_VBMETIS 
    and                  ML_Aggregate_Set_CoarsenScheme_VBMETIS         
    data is destroyed using ML_Aggregate_Destroy_Vblocks_CoarsenScheme_VBMETIS
    data can be handled using ML_Aggregate_Get_Vblocks_CoarsenScheme_VBMETIS
    
    Author:  Michael W. Gee, SNL Org. 9214, November 2004
     
 </pre>
 
 \sa ML_Aggregate_CoarsenVBMETIS ML_Operator_AmalgamateAndDropWeak_VBlocks
     ML_Aggregate_Set_Vblocks_CoarsenScheme_VBMETIS
     ML_Aggregate_Set_CoarsenScheme_VBMETIS
     ML_Aggregate_Destroy_Vblocks_CoarsenScheme_VBMETIS
     ML_Aggregate_Get_Vblocks_CoarsenScheme_VBMETIS
*/
/* ------------------------------------------------------------------------ */
struct aggr_vblock {
   int                  nblocks;            /*!< number of variable blocks */
   int                  block_dim;          /*!< dimension of blocks & block_pde */
   int                 *blocks;             /*!< variable block indizes    */
   int                 *block_pde;          /*!< which pde for each row*/
   int                  old_invec_leng;     /*!< original invec_leng       */
   int                  old_outvec_leng;    /*!< original outvec_leng      */
};

/*MS*/
#define ML_AGGREGATE_OPTIONS_ID 13579

/* undefined will be a negative number */
#define ML_NUM_LOCAL_AGGREGATES     0
#define ML_NUM_GLOBAL_AGGREGATES    1
#define ML_NUM_NODES_PER_AGGREGATE  2


#ifndef ML_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

int ML_Aggregate_Set_CoarsenScheme_VBMETIS( ML_Aggregate *ag  );
int ML_Aggregate_Set_Vblocks_CoarsenScheme_VBMETIS( ML_Aggregate *ag, 
                                                    const int level,
                                                    const int N_levels, 
                                                    const int nblocks, 
                                                    const int *blocks, 
                                                    const int *block_pde,
                                                    const int block_dim);
int ML_Aggregate_Get_Vblocks_CoarsenScheme_VBMETIS( const ML_Aggregate *ag, 
                                                    const int level,
                                                    const int N_levels, 
                                                          int *nblocks, 
                                                          int **blocks, 
                                                          int **block_pde);
int ML_Aggregate_Destroy_Vblocks_CoarsenScheme_VBMETIS( const ML_Aggregate *ag, 
                                                        const int level);
int ML_Aggregate_CoarsenVBMETIS( ML_Aggregate *ml_ag, ML_Operator *Amatrix, 
			         ML_Operator **Pmatrix, ML_Comm *comm);
int ML_Operator_AmalgamateAndDropWeak_VBlocks(ML_Operator *Amat, int block_size, 
                                double drop_tolerance, int nblocks, int* blocks);
int ML_Operator_UnAmalgamateAndDropWeak_Vblocks(ML_Operator *Amat, int block_size,
	double drop_tolerance);
int ML_amalg_drop_getrow_VBlocks(ML_Operator *data, int N_requested_rows, 
   int requested_rows[],int allocated_space, int columns[], double values[], 
   int row_lengths[]);
                                 
                                 
  

#ifndef ML_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif /* #ifndef __MLAGGVBMETIS__ */