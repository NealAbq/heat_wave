// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_env_fractional_fixed_point.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef GL_ENV_FRACTIONAL_FIXED_POINT_H
# define GL_ENV_FRACTIONAL_FIXED_POINT_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Fractional fixed-point functions
//
//   When working with GL color-components and normal coord xyz-triplets, signed integer values
//   are mapped to the range -1..+1, and unsigned integers are mapped to 0..+1.
//   GL does not scale floats and doubles in these situations.
//
//   With floats and signed integers the min value is -1, but with unsigned integers the min
//   value is zero. Since color components are usually not negative, signed integers are rarely
//   used to represent them.
//
//   Signed integers do not represent zero exactly. Underlying zero maps to the smallest positive
//   number representable.
// _______________________________________________________________________________________________

# include "all.h"
# include "gl_env_type_primitives.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
namespace gl_env {
// _______________________________________________________________________________________________

// ---------------------------------------------------------------------------------------------
// get_min_fractional_fixed_point< type >( )

  /* declaration, no implementation */
  template< typename TYPE >
  TYPE
get_min_fractional_fixed_point( bool allow_negative = false)
  { d_static_assert( std::numeric_limits< TYPE >::is_integer);
    return allow_negative ? std::numeric_limits< TYPE >::min( ) : 0;
  }

  /* specialization */
  template< >
  inline
  float_type
get_min_fractional_fixed_point< float_type >( bool allow_negative)
  { return allow_negative ? -1.0f : 0; }

  /* specialization */
  template< >
  inline
  double_type
get_min_fractional_fixed_point< double_type >( bool allow_negative)
  { return allow_negative ? -1.0 : 0; }

// ---------------------------------------------------------------------------------------------
// get_max_fractional_fixed_point< type >( )

  template< typename TYPE >
  inline
  TYPE
get_max_fractional_fixed_point( )
  { d_static_assert( std::numeric_limits< TYPE >::is_integer);
    return std::numeric_limits< int_type >::max( );
  }

  template< >
  inline
  float_type
get_max_fractional_fixed_point< float_type >( )
  { return +1.0f; }

  template< >
  inline
  double_type
get_max_fractional_fixed_point< double_type >( )
  { return +1.0; }

// ---------------------------------------------------------------------------------------------
// is_clamped__fractional_fixed_point( color_component, allow_negative = false)
// get_clamp__fractional_fixed_point( color_component, allow_negative = false)
// clamp__fractional_fixed_point( color_component, allow_negative = false)

  template< typename TYPE >
  inline
  bool
is_clamped__fractional_fixed_point( TYPE cc, bool allow_negative = false)
  { return
      (cc >= get_min_fractional_fixed_point< TYPE >( allow_negative)) &&
      (cc <= get_max_fractional_fixed_point< TYPE >( ));
  }

  template< typename TYPE >
  inline
  TYPE
get_clamp__fractional_fixed_point( TYPE cc, bool allow_negative = false)
  { TYPE const min_value = get_min_fractional_fixed_point< TYPE >( allow_negative);
    TYPE const max_value = get_max_fractional_fixed_point< TYPE >( );
    d_assert( min_value < max_value);
    if ( cc < min_value ) {
        return min_value;
    }
    if ( cc > max_value ) {
        return max_value;
    }
    return cc;
  }

  template< typename TYPE >
  inline
  void
clamp__fractional_fixed_point( TYPE & cc, bool allow_negative = false)
  { TYPE const min_value = get_min_fractional_fixed_point< TYPE >( allow_negative);
    TYPE const max_value = get_max_fractional_fixed_point< TYPE >( );
    d_assert( min_value < max_value);
    if ( cc < min_value ) {
        cc = min_value;
    } else
    if ( cc > max_value ) {
        cc = max_value;
    }
  }

// ---------------------------------------------------------------------------------------------
// convert_fractional_fixed_point_to< trg_type >( src_type)  - not defined yet
//
//   There are 8 types to convert between, and thus 64 different conversions:
//     float_type
//     double_type
//
//     ubyte_type
//     ushort_type
//     uint_type
//
//     byte_type
//     short_type
//     int_type

  template
   <  typename TRG_TYPE
    , typename SRC_TYPE

    , bool     IS_TRG_INTEGRAL = std::numeric_limits< TRG_TYPE >::is_integer
    , bool     IS_TRG_SIGNED   = std::numeric_limits< TRG_TYPE >::is_signed

    , bool     IS_SRC_INTEGRAL = std::numeric_limits< SRC_TYPE >::is_integer
    , bool     IS_SRC_SIGNED   = std::numeric_limits< SRC_TYPE >::is_signed

    , bool     IS_NARROWING    = (std::numeric_limits< TRG_TYPE >::digits < std::numeric_limits< SRC_TYPE >::digits)
    , bool     IS_WIDENING     = (std::numeric_limits< TRG_TYPE >::digits > std::numeric_limits< SRC_TYPE >::digits)
   >
  struct
convert_ffp_helper
  {
    static TRG_TYPE eval( SRC_TYPE const &); /* implemented in specializations */
  };

  // ---------------------------------------------------------------
  // specialization when src and trg types are the same
  template
   <  typename SAME_TYPE
    , bool     IS_INTEGRAL
    , bool     IS_SIGNED
   >
  struct
convert_ffp_helper
 <  SAME_TYPE, SAME_TYPE
  , IS_INTEGRAL, IS_SIGNED
  , IS_INTEGRAL, IS_SIGNED
  , false, false
 >
  {
    static SAME_TYPE eval( SAME_TYPE const & src) { return src; }
  };

  // ---------------------------------------------------------------
  // specialization for float->double
  template< >
  struct
convert_ffp_helper
 <  double_type, float_type
  , false, true  /* trg is float/double */
  , false, true  /* src is float/double */
  , false, true  /* is widening */
 >
  {
    static double_type eval( float_type const & src) { return src; }
  };

  // ---------------------------------------------------------------
  // specialization for double->float
  template< >
  struct
convert_ffp_helper
 <  float_type, double_type
  , false, true  /* trg is float/double */
  , false, true  /* src is float/double */
  , true, false  /* is narrowing */
 >
  {
    static float_type eval( double_type const & src) { return static_cast< float_type >( src); }
  };

  // ---------------------------------------------------------------
  // specialization when src is float/double and trg is signed integer
  template
   <  typename s_TRG_TYPE
    , typename f_SRC_TYPE
    , bool     IS_NARROWING
    , bool     IS_WIDENING
   >
  struct
convert_ffp_helper
 <  s_TRG_TYPE, f_SRC_TYPE
  , true , true  /* trg is signed integer */
  , false, true  /* src is float/double */
  , IS_NARROWING, IS_WIDENING
 >
  {
    d_static_assert(   std::numeric_limits< s_TRG_TYPE >::is_integer); // trg is integer
    d_static_assert(   std::numeric_limits< s_TRG_TYPE >::is_signed ); // trg is signed
    d_static_assert( ! std::numeric_limits< f_SRC_TYPE >::is_integer); // src is float/double

    d_static_assert( (boost::integer_traits< s_TRG_TYPE >::const_min) < 0);
    d_static_assert( ((boost::integer_traits< s_TRG_TYPE >::const_min) & 1) == 0); /* const_min is even */

      static s_TRG_TYPE
    eval( f_SRC_TYPE const & f_src)
      { f_SRC_TYPE const min_int_as_f = boost::integer_traits< s_TRG_TYPE >::const_min;
        f_SRC_TYPE const max_int_as_f = boost::integer_traits< s_TRG_TYPE >::const_max;
        if ( f_src ==  0 ) return 0; /* map zero to zero */
        if ( f_src <= -1 ) return min_int_as_f; /* even, bits look like 100000... */
        if ( f_src >= +1 ) return max_int_as_f;
        f_SRC_TYPE const non_int_result = f_src * max_int_as_f;
        return static_cast< s_TRG_TYPE >( std::floor( non_int_result + 0.5));
      }
  };

  // ---------------------------------------------------------------
  // specialization when src is float/double and trg is unsigned int
  template
   <  typename u_TRG_TYPE
    , typename f_SRC_TYPE
    , bool     IS_NARROWING
    , bool     IS_WIDENING
   >
  struct
convert_ffp_helper
 <  u_TRG_TYPE, f_SRC_TYPE
  , true , false  /* trg is unsigned integer */
  , false, true   /* src is float/double */
  , IS_NARROWING, IS_WIDENING
 >
  {
    d_static_assert(   std::numeric_limits< u_TRG_TYPE >::is_integer); // trg is integer
    d_static_assert( ! std::numeric_limits< u_TRG_TYPE >::is_signed ); // trg is unsigned
    d_static_assert( ! std::numeric_limits< f_SRC_TYPE >::is_integer); // src is float/double

    d_static_assert( (boost::integer_traits< u_TRG_TYPE >::const_min) == 0);

      static u_TRG_TYPE
    eval( f_SRC_TYPE const & f_src)
      { f_SRC_TYPE const max_int_as_f = boost::integer_traits< u_TRG_TYPE >::const_max;
        if ( f_src <=  0 ) return 0;
        if ( f_src >= +1 ) return max_int_as_f;
        f_SRC_TYPE const non_int_result = f_src * max_int_as_f;
        return static_cast< u_TRG_TYPE >( std::floor( non_int_result + 0.5));
      }
  };

  // ---------------------------------------------------------------
  // specialization when src is signed integer and trg is float/double
  template
   <  typename f_TRG_TYPE
    , typename s_SRC_TYPE
    , bool     IS_NARROWING
    , bool     IS_WIDENING
   >
  struct
convert_ffp_helper
 <  f_TRG_TYPE, s_SRC_TYPE
  , false, true  /* trg is float/double */
  , true , true  /* src is signed int */
  , IS_NARROWING, IS_WIDENING
 >
  {
    d_static_assert( ! std::numeric_limits< f_TRG_TYPE >::is_integer); // trg is float/double
    d_static_assert(   std::numeric_limits< s_SRC_TYPE >::is_integer); // src is integer
    d_static_assert(   std::numeric_limits< s_SRC_TYPE >::is_signed ); // src is signed

    d_static_assert( (boost::integer_traits< s_SRC_TYPE >::const_min) < 0);
    d_static_assert( ((boost::integer_traits< s_SRC_TYPE >::const_min) & 1) == 0); /* const_min is even */

      static f_TRG_TYPE
    eval( s_SRC_TYPE const & s_src)
      { f_TRG_TYPE const min_int_as_f   = boost::integer_traits< s_SRC_TYPE >::const_min;
        f_TRG_TYPE const max_int_as_f   = boost::integer_traits< s_SRC_TYPE >::const_max;
        f_TRG_TYPE const two_over_range = 2 / (max_int_as_f - min_int_as_f);
        f_TRG_TYPE const f_src          = s_src; /* convert src to float/double */
        return (two_over_range * (f_src - min_int_as_f)) - 1;
      }
  };

  // ---------------------------------------------------------------
  // specialization when src is unsigned integer and trg is float/double
  template
   <  typename f_TRG_TYPE
    , typename u_SRC_TYPE
    , bool     IS_NARROWING
    , bool     IS_WIDENING
   >
  struct
convert_ffp_helper
 <  f_TRG_TYPE, u_SRC_TYPE
  , false, true   /* trg is float/double */
  , true , false  /* src is unsigned int */
  , IS_NARROWING, IS_WIDENING
 >
  {
    d_static_assert( ! std::numeric_limits< f_TRG_TYPE >::is_integer); // trg is float/double
    d_static_assert(   std::numeric_limits< u_SRC_TYPE >::is_integer); // src is integer
    d_static_assert( ! std::numeric_limits< u_SRC_TYPE >::is_signed ); // src is unsigned

    d_static_assert( (boost::integer_traits< u_SRC_TYPE >::const_min) == 0);

      static f_TRG_TYPE
    eval( u_SRC_TYPE const & u_src)
      { f_TRG_TYPE const max_int_as_f = boost::integer_traits< u_SRC_TYPE >::const_max;
        f_TRG_TYPE const f_src        = u_src; /* convert src to float/double */
        return f_src / max_int_as_f;
      }
  };

  // ---------------------------------------------------------------
  // specialization when src/trg are both either signed or unsigned integer, and trg is narrower than src
  template
   <  typename i_TRG_TYPE
    , typename i_SRC_TYPE
    , bool     IS_SIGNED
   >
  struct
convert_ffp_helper
 <  i_TRG_TYPE, i_SRC_TYPE
  , true, IS_SIGNED  /* trg is int */
  , true, IS_SIGNED  /* src is int */
  , true, false      /* is narrowing */
 >
  {
    // For unsigned ints, example counts are: 8, 16, 32.
    // For signed ints, example counts are: 7, 15, 31.
    static int const  trg_bit_count  = std::numeric_limits< i_TRG_TYPE >::digits;
    static int const  src_bit_count  = std::numeric_limits< i_SRC_TYPE >::digits;
    d_static_assert( src_bit_count > trg_bit_count); /* narrowing means src is wider than trg */
    static int const  shift_count    = src_bit_count - trg_bit_count;
    d_static_assert( (0 < shift_count) && (shift_count < src_bit_count));
    d_static_assert( (shift_count == 8) || (shift_count == 16) || (shift_count == 24));

    // Note. This will also work (except for the asserts) to narrow unsigned->signed.

    // Note. This right-shifts a signed integer. This is considered unsafe (for negative signed ints)
    // because the standard doesn't specify whether you should use an arithmetic or logical shift
    // (whether or not you extend the sign bit). However, I think all (?) C++ compilers use arithmetic
    // shift. And the following code should work with either kind of shift, even for signed negatives.
    // And there is a d_assert(..) that will tell us if we're wrong (although not a compile time).

      static i_TRG_TYPE
    eval( i_SRC_TYPE const & i_src)
      { /* truncate is appropriate here */
        i_SRC_TYPE const result_src = (i_src >> shift_count); /* move the bits we want to the lo end */
        i_TRG_TYPE const result_trg = static_cast< i_TRG_TYPE >( result_src); /* only take the lo bits */
        // The following assert will fail if right-shift for negative signed numbers does not sign extend,
        // although the answer should still be correct.
        d_assert( static_cast< i_SRC_TYPE >( result_trg) == result_src);
        return result_trg;
      }
  };

  // ---------------------------------------------------------------
  // specialization to narrow unsigned -> signed
  template
   <  typename s_TRG_TYPE
    , typename u_SRC_TYPE
   >
  struct
convert_ffp_helper
 <  s_TRG_TYPE, u_SRC_TYPE
  , true, true   /* trg is signed int */
  , true, false  /* src is unsigned int */
  , true, false  /* is narrowing */
 >
  {
    // This is the same as the specialization above, except the asserts.
    static int const  trg_bit_count  = std::numeric_limits< s_TRG_TYPE >::digits;
    static int const  src_bit_count  = std::numeric_limits< u_SRC_TYPE >::digits;
    d_static_assert( src_bit_count > trg_bit_count); /* narrowing means src is wider than trg */
    static int const  shift_count    = src_bit_count - trg_bit_count;
    d_static_assert( (0 < shift_count) && (shift_count < src_bit_count));
    d_static_assert( (shift_count == 1) || (shift_count == 9) || (shift_count == 17) || (shift_count == 25));

      static s_TRG_TYPE
    eval( u_SRC_TYPE const & u_src)
      { /* truncate is appropriate here */
        u_SRC_TYPE const result_src = (u_src >> shift_count); /* move the bits we want to the lo end */
        s_TRG_TYPE const result_trg = static_cast< s_TRG_TYPE >( result_src); /* only take the lo bits */
        d_assert( result_trg >= 0);
        d_assert( static_cast< u_SRC_TYPE >( result_trg) == result_src);
        return result_trg;
      }
  };

  // ---------------------------------------------------------------
  // specialization to narrow signed -> unsigned
  template
   <  typename u_TRG_TYPE
    , typename s_SRC_TYPE
   >
  struct
convert_ffp_helper
 <  u_TRG_TYPE, s_SRC_TYPE
  , true, false  /* trg is unsigned int */
  , true, true   /* src is signed int */
  , true, false  /* is narrowing */
 >
  {
    static int const  trg_bit_count  = std::numeric_limits< u_TRG_TYPE >::digits;
    static int const  src_bit_count  = std::numeric_limits< s_SRC_TYPE >::digits;
    d_static_assert( src_bit_count > trg_bit_count); /* narrowing means src is wider than trg */
    static int const  shift_count    = src_bit_count - trg_bit_count;
    d_static_assert( (0 < shift_count) && (shift_count < src_bit_count));
    d_static_assert( (shift_count == 7) || (shift_count == 15) || (shift_count == 23));

    // Note. This right-shifts a signed integer, but only if it's non-negative.
    // See the longer note above about right-shifting signed integers.

      static u_TRG_TYPE
    eval( s_SRC_TYPE const & s_src)
      {
        if ( s_src < 0 ) return 0;
        s_SRC_TYPE const result_src = (s_src >> shift_count); /* move the bits we want to the lo end */
        u_TRG_TYPE const result_trg = static_cast< u_TRG_TYPE >( result_src); /* only take the lo bits */
        d_assert( static_cast< s_SRC_TYPE >( result_trg) == result_src);
        return result_trg;
      }
  };

  // ---------------------------------------------------------------
  // specialization when src/trg are both either signed or unsigned integer, and trg is wider than src
  template
   <  typename i_TRG_TYPE
    , typename i_SRC_TYPE
    , bool     IS_SIGNED
   >
  struct
convert_ffp_helper
 <  i_TRG_TYPE, i_SRC_TYPE
  , true, IS_SIGNED  /* trg is int */
  , true, IS_SIGNED  /* src is int */
  , false, true      /* is widening */
 >
  {
    // For unsigned ints, example counts are: 8, 16, 32.
    // For signed ints, example counts are: 7, 15, 31.
    static int const  trg_bit_count  = std::numeric_limits< i_TRG_TYPE >::digits;
    static int const  src_bit_count  = std::numeric_limits< i_SRC_TYPE >::digits;
    d_static_assert( src_bit_count < trg_bit_count); /* widening means trg is wider than src */
    static int const  shift_count    = trg_bit_count - src_bit_count;
    d_static_assert( (0 < shift_count) && (shift_count < trg_bit_count));
    d_static_assert( (shift_count == 8) || (shift_count == 16) || (shift_count == 24));

    // Note. This will also work (except for the asserts) to widen unsigned->signed.
    // Note. This left-shifts a signed integer. Some style guides say to only shift unsigned types.

      static i_TRG_TYPE
    eval( i_SRC_TYPE const & i_src)
      {
        i_TRG_TYPE result_trg = i_src; /* widen */
        result_trg <<= shift_count; /* move the bits to the hi end */
        // At this point all the lo bits are zero. This means that 0xff->0xff00, or 1 maps to a
        // number slightly less than 1 (although 0->0 and (-1)->(-1)). 0xff->0xffff would be better.
        // Solutions:
        //  - Make the first bit shifted in 1 instead of zero. This would place the result
        //    in the middle of the target range, but 1 still maps to slightly less.
        //  - Shift in ones, maybe only if the src is positive and > half_max.
        //  - Copy the hi bits into the lo bits as you shift them. This also works for zero, but
        //    not for -1. Perhaps for a negative start the shift after the sign bit.
        return result_trg;
      }
  };

  // ---------------------------------------------------------------
  // specialization to widen unsigned -> signed
  template
   <  typename s_TRG_TYPE
    , typename u_SRC_TYPE
   >
  struct
convert_ffp_helper
 <  s_TRG_TYPE, u_SRC_TYPE
  , true, true   /* trg is signed int */
  , true, false  /* src is unsigned int */
  , false, true  /* is widening */
 >
  {
    // This is the same as the specialization above, except the asserts.
    static int const  trg_bit_count  = std::numeric_limits< s_TRG_TYPE >::digits;
    static int const  src_bit_count  = std::numeric_limits< u_SRC_TYPE >::digits;
    d_static_assert( src_bit_count < trg_bit_count); /* widening means trg is wider than src */
    static int const  shift_count    = trg_bit_count - src_bit_count;
    d_static_assert( (0 < shift_count) && (shift_count < trg_bit_count));
    d_static_assert( (shift_count == 7) || (shift_count == 15) || (shift_count == 23));

    // Note. This left-shifts a signed integer.

      static s_TRG_TYPE
    eval( u_SRC_TYPE const & u_src)
      {
        s_TRG_TYPE result_trg = u_src; /* widen */
        d_assert( result_trg >= 0);
        result_trg <<= shift_count; /* move the bits to the hi end */
        d_assert( result_trg >= 0);
        return result_trg;
      }
  };

  // ---------------------------------------------------------------
  // specialization to widen signed -> unsigned
  template
   <  typename u_TRG_TYPE
    , typename s_SRC_TYPE
   >
  struct
convert_ffp_helper
 <  u_TRG_TYPE, s_SRC_TYPE
  , true, false  /* trg is unsigned int */
  , true, true   /* src is signed int */
  , false, true  /* is widening */
 >
  {
    static int const  trg_bit_count  = std::numeric_limits< u_TRG_TYPE >::digits;
    static int const  src_bit_count  = std::numeric_limits< s_SRC_TYPE >::digits;
    d_static_assert( src_bit_count < trg_bit_count); /* widening means trg is wider than src */
    static int const  shift_count    = trg_bit_count - src_bit_count;
    d_static_assert( (0 < shift_count) && (shift_count < trg_bit_count));
    d_static_assert( (shift_count == 1) || (shift_count == 9) || (shift_count == 17) || (shift_count == 25));

      static u_TRG_TYPE
    eval( s_SRC_TYPE const & s_src)
      {
        if ( s_src < 0 ) return 0; /* unsigned cannot represent negatives */
        u_TRG_TYPE result_trg = s_src; /* widen */
        result_trg <<= shift_count; /* move the bits to the hi end */
        return result_trg;
      }
  };

// ---------------------------------------------------------------------------------------------

  template< typename TRG_TYPE, typename SRC_TYPE >
  TRG_TYPE
convert_fractional_fixed_point_to( SRC_TYPE const & src)
  { return convert_ffp_helper< TRG_TYPE, SRC_TYPE >::eval( src); }

// _______________________________________________________________________________________________
//
} /* end namespace gl_env */
// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef GL_ENV_FRACTIONAL_FIXED_POINT_H
//
// gl_env_fractional_fixed_point.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
