/*  testMMM.c  */

#include "../InpMtx.h"
#include "../../Drand.h"

/*--------------------------------------------------------------------*/
int
main ( int argc, char *argv[] )
/*
   ------------------------------------------------------------------
   generate a random matrix and test a matrix-matrix multiply method.
   the output is a matlab file to test correctness.

   created -- 98jan29, cca
 --------------------------------------------------------------------
*/
{
DenseMtx   *X, *Y ;
double     alpha[2] ;
double     alphaImag, alphaReal ;
double     *zvec ;
Drand      *drand ;
int        col, dataType, ii, msglvl, ncolA, nitem, nrhs, nrowA, nrowX, 
           nrowY, row, seed, storageMode, symflag, transposeflag ;
int        *colids, *rowids ;
InpMtx     *A ;
FILE       *msgFile ;

if ( argc != 14 ) {
   fprintf(stdout, 
      "\n\n %% usage : %s msglvl msgFile symflag storageMode "
      "\n %%         nrow ncol nent nrhs seed alphaReal alphaImag"
      "\n %%    msglvl   -- message level"
      "\n %%    msgFile  -- message file"
      "\n %%    dataType -- type of matrix entries"
      "\n %%       1 -- real"
      "\n %%       2 -- complex"
      "\n %%    symflag  -- symmetry flag"
      "\n %%       0 -- symmetric"
      "\n %%       1 -- hermitian"
      "\n %%       2 -- nonsymmetric"
      "\n %%    storageMode -- storage mode"
      "\n %%       1 -- by rows"
      "\n %%       2 -- by columns"
      "\n %%       3 -- by chevrons, (requires nrow = ncol)"
      "\n %%    transpose -- transpose flag"
      "\n %%       0 -- Y := Y + alpha * A * X"
      "\n %%       1 -- Y := Y + alpha * A^H * X, nonsymmetric only"
      "\n %%       2 -- Y := Y + alpha * A^T * X, nonsymmetric only"
      "\n %%    nrowA    -- number of rows in A"
      "\n %%    ncolA    -- number of columns in A"
      "\n %%    nitem    -- number of items"
      "\n %%    nrhs     -- number of right hand sides"
      "\n %%    seed     -- random number seed"
      "\n %%    alphaReal -- y := y + alpha*A*x"
      "\n %%    alphaImag -- y := y + alpha*A*x"
      "\n", argv[0]) ;
   return(0) ;
}
msglvl = atoi(argv[1]) ;
if ( strcmp(argv[2], "stdout") == 0 ) {
   msgFile = stdout ;
} else if ( (msgFile = fopen(argv[2], "a")) == NULL ) {
   fprintf(stderr, "\n fatal error in %s"
           "\n unable to open file %s\n",
           argv[0], argv[2]) ;
   return(-1) ;
}
dataType      = atoi(argv[3]) ;
symflag       = atoi(argv[4]) ;
storageMode   = atoi(argv[5]) ;
transposeflag = atoi(argv[6]) ;
nrowA         = atoi(argv[7]) ;
ncolA         = atoi(argv[8]) ;
nitem         = atoi(argv[9]) ;
nrhs          = atoi(argv[10]) ;
seed          = atoi(argv[11]) ;
alphaReal     = atof(argv[12]) ;
alphaImag     = atof(argv[13]) ;
fprintf(msgFile, 
        "\n %% %s "
        "\n %% msglvl        -- %d" 
        "\n %% msgFile       -- %s" 
        "\n %% dataType      -- %d" 
        "\n %% symflag       -- %d" 
        "\n %% storageMode   -- %d" 
        "\n %% transposeflag -- %d" 
        "\n %% nrowA         -- %d" 
        "\n %% ncolA         -- %d" 
        "\n %% nitem         -- %d" 
        "\n %% nrhs          -- %d" 
        "\n %% seed          -- %d"
        "\n %% alphaReal     -- %e"
        "\n %% alphaImag     -- %e"
        "\n",
        argv[0], msglvl, argv[2], dataType, symflag, storageMode,
        transposeflag, nrowA, ncolA, nitem, nrhs, seed, 
        alphaReal, alphaImag) ;
fflush(msgFile) ;
if ( dataType != 1 && dataType != 2 ) {
   fprintf(stderr, "\n invalid value %d for dataType\n", dataType) ;
   spoolesFatal();
}
if ( symflag != 0 && symflag != 1 && symflag != 2 ) {
   fprintf(stderr, "\n invalid value %d for symflag\n", symflag) ;
   spoolesFatal();
}
if ( storageMode != 1 && storageMode != 2 && storageMode != 3 ) {
   fprintf(stderr, 
           "\n invalid value %d for storageMode\n", storageMode) ;
   spoolesFatal();
}
if ( transposeflag < 0
   || transposeflag > 2 ) {
   fprintf(stderr, "\n error, transposeflag = %d, must be 0, 1 or 2",
           transposeflag) ;
   spoolesFatal();
}
if ( (transposeflag == 1 && symflag != 2)
   || (transposeflag == 2 && symflag != 2) ) {
   fprintf(stderr, "\n error, transposeflag = %d, symflag = %d",
           transposeflag, symflag) ;
   spoolesFatal();
}
if ( transposeflag == 1 && dataType != 2 ) {
   fprintf(stderr, "\n error, transposeflag = %d, dataType = %d",
           transposeflag, dataType) ;
   spoolesFatal();
}
if ( symflag == 1 && dataType != 2 ) {
   fprintf(stderr, 
           "\n symflag = 1 (hermitian), dataType != 2 (complex)") ;
   spoolesFatal();
}
if ( nrowA <= 0 || ncolA <= 0 || nitem <= 0 ) {
   fprintf(stderr, 
           "\n invalid value: nrow = %d, ncol = %d, nitem = %d",
           nrowA, ncolA, nitem) ;
   spoolesFatal();
}
if ( symflag < 2 && nrowA != ncolA ) {
   fprintf(stderr,
           "\n invalid data: symflag = %d, nrow = %d, ncol = %d",
           symflag, nrowA, ncolA) ;
   spoolesFatal();
}
alpha[0] = alphaReal ;
alpha[1] = alphaImag ;
/*
   ----------------------------
   initialize the matrix object
   ----------------------------
*/
A = InpMtx_new() ;
InpMtx_init(A, storageMode, dataType, 0, 0) ;
drand = Drand_new() ;
/*
   ----------------------------------
   generate a vector of nitem triples
   ----------------------------------
*/
rowids = IVinit(nitem,   -1) ;
Drand_setUniform(drand, 0, nrowA) ;
Drand_fillIvector(drand, nitem, rowids) ;
colids = IVinit(nitem,   -1) ;
Drand_setUniform(drand, 0, ncolA) ;
Drand_fillIvector(drand, nitem, colids) ;
Drand_setUniform(drand, 0.0, 1.0) ;
if ( INPMTX_IS_REAL_ENTRIES(A) ) {
   zvec = DVinit(nitem, 0.0) ;
   Drand_fillDvector(drand, nitem, zvec) ;
} else if ( INPMTX_IS_COMPLEX_ENTRIES(A) ) {
   zvec = ZVinit(nitem, 0.0, 0.0) ;
   Drand_fillDvector(drand, 2*nitem, zvec) ;
}
/*
   -----------------------------------
   assemble the entries entry by entry
   -----------------------------------
*/
fprintf(msgFile, "\n\n A = zeros(%d,%d) ;", nrowA, ncolA) ;
if ( symflag == 1 ) {
/*
   ----------------
   hermitian matrix
   ----------------
*/
   for ( ii = 0 ; ii < nitem ; ii++ ) {
      if ( rowids[ii] == colids[ii] ) {
         zvec[2*ii+1] = 0.0 ;
      }
      if ( rowids[ii] <= colids[ii] ) {
         row = rowids[ii] ; col = colids[ii] ;
      } else {
         row = colids[ii] ; col = rowids[ii] ;
      }
      InpMtx_inputComplexEntry(A, row, col, zvec[2*ii], zvec[2*ii+1]) ;
   }
} else if ( symflag == 0 ) {
/*
   ----------------
   symmetric matrix
   ----------------
*/
   if ( INPMTX_IS_REAL_ENTRIES(A) ) {
      for ( ii = 0 ; ii < nitem ; ii++ ) {
         if ( rowids[ii] <= colids[ii] ) {
            row = rowids[ii] ; col = colids[ii] ;
         } else {
            row = colids[ii] ; col = rowids[ii] ;
         }
         InpMtx_inputRealEntry(A, row, col, zvec[ii]) ;
      }
   } else if ( INPMTX_IS_COMPLEX_ENTRIES(A) ) {
      for ( ii = 0 ; ii < nitem ; ii++ ) {
         if ( rowids[ii] <= colids[ii] ) {
            row = rowids[ii] ; col = colids[ii] ;
         } else {
            row = colids[ii] ; col = rowids[ii] ;
         }
         InpMtx_inputComplexEntry(A, row, col,
                                  zvec[2*ii], zvec[2*ii+1]) ;
      }
   }
} else {
/*
   -------------------
   nonsymmetric matrix
   -------------------
*/
   if ( INPMTX_IS_REAL_ENTRIES(A) ) {
      for ( ii = 0 ; ii < nitem ; ii++ ) {
         InpMtx_inputRealEntry(A, rowids[ii], colids[ii], zvec[ii]) ;
      }
   } else if ( INPMTX_IS_COMPLEX_ENTRIES(A) ) {
      for ( ii = 0 ; ii < nitem ; ii++ ) {
         InpMtx_inputComplexEntry(A, rowids[ii], colids[ii], 
                                  zvec[2*ii], zvec[2*ii+1]) ;
      }
   }
}
InpMtx_changeStorageMode(A, INPMTX_BY_VECTORS) ;
DVfree(zvec) ;
/*
   -------------------------------------------
   write the assembled matrix to a matlab file
   -------------------------------------------
*/
InpMtx_writeForMatlab(A, "A", msgFile) ;
if ( symflag == 0 ) {
   fprintf(msgFile,
           "\n   for k = 1:%d"
           "\n      for j = k+1:%d"
           "\n         A(j,k) = A(k,j) ;"
           "\n      end"
           "\n   end", nrowA, ncolA) ;
} else if ( symflag == 1 ) {
   fprintf(msgFile,
           "\n   for k = 1:%d"
           "\n      for j = k+1:%d"
           "\n         A(j,k) = ctranspose(A(k,j)) ;"
           "\n      end"
           "\n   end", nrowA, ncolA) ;
}
/*
   -------------------------------
   generate dense matrices X and Y
   -------------------------------
*/
if ( transposeflag == 0 ) {
   nrowX = ncolA ;
   nrowY = nrowA ;
} else {
   nrowX = nrowA ;
   nrowY = ncolA ;
}
X = DenseMtx_new() ;
Y = DenseMtx_new() ;
if ( INPMTX_IS_REAL_ENTRIES(A) ) {
   DenseMtx_init(X, SPOOLES_REAL, 0, 0, nrowX, nrhs, 1, nrowX) ;
   Drand_fillDvector(drand, nrowX*nrhs, DenseMtx_entries(X)) ;
   DenseMtx_init(Y, SPOOLES_REAL, 0, 0, nrowY, nrhs, 1, nrowY) ;
   Drand_fillDvector(drand, nrowY*nrhs, DenseMtx_entries(Y)) ;
} else if ( INPMTX_IS_COMPLEX_ENTRIES(A) ) {
   DenseMtx_init(X, SPOOLES_COMPLEX, 0, 0, nrowX, nrhs, 1, nrowX) ;
   Drand_fillDvector(drand, 2*nrowX*nrhs, DenseMtx_entries(X)) ;
   DenseMtx_init(Y, SPOOLES_COMPLEX, 0, 0, nrowY, nrhs, 1, nrowY) ;
   Drand_fillDvector(drand, 2*nrowY*nrhs, DenseMtx_entries(Y)) ;
}
fprintf(msgFile, "\n X = zeros(%d,%d) ;", nrowX, nrhs) ;
DenseMtx_writeForMatlab(X, "X", msgFile) ;
fprintf(msgFile, "\n Y = zeros(%d,%d) ;", nrowY, nrhs) ;
DenseMtx_writeForMatlab(Y, "Y", msgFile) ;
/*
   ----------------------------------
   perform the matrix-matrix multiply
   ----------------------------------
*/
fprintf(msgFile, "\n alpha = %20.12e + %20.2e*i;", alpha[0], alpha[1]);
fprintf(msgFile, "\n Z = zeros(%d,1) ;", nrowY) ;
if ( transposeflag == 0 ) {
   if ( symflag == 0 ) {
      InpMtx_sym_mmm(A, Y, alpha, X) ;
   } else if ( symflag == 1 ) {
      InpMtx_herm_mmm(A, Y, alpha, X) ;
   } else if ( symflag == 2 ) {
      InpMtx_nonsym_mmm(A, Y, alpha, X) ;
   }
   DenseMtx_writeForMatlab(Y, "Z", msgFile) ;
   fprintf(msgFile, "\n maxerr = max(Z - Y - alpha*A*X) ") ;
   fprintf(msgFile, "\n") ;
} else if ( transposeflag == 1 ) {
   InpMtx_nonsym_mmm_H(A, Y, alpha, X) ;
   DenseMtx_writeForMatlab(Y, "Z", msgFile) ;
   fprintf(msgFile, "\n maxerr = max(Z - Y - alpha*ctranspose(A)*X) ") ;
   fprintf(msgFile, "\n") ;
} else if ( transposeflag == 2 ) {
   InpMtx_nonsym_mmm_T(A, Y, alpha, X) ;
   DenseMtx_writeForMatlab(Y, "Z", msgFile) ;
   fprintf(msgFile, "\n maxerr = max(Z - Y - alpha*transpose(A)*X) ") ;
   fprintf(msgFile, "\n") ;
}
/*
   ------------------------
   free the working storage
   ------------------------
*/
InpMtx_free(A) ;
DenseMtx_free(X) ;
DenseMtx_free(Y) ;
IVfree(rowids) ;
IVfree(colids) ;
Drand_free(drand) ;

fclose(msgFile) ;

return(1) ; }

/*--------------------------------------------------------------------*/
