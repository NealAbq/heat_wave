// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// pt3.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef PT3_H
# define PT3_H
// _______________________________________________________________________________________________
//
// 3D point template
// For example: pt3< float >
//
// This should have supertype boost::array< TYPE, COUNT >.
// # include <boost/array.hpp>
// _______________________________________________________________________________________________
//
// This should be in a namespace. Like "geo".
// Instead of pt3, should we call this:
//   point
//   vector  - there's already a std::vector
//
// Should we have more template params?
//   pt< 3, float >
// We certainly use pt2 occasionally, and maybe pt4 and pt1 even less.
// And pt< 0, float > is useful in typedef situations.
//
// Special operations just for dense (floating) coord_types and discrete (integer) types.
//
// Should we have a supertype ortho_space< 3, float >?
// Another class: range< 3, float >, which is useful for defining bounding ortho-boxes.
//
// class pt_polar< float, float >      - 2D point expressed as polar coords
// class pt_spherical< float, float >  - 3D point expressed as spherical coords
//
// Transformation classes
//   General matrix (affine) transforms.
//   Translation, Rotation, Scale
//
// Can this be generalized to matrix and tensor?
//
// Difference coords vs absolute coords
//   Like a unit attached to the coord type.
//   Instead of a coord_type like "float" or "int".
//   Could wrap this as a class:
//     class absolute_coord< float >
//     class relative_coord< float >
//       abs + abs -> illegal (although you can convert the 2nd abs to a rel)
//       abs - abs -> relative
//       abs + rel -> absolute
//       abs - rel -> absolute
//       rel + abs -> illegal?
//       rel - abs -> illegal?
//       rel + rel -> relative
//       rel - rel -> relative
//     abs converts easily to relative, but to convert relative to absolute you usually have
//       to add it to another absolute.
//
//   Since GL uses scaled integers to represent values in the -1..+1 range we should support
//   this in a class. Float->ScaledInt< int_type >, etc, for signed and unsigned.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  template< typename COORD_T >
  class
pt3
{
  // Typedefs
  public:
    typedef COORD_T            coord_type;
  private:
    typedef pt3< coord_type >  this_type;

  // Ctors and dtor
  public:
    /* ctor */     pt3( )                  : x_( 0), y_( 0), z_( 0) { }
    /* ctor */     pt3( coord_type x, coord_type y, coord_type z)
                                           : x_( x), y_( y), z_( z) { }
    /* dtor */     ~pt3( )                 { }

  // Copy
  //   r-value copy not needed
  public:
    /* copy */     pt3( this_type const & b)
                                           : x_( b.get_x( )), y_( b.get_y( )), z_( b.get_z( ))
                                           { }
    this_type &    operator =( this_type const & b)
                                           { x_ = b.get_x( ); y_ = b.get_y( ); z_ = b.get_z( );
                                             return *this;
                                           }

  // Reset and Zero
  public:
    this_type &    set_zero( )             { set_x( 0);
                                             set_y( 0);
                                             set_z( 0);
                                             return *this;
                                           }
    bool           is_zero( )        const { return
                                               (get_x( ) == 0) &&
                                               (get_y( ) == 0) &&
                                               (get_z( ) == 0) ;
                                           }
    bool           not_zero( )       const { return ! is_zero( ); }

    this_type &    reset( )                { set_zero( ); }
    bool           is_reset( )       const { return is_zero( ); }
    bool           not_reset( )      const { return not_zero( ); }

  // Safe bool idiom
  public:
      typedef coord_type this_type::*
    unspecified_bool_type;

      operator
    unspecified_bool_type( )         const { return not_zero( ) ? & this_type::x_ : 0; }

      unspecified_bool_type
    operator !( )                    const { return is_zero( ) ? & this_type::x_ : 0; }

  // Getters
  public:
    coord_type     get_x( )          const { return x_; }
    coord_type     get_y( )          const { return y_; }
    coord_type     get_z( )          const { return z_; }

  // Setters
  public:
    this_type &    set_x( coord_type x)    { x_ = x; return *this; }
    this_type &    set_y( coord_type y)    { y_ = y; return *this; }
    this_type &    set_z( coord_type z)    { z_ = z; return *this; }

    coord_type &   ref_x( )                { return x_; }
    coord_type &   ref_y( )                { return y_; }
    coord_type &   ref_z( )                { return z_; }

  // Asymmetric Operators
  public:
    this_type      operator -( )     const { return this_type( - get_x( ), - get_y( ), - get_z( )); }
    this_type      operator +( )     const { return *this; }

    this_type &    operator +=( this_type const & b)
                                           { ref_x( ) += b.get_x( );
                                             ref_y( ) += b.get_y( );
                                             ref_z( ) += b.get_z( );
                                             return *this;
                                           }
    this_type &    operator -=( this_type const & b)
                                           { ref_x( ) -= b.get_x( );
                                             ref_y( ) -= b.get_y( );
                                             ref_z( ) -= b.get_z( );
                                             return *this;
                                           }

    this_type &    operator *=( coord_type s)
                                           { ref_x( ) *= s;
                                             ref_y( ) *= s;
                                             ref_z( ) *= s;
                                             return *this;
                                           }
    this_type &    operator /=( coord_type s)
                                           { d_assert( s != 0);
                                             ref_x( ) /= s;
                                             ref_y( ) /= s;
                                             ref_z( ) /= s;
                                             return *this;
                                           }

  // Transformations
  public:
    coord_type     get_magnitude( )  const ;
    this_type &    set_magnitude( coord_type final_magnitude)
                                           ;

  // Member vars
  private:
    coord_type  x_ ;
    coord_type  y_ ;
    coord_type  z_ ;
};

// _______________________________________________________________________________________________

  template< typename COORD_T >
  bool
operator ==( pt3< COORD_T > const & a, pt3< COORD_T > const & b)
{
    return
        (a.get_x( ) == b.get_x( )) &&
        (a.get_y( ) == b.get_y( )) &&
        (a.get_z( ) == b.get_z( )) ;
}

  template< typename COORD_T >
  bool
operator <( pt3< COORD_T > const & a, pt3< COORD_T > const & b)
{
    return
        (a.get_x( ) < b.get_x( )) ? true : (
        (a.get_y( ) < b.get_y( )) ? true : (
        (a.get_z( ) < b.get_z( )) ? true : (
        false )));
}

  template< typename COORD_T >
  bool
operator <=( pt3< COORD_T > const & a, pt3< COORD_T > const & b)
{
    return
        (a.get_x( ) <= b.get_x( )) ? true : (
        (a.get_y( ) <= b.get_y( )) ? true : (
        (a.get_z( ) <= b.get_z( )) ? true : (
        false )));
}

  template< typename COORD_T >
  int
compare( pt3< COORD_T > const & a, pt3< COORD_T > const & b)
{
#   define COMPARE( GET ) \
        (a.GET( ) != b.GET( )) ? ((a.GET( ) < b.GET( )) ? -1 : 1)

    return
        COMPARE( get_x ) : (
        COMPARE( get_y ) : (
        COMPARE( get_z ) : 0 ));

#   undef COMPARE
}

  // We could get the following from "using namespace std::rel_ops;" (see <utility>)

  template< typename COORD_T >
  inline
  bool
operator !=( pt3< COORD_T > const & a, pt3< COORD_T > const & b)
{
    return ! (a == b);
}

  template< typename COORD_T >
  inline
  bool
operator >( pt3< COORD_T > const & a, pt3< COORD_T > const & b)
{
    return b <  a;
}

  template< typename COORD_T >
  inline
  bool
operator >=( pt3< COORD_T > const & a, pt3< COORD_T > const & b)
{
    return b <= a;
}

// _______________________________________________________________________________________________

  template< typename COORD_T >
  inline
  pt3< COORD_T >
operator +( pt3< COORD_T > const & a, pt3< COORD_T > const & b)
  {
    // This is the preferable implementation even though it's not as nice as { return a += b; }
    // where a is a copy and not a const ref.
    // This can be optimized by the compiler.
    pt3< COORD_T > copy( a);
    copy += b;
    return copy;
  }

  template< typename COORD_T >
  inline
  pt3< COORD_T >
operator -( pt3< COORD_T > a, pt3< COORD_T > const & b)  { return a -= b; }

  template< typename COORD_T >
  inline
  pt3< COORD_T >
operator *( pt3< COORD_T > a, typename pt3< COORD_T >::coord_type s)  { return a *= s; }

  template< typename COORD_T >
  inline
  pt3< COORD_T >
operator /( pt3< COORD_T > a, typename pt3< COORD_T >::coord_type s)  { return a /= s; }

// _______________________________________________________________________________________________

  template< typename COORD_T >
  void
swap( pt3< COORD_T > & a, pt3< COORD_T > & b)
{
    boost::swap( a.ref_x( ), b.ref_x( ));
    boost::swap( a.ref_y( ), b.ref_y( ));
    boost::swap( a.ref_z( ), b.ref_z( ));
}

  template< typename COORD_T >
  std::size_t
hash_value( pt3< COORD_T > const & a)
{
    std::size_t seed = 0;
    boost::hash_combine( seed, a.get_x( ));
    boost::hash_combine( seed, a.get_y( ));
    boost::hash_combine( seed, a.get_z( ));
    return seed;
}

// _______________________________________________________________________________________________

  template< typename COORD_T >
  pt3< COORD_T >
get_min( pt3< COORD_T > const & a, pt3< COORD_T > const & b)
{
    // The following lets use compile whether or not we have min #defined.
    // <WinDef.h> #defines min and max.
  # ifndef min
    using std::min;
  # endif

    return pt3< COORD_T >(
        min( a.get_x( ), b.get_x( )),
        min( a.get_y( ), b.get_y( )),
        min( a.get_z( ), b.get_z( )));
}

  template< typename COORD_T >
  pt3< COORD_T >
get_max( pt3< COORD_T > const & a, pt3< COORD_T > const & b)
{
    // The following lets use compile whether or not we have max #defined.
    // <WinDef.h> #defines min and max.
  # ifndef max
    using std::max;
  # endif

    return pt3< COORD_T >(
        max( a.get_x( ), b.get_x( )),
        max( a.get_y( ), b.get_y( )),
        max( a.get_z( ), b.get_z( )));
}

// _______________________________________________________________________________________________

  template< typename COORD_T >
  pt3< COORD_T >
get_abs( pt3< COORD_T > const & a)
{
    return pt3< COORD_T >(
        std::abs( a.get_x( )),
        std::abs( a.get_y( )),
        std::abs( a.get_z( )));
}

// _______________________________________________________________________________________________

  template< typename COORD_T >
  COORD_T
get_orthogonal_length( pt3< COORD_T > const & a)
{
    return a.get_x( ) + a.get_y( ) + a.get_z( );
}

  template< typename COORD_T >
  COORD_T
get_magnitude( pt3< COORD_T > const & a)
{
    // std::sqrt(..) (from math.h) takes a double and returns a double.
    // 
    return static_cast< COORD_T >( std::sqrt( dot_product( a, a)));
}

  template< typename COORD_T >
  inline
  COORD_T
  pt3< COORD_T >::
get_magnitude( ) const
{
    return ::get_magnitude( *this);
}

  template< typename COORD_T >
  pt3< COORD_T > &
  pt3< COORD_T >::
set_magnitude( COORD_T new_magnitude)
{
    COORD_T const old_magnitude = get_magnitude( );
    d_assert( old_magnitude > 0);

    *this *= (new_magnitude / old_magnitude);
    return *this;
}

// _______________________________________________________________________________________________

  template< typename COORD_T >
  COORD_T
dot_product( pt3< COORD_T > const & a, pt3< COORD_T > const & b)
  //
  // Also known as (AKA):
  //   Inner product
  //   Dot product
  //   Scalar product of vectors
  //
  // Dot characters:
  //   ·
  //   •
  //
  // A•B = |A| * |B| * cos( angle)
  // angle = arc_cos( (A•B) / (|A| * |B|))
  //
  // A•A = magnitude squared
  //
  // Properties (from http://en.wikipedia.org/wiki/Dot_product)
  //   (A, B, and C are real vectors, r and s are scalars)
  //
  //   Commutative:
  //     A•B = B•A
  //   Distributive over vector addition:
  //     A•(B+C) = (A•B) + (A•C)
  //   Bilinear:
  //     A•(rB+C) = r(A•B) + (A•C)
  //   Not associative:
  //     A(B•C) ? (A•B)C
  //   Matrix multiply, assuming vectors are column matrices:
  //     (A•B)C = (C * transpose(B))A
  //   Scalar multiply:
  //     (rA)•(sB) = (rs)(A•B)
  //
  //   Perpendicular iff A•B=0
  //
  //   Does not obey cancellation (ab=ac and a?0 then b=c):
  //     If A•B=A•C and A?0, then
  //     A•(B?C)=0 by the distributive law; and from the previous result above:
  //     If A is perpendicular to (B?C), we can have (B?C)?0 and therefore B?C.
  //
  //   Derivative:
  //     If F(t) and G(t) are functions that return vectors, then the derivative of F•G is (F'•G)+(F•G').
{
    return
        (a.get_x( ) * b.get_x( )) +
        (a.get_y( ) * b.get_y( )) +
        (a.get_z( ) * b.get_z( ));
}

// _______________________________________________________________________________________________

  template< typename COORD_T >
  pt3< COORD_T >
cross_product( pt3< COORD_T > const & a, pt3< COORD_T > const & b)
  //
  // Also known as (AKA):
  //   Outer product
  //   Cross product
  //   Vector product of vectors
  //
  // A × B = (a * b * sin( angle )) normal_unit_vector
  // |A × B| = |a| |b| sin
  //
  // Algebraic properties (from http://en.wikipedia.org/wiki/Cross_product)
  //   Anticommutative:
  //     a × b = ?b × a
  //   Distributive over addition:
  //     a × (b + c) = (a × b) + (a × c)
  //   Compatible with scalar multiplication:
  //     (r a) × b = a × (r b) = r (a × b).
  //
  //   Not associative:
  //     a × (b × c) ? (a × b) × c
  //   Satisfies the Jacobi identity:
  //     a × (b × c) + b × (c × a) + c × (a × b) = 0
  //
  //   Does NOT obey the cancellation law: (Cancellation law: if ab=ac and a?0 then b=c).
  //     If a×b = a×c and a ? 0 then:
  //     (a×b) ? (a×c) = 0 and, by the distributive law above:
  //     a × (b ? c) = 0
  //     Now, if a is parallel to (b?c), then even if a?0 it is possible that (b?c)?0 and therefore that b?c.
  //   However, if both a•b=a•c and a×b=a×c, then it can be concluded that b=c. Indeed,
  //     a • (b - c) = 0, and
  //     a × (b - c) = 0
  //   so that b-c is both parallel and perpendicular to the non-zero vector a. This is only possible if b-c=0.
  //
  //   Two non-zero vectors a and b are parallel if and only if a × b = 0.
  //   The cross product is invariant under rotations about the axis defined by a×b.
  //
  //   Scalar triple product (volume of parallelepiped):
  //     A•(B×C) = B•(C×A) = C•(A×B)
  //     A•(B×C) = (A×B)•C
  //
  //   Vector triple product
  //     A×(B×C) = (A•C)B ? (A•B)C
  //     (A×B)×C = (A•C)B ? (B•C)A
  //
  //   Cross product is area of parallelogram:
  //     area = |A × B|
  //   Volume of parallelepiped:
  //     volume = A•(B×C)
  //
  //   Cross products and the triple product:
  //     (a × b) × (a × c) = (a • (b × c)) a
  //
  //   Identity under matrix transformations:
  //     (MA)×(MB) = determinant( M) transpose( inverse( M)) (A×B)
  //   where M is a 3 by 3 matrix.
  //
  // The cross product is not invariant between left-handed and right-handed coord systems.
  // Because of this, the vector produced as the cross-product is sometimes a pseudovector.
  // Crossing a pseudovector with a normal vector yields a normal vector.
  //   vector × vector -> pseudovector
  //   vector × pseudovector -> vector
  //   pseudovector × pseudovector -> pseudovector
  //
  // Unlike dot-product, which is defined for vectors of any dimensionality, cross-product
  // is only defined for 3-dimensional and 7-dimensional vector pairs.
{
    return pt3< COORD_T >(
        (a.get_y( ) * b.get_z( )) - (a.get_z( ) * b.get_y( )),
        (a.get_z( ) * b.get_x( )) - (a.get_x( ) * b.get_z( )),
        (a.get_x( ) * b.get_y( )) - (a.get_y( ) * b.get_x( )));
}

// _______________________________________________________________________________________________

  template< typename COORD_T, typename FACTOR_T >
  pt3< COORD_T >
interpolate( FACTOR_T factor, pt3< COORD_T > const & v0, pt3< COORD_T > const & v1)
  //
  // Returns a vector on the line between v0 and v1.
  // How far along the line depends on factor.
  // If factor == 0, returns v0.
  // If factor == 1, returns v1.
  //
  // FACTOR_T is usually float or double.
  // factor is usually in the range 0..1, although it is allowed to be outside that range.
  //
  // interpolate(..) is like swap(..) or hash. It is a basic building block for other functions,
  // like the bezier(..) functions below. There should be a standard idiomatic way to define
  // interpolate(..) for appropriate classes and get the bezier(..) functions for free from
  // template definitions.
{
    float const zfactor = 1.0f - factor;

  # define INTERPOLATE_COMPONENT( X )            \
      ((v0.get_ ## X( ) == v1.get_ ## X( )) ?    \
         v0.get_ ## X( ) :                       \
         (zfactor * v0.get_ ## X( )) +           \
         ( factor * v1.get_ ## X( ))             \
      )                                          /* end macro */

    return pt3< COORD_T >(
        INTERPOLATE_COMPONENT( x),
        INTERPOLATE_COMPONENT( y),
        INTERPOLATE_COMPONENT( z) );

  # undef INTERPOLATE_COMPONENT
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef PT3_H
//
// pt3.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
