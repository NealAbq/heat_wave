// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// all.h
//
//   Copyright Neal Binnendyk 2009, 2010. nealabq@gmail.com. nealabq.com
//
//   == GPL License Notice ====================================================================|
//   This code is free software: you can redistribute it and/or modify it under the terms      |
//   of the GNU General Public License as published by the Free Software Foundation, either    |
//   version 3 of the License, or (at your option) any later version.                          |
//                                                                                             |
//   This code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;    |
//   without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. |
//   See the GNU General Public License for more details: <http://www.gnu.org/licenses/>       |
//   == END License Notice ====================================================================|
//
# pragma once
# ifndef ALL_H
# define ALL_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "debug.h"

# include <cstdlib>
# include <cstddef>
# include <cstring>
# include <cmath>

# include <limits>
# include <boost/integer_traits.hpp>
// boost::integer_traits< int >::const_min
// boost::integer_traits< int >::const_max

# include <boost/cstdint.hpp>
using boost::uint8_t;
using boost::uint16_t;
using boost::uint32_t;
using boost::uint64_t;

using boost::int8_t;
using boost::int16_t;
using boost::int32_t;
using boost::int64_t;

# include <boost/utility/swap.hpp>
# include <boost/functional/hash.hpp>

# include <boost/type_traits.hpp>
# include <boost/mpl/if.hpp>
# include <boost/mpl/and.hpp>
# include <boost/mpl/or.hpp>

# include <boost/tuple/tuple.hpp>
using boost::tuples::tuple;
using boost::tuples::get;

// _______________________________________________________________________________________________

// General macro utility.
// Always use X_CONCAT..(..) with only simple, one-token params.
// It's simpler to just always use CONCAT..(..), below.
# define X_CONCAT_2( A, B)           A ## B
# define X_CONCAT_3( A, B, C)        A ## B ## C
# define X_CONCAT_4( A, B, C, D)     A ## B ## C ## D
# define X_CONCAT_5( A, B, C, D, E)  A ## B ## C ## D ## E

// Use CONCAT..(..) to force parameter evaluation.
// Params are fully eval'ed ahead of time so you don't have to restrict yourself to
// simple one-token params.
# define CONCAT_2( A, B)             X_CONCAT_2( A, B)
# define CONCAT_3( A, B, C)          X_CONCAT_3( A, B, C)
# define CONCAT_4( A, B, C, D)       X_CONCAT_4( A, B, C, D)
# define CONCAT_5( A, B, C, D, E)    X_CONCAT_5( A, B, C, D, E)

// _______________________________________________________________________________________________

  template< typename T >
  inline
  void
maybe_unused( T const & x)
  { (void) x; }

# ifdef NDEBUG
#   define maybe_used_only_for_debug( x)  maybe_unused( x)
# else
#   define maybe_used_only_for_debug( x)  ((void)0)
# endif

// _______________________________________________________________________________________________

  inline
  bool
implies( bool a, bool b)
  { return (! a) || b; }

// _______________________________________________________________________________________________

  template< typename TRG_T, typename SRC_T >
  inline
  TRG_T
implicit_cast( SRC_T p)
  { return p; }

// _______________________________________________________________________________________________
//
// Universal compile-time tags.
//   These should be declared their own header file.

namespace tag {

// "not-init" is sort-of the same as "reset".
// But some kinds of objects have two distinct states. Other reset states:
//   not-init
//   is in initial state (is init)
//   is reset
//   is zero
//   is clear, is ground state
//   is null, is blank, is black, is white, is virgin
//   is default
//   is empty
//   is partially reset
//   is fully reset

// Class (and ooak object) used as param to ctors where you do not want to go thru the usual initialization.
struct do_not_init_type { };
//static do_not_init_type const do_not_init;

struct reset_type { };
//static reset_type const reset;
// We do not need these insts. Instead of tag::reset just say tag::reset_type( ).

} /* end namespace tag */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef ALL_H
//
// all.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
