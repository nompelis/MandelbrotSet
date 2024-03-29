/******************************************************************************
 Code to approximate the Mandelbrot set by plotting the number of interations
 performed without termination or -1 for divergent points on the interesting
 part of the complex plane.

 IN <nompelis@nobelware.com> 2019/06/07
 ******************************************************************************/

/******************************************************************************
 Copyright (c) 2019, Ioannis Nompelis
 All rights reserved.

 Redistribution and use in source and binary forms, with or without any
 modification, are permitted provided that the following conditions are met:
 1. Redistribution of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
 2. Redistribution in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
 3. All advertising materials mentioning features or use of this software
    must display the following acknowledgement:
    "This product includes software developed by Ioannis Nompelis."
 4. Neither the name of Ioannis Nompelis and his partners/affiliates nor the
    names of other contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.
 5. Redistribution or use of source code and binary forms for profit must
    have written permission of the copyright holder.
 
 THIS SOFTWARE IS PROVIDED BY IOANNIS NOMPELIS ''AS IS'' AND ANY
 EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL IOANNIS NOMPELIS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

//
// we will work with our own complex numbers to make this portable
// (alternatively, we could use GNU's complex arithmetic intrinsics)
// 
typedef struct complex_s {
   double re,im;
} complex_t;

//
// Functions to act as algebraic operators for our (complex numbers) field
//

complex_t inComplex_Addition( complex_t a, complex_t b )
{
   // use "a" as a placeholder to return the result
   a.re += b.re;
   a.im += b.im;
   return( a );
}

complex_t inComplex_Multiplication( complex_t a, complex_t b )
{
   complex_t c;
   c.re = a.re * b.re - a.im * b.im;
   c.im = a.im * b.re + a.re * b.im;
   return( c );
}

complex_t inComplex_MultReal( double r, complex_t a )
{
   // use "a" as a placeholder to return the result
   a.re = r * a.re;
   a.im = r * a.im;
   return( a );
}

complex_t inComplex_MultImag( double i, complex_t a )
{
   complex_t c;
   c.im =   i * a.re;
   c.re = -(i * a.im);
   return( c );
}



//
// a function to test our operators
//
void inComplex_OperatorTester()
{
   complex_t a = { 1.0,  1.0 };
   complex_t b = { 1.0, -2.0 };
   complex_t c;
   double r;


   printf("a = (%lf, %lf) \n", a.re, a.im );
   printf("b = (%lf, %lf) \n", b.re, b.im );
   c = inComplex_Addition( a, b );
   printf("Result of a + b = (%lf, %lf) \n\n", c.re, c.im );

   printf("a = (%lf, %lf) \n", a.re, a.im );
   printf("b = (%lf, %lf) \n", b.re, b.im );
   c = inComplex_Multiplication( a, b );
   printf("Result of a * b = (%lf, %lf) \n\n", c.re, c.im );

   printf("a = (%lf, %lf) \n", a.re, a.im );
   r = 1.5;
   c = inComplex_MultReal( r, a );
   printf("Result of (%lf,0) * a = (%lf, %lf) \n\n", r, c.re, c.im );

   printf("a = (%lf, %lf) \n", a.re, a.im );
   r = 1.5;
   c = inComplex_MultImag( r, a );
   printf("Result of (0,%lf) * a = (%lf, %lf) \n\n", r, c.re, c.im );



}


//
// a struct to hold the Mandelbrot set computation results
// for a small region of the complex plane
//
struct region_s {
   int im,jm;            // resolution of the rectangle
   complex_t pA,pB;      // points on the complex plane to define the rectangle
   complex_t *points;    // array of points on the grid on the complex plane
   long *it;             // array to store iteration number per grid point
};


//
// a function to create a small rectangular region of the complex plane
// (this construction is certainly a little wasteful in terms of memory when
// it comes to storing the full set of grid points, when we can be storing
// two arrays of reals, one for the real and one for the imaginary axis)
//
int inMandelbrot_MakeRegion( struct region_s *s, 
                             complex_t pA, complex_t pB, int im, int jm )
{
   size_t isize;
   int i,j,n;

   if( s == NULL ) return(1);
   if( im <= 0 || jm <= 0 ) return(2);


   isize = (size_t) (im*jm);
   s->points = (complex_t *) malloc( isize * sizeof(complex_t) );
   s->it     = (long *)      malloc( isize * sizeof(long) );
   if( s->points == NULL || s->it == NULL ) {
      if( s->points != NULL ) free( s->points );
      if( s->it != NULL ) free( s->it );
      s->points = NULL;
      s->it = NULL;
      return(-1);
   }

   s->im = im;
   s->jm = jm;
   s->pA.re = pA.re;
   s->pA.im = pA.im;
   s->pB.re = pB.re;
   s->pB.im = pB.im;

   n = 0;
   for(j=0;j<jm;++j) {
      double y = pA.im + (pB.im - pA.im) * ((double) j)/((double) (jm-1));
      for(i=0;i<im;++i) {
         double x = pA.re + (pB.re - pA.re) * ((double) i)/((double) (im-1));
         s->points[n].re = x;
         s->points[n].im = y;
         s->it[n] = 0;
         ++n;
      }
   }

   return(0);
}


//
// a function to write the region's grid to a file in tecplot format
// (this can be read by Paraview and Gnuplot)
//
int inMandelbrot_DumpRegion( struct region_s *s, char filename[] )
{
   int i,j,n;
   FILE *fp;

   fp = fopen( filename, "w" );
   if( fp == NULL ) return(1);

   fprintf(fp,"#### Region of the complex plane ### \n");
   fprintf(fp,"#### Point A (%lf, %lf) \n", s->pA.re, s->pA.im );
   fprintf(fp,"#### Point B (%lf, %lf) \n", s->pB.re, s->pB.im );
   fprintf(fp,"#### Resolution %d x %d \n", s->im, s->jm );
   fprintf(fp,"VARIABLES = x y n \n");
   fprintf(fp,"ZONE T=\"Grid\",\n");
   fprintf(fp,"     I=,%d, J=%d, F=POINT \n",s->im,s->jm);

   n = 0;
   for(j=0;j<s->jm;++j) {
      for(i=0;i<s->im;++i) {
         fprintf(fp, "%21.16e %21.16e ", s->points[n].re, s->points[n].im );
         fprintf(fp, "%ld \n", s->it[n] );
         ++n;
      }
   }

   return(0);


   fclose( fp );

   return(0);
}


long inMandelbrot_Iterate( complex_t c, long nstop )
{
   register complex_t tmp = { 0.0, 0.0 };
   register long n = 0;

   while( n < nstop ) {
      double d;

      tmp = inComplex_Multiplication( tmp, tmp );    // square 
      tmp = inComplex_Addition( tmp, c );            // add constant
      d = sqrt( tmp.re * tmp.re + tmp.im * tmp.im );
      if( d > 2.0 ) return( n );    // return "steps to certain divergence"
      ++n;
   }

   return( -1 );
}


//
// a function to iterate the function to search for whether points are
// possibly in the Mandelbrot set
//
int inMandelbrot_IterateRegion( struct region_s *s, long nstop )
{
   int i,j,n;


   n = 0;
   for(j=0;j<s->jm;++j) {
      for(i=0;i<s->im;++i) {

         s->it[n] = inMandelbrot_Iterate( s->points[n], nstop );

         ++n;
      }
   }

   return(0);


}

//
// Driver
//

int main( int argc, char *argv[] )
{
// complex_t a = {-2.1,-2.1 };   // baseline
// complex_t b = { 2.1, 2.1 };
// complex_t a = { 0.1, 0.4 };   // interesting zoomed region
// complex_t b = { 0.4, 0.7 };
   complex_t a = { 0.24, 0.58 };   // interesting zoomed region
   complex_t b = { 0.31, 0.63 };
   struct region_s reg;
   int iret;

   inComplex_OperatorTester();

   iret = inMandelbrot_MakeRegion( &reg, a, b, 1000, 1001 );
   if( iret != 0 ) {
      printf("Error creating the region: %d \n",iret);
   } else {
      printf("Created a %d x %d grid \n",reg.im, reg.jm );
   }

   inMandelbrot_IterateRegion( &reg, 10000 );

   inMandelbrot_DumpRegion( &reg, "grid.dat" );

   return( EXIT_SUCCESS );
}

