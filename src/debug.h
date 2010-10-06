// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// debug.h
//
//   Copyright (c) Neal Binnendyk 2009, 2010. <nealabq@gmail.com> nealabq.com
//
//   |=== GPL License Notice ====================================================================|
//   | This code is free software: you can redistribute it and/or modify it under the terms      |
//   | of the GNU General Public License as published by the Free Software Foundation, either    |
//   | version 3 of the License, or (at your option) any later version.                          |
//   |                                                                                           |
//   | This code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;    |
//   | without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. |
//   | See the GNU General Public License for more details: <http://www.gnu.org/licenses/>       |
//   |=== END License Notice ====================================================================|
//
# pragma once
//
// Use a complicated name in the next line because DEBUG_H looks too much like DEBUG or NDEBUG.
# ifndef DEBUG_H_included
# define DEBUG_H_included
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# ifdef NDEBUG
#   define BOOST_DISABLE_ASSERTS
# endif

# include <boost/assert.hpp>

# ifdef NDEBUG
#   define d_assert BOOST_ASSERT
#   define d_verify BOOST_VERIFY
# else
      inline
      void
    d_assert( bool is_ok)
    {
      if ( ! is_ok ) {
        BOOST_ASSERT( false);
      }
    }
#   define d_verify d_assert
# endif

// -------------------------------------------------------------------------------

# include <boost/static_assert.hpp>
# include <boost/mpl/assert.hpp>

# define d_static_assert BOOST_STATIC_ASSERT

  template< typename T >
  T *
d_verify_not_zero( T * p)
  {
    d_assert( p != 0);
    return p;
  }

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef DEBUG_H_included
//
// debug.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
