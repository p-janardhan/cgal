// Copyright (c) 1997-2001  ETH Zurich (Switzerland).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org); you may redistribute it under
// the terms of the Q Public License version 1.0.
// See the file LICENSE.QPL distributed with CGAL.
//
// Licensees holding a valid commercial license may use this file in
// accordance with the commercial license agreement provided with the software.
//
// This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
//
// $URL: svn+ssh://gaertner@scm.gforge.inria.fr/svn/cgal/trunk/QP_solver/include/CGAL/QP_solver/gmp_double.h $
// $Id: gmp_double.h 33782 2006-08-25 14:06:31Z gaertner $
// 
//
// Author(s)     : Sven Schoenherr <sven@inf.fu-berlin.de>
//                 Bernd Gaertner <gaertner@inf.ethz.ch>
//                 Franz Wessendorp <fransw@inf.ethz.ch>
//                 Kaspar Fischer <fischerk@inf.ethz.ch>

#ifndef CGAL_GMPZF_H
#define CGAL_GMPZF_H

// includes
#include <CGAL/basic.h>
#include <CGAL/Handle_for.h>
#include <iostream>
#include <cmath>
#include <limits>
#include <string>

#ifndef CGAL_CFG_NO_LOCALE
#  include <locale>
#else
#  include <cctype>
#endif

#include <gmp.h>
#include <mpfr.h>
#include <CGAL/Quotient.h>
#include <CGAL/Gmpz.h>

#include <boost/operators.hpp>

CGAL_BEGIN_NAMESPACE
struct Gmpzf_rep // as in Gmpz.h
{
// FIXME : bug if ~() is called before an mpz_init*() is called.
// not a problem in practice, but not nice.
// maybe the mpz_init_set* functions should move back to Gmpz_rep.
// But then we should use the Storage_traits::construct/get...

  mpz_t mpZ;

  Gmpzf_rep() {}
  ~Gmpzf_rep() { mpz_clear(mpZ); }

private:
  // Make sure it does not get accidentally copied.
  Gmpzf_rep(const Gmpzf_rep &);
  Gmpzf_rep & operator= (const Gmpzf_rep &);
};

// forward declarations
class Gmpzf;
class Gmpz;
bool operator==(const Gmpzf &a, const Gmpzf &b);

// class declaration
// =================
// This is an exact floating point type; it can represent numbers
// of the form m*2^e, where m is of type mpz_t and e (currently)
// of type long

class Gmpzf :
  Handle_for<Gmpzf_rep>,
    boost::ordered_euclidian_ring_operators1< Gmpzf
  , boost::ordered_euclidian_ring_operators2< Gmpzf, int
    > >
{
  typedef Handle_for<Gmpzf_rep> Base; 

public:
  // tags
  // ----
  typedef Tag_true   Has_gcd;
  typedef Tag_true   Has_division;
  typedef Tag_true   Has_sqrt;
  typedef Tag_true   Has_exact_ring_operations;
  typedef Tag_true   Has_exact_division;
  typedef Tag_false  Has_exact_sqrt;

  // exponent type
  // --------------------
  typedef long Exponent; // may overflow, but if it does, the mantissa is
                         // potentially too large to be useful, anyway;
                         // still, repeated squaring of a power of two
                         // quickly brings this type to its limits...

private:
  // data members (mantissa is from Gmpzf_rep)
  // ----------------------------------------
  // Invariant: 
  // - number is in canonical form, i.e.(m,e) == 0 or m is odd
  Exponent  e; 

  // some static helpers
  static const int double_precision;  // std::numeric_limits<double>::digits
  static Gmpzf s;                     // for intermediate computations 

public:
  // access
  // ------
  const mpz_t& man() const 
  {
    return Ptr()->mpZ;
  }

  mpz_t& man() // actually, this shouldn't be public
  {
    return ptr()->mpZ;
  }

  const Exponent& exp() const
  {
    return e;
  }

  // construction
  // ------------
  Gmpzf( ) 
    : e(0)
  {
    mpz_init(man());
    CGAL_postcondition(is_canonical());
  }

  Gmpzf(const mpz_t z)
    : e(0)
  { 
    mpz_init_set(man(), z); 
    canonicalize();
  }  

  Gmpzf(const Gmpz& n )
    : e(0)
  { 
    mpz_init_set(man(), n.mpz()); 
    canonicalize();
  }

  Gmpzf( int i)
    : e(0)
  {   
    mpz_init_set_si( man(), i);  
    canonicalize();
  }

  Gmpzf( long l)
    : e(0)
  {   
    mpz_init_set_si( man(), l);
    canonicalize();
  }

  Gmpzf( double d)    
  {
    Protect_FPU_rounding<> P(CGAL_FE_TONEAREST);
    if (d == 0) {
      mpz_init (man());
      e = 0;
      return;
    }
    CGAL_assertion(is_finite(d) && is_valid(d));
    int exp;
    double x = std::frexp(d, &exp); // x in [1/2, 1], x*2^exp = d
    mpz_init_set_d (man(), std::ldexp( x, double_precision)); // an integer   
    e = exp - double_precision;
    canonicalize();
  }

  // arithmetics 
  // -----------
  Gmpzf operator-() const 
  {
    Gmpzf result;
    mpz_neg (result.man(), man());
    result.e = exp();
    CGAL_postcondition(is_canonical());
    return result;
  }

  Gmpzf& operator+=( const Gmpzf& b)
  {  
    Gmpzf result;
    if (b.is_zero()) return *this; // important in sparse contexts
    const mpz_t *a_aligned, *b_aligned;
    align (a_aligned, b_aligned, e, *this, b);
    mpz_add(result.man(), *a_aligned, *b_aligned);
    swap(result);
    canonicalize();
    return(*this);
  }

  Gmpzf& operator+=( int i)
  {
    return operator+=(Gmpzf (i));   // could be optimized, but why?
  }

  Gmpzf& operator-=( const Gmpzf& b)
  {    
    Gmpzf result;
    if (b.is_zero()) return *this; // important in sparse contexts
    const mpz_t *a_aligned, *b_aligned;
    align (a_aligned, b_aligned, e, *this, b);
    mpz_sub(result.man(), *a_aligned, *b_aligned);
    swap(result);
    canonicalize();
    return(*this);   
  }

  Gmpzf& operator-=( int i)
  {
    return operator-=(Gmpzf (i));   // could be optimized, but why?
  }

  Gmpzf& operator*=( const Gmpzf& b)
  { 
    Gmpzf result;
    mpz_mul(result.man(), man(), b.man());
    e += b.exp();
    swap (result);
    canonicalize();
    return *this; 
  }  

  Gmpzf& operator*=( int i)
  {
    Gmpzf result;
    mpz_mul_si(result.man(), man(), i);   
    swap (result);
    canonicalize();
    return *this; 
  }

  // *this = m1 * 2 ^ e1 = a_aligned * 2 ^ rexp
  //     b = m2 * 2 ^ e2 = b_aligned * 2 ^ rexp,   where rexp = min (e1, e2)
  // 
  // => a / b = a div b = (a_aligned div b_aligned)
  //            a mod b = (a_aligned mod b_aligned) * 2 ^ rexp
  Gmpzf& operator/= (const Gmpzf& b)
  {
    CGAL_precondition(!b.is_zero());
    Gmpzf result;
    const mpz_t *a_aligned, *b_aligned;
    align (a_aligned, b_aligned, e, *this, b);
    mpz_tdiv_q (result.man(), *a_aligned, *b_aligned); // round towards zero
    e = 0; 
    swap(result);
    canonicalize();
    return(*this);   
  }

  Gmpzf& operator%= (const Gmpzf& b)
  {
    CGAL_precondition(!b.is_zero());
    Gmpzf result;
    const mpz_t *a_aligned, *b_aligned;
    align (a_aligned, b_aligned, e, *this, b);
    mpz_tdiv_r (result.man(), *a_aligned, *b_aligned);
    swap(result);
    canonicalize();
    return(*this);   
  }

  Gmpzf& operator/= (int i)
  {
    return operator/= (Gmpzf(i));
  }

  Gmpzf& operator%= (int i) 
  {
    return operator%= (Gmpzf(i));
  }

  bool is_zero() const
  {
    return mpz_sgn( man()) == 0;
  }

  Sign sign() const 
  {
    return static_cast<Sign>(mpz_sgn( man()));
  }

  Gmpzf exact_division(const Gmpzf& b) const
  {
    Gmpzf result;
    mpz_divexact(result.man(), man(), b.man());
    result.e = exp()-b.exp(); 
    result.canonicalize();
    CGAL_postcondition (*this == b * result);
    return result;
  }

  Gmpzf gcd (const Gmpzf& b) const
  {
    Gmpzf result;
    mpz_gcd (result.man(), man(), b.man()); // exponent is 0
    result.canonicalize();
    return result;  
  }

  Gmpzf sqrt() const
  {
    // is there a well-defined sqrt at all?? Here we do the
    // following: write *this as m * 2 ^ e with e even, and 
    // then return sqrt(m) * 2 ^ (e/2)
    Gmpzf result;
    // make exponent even
    if (exp() % 2 == 0) {
      mpz_set (result.man(), man());
    } else {
      mpz_mul_2exp (result.man(), man(), 1); 
    }
    mpz_sqrt(result.man(), result.man());
    result.e = exp() / 2;
    result.canonicalize();
    return result;  
  }

  Comparison_result compare (const Gmpzf &b) const
  {
    const mpz_t *a_aligned, *b_aligned;
    Exponent rexp; // ignored
    align (a_aligned, b_aligned, rexp, *this, b);
    int c = mpz_cmp(*a_aligned, *b_aligned);
    if (c < 0) return SMALLER;
    if (c > 0) return LARGER;
    return EQUAL;
  }
  
private:
  void canonicalize()
  {
    if (!is_zero()) {
      // chop off trailing zeros in m
      unsigned long zeros = mpz_scan1(man(), 0);
      mpz_tdiv_q_2exp( man(), man(), zeros);  // bit-wise right-shift
      e += zeros;
    } else {
      e = 0;
    }
    CGAL_postcondition(is_canonical());
  }

  bool is_canonical() const
  {
    return (is_zero() && e==0) || mpz_odd_p (man());
  }

  // align a and b such that they have the same exponent:
  // a = m1 * 2 ^ e1 -> a_aligned * 2 ^ rexp,
  // b = m2 * 2 ^ e2 -> b_aligned * 2 ^ rexp,   where rexp = min (e1, e2)
  // 
  // function sets (pointers to) a_aligned and b_aligned and rexp;
  // it uses the static s to store the shifted number
  static void align ( const mpz_t*& a_aligned, const mpz_t*& b_aligned, 
		     Exponent& rexp, const Gmpzf& a, const Gmpzf& b) {
    switch (CGAL::compare (b.exp(), a.exp())) {
    case SMALLER:
      // leftshift of a to reach b.exp()
      mpz_mul_2exp (s.man(), a.man(), a.exp() - b.exp()); 
      a_aligned = &s.man();  // leftshifted a
      b_aligned = &b.man();  // b
      rexp = b.exp();
      break;
    case LARGER:
      // leftshift of b to reach a.exp()
      mpz_mul_2exp (s.man(), b.man(), b.exp() - a.exp());
      a_aligned = &a.man(); // a
      b_aligned = &s.man(); // leftshifted b
      rexp = a.exp();
      break;
    case EQUAL:
      a_aligned = &a.man();
      b_aligned = &b.man();
      rexp = a.exp();
    }
  }
  
};

// initialization of static members
// ================================
const int Gmpzf::double_precision = std::numeric_limits<double>::digits;
Gmpzf Gmpzf::s = Gmpzf();

// global functions
// ================

// to_double functions
// -------------------
double to_double( const Gmpzf& a ) 
{
   return std::ldexp( mpz_get_d(a.man()), a.exp());
}


// overload to protect against overflow
double to_double( const Quotient<Gmpzf > &q)
{
  // convert quotient of mantissas, then shift by difference of exponents
  // note: this fails if difference of exponents doesn't fit into an int
  return std::ldexp( 
     to_double(CGAL::Quotient<Gmpz>(
       q.numerator().man(), q.denominator().man())),
       q.numerator().exp() -q.denominator().exp()
     );  
}


// input/output
// ------------

std::ostream& operator<< (std::ostream& os, const Gmpzf& a) 
{
  return os << CGAL::to_double(a);
}


std::ostream& print (std::ostream& os, const Gmpzf& a) 
{
  return os << a.man() << "*2^" << a.exp();
}


std::istream&  operator>> ( std::istream& is, Gmpzf& a) 
{
  // simply read from double
  double d;
  is >> d;
  if (is.good()) 
    a = Gmpzf(d);
  return is;
}

// comparisons
// -----------

Comparison_result compare (Gmpzf &a, const Gmpzf &b)
{
  return a.compare(b);
}

bool operator<(const Gmpzf &a, const Gmpzf &b)
{ 
  return a.compare(b) == SMALLER;
}

bool operator==(const Gmpzf &a, const Gmpzf &b)
{ 
  return ( (mpz_cmp(a.man(), b.man()) == 0) && a.exp() == b.exp() );
}

// mixed operators
bool operator<(const Gmpzf &a, int b)
{
  return operator<(a, Gmpzf(b));
}

bool operator==(const Gmpzf &a, int b)
{
  return operator==(a, Gmpzf(b));
}

bool operator>(const Gmpzf &a, int b)
{
  return operator>(a, Gmpzf(b));
}

Sign sign (const Gmpzf &a)
{
  return a.sign();
}

bool is_finite(const Gmpzf &)
{
  return true;
}
 
bool is_valid(const Gmpzf &)
{
  return true;
}

io_Operator io_tag(const Gmpzf &)
{
  return io_Operator();
}

// arithmetic functions
// --------------------

// div, sqrt,...(interface like Gmpz)

Gmpzf exact_division( const Gmpzf& a, const Gmpzf& b) 
{
  return a.exact_division(b);
}

Gmpzf gcd ( const Gmpzf& a, const Gmpzf& b)
{
  return a.gcd(b);
}

Gmpzf gcd ( const Gmpzf& a, int i)
{
  return gcd (a, Gmpzf(i)); // optimization possible, but why?
}

Gmpzf div ( const Gmpzf& a, const Gmpzf& b)
{
  return a / b;
}

Gmpzf sqrt (  const Gmpzf& b) 
{
  return b.sqrt();
}

#if ! defined( CGAL_DONT_USE_LINK_PRAGMA) && defined( _MSC_VER )
    #pragma comment(lib, "gmp.lib")
    #pragma comment(lib, "mpfr.lib")
#endif 

CGAL_END_NAMESPACE

#endif // CGAL_GMPZF_H

// ===== EOF ==================================================================
