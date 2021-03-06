/*  testGMVM.c  */

#include "../InpMtx.h"
#include "../../Drand.h"

/*--------------------------------------------------------------------*/
int
main ( int argc, char *argv[] )
/*
   ------------------------------------------------------
   generate a random matrix and test the InpMtx_*_gmvm*()  
   matrix-matrix multiply methods.
   the output is a matlab file to test correctness.

   created -- 98nov14, cca
   ------------------------------------------------------
*/
{
DenseMtx   *X, *Y ;
double     alpha[2], beta[2] ;
double     alphaImag, alphaReal, betaImag, betaReal ;
double     *x, *y ;
Drand      *drand ;
int        dataType, msglvl, ncolA, nitem, nrowA, nrowX, 
           nrowY, seed, coordType, rc, symflag, transposeflag ;
InpMtx     *A ;
FILE       *msgFile ;

if ( argc != 15 ) {
   fprintf(stdout, 
   "\n\n %% usage : %s msglvl msgFile symflag coordType "
   "\n %%         nrow ncol nent seed "
   "\n %%         alphaReal alphaImag betaReal betaImag"
   "\n %%    msglvl   -- message level"
   "\n %%    msgFile  -- message file"
   "\n %%    dataType -- type of matrix entries"
   "\n %%       1 -- real"
   "\n %%       2 -- complex"
   "\n %%    symflag  -- symmetry flag"
   "\n %%       0 -- symmetric"
   "\n %%       1 -- hermitian"
   "\n %%       2 -- nonsymmetric"
   "\n %%    coordType -- storage mode"
   "\n %%       1 -- by rows"
   "\n %%       2 -- by columns"
   "\n %%       3 -- by chevrons, (requires nrow = ncol)"
   "\n %%    transpose -- transpose flag"
   "\n %%       0 -- Y := beta * Y + alpha * A * X"
   "\n %%       1 -- Y := beta * Y + alpha * A^H * X, nonsymmetric only"
   "\n %%       2 -- Y := beta * Y + alpha * A^T * X, nonsymmetric only"
   "\n %%    nrowA     -- number of rows in A"
   "\n %%    ncolA     -- number of columns in A"
   "\n %%    nitem     -- number of items"
   "\n %%    seed      -- random number seed"
   "\n %%    alphaReal -- y := beta*y + alpha*A*x"
   "\n %%    alphaImag -- y := beta*y + alpha*A*x"
   "\n %%    betaReal  -- y := beta*y + alpha*A*x"
   "\n %%    betaImag  -- y := beta*y + alpha*A*x"
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
coordType   = atoi(argv[5]) ;
transposeflag = atoi(argv[6]) ;
nrowA         = atoi(argv[7]) ;
ncolA         = atoi(argv[8]) ;
nitem         = atoi(argv[9]) ;
seed          = atoi(argv[10]) ;
alphaReal     = atof(argv[11]) ;
alphaImag     = atof(argv[12]) ;
betaReal      = atof(argv[13]) ;
betaImag      = atof(argv[14]) ;
fprintf(msgFile, 
        "\n %% %s "
        "\n %% msglvl        -- %d" 
        "\n %% msgFile       -- %s" 
        "\n %% dataType      -- %d" 
        "\n %% symflag       -- %d" 
        "\n %% coordType     -- %d" 
        "\n %% transposeflag -- %d" 
        "\n %% nrowA         -- %d" 
        "\n %% ncolA         -- %d" 
        "\n %% nitem         -- %d" 
        "\n %% seed          -- %d"
        "\n %% alphaReal     -- %e"
        "\n %% alphaImag     -- %e"
        "\n %% betaReal      -- %e"
        "\n %% betaImag      -- %e"
        "\n",
        argv[0], msglvl, argv[2], dataType, symflag, coordType,
        transposeflag, nrowA, ncolA, nitem, seed, 
        alphaReal, alphaImag, betaReal, betaImag) ;
fflush(msgFile) ;
if ( dataType != 1 && dataType != 2 ) {
   fprintf(stderr, "\n invalid value %d for dataType\n", dataType) ;
   spoolesFatal();
}
if ( symflag != 0 && symflag != 1 && symflag != 2 ) {
   fprintf(stderr, "\n invalid value %d for symflag\n", symflag) ;
   spoolesFatal();
}
if ( coordType != 1 && coordType != 2 && coordType != 3 ) {
   fprintf(stderr, 
           "\n invalid value %d for coordType\n", coordType) ;
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
beta[0]  = betaReal ;
beta[1]  = betaImag ;
drand = Drand_new() ;
Drand_setSeed(drand, seed) ;
Drand_setUniform(drand, -1.0, 1.0) ;
/*
   ----------------------------
   initialize the matrix object
   and fill with random entries
   ----------------------------
*/
A = InpMtx_new() ;
InpMtx_init(A, coordType, dataType, 0, 0) ;
rc = InpMtx_randomMatrix(A, dataType, coordType, INPMTX_BY_VECTORS,
                         nrowA, ncolA, symflag, 1, nitem, seed) ;
if ( rc != 1 ) {
   fprintf(stderr, "\n error return %d from InpMtx_randomMatrix()", rc);
   spoolesFatal();
}
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
DenseMtx_init(X, dataType, 0, 0, nrowX, 1, 1, nrowX) ;
DenseMtx_fillRandomEntries(X, drand) ;
DenseMtx_init(Y, dataType, 0, 0, nrowY, 1, 1, nrowY) ;
DenseMtx_fillRandomEntries(Y, drand) ;
fprintf(msgFile, "\n X = zeros(%d,%d) ;", nrowX, 1) ;
DenseMtx_writeForMatlab(X, "X", msgFile) ;
fprintf(msgFile, "\n Y = zeros(%d,%d) ;", nrowY, 1) ;
DenseMtx_writeForMatlab(Y, "Y", msgFile) ;
x = DenseMtx_entries(X) ;
y = DenseMtx_entries(Y) ;
/*
   ----------------------------------
   perform the matrix-matrix multiply
   ----------------------------------
*/
fprintf(msgFile, "\n beta = %20.12e + %20.2e*i;", beta[0], beta[1]);
fprintf(msgFile, "\n alpha = %20.12e + %20.2e*i;", alpha[0], alpha[1]);
fprintf(msgFile, "\n Z = zeros(%d,1) ;", nrowY) ;
if ( transposeflag == 0 ) {
   if ( symflag == 0 ) {
      InpMtx_sym_gmvm(A, beta, nrowY, y, alpha, nrowX, x) ;
   } else if ( symflag == 1 ) {
      InpMtx_herm_gmvm(A, beta, nrowY, y, alpha, nrowX, x) ;
   } else if ( symflag == 2 ) {
      InpMtx_nonsym_gmvm(A, beta, nrowY, y, alpha, nrowX, x) ;
   }
   DenseMtx_writeForMatlab(Y, "Z", msgFile) ;
   fprintf(msgFile, "\n maxerr = max(Z - beta*Y - alpha*A*X) ") ;
   fprintf(msgFile, "\n") ;
} else if ( transposeflag == 1 ) {
   InpMtx_nonsym_gmvm_H(A, beta, nrowY, y, alpha, nrowX, x) ;
   DenseMtx_writeForMatlab(Y, "Z", msgFile) ;
   fprintf(msgFile, 
           "\n maxerr = max(Z - beta*Y - alpha*ctranspose(A)*X) ") ;
   fprintf(msgFile, "\n") ;
} else if ( transposeflag == 2 ) {
   InpMtx_nonsym_gmvm_T(A, beta, nrowY, y, alpha, nrowX, x) ;
   DenseMtx_writeForMatlab(Y, "Z", msgFile) ;
   fprintf(msgFile, 
           "\n maxerr = max(Z - beta*Y - alpha*transpose(A)*X) ") ;
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
Drand_free(drand) ;

fclose(msgFile) ;

return(1) ; }

/*--------------------------------------------------------------------*/
