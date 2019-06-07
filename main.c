#include <stdio.h>
#include <stdlib.h>
#include <math.h>

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



//
// Driver
//

int main( int argc, char *argv[] )
{
   complex_t a = {-2.0,-2.0 };
   complex_t b = { 2.0, 2.0 };
   struct region_s reg;
   int iret;

   inComplex_OperatorTester();

   iret = inMandelbrot_MakeRegion( &reg, a, b, 100, 100 );
   if( iret != 0 ) {
      printf("Error creating the region: %d \n",iret);
   } else {
      printf("Created a %d x %d grid \n",reg.im, reg.jm );
   }

   inMandelbrot_DumpRegion( &reg, "grid.dat" );


   return( EXIT_SUCCESS );
}

