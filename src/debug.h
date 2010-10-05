// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// debug.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
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
