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
// Driver
//

int main( int argc, char *argv[] )
{
// complex_t a = { 1.0, 2.0 };

   inComplex_OperatorTester();

   return( EXIT_SUCCESS );
}

