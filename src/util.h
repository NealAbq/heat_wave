// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// util.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef UTIL_H
# define UTIL_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include <limits>
# include <boost/type_traits.hpp>
# include <boost/iterator/iterator_traits.hpp>

// _______________________________________________________________________________________________

namespace util {

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  template< typename INT_TYPE >
  inline
  bool
is_even( INT_TYPE n)
  { return 0 == (n & 1); }

  template< typename INT_TYPE >
  inline
  bool
is_odd( INT_TYPE n)
  { return ! is_even( n); }

  template< typename INT_TYPE >
  bool
is_power_of_2( INT_TYPE n)
  {
    if ( n < 0 ) return false; // meaningless if unsigned type
    for ( ; ; ) {
        if ( 0 == n ) return false; // 0 is not a power of 2
        if ( 1 == n ) return true ; // 1 is the only odd power of 2
        if ( is_odd( n) ) return false;
        n >>= 1;
    }
  }

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// maybe_assign(..)

  template< typename T >
  inline
  bool
maybe_assign( T & old_value, T const & new_value)
{
    if ( old_value == new_value ) return false;
    old_value = new_value;
    return true;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Assignment functors

  template< typename ITEM_TYPE >
  struct
assign_set_type
  {
    void operator ()( ITEM_TYPE & lhs, ITEM_TYPE const & rhs) { lhs = rhs; }
  };

  template< typename ITEM_TYPE >
  struct
assign_sum_type
  {
    void operator ()( ITEM_TYPE & lhs, ITEM_TYPE const & rhs) { lhs += rhs; }
  };

  template< typename ITEM_TYPE >
  struct
assign_subtract_out_type
  {
    void operator ()( ITEM_TYPE & lhs, ITEM_TYPE const & rhs) { lhs -= rhs; }
  };

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// apply_default_ctor( p_obj)
// apply_dtor( p_obj)

  template< class TYPE >
  inline
  void
apply_default_ctor( TYPE* p_obj)
  // Improve: Should be generic.
  // We need this because std::vector<..>::clear( ) doesn't release memory in the microsoft
  // std library.
  {
  # ifndef NDEBUG
    TYPE* p_debug =
  # endif
    ::new( (void*) p_obj) TYPE;
    d_assert( p_debug == p_obj);
  }

  template< class TYPE >
  inline void
apply_dtor( TYPE* p_obj)
  // Improve: Should be generic.
  // We need this because std::vector<..>::clear( ) doesn't release memory in the microsoft
  // std library.
  {
    p_obj->~TYPE( );
  }

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// remove_negative_zero(..)

// These negative-zero tests do not work because positive and negative zero test the same.
// Do not use them because they return false trues.
//inline bool  is_negative_zero_(       float n)  { return n == -0.0f; }
//inline bool  is_negative_zero_(      double n)  { return n == -0.0 ; }
//inline bool  is_negative_zero_( long double n)  { return n == -0.0L; }

// It's OK to use the above tests here even with false positives.
//inline       float  remove_negative_zero(       float n)  { return is_negative_zero_( n) ? 0 : n; }
//inline      double  remove_negative_zero(      double n)  { return is_negative_zero_( n) ? 0 : n; }
//inline long double  remove_negative_zero( long double n)  { return is_negative_zero_( n) ? 0 : n; }

// I'm having trouble with this function signaling hardware interrupts.
// It happens even after I comment out signaling_NaN.
# if 0
  template< typename REAL_TYPE >
  bool
is_non_number( REAL_TYPE n)
  //
  // This just scratches the surface. There are lots of non-numbers.
  // See teem at sourceforge.net -- http://teem.sourceforge.net/air/754.html
  // Also see http://en.wikipedia.org/wiki/%E2%88%920_(number)
  {
    // Bug. I've had this break the program during debug because std::..::signaling_NaN( ) was
    // doing an illegal floating-point operation and the CPU was hardware interrupting. The number
    // passed in wasn't the problem, it was a normal number about 1.8.
    // The problem is the number in the std-library code that generates a signaling NaN. Usually the
    // "signal" (hardware interrupt) is disabled, but somehow it's getting enabled. So when
    // std::numeric_limits< float >::signaling_NaN( ) innocently makes a signaling float, even though
    // it is not used (except during ==), that's enough to kick the interrupt.
    //
    // This happens (sometimes) when I save an image to a file. It doesn't matter if the image comes
    // from a pbuffer or from renderPixmap(..). I think it has something to do with:
    //   Having heat_wave open
    //   Having the "Save as ..." dialog open on top of heat_wave
    //   Image previewing from the save-as dialog, so there's another window on top of that.
    //   Clicking on the save-as dialog and canceling it.
    //
    // Although it might have something to do with creating and destroying GL contexts and maybe
    // messing up state.
    // Since this is only used in assert(..)s we could easily get rid of it or replace it with
    // a macro like d_assert_is_valid_number(..). For now I'll comment out the signaling part.
    return
        (std::numeric_limits< REAL_TYPE >::infinity(      ) == n) ||
        (std::numeric_limits< REAL_TYPE >::quiet_NaN(     ) == n) /* ||
        (std::numeric_limits< REAL_TYPE >::signaling_NaN( ) == n) */ ;
  }
# endif

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// absolute_value( a)
//
//   Returns the absolute value of a. Works for all number types (and any type that supports
//   (a < 0) and (- a) and some copy semantics).
//
//   Should this be generalized to calcluate a magnitude for complex numbers and
//   multi-dimensional vectors?

  template< typename NUMBER_TYPE >
  NUMBER_TYPE
absolute_value( NUMBER_TYPE const n)
  { return (n < 0) ? (- n) : n; }

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// average_into( trg, src, count)

  template< typename ITEM_TYPE >
  void
average_into_n(
    ITEM_TYPE       * trg
  , ITEM_TYPE const * src
  , std::size_t       count
  )
  {
    ITEM_TYPE const * const src_limit = src + count;
    while ( src < src_limit ) {
        *trg = (*trg + *src) / 2;
        src += 1;
        trg += 1;
    }
  }

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// random_uniform< value_type >( interval_min, interval_max)
//
//   This is a quick-and-dirty random number generator with a uniform distribution.
//   Boost provide higher quality random generators. For example:
//     boost::mt19937 rng;
//     boost::uniform_real< value_type > interval( interval_min, max);
//     boost::variate_generator< boost::mt19937 & , boost::uniform_real< value_type > >
//              random_maker( rng, interval);
//     ...
//     return random_maker( );  // generates random numbers

  template< typename VALUE_TYPE >
  VALUE_TYPE
random_uniform( VALUE_TYPE min_val, VALUE_TYPE max_val)
  {
    static bool is_init = false;
    if ( ! is_init ) {
        // Seed the random number generator.
        ::srand( static_cast< unsigned >( ::time( 0)));
        is_init = true;
    }
    VALUE_TYPE const  rand_max  = static_cast< VALUE_TYPE >( RAND_MAX);
    VALUE_TYPE const  rand_val  = static_cast< VALUE_TYPE >( ::rand( ));
    return min_val + (((max_val - min_val) * rand_val) / rand_max);
  }

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// copy_vector( dst, src, src_stride, count)
//
//   If we just get an iterator that takes a stride then we can just use
//   std::copy( src, src + count, dst).

// No longer needed.
# if 0
  template< typename ITEM_TYPE >
  void
copy_vector
 (  ITEM_TYPE            * dst
  , ITEM_TYPE      const * src
  , std::ptrdiff_t const   s_stride
  , std::size_t    const   count
 )
  {
    // Assumes dst stride is 1.
    ITEM_TYPE const * const dst_limit = dst + count;

    // Assumes dst stide is > 0.
    while ( dst < dst_limit ) {
        *dst = *src;
        src += s_stride;
        dst += 1;
    }
  }
# endif

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// stripped_type<T> - meta-class
//
//   stripped_type<T>::type
//   stripped_type<T>::value_type
//   stripped_type<T>::is_int
//   stripped_type<T>::is_sint
//   stripped_type<T>::is_uint
//
//   (Not specific to this class. Should be generic.)
// _______________________________________________________________________________________________

  template< typename TYPE_T >
  struct
stripped_type
{
    typedef typename
    boost::remove_cv
     < typename
       boost::remove_reference
        < TYPE_T
        >::type
     >::type
  value_type;

    static
    bool const
  is_int
    = std::numeric_limits< TYPE_T >::is_integer;

    static
    bool const
  is_sint
    = is_int && std::numeric_limits< TYPE_T >::is_signed;

    static
    bool const
  is_uint
    = is_int && ! std::numeric_limits< TYPE_T >::is_signed;

    typedef value_type
  type;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// non_integer_calc_type<..> - meta-class
//
//   non_integer_calc_type< src_iter_type, trg_iter_type >::type
//   non_integer_calc_type< src_iter_type, trg_iter_type >::value_type
//   non_integer_calc_type< src_iter_type, trg_iter_type >::src_value_type
//   non_integer_calc_type< src_iter_type, trg_iter_type >::trg_value_type
//
//   (Not specific to this class. Should be generic.)
// _______________________________________________________________________________________________

  template
   <  typename SRC_ITER_T
    , typename TRG_ITER_T
   >
  struct
non_integer_calc_type
  //
  // Given two iterators, this returns an intermediate calculation type for the values behind the iterators.
  //
  // This avoids integer types because we divide by src_count and trg_count in our calculations, although
  // I suppose there are times when you want intermediate integer types.
  //
  // Returns the first of these types that is not integer:
  //   src_iter_type::value_type
  //   trg_iter_type::value_type
  //   double (built-in non-integer type)
{
    typedef stripped_type< typename boost::detail::iterator_traits< SRC_ITER_T >::value_type >
  src_stripped_meta_type;

    typedef stripped_type< typename boost::detail::iterator_traits< TRG_ITER_T >::value_type >
  trg_stripped_meta_type;

    typedef typename src_stripped_meta_type::value_type
  src_value_type;

    typedef typename trg_stripped_meta_type::value_type
  trg_value_type;

    typedef typename
    boost::mpl::if_c
     <  src_stripped_meta_type::is_int
      , boost::mpl::if_c
         <  trg_stripped_meta_type::is_int
          , double
          , trg_value_type
         >
      , src_value_type
     >::type
  value_type;

    typedef value_type
  type;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// round_if_appropriate< return_type >::doit(..) - helper function
//
//   Only rounds if we are converting from a real (float, double) class to an integer.
//   Rounds differently if trg is signed or unsigned int.
//
//   This is the same as calc_nearest(..).
// _______________________________________________________________________________________________

  // Functor decl, no body. Specializations follow.
  // I'd prefer to use a templated function but the MSVC9 compiler hasn't caught up with partial specialization yet.
  template
   <  typename RETURN_T
    , typename PARAM_T
    , bool     is_param_real  = ! stripped_type< PARAM_T >::is_int
    , bool     is_return_sint = stripped_type< RETURN_T >::is_sint
    , bool     is_return_uint = stripped_type< RETURN_T >::is_uint
   >
  struct
round_if_appropriate
{
    static
    RETURN_T
  doit( PARAM_T a)
    { return a; }

    // Functor operator cannot be static.
    // This is copied (exactly) in all specializations.
    RETURN_T
  operator ()( PARAM_T a)
    { return doit( a); }
};

// _______________________________________________________________________________________________

  // Template specialization
  //
  template
   <  typename RETURN_T /* unsigned int type */
    , typename PARAM_T  /* non-int type */
   >
  struct
round_if_appropriate< RETURN_T, PARAM_T, true, false, true >
{
    static
    RETURN_T
  doit( PARAM_T a)
    // This should probably use std::floor(..).
    { d_assert( a >= 0);
      return a + static_cast< PARAM_T >( 0.5);
    }

    // Functor operator cannot be static.
    // This is copied (exactly) in all specializations.
    RETURN_T
  operator ()( PARAM_T a)
    { return doit( a); }
};

  // Template specialization
  template
   <  typename RETURN_T /* signed int type */
    , typename PARAM_T  /* non-int type */
   >
  struct
round_if_appropriate< RETURN_T, PARAM_T, true, true, false >
{
    static
    RETURN_T
  doit( PARAM_T a)
    // This should probably use std::floor(..).
    { return a + ((a < 0) ?
                    static_cast< PARAM_T >( -0.5) :
                    static_cast< PARAM_T >( +0.5) );
    }

    // Functor operator cannot be static.
    // This is copied (exactly) in all specializations.
    RETURN_T
  operator ()( PARAM_T a)
    { return doit( a); }
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// calc_nearest(..)
//
//   This is a rounding function.
//   This is the same as the functor round_if_appropriate( a), except it is expressed as a
//   function instead of as a functor.
//   Except when converting between double and bool, which is unique.

  template< typename TRG_T, typename SRC_T >
  TRG_T
cast_nearest( SRC_T src)
  ;

  // You cannot partially specialize template functions like you can structs (functors).
  // Otherwise we'd partially specialize for when SRC_T and TRG_T were the same.

  template< >
  inline
  double
cast_nearest< double, double >( double src)
  {
    return src;
  }

  template< >
  inline
  int
cast_nearest< int, int >( int src)
  {
    return src;
  }

  template< >
  inline
  bool
cast_nearest< bool, bool >( bool src)
  {
    return src;
  }

  template< >
  inline
  int
cast_nearest< int, double >( double src)
  {
    // We should assert a castable value.
    // A double can represent a bigger number than a 32-bit int.
    return static_cast< int >( std::floor( src + 0.5));
  }

  template< >
  inline
  double
cast_nearest< double, int >( int src)
  {
    // We could assert that the int will not be truncated if it is 64-bit.
    return static_cast< double >( src);
  }

  /* unique and perhaps unexpected definition */
  template< >
  inline
  bool
cast_nearest< bool, double >( double src)
  {
    // Cast the range (-1,+1) to false.
    return (src <= -1.0) || (src >= +1.0);
  }

  template< >
  inline
  double
cast_nearest< double, bool >( bool src)
  {
    return src ? +1.0 : 0.0;
  }

// _______________________________________________________________________________________________
} /* end namespace util */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef UTIL_H
//
// util.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
