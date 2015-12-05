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
 * $Revision: 1.10 $
 ***********************************************************************EHEADER*/



/******************************************************************************
 *
 * Member functions for hypre_CSRBlockMatrix class.
 *
 *****************************************************************************/

#include "headers.h"

#define LB_VERSION 0 /* lapack and blas version 5/12/06 - preliminary testing shows this
                      is slower for block sizes < 4 (did not try larger block sizes)-
                      need to edit the Makefile to find blas and lapack */



#if LB_VERSION
#include "hypre_blas.h"
#include "hypre_lapack.h"
#endif


/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixCreate
 *--------------------------------------------------------------------------*/

hypre_CSRBlockMatrix *
hypre_CSRBlockMatrixCreate(int block_size, int num_rows, int num_cols,
			   int num_nonzeros)
{
   hypre_CSRBlockMatrix  *matrix;

   matrix = hypre_CTAlloc(hypre_CSRBlockMatrix, 1);

   hypre_CSRBlockMatrixData(matrix) = NULL;
   hypre_CSRBlockMatrixI(matrix)    = NULL;
   hypre_CSRBlockMatrixJ(matrix)    = NULL;
   hypre_CSRBlockMatrixBlockSize(matrix) = block_size;
   hypre_CSRBlockMatrixNumRows(matrix) = num_rows;
   hypre_CSRBlockMatrixNumCols(matrix) = num_cols;
   hypre_CSRBlockMatrixNumNonzeros(matrix) = num_nonzeros;

   /* set defaults */
   hypre_CSRBlockMatrixOwnsData(matrix) = 1;

   return matrix;
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixDestroy
 *--------------------------------------------------------------------------*/

int 
hypre_CSRBlockMatrixDestroy(hypre_CSRBlockMatrix *matrix)
{
   int  ierr=0;

   if (matrix)
   {
      hypre_TFree(hypre_CSRBlockMatrixI(matrix));
      if ( hypre_CSRBlockMatrixOwnsData(matrix) )
      {
         hypre_TFree(hypre_CSRBlockMatrixData(matrix));
         hypre_TFree(hypre_CSRBlockMatrixJ(matrix));
      }
      hypre_TFree(matrix);
   }

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixInitialize
 *--------------------------------------------------------------------------*/

int 
hypre_CSRBlockMatrixInitialize(hypre_CSRBlockMatrix *matrix)
{
   int block_size   = hypre_CSRBlockMatrixBlockSize(matrix);
   int num_rows     = hypre_CSRBlockMatrixNumRows(matrix);
   int num_nonzeros = hypre_CSRBlockMatrixNumNonzeros(matrix);
   int ierr=0, nnz;

   if ( ! hypre_CSRBlockMatrixI(matrix) )
      hypre_TFree(hypre_CSRBlockMatrixI(matrix));
   if ( ! hypre_CSRBlockMatrixJ(matrix) )
      hypre_TFree(hypre_CSRBlockMatrixJ(matrix));
   if ( ! hypre_CSRBlockMatrixData(matrix) )
      hypre_TFree(hypre_CSRBlockMatrixData(matrix));

   nnz = num_nonzeros * block_size * block_size;
   hypre_CSRBlockMatrixI(matrix) = hypre_CTAlloc(int, num_rows + 1);
   if (nnz) hypre_CSRBlockMatrixData(matrix) = hypre_CTAlloc(double, nnz);
   else     hypre_CSRBlockMatrixData(matrix) = NULL;
   if (nnz) hypre_CSRBlockMatrixJ(matrix) = hypre_CTAlloc(int,num_nonzeros);
   else     hypre_CSRBlockMatrixJ(matrix) = NULL;

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixSetDataOwner
 *--------------------------------------------------------------------------*/

int 
hypre_CSRBlockMatrixSetDataOwner(hypre_CSRBlockMatrix *matrix, int owns_data)
{
   int    ierr=0;

   hypre_CSRBlockMatrixOwnsData(matrix) = owns_data;

   return ierr;
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixCompress
 *--------------------------------------------------------------------------*/

hypre_CSRMatrix *
hypre_CSRBlockMatrixCompress(hypre_CSRBlockMatrix *matrix)
{
   int    block_size = hypre_CSRBlockMatrixBlockSize(matrix);
   int    num_rows = hypre_CSRBlockMatrixNumRows(matrix);
   int    num_cols = hypre_CSRBlockMatrixNumCols(matrix);
   int    num_nonzeros = hypre_CSRBlockMatrixNumNonzeros(matrix);
   int    *matrix_i = hypre_CSRBlockMatrixI(matrix);
   int    *matrix_j = hypre_CSRBlockMatrixJ(matrix);
   double *matrix_data = hypre_CSRBlockMatrixData(matrix);
   hypre_CSRMatrix* matrix_C;
   int    *matrix_C_i, *matrix_C_j, i, j, bnnz;
   double *matrix_C_data, ddata;

   matrix_C = hypre_CSRMatrixCreate(num_rows,num_cols,num_nonzeros);
   hypre_CSRMatrixInitialize(matrix_C);
   matrix_C_i = hypre_CSRMatrixI(matrix_C);
   matrix_C_j = hypre_CSRMatrixJ(matrix_C);
   matrix_C_data = hypre_CSRMatrixData(matrix_C);

   bnnz = block_size * block_size;
   for(i = 0; i < num_rows + 1; i++) matrix_C_i[i] = matrix_i[i];
   for(i = 0; i < num_nonzeros; i++)
   {
      matrix_C_j[i] = matrix_j[i];
      ddata = 0.0;
      for(j = 0; j < bnnz; j++)
         ddata += matrix_data[i*bnnz+j] * matrix_data[i*bnnz+j];
      matrix_C_data[i] = sqrt(ddata);
   }
   return matrix_C;
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixConvertToCSRMatrix
 *--------------------------------------------------------------------------*/

hypre_CSRMatrix *
hypre_CSRBlockMatrixConvertToCSRMatrix( hypre_CSRBlockMatrix *matrix )
{
   int block_size = hypre_CSRBlockMatrixBlockSize(matrix);
   int num_rows = hypre_CSRBlockMatrixNumRows(matrix);
   int num_cols = hypre_CSRBlockMatrixNumCols(matrix);
   int num_nonzeros = hypre_CSRBlockMatrixNumNonzeros(matrix);
   int *matrix_i = hypre_CSRBlockMatrixI(matrix);
   int *matrix_j = hypre_CSRBlockMatrixJ(matrix);
   double* matrix_data = hypre_CSRBlockMatrixData(matrix);

   hypre_CSRMatrix* matrix_C;
   int    i, j, k, ii, C_ii, bnnz, new_nrows, new_ncols, new_num_nonzeros;
   int    *matrix_C_i, *matrix_C_j;
   double *matrix_C_data;

   bnnz      = block_size * block_size;
   new_nrows = num_rows * block_size;
   new_ncols = num_cols * block_size;
   new_num_nonzeros = block_size * block_size * num_nonzeros;
   matrix_C = hypre_CSRMatrixCreate(new_nrows,new_ncols,new_num_nonzeros);
   hypre_CSRMatrixInitialize(matrix_C);
   matrix_C_i    = hypre_CSRMatrixI(matrix_C);
   matrix_C_j    = hypre_CSRMatrixJ(matrix_C);
   matrix_C_data = hypre_CSRMatrixData(matrix_C);
   for(i = 0; i < num_rows; i++)
   {
      for(j = 0; j < block_size; j++)
         matrix_C_i[i*block_size + j] = matrix_i[i]*bnnz + 
                               j * (matrix_i[i + 1] - matrix_i[i])*block_size;
   }
   matrix_C_i[new_nrows] = matrix_i[num_rows] * bnnz;

   C_ii = 0;
   for(i = 0; i < num_rows; i++)
   {
      for(j = 0; j < block_size; j++)
      {
         for(ii = matrix_i[i]; ii < matrix_i[i + 1]; ii++)
         {
	    k = j;
	    matrix_C_j[C_ii] = matrix_j[ii]*block_size + k;
	    matrix_C_data[C_ii] = matrix_data[ii*bnnz+j*block_size+k];
	    C_ii++;
	    for(k = 0; k < block_size; k++)
            {
	       if(j != k)
               {
	          matrix_C_j[C_ii] = matrix_j[ii]*block_size + k;
	          matrix_C_data[C_ii] = matrix_data[ii*bnnz+j*block_size+k];
	          C_ii++;
	       }
	    }
         }
      }
   }
   return matrix_C;
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixConvertFromCSRMatrix

 * this doesn't properly convert the parcsr off_diag matrices - AB 12/7/05
   (because here we assume the matrix is square - we don't check what the
    number of columns should be ) - it can only be used for the diag part
 *--------------------------------------------------------------------------*/

hypre_CSRBlockMatrix *
hypre_CSRBlockMatrixConvertFromCSRMatrix(hypre_CSRMatrix *matrix, 
                                         int matrix_C_block_size )
{
   int num_rows = hypre_CSRMatrixNumRows(matrix);
   int num_cols = hypre_CSRMatrixNumCols(matrix);
   int *matrix_i = hypre_CSRMatrixI(matrix);
   int *matrix_j = hypre_CSRMatrixJ(matrix);
   double* matrix_data = hypre_CSRMatrixData(matrix);

   hypre_CSRBlockMatrix* matrix_C;
   int    *matrix_C_i, *matrix_C_j;
   double *matrix_C_data;
   int    matrix_C_num_rows, matrix_C_num_cols, matrix_C_num_nonzeros;
   int    i, j, ii, jj, s_jj, index, *counter;

   matrix_C_num_rows = num_rows/matrix_C_block_size;
   matrix_C_num_cols = num_cols/matrix_C_block_size;

   counter = hypre_CTAlloc(int, matrix_C_num_cols);
   for(i = 0; i < matrix_C_num_cols; i++) counter[i] = -1;
   matrix_C_num_nonzeros = 0;
   for(i = 0; i < matrix_C_num_rows; i++)
   {
      for(j = 0; j < matrix_C_block_size; j++)
      {
         for(ii = matrix_i[i*matrix_C_block_size+j]; 
             ii < matrix_i[i*matrix_C_block_size+j+1]; ii++)
         {
	    if(counter[matrix_j[ii]/matrix_C_block_size] < i)
            {
	       counter[matrix_j[ii]/matrix_C_block_size] = i;
	       matrix_C_num_nonzeros++;
	    }
         }
      }
   }
   matrix_C = hypre_CSRBlockMatrixCreate(matrix_C_block_size, matrix_C_num_rows, 
                                       matrix_C_num_cols, matrix_C_num_nonzeros);
   hypre_CSRBlockMatrixInitialize(matrix_C);
   matrix_C_i = hypre_CSRBlockMatrixI(matrix_C);
   matrix_C_j = hypre_CSRBlockMatrixJ(matrix_C);
   matrix_C_data = hypre_CSRBlockMatrixData(matrix_C);
 
   for(i = 0; i < matrix_C_num_cols; i++) counter[i] = -1;
   jj = s_jj = 0;
   for (i = 0; i < matrix_C_num_rows; i++)
   {
      matrix_C_i[i] = jj;
      for(j = 0; j < matrix_C_block_size; j++)
      {
         for(ii = matrix_i[i*matrix_C_block_size+j];
             ii < matrix_i[i*matrix_C_block_size+j+1]; ii++)
         {
 	    if(counter[matrix_j[ii]/matrix_C_block_size] < s_jj)
            {
 	       counter[matrix_j[ii]/matrix_C_block_size] = jj;
 	       matrix_C_j[jj] = matrix_j[ii]/matrix_C_block_size;
 	       jj++;
 	    }
 	    index = counter[matrix_j[ii]/matrix_C_block_size] * matrix_C_block_size *
                    matrix_C_block_size + j * matrix_C_block_size + 
                    matrix_j[ii]%matrix_C_block_size;
 	    matrix_C_data[index] = matrix_data[ii];
         }
      }
      s_jj = jj;
   }
   matrix_C_i[matrix_C_num_rows] = matrix_C_num_nonzeros;

   hypre_TFree(counter);
   

   return matrix_C;
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockAdd
 * (o = i1 + i2) 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockAdd(double* i1, double* i2, double* o, int block_size)
{


   int i;
   int sz = block_size*block_size;
   
   for (i = 0; i < sz; i++)
      o[i] = i1[i] + i2[i];

   return 0;
}


/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockAddAccumulate
 * (o = i1 + o) 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockAddAccumulate(double* i1, double* o, int block_size)
{


   int i;
   int sz = block_size*block_size;
   
   for (i = 0; i < sz; i++)
      o[i] += i1[i];

   return 0;
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockAddAccumulateDiag
 * (diag(o) = diag(i1) + diag(o)) 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockAddAccumulateDiag(double* i1, double* o, int block_size)
{
   int i;

   for (i = 0; i < block_size; i++)
         o[i*block_size+i] += i1[i*block_size+i];
   return 0;
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockSetScalar
 * (each entry in block o is set to beta ) 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockSetScalar(double* o, double beta, int block_size)
{


   int i;
   int sz = block_size*block_size;
   
   for (i = 0; i < sz; i++)
      o[i] = beta;

   return 0;
}


/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockCopyData
 * (o = beta*i1 ) 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockCopyData(double* i1, double* o, double beta, int block_size)
{
   
   int i;
   int sz = block_size*block_size;
   
   for (i = 0; i < sz; i++)
      o[i] = beta*i1[i];
   
   return 0;
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockCopyDataDiag - zeros off-diag entries
 * (o = beta*diag(i1)) 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockCopyDataDiag(double* i1, double* o, double beta, int block_size)
{
    int i;

    int sz = block_size*block_size;
   
    for (i = 0; i < sz; i++)
       o[i] = 0.0;

    for (i = 0; i < block_size; i++)
       o[i*block_size+i] = beta*i1[i*block_size+i];


   return 0;
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockTranspose
 * (o = i1' ) 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockTranspose(double* i1, double* o, int block_size)
{
   int i, j;

   for (i = 0; i < block_size; i++)
      for (j = 0; j < block_size; j++)
         o[i*block_size+j] = i1[j*block_size+i];
   return 0;
}



/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockNorm
 * (out = norm(data) ) 
 *
 *  (note these are not all actually "norms")
 *  
 *
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockNorm(int norm_type, double* data, double* out, int block_size)
{


   int ierr = 0;
   int i,j;
   double sum = 0.0;
   double *totals;
   int sz = block_size*block_size;

   switch (norm_type)
   {
      
      case 5: /* one norm  - max col sum*/
      {
        
         totals = hypre_CTAlloc(double, block_size);
         for(i = 0; i < block_size; i++) /* row */
         {
            for(j = 0; j < block_size; j++) /* col */
            {
               totals[j] += fabs(data[i*block_size + j]);
            }
         }

         sum = totals[0];
         for(j = 1; j < block_size; j++) /* col */
         {
            if (totals[j] > sum) sum = totals[j];
         }
         hypre_TFree(totals);
         
         break;
         
      }
      case 4: /* inf norm - max row sum */
      {
      
         totals = hypre_CTAlloc(double, block_size);
         for(i = 0; i < block_size; i++) /* row */
         {
            for(j = 0; j < block_size; j++) /* col */
            {
               totals[i] += fabs(data[i*block_size + j]);
            }
         }

         sum = totals[0];
         for(i = 1; i < block_size; i++) /* row */
         {
            if (totals[i] > sum) sum = totals[i];
         }
         hypre_TFree(totals);
         
         break;
      }

      case 3: /* largest element of block (return value includes sign) */
      {
      
         sum = data[0];
  
         for(i = 0; i < sz; i++) 
         {
               if (fabs(data[i]) > fabs(sum))  sum =data[i];
         }
         
         break;
      }
      case 2: /* sum of abs values of all elements in the block  */
      {
         for(i = 0; i < sz; i++) 
         {
               sum += fabs(data[i]);
         }
         break;
      }


      default: /* 1 = frobenius*/
      {
          for(i = 0; i < sz; i++) 
          {
               sum += data[i]*data[i];
          }
          sum = sqrt(sum);
      }
   }
   
   *out = sum;
   

   return ierr;
      

   
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockMultAdd
 * (o = i1 * i2 + beta * o) 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockMultAdd(double* i1, double* i2, double beta, 
                                 double* o, int block_size)
{

#if LB_VERSION
   {
      double alp = 1.0;
      dgemm_("N","N", &block_size, &block_size, &block_size, &alp, i2, &block_size, i1,
          &block_size, &beta, o, &block_size);
   }
#else
   {
      
      int    i, j, k;
      double ddata;

      if (beta == 0.0)
      {
         for (i = 0; i < block_size; i++)
         {
            for (j = 0; j < block_size; j++)
            {
               ddata = 0.0;
               for (k = 0; k < block_size; k++)
               {
                  ddata += i1[i*block_size + k] * i2[k*block_size + j];
               }
               o[i*block_size + j] = ddata;
            }
         }
      }
      else if (beta == 1.0)
      {
         for(i = 0; i < block_size; i++)
         {
            for(j = 0; j < block_size; j++)
            {
               ddata = o[i*block_size + j];
               for(k = 0; k < block_size; k++)
                  ddata += i1[i*block_size + k] * i2[k*block_size + j];
               o[i*block_size + j] = ddata;
            }
         }
      }
      else
      {
         for(i = 0; i < block_size; i++)
         {
            for(j = 0; j < block_size; j++)
            {
               ddata = beta * o[i*block_size + j];
               for(k = 0; k < block_size; k++)
                  ddata += i1[i*block_size + k] * i2[k*block_size + j];
               o[i*block_size + j] = ddata;
            }
         }
      }
   }
   
#endif

   return 0;
}


/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockMultAddDiag
 * (diag(o) = diag(i1) * diag(i2) + beta * diag(o)) 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockMultAddDiag(double* i1, double* i2, double beta, 
                                 double* o, int block_size)
{
   int    i;

   if (beta == 0.0)
   {
      for (i = 0; i < block_size; i++)
      {
         o[i*block_size + i] = i1[i*block_size + i] * i2[i*block_size + i];
      }
   }
   else if (beta == 1.0)
   {
      for(i = 0; i < block_size; i++)
      {
         o[i*block_size + i] = o[i*block_size + i] + i1[i*block_size + i] * i2[i*block_size + i];
      }
   }
   else
   {
      for(i = 0; i < block_size; i++)
      {
            o[i*block_size + i] = beta* o[i*block_size + i] + i1[i*block_size + i] * i2[i*block_size + i];
      }
   }
   return 0;
}

/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockMatvec
 * (ov = alpha* mat * v + beta * ov)
 * mat is the matrix - size is block_size^2 
 * alpha and beta are scalars
 *--------------------------------------------------------------------------*/

int 
hypre_CSRBlockMatrixBlockMatvec(double alpha, double* mat, double* v, double beta, 
                                double* ov, int block_size)
{

   int ierr = 0;

#if LB_VERSION
   {
      int one = 1;

      dgemv_("T",  &block_size, &block_size, &alpha, mat, &block_size, v,
             &one, &beta, ov, &one);
   }
   
#else
   {
      int    i, j;
      double ddata;

      /* if alpha = 0, then no matvec */
      if (alpha == 0.0)
      {
         for (j = 0; j < block_size; j++)
         {
            ov[j] *= beta;
         }
         return ierr;
      }
      
      /* ov = (beta/alpha) * ov; */
      ddata = beta / alpha;
      if (ddata != 1.0)
      {
         if (ddata == 0.0)
         {
            for (j = 0; j < block_size; j++)
            {
               ov[j] = 0.0;
            }
         }
         else 
         {
            for (j = 0; j < block_size; j++)
            {
               ov[j] *= ddata;
            }
         }
      }
      
      /* ov = ov + mat*v */
      for (i = 0; i < block_size; i++)
      {
         ddata =  ov[i];
         for (j = 0; j < block_size; j++)
         {
            ddata += mat[i*block_size + j] * v[j];
         }
         ov[i] = ddata;
      }
      
      /* ov = alpha*ov */
      if (alpha != 1.0)
      {
         for (j = 0; j < block_size; j++)
         {
            ov[j] *= alpha;
         }
      }
   }
   
#endif

   return ierr;
   
}


/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockInvMatvec
 * (ov = mat^{-1} * v) 
 * o and v are vectors
 * mat is the matrix - size is block_size^2 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockInvMatvec(double* mat, double* v, 
                                   double* ov, int block_size)
{
   int ierr = 0;
   double *mat_i;

   mat_i = hypre_CTAlloc(double, block_size*block_size);

#if LB_VERSION
   {

      int one, info;
      int *piv;
      int sz;
      
      
      one = 1;
      piv = hypre_CTAlloc(int, block_size);
      sz = block_size*block_size;
      

     /* copy v to ov and  mat to mat_i*/

      dcopy_(&sz, mat, &one, mat_i, &one);
      dcopy_(&block_size, v, &one, ov, &one);
      
      /* writes over mat_i with LU */
      dgetrf_(&block_size,&block_size, mat_i, &block_size, piv , &info);
      if (info) 
      {
         hypre_TFree(mat_i);
         hypre_TFree(piv);
         return(-1);
      }
      
      /* writes over ov */
      dgetrs_("T", &block_size, &one , 
           mat_i, &block_size, piv, ov, &block_size, &info);
      if (info) 
      {
         hypre_TFree(mat_i);
         hypre_TFree(piv);
         return(-1);
      }

      hypre_TFree(piv);

   }
   
#else
   {
      
      int m, j, k;
      int piv_row;
      double factor, eps;
      double piv, tmp;
      eps = 1.0e-6;
      
      if (block_size ==1 )
      {
         if (fabs(mat[0]) > 1e-10)
         {
            ov[0] = v[0]/mat[0];
            hypre_TFree(mat_i);
            return(ierr);
         }
         else
         {
            /* printf("GE zero pivot error\n"); */
            hypre_TFree(mat_i);
            return(-1);
         }
      }
      else
      {
         /* copy v to ov and mat to mat_i*/
         for (k = 0; k < block_size; k++)   
         {
            ov[k] = v[k];
            for (j=0; j<block_size; j++)
            {
               mat_i[k*block_size + j] =  mat[k*block_size + j];
            }
         }
         /* start ge  - turning m_i into U factor (don't save L - just apply to 
            rhs - which is ov)*/
         /* we do partial pivoting for size */
         
         /* loop through the rows (row k) */ 
         for (k = 0; k < block_size-1; k++)
         {
            piv = mat_i[k*block_size+k];
            piv_row = k;
            
            /* find the largest pivot in position k*/
            for (j=k+1; j < block_size; j++)         
            {
               if (fabs(mat_i[j*block_size+k]) > fabs(piv))
               {
                  piv =  mat_i[j*block_size+k];
                  piv_row = j;
               }
               
            }
            if (piv_row !=k) /* do a row exchange  - rows k and piv_row*/
            {
               for (j=0; j < block_size; j++)
               {
                  tmp = mat_i[k*block_size + j];
                  mat_i[k*block_size + j] = mat_i[piv_row*block_size + j];
                  mat_i[piv_row*block_size + j] = tmp;
               }
               tmp = ov[k];
               ov[k] = ov[piv_row];
               ov[piv_row] = tmp;
            }
            /* end of pivoting */
            
            if (fabs(piv) > eps)
            {
               /* now we can factor into U */
               for (j = k+1; j < block_size; j++)
               {
                  factor = mat_i[j*block_size+k]/piv;
                  for (m = k+1; m < block_size; m++)
                  {
                     mat_i[j*block_size+m]  -= factor * mat_i[k*block_size+m];
                  }
                  /* Elimination step for rhs */ 
                  ov[j]  -= factor * ov[k];
               }
            }
            else
            {
               /* printf("Block of matrix is nearly singular: zero pivot error\n");  */
               hypre_TFree(mat_i);
               return(-1);
            }
         }

         /* we also need to check the pivot in the last row to see if it is zero */  
         k = block_size - 1; /* last row */
         if ( fabs(mat_i[k*block_size+k]) < eps)
         {
            /* printf("Block of matrix is nearly singular: zero pivot error\n");  */
            hypre_TFree(mat_i);
            return(-1);
         }
         
         /* Back Substitution  - do rhs (U is now in m_i1)*/
         for (k = block_size-1; k > 0; --k)
         {
            ov[k] /= mat_i[k*block_size+k];
            for (j = 0; j < k; j++)
            {
               if (mat_i[j*block_size+k] != 0.0)
               {
                  ov[j] -= ov[k] * mat_i[j*block_size+k];
               }
            }
         }
         ov[0] /= mat_i[0];
         
      }
      
   }
#endif
  

   hypre_TFree(mat_i);
   
   return (ierr);
}



/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockInvMult
 * (o = i1^{-1} * i2) 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockInvMult(double* i1, double* i2, double* o, int block_size)
{

   int ierr = 0;
   int i, j;
   double *m_i1;

   m_i1 = hypre_CTAlloc(double, block_size*block_size);

#if LB_VERSION
   {

      int one, info;
      int *piv;
      int sz;
      
      double *i2_t;

      one = 1;
      i2_t = hypre_CTAlloc(double, block_size*block_size);
      piv = hypre_CTAlloc(int, block_size);
      

     /* copy i1 to m_i1*/
      sz = block_size*block_size;
      dcopy_(&sz, i1, &one, m_i1, &one);


      /* writes over m_i1 with LU */
      dgetrf_(&block_size, &block_size, m_i1, &block_size, piv , &info);
      if (info) 
      {
         hypre_TFree(m_i1);
         hypre_TFree(i2_t);
         hypre_TFree(piv);
         return (-1);
      }
      
      /* need the transpose of i_2*/
     for (i=0; i < block_size; i++)
     {
        for (j=0; j< block_size; j++)
        {
           i2_t[i*block_size+j] = i2[j*block_size+i];  
        }
     }
     
      /* writes over i2_t */
      dgetrs_("T", &block_size, &block_size, 
           m_i1, &block_size, piv, i2_t, &block_size, &info);
      if (info) 
      {
         hypre_TFree(m_i1);
         hypre_TFree(i2_t);
         hypre_TFree(piv);
         return (-1);
      }
   
      /* ans. is the transpose of i2_t*/
      for (i=0; i < block_size; i++)
      {
         for (j=0; j< block_size; j++)
         {
            o[i*block_size+j] = i2_t[j*block_size+i];  
         }
      }
     
      hypre_TFree(i2_t);
      hypre_TFree(piv);
      
   }
   
#else
   {
      
      int m, k;
      int piv_row;
      double factor, eps;
      double piv, tmp;
      
      eps = 1.0e-6;
      
      if (block_size ==1 )
      {
         if (fabs(m_i1[0]) > 1e-10)
         {
            o[0] = i2[0]/i1[0];
            hypre_TFree(m_i1);
            return(ierr);
         }
         else
         {
            /* printf("GE zero pivot error\n"); */
            hypre_TFree(m_i1);
            return(-1);
         }
      }
      else
      {
         /* copy i2 to o and i1 to m_i1*/
         for (k = 0; k < block_size*block_size; k++)   
         {
            o[k] = i2[k];
            m_i1[k] = i1[k];
         }
         
         
         /* start ge  - turning m_i1 into U factor (don't save L - just apply to 
            rhs - which is o)*/
         /* we do partial pivoting for size */
         
         /* loop through the rows (row k) */ 
         for (k = 0; k < block_size-1; k++)
         {
            piv = m_i1[k*block_size+k];
            piv_row = k;
            
            /* find the largest pivot in position k*/
            for (j=k+1; j < block_size; j++)         
            {
               if (fabs(m_i1[j*block_size+k]) > fabs(piv))
               {
                  piv =  m_i1[j*block_size+k];
                  piv_row = j;
               }
               
            }
            if (piv_row !=k) /* do a row exchange  - rows k and piv_row*/
            {
               for (j=0; j < block_size; j++)
               {
                  tmp = m_i1[k*block_size + j];
                  m_i1[k*block_size + j] = m_i1[piv_row*block_size + j];
                  m_i1[piv_row*block_size + j] = tmp;
                  
                  tmp = o[k*block_size + j];
                  o[k*block_size + j] = o[piv_row*block_size + j];
                  o[piv_row*block_size + j] = tmp;
                  
               }
            }
            /* end of pivoting */
            
            
            if (fabs(piv) > eps)
            {
               /* now we can factor into U */
               for (j = k+1; j < block_size; j++)
               {
                  factor = m_i1[j*block_size+k]/piv;
                  for (m = k+1; m < block_size; m++)
                  {
                     m_i1[j*block_size+m]  -= factor * m_i1[k*block_size+m];
                  }
                  /* Elimination step for rhs */ 
                  /* do for each of the "rhs" */
                  for (i=0; i < block_size; i++)
                  {
                     /* o(row, col) = o(row*block_size + col) */ 
                     o[j*block_size+i] -= factor * o[k*block_size + i];              
                  }
               }
            }
            else
            {
               /* printf("Block of matrix is nearly singular: zero pivot error\n"); */
               hypre_TFree(m_i1);
               return(-1);
            }
         }


         /* we also need to check the pivot in the last row to see if it is zero */  
         k = block_size - 1; /* last row */
         if ( fabs(m_i1[k*block_size+k]) < eps)
         {
            /* printf("Block of matrix is nearly singular: zero pivot error\n"); */
            hypre_TFree(m_i1);
            return(-1);
         }
         
         
         /* Back Substitution  - do for each "rhs" (U is now in m_i1)*/
         for (i=0; i < block_size; i++)
         {
            for (k = block_size-1; k > 0; --k)
            {
               o[k*block_size + i] /= m_i1[k*block_size+k];
               for (j = 0; j < k; j++)
               {
                  if (m_i1[j*block_size+k] != 0.0)
                  {
                     o[j*block_size + i] -= o[k*block_size + i] * m_i1[j*block_size+k];
                  }
               }
            }
            o[0*block_size + i] /= m_i1[0];
         }
      }
   }
   
#endif
   hypre_TFree(m_i1);
   
   return ierr;
}


/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockMultInv
 * (o = i2*il^(-1)) 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockMultInv(double* i1, double* i2, double* o, int block_size)
{

   int ierr = 0;
 
   
#if LB_VERSION
   
   {
     /* same as solving A^T C^T = B^T */
      double *m_i1;
      int info;
      int *piv;
      int sz, one;
      

      piv = hypre_CTAlloc(int, block_size);
      m_i1 = hypre_CTAlloc(double, block_size*block_size);
      one = 1;
      sz = block_size*block_size;

     /* copy i1 to m_i1 and i2 to o*/

      dcopy_(&sz, i1, &one, m_i1, &one);
      dcopy_(&sz, i2, &one, o, &one);

      /* writes over m_i1 with LU */
      dgetrf_(&block_size, &block_size, m_i1, &block_size, piv , &info);
      if (info) 
      {
         hypre_TFree(m_i1);
         hypre_TFree(piv);
         return (-1);
      }
     /* writes over B */
      dgetrs_("N", &block_size, &block_size, 
           m_i1, &block_size, piv, o, &block_size, &info);
      if (info) 
      {
         hypre_TFree(m_i1);
         hypre_TFree(piv);
         return (-1);
      }

      hypre_TFree(m_i1);
      hypre_TFree(piv);
   }
   
#else
   {
      
      double eps;
      double *i1_t, *i2_t, *o_t;
      
      
      eps = 1.0e-12;
      
      if (block_size ==1 )
      {
         if (fabs(i1[0]) > eps)
         {
            o[0] = i2[0]/i1[0];
            return(ierr);
         }
         else
         {
            /* printf("GE zero pivot error\n"); */
            return(-1);
         }
      }
      else
      {
         
         i1_t = hypre_CTAlloc(double, block_size*block_size);
         i2_t = hypre_CTAlloc(double, block_size*block_size);
         o_t = hypre_CTAlloc(double, block_size*block_size);
         
         /* TO DO:: this could be done more efficiently! */  
         hypre_CSRBlockMatrixBlockTranspose(i1, i1_t, block_size);
         hypre_CSRBlockMatrixBlockTranspose(i2, i2_t, block_size);
         ierr = hypre_CSRBlockMatrixBlockInvMult(i1_t, i2_t, o_t, block_size);
         
         if (!ierr) hypre_CSRBlockMatrixBlockTranspose(o_t, o, block_size);
         
         hypre_TFree(i1_t);
         hypre_TFree(i2_t);
         hypre_TFree(o_t);
         
      }
   }
   
#endif   
   return (ierr);
}



/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixBlockInvMultDiag - zeros off-d entires
 * (o = diag(i1)^{-1} * diag(i2)) 
 *--------------------------------------------------------------------------*/
int
hypre_CSRBlockMatrixBlockInvMultDiag(double* i1, double* i2, double* o, int block_size)
{

   int ierr = 0;
   int i;
   int  sz = block_size*block_size;

   double eps;
   
   eps = 1.0e-8;

   for (i = 0; i < sz; i++)
      o[i] = 0.0;
   
   for (i = 0; i < block_size; i++)
   {
      if (fabs(i1[i*block_size + i]) > eps)
      {
         o[i*block_size + i] = i2[i*block_size + i] / i1[i*block_size + i];
      }
      else
      {
         /* printf("GE zero pivot error\n"); */
         return(-1);
      }
   }
   
   return (ierr);
}



/*--------------------------------------------------------------------------
 * hypre_CSRBlockMatrixTranspose
 *--------------------------------------------------------------------------*/

int hypre_CSRBlockMatrixTranspose(hypre_CSRBlockMatrix *A, 
                                  hypre_CSRBlockMatrix **AT, int data)

{
   double       *A_data = hypre_CSRBlockMatrixData(A);
   int          *A_i = hypre_CSRBlockMatrixI(A);
   int          *A_j = hypre_CSRBlockMatrixJ(A);
   int           num_rowsA = hypre_CSRBlockMatrixNumRows(A);
   int           num_colsA = hypre_CSRBlockMatrixNumCols(A);
   int           num_nonzerosA = hypre_CSRBlockMatrixNumNonzeros(A);
   int           block_size = hypre_CSRBlockMatrixBlockSize(A);

   double       *AT_data;
   int          *AT_i;
   int          *AT_j;
   int           num_rowsAT;
   int           num_colsAT;
   int           num_nonzerosAT;

   int           max_col;
   int           i, j, k, m, offset, bnnz;

   /*-------------------------------------------------------------- 
    * First, ascertain that num_cols and num_nonzeros has been set. 
    * If not, set them.
    *--------------------------------------------------------------*/

   if (! num_nonzerosA) num_nonzerosA = A_i[num_rowsA];
   if (num_rowsA && ! num_colsA)
   {
      max_col = -1;
      for (i = 0; i < num_rowsA; ++i)
         for (j = A_i[i]; j < A_i[i+1]; j++)
            if (A_j[j] > max_col) max_col = A_j[j];
      num_colsA = max_col+1;
   }
   num_rowsAT = num_colsA;
   num_colsAT = num_rowsA;
   num_nonzerosAT = num_nonzerosA;
   bnnz = block_size * block_size;

   *AT = hypre_CSRBlockMatrixCreate(block_size, num_rowsAT, num_colsAT, 
                                    num_nonzerosAT);

   AT_i = hypre_CTAlloc(int, num_rowsAT+1);
   AT_j = hypre_CTAlloc(int, num_nonzerosAT);
   hypre_CSRBlockMatrixI(*AT) = AT_i;
   hypre_CSRBlockMatrixJ(*AT) = AT_j;
   if (data) 
   {
      AT_data = hypre_CTAlloc(double, num_nonzerosAT*bnnz);
      hypre_CSRBlockMatrixData(*AT) = AT_data;
   }

   /*-----------------------------------------------------------------
    * Count the number of entries in each column of A (row of AT)
    * and fill the AT_i array.
    *-----------------------------------------------------------------*/

   for (i = 0; i < num_nonzerosA; i++) ++AT_i[A_j[i]+1];
   for (i = 2; i <= num_rowsAT; i++) AT_i[i] += AT_i[i-1];

   /*----------------------------------------------------------------
    * Load the data and column numbers of AT
    *----------------------------------------------------------------*/

   for (i = 0; i < num_rowsA; i++)
   {
      for (j = A_i[i]; j < A_i[i+1]; j++)
      {
         AT_j[AT_i[A_j[j]]] = i;
         if (data)
         {
            offset = AT_i[A_j[j]] * bnnz;
            for (k = 0; k < block_size; k++)
               for (m = 0; m < block_size; m++)
                  AT_data[offset+k*block_size+m] = 
                       A_data[j*bnnz+m*block_size+k];
         }
         AT_i[A_j[j]]++;
      }
   }

   /*------------------------------------------------------------
    * AT_i[j] now points to the *end* of the jth row of entries
    * instead of the beginning.  Restore AT_i to front of row.
    *------------------------------------------------------------*/

   for (i = num_rowsAT; i > 0; i--) AT_i[i] = AT_i[i-1];
   AT_i[0] = 0;

   return(0);
}
