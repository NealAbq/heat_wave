// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// finite_diff.h
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
# ifndef FINITE_DIFF_H
# define FINITE_DIFF_H
// _______________________________________________________________________________________________
// Notes
//
//   For 1D forward diff, a rate of 0.3333 is the same as a moving average and is probably a
//   good/fast value to pick. Although values up to (but not including) 0.5 seem to be stable in
//   most cases, although you end up with some oscillation. Think of the 3 cell [0,1,0] case,
//   where any value >0.333 will cause damped oscillation.
//
//   For 2D forward diff, a rate of 0.2 makes sense since a cell is assigned a weighted average
//   of 5 cells.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include <iterator>
# include <algorithm>
# include "tri_diag.h"

// This #define is here so we can experiment with solves will rates like -1 or 2, that blow up.
// This is also defined in other files.
# define ASSERT_REASONABLE_RATES 0

namespace finite_difference {

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// assign3_functor_type
//
//   These are used for the calc_forward_diff..(..) functions below.
//   The backward and central-diff solvers use these assign2_functor_types instead:
//     util::assign_set_type
//     util::assign_sum_type

  template< typename ITEM_TYPE >
  struct
assign3_set_type
  //
  // Use this when damping==1 or when you want a heat/diffusion equation without a wave component.
{
    void operator ()( ITEM_TYPE & trg, ITEM_TYPE const & /* src */, ITEM_TYPE const & side)
      { trg = side; }
};

  template< typename ITEM_TYPE >
  struct
assign3_src_minus_trg_type
  //
  // Use this when damping==0.
{
    void operator ()( ITEM_TYPE & trg, ITEM_TYPE const & src, ITEM_TYPE const & side)
      { trg = side + src - trg; }
};

  template< typename ITEM_TYPE, typename RATE_TYPE >
  struct
assign3_damping_type
  //
  // Use this when damping is some value other than 0 or 1.
{
    void operator ()( ITEM_TYPE & trg, ITEM_TYPE const & src, ITEM_TYPE const & side)
      { trg = side + (one_minus_damp_ * (src - trg)); }

    assign3_damping_type( RATE_TYPE const & damp)
      : one_minus_damp_( static_cast< RATE_TYPE >( 1) - damp) { }
      RATE_TYPE const one_minus_damp_;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Damping function to use with backward and central diff

  template< typename RATE_TYPE >
  RATE_TYPE const &
get_no_init_damping_set_value( )
{
    // Choose some unlikely and very illegal value.
    // If the user sends in this value, use the util::assign_set_type( ) assign functor.

    // Normally (0 <= damping) and (damping <= 1), although we may allow some values outside those
    // limits just to see what happens. But not very far outside those limits.
    static RATE_TYPE const no_damping_value = static_cast< RATE_TYPE >( 126);
    return no_damping_value;
}

  template< typename RATE_TYPE >
  RATE_TYPE const &
get_no_init_damping_sum_value( )
{
    // Choose some unlikely and very illegal value.
    // If the user sends in this value, use the util::assign_sum_type( ) assign functor.
    static RATE_TYPE const no_damping_value = static_cast< RATE_TYPE >( 127);
    return no_damping_value;
}

// _______________________________________________________________________________________________

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  void
init_wave_damping
 (  RATE_TYPE      damping
  , size_t         count
  , SRC_ITER_TYPE  src_iter
  , TRG_ITER_TYPE  trg_iter
 )
{
    d_assert( damping != get_no_init_damping_set_value< RATE_TYPE >( ));
    d_assert( damping != get_no_init_damping_sum_value< RATE_TYPE >( ));

    if ( damping == 0 ) {
        for ( ; count ; -- count ) {
            *trg_iter = - *trg_iter;
            ++ trg_iter;
        }
    } else
    if ( damping == 1 ) {
        for ( ; count ; -- count ) {
            *trg_iter = - *src_iter;
            ++ src_iter;
            ++ trg_iter;
        }
    } else {
        for ( ; count ; -- count ) {
            *trg_iter = (damping * (*trg_iter - *src_iter)) - *trg_iter;
            ++ src_iter;
            ++ trg_iter;
        }
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// calc_forward_diff_..(..)

// _______________________________________________________________________________________________
// calc_forward_diff_2d_middle_
//  (  assign3_functor
//   , rate, rate_side
//   , src_iter, src_iter_limit
//   , src_iter_side_a, src_iter_side_b
//   , trg_iter
//  )

  template
   <  typename ASSIGN3_FUNCTOR_TYPE
    , typename RATE_TYPE        // double, float
    , typename SRC_ITER_TYPE    // std::vector< float >::const_iterator
    , typename TRG_ITER_TYPE    // std::vector< float >::iterator
   >
  void
calc_forward_diff_2d_middle_
 (  ASSIGN3_FUNCTOR_TYPE  assign3_functor
  , RATE_TYPE const       rate
  , RATE_TYPE const       rate_side
  , SRC_ITER_TYPE         src_iter         // previous state, not changed, can be same as trg_iter
  , SRC_ITER_TYPE         src_iter_limit   // one past the end
  , SRC_ITER_TYPE         src_iter_side_a  // previous state, not changed, next to src iter
  , SRC_ITER_TYPE         src_iter_side_b  // previous state, not changed, next to src iter
  , TRG_ITER_TYPE         trg_iter         // result, same size as src
 )
{
    d_assert( src_iter <= src_iter_limit);
    // OK if (src_iter == trg_iter).
    // No, maybe it's not. It's OK for this one pass, but if you were doing these row-by-row
    // you'd be changing the current row (trg) and then using that row as input when calc'ing
    // the next row.

    typedef typename std::iterator_traits< TRG_ITER_TYPE >::value_type item_type;
    RATE_TYPE const base = 1;

    if ( src_iter < src_iter_limit ) {
        RATE_TYPE const carry_edge    = base - (rate + rate_side + rate_side);
        RATE_TYPE const carry_middle  = carry_edge - rate;

        // We are going to walk over a 3-wide window of src values [src_0, src_1, src_2].
        item_type src_0 = *src_iter;
        ++ src_iter;

        item_type side_contrib = (*src_iter_side_a + *src_iter_side_b) * rate_side;
        ++ src_iter_side_a;
        ++ src_iter_side_b;

        // If the src has only one cell we should just copy to the trg.
        // This does not leak heat off the edges.
        if ( src_iter == src_iter_limit ) {
            assign3_functor( *trg_iter, src_0, src_0 + side_contrib);
        }
        else {
            d_assert( src_iter < src_iter_limit);

            // Get src_1, the 2nd value in the window.
            item_type src_1 = *src_iter; /* ITEM_VAL_TYPE */
            ++ src_iter;

            // Calculate the lo end. This does not leak heat off the lo edge.
            assign3_functor( *trg_iter, src_0,
                (carry_edge * src_0) + (rate * src_1) + side_contrib);

            // This works if the src has only two cells.
            while ( src_iter != src_iter_limit ) {
                // Get the third value in the src window.
                item_type const src_2 = *src_iter; /* ITEM_VAL_TYPE */
                ++ src_iter;

                // Keep side_contrib aligned with src_1, and one behind src_2.
                side_contrib = (*src_iter_side_a + *src_iter_side_b) * rate_side;
                ++ src_iter_side_a;
                ++ src_iter_side_b;

                // Move trg_iter so it is across from src_1.
                ++ trg_iter; /* trg_0 -> trg_1 */
                // Calculate the target value. This only assigns the middle, not the edges.
                assign3_functor( *trg_iter, src_1,
                    (carry_middle * src_1) + (rate * (src_0 + src_2)) + side_contrib);

                // Remember the tail values before we move forward.
                src_0 = src_1;
                src_1 = src_2;
                // trg_iter is again across from src_0.
            }

            // At this point:
            //   src_0 is the 2nd-to-last value in the src.
            //   src_1 is the last value in the src.
            //   trg_iter points to the 2nd-to-last trg.
            //   side_contrib has the 2nd-to-last contribution (aligned with src_0).
            //   src_iter_side_a and src_iter_side_b point to the last side values.

            // Align side_contrib with src_1.
            side_contrib = (*src_iter_side_a + *src_iter_side_b) * rate_side;

            // Move trg_iter to point to the last location in trg.
            ++ trg_iter; // trg_0 -> trg_1

            // Calculate the hi end (the last trg). This does not leak heat off the hi edge.
            assign3_functor( *trg_iter, src_1,
                (carry_edge * src_1) + (rate * src_0) + side_contrib);
        }
    }
}

// _______________________________________________________________________________________________
// calc_forward_diff_2d_edge_
//  (  assign3_functor
//   , rate, rate_side
//   , src_iter, src_iter_limit
//   , src_iter_side
//   , trg_iter
//  )

  template
   <  typename ASSIGN3_FUNCTOR_TYPE
    , typename RATE_TYPE        // double, float
    , typename SRC_ITER_TYPE    // std::vector< float >::const_iterator
    , typename TRG_ITER_TYPE    // std::vector< float >::iterator
   >
  void
calc_forward_diff_2d_edge_
 (  ASSIGN3_FUNCTOR_TYPE  assign3_functor
  , RATE_TYPE const       rate
  , RATE_TYPE const       rate_side
  , SRC_ITER_TYPE         src_iter        // previous state, not changed, can be same as trg_iter
  , SRC_ITER_TYPE         src_iter_limit  // one past the end
  , SRC_ITER_TYPE         src_iter_side   // previous state, not changed, next to src iter
  , TRG_ITER_TYPE         trg_iter        // result, same size as src
 )
{
    d_assert( src_iter <= src_iter_limit);
    // OK if (src_iter == trg_iter).

    typedef typename std::iterator_traits< TRG_ITER_TYPE >::value_type item_type;
    RATE_TYPE const base = 1;

    if ( src_iter < src_iter_limit ) {
        RATE_TYPE const carry_edge   = base - (rate + rate_side);
        RATE_TYPE const carry_middle = carry_edge - rate;

        // We are going to walk over a 3-wide window of src values [src_0, src_1, src_2].
        item_type src_0 = *src_iter; /* ITEM_VAL_TYPE */
        ++ src_iter;

        item_type side_contrib = (*src_iter_side) * rate_side;
        ++ src_iter_side;

        // If the src has only one cell we should just copy to the trg.
        // This does not leak heat off the edges.
        if ( src_iter == src_iter_limit ) {
            assign3_functor( *trg_iter, src_0, src_0 + side_contrib);
        }
        else {
            d_assert( src_iter < src_iter_limit);

            // Get src_1, the 2nd value in the window.
            item_type src_1 = *src_iter; /* ITEM_VAL_TYPE */
            ++ src_iter;

            // Calculate the lo end. This does not leak heat off the lo edge.
            assign3_functor( *trg_iter, src_0,
                (carry_edge * src_0) + (rate * src_1) + side_contrib);

            // This works if the src has only two cells.
            while ( src_iter != src_iter_limit ) {
                // Get the third value in the src window.
                item_type const src_2 = *src_iter; /* ITEM_VAL_TYPE */
                ++ src_iter;

                // Keep side_contrib aligned with src_1, and one behind src_2.
                side_contrib = (*src_iter_side) * rate_side;
                ++ src_iter_side;

                // Move trg_iter so it is across from src_1.
                ++ trg_iter; /* trg_0 -> trg_1 */
                // Calculate the target value. This only assigns the middle, not the edges.
                assign3_functor( *trg_iter, src_1,
                    (carry_middle * src_1) + (rate * (src_0 + src_2)) + side_contrib);

                // Remember the tail values before we move forward.
                src_0 = src_1;
                src_1 = src_2;
                // trg_iter is again across from src_0.
            }

            // At this point:
            //   src_0 is the 2nd-to-last value in the src.
            //   src_1 is the last value in the src.
            //   trg_iter points to the 2nd-to-last trg.
            //   side_contrib has the 2nd-to-last contribution (aligned with src_0).
            //   src_iter_side points to the last side value.

            // Align side_contrib with src_1.
            side_contrib = (*src_iter_side) * rate_side;

            // Move trg_iter to point to the last location in trg.
            ++ trg_iter; // trg_0 -> trg_1

            // Calculate the hi end (the last trg). This does not leak heat off the hi edge.
            assign3_functor( *trg_iter, src_1,
                (carry_edge * src_1) + (rate * src_0) + side_contrib);
        }
    }
}

// _______________________________________________________________________________________________
// calc_forward_diff_thin_strip_
//  (  assign3_functor
//   , base
//   , rate
//   , src_iter, src_iter_limit
//   , trg_iter
//  )

  template
   <  typename ASSIGN3_FUNCTOR_TYPE
    , typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  void
calc_forward_diff_thin_strip_
 (  ASSIGN3_FUNCTOR_TYPE  assign3_functor
  , RATE_TYPE const       base
  , RATE_TYPE const       rate
  , SRC_ITER_TYPE         src_iter        // not changed, can be same as trg_iter
  , SRC_ITER_TYPE         src_iter_limit  // one past the end
  , TRG_ITER_TYPE         trg_iter        // result, same size as src
 )
{
    d_assert( src_iter <= src_iter_limit);
    // OK if (src_iter == trg_iter).

    typedef typename std::iterator_traits< TRG_ITER_TYPE >::value_type item_type;

    if ( src_iter < src_iter_limit ) {
        // (b -  r) - carry_edge is used for the first and last cell.
        // (b - 2r) - carry_middle is used for all the cells in the middle.
        RATE_TYPE const carry_edge   = base - rate;
        RATE_TYPE const carry_middle = carry_edge - rate;

#       if ASSERT_REASONABLE_RATES
        d_assert( carry_middle >= 0);
        d_assert( carry_middle < carry_edge);
#       endif

        // We are going to walk over a 3-wide window of src values [src_0, src_1, src_2].
        item_type src_0 = *src_iter; /* ITEM_VAL_TYPE */
        ++ src_iter;

        // If the src has only one cell we should just copy to the trg.
        // This does not leak heat off the edges.
        if ( src_iter == src_iter_limit ) {
            assign3_functor( *trg_iter, src_0, src_0);
        }
        else {
            d_assert( src_iter < src_iter_limit);

            // Get src_1, the 2nd value in the window.
            item_type src_1 = *src_iter; /* ITEM_VAL_TYPE */
            ++ src_iter;

            // Calculate the lo end. This does not leak heat off the lo edge.
            assign3_functor( *trg_iter, src_0, (carry_edge * src_0) + (rate * src_1));

            // This works if the src has only two cells.
            while ( src_iter != src_iter_limit ) {
                // Get the third value in the src window.
                item_type const src_2 = *src_iter; /* ITEM_VAL_TYPE */
                ++ src_iter;

                // Move trg_iter so it is across from src_1.
                ++ trg_iter; /* trg_0 -> trg_1 */
                // Calculate the target value. This only assigns the middle, not the edges.
                assign3_functor( *trg_iter, src_1, (carry_middle * src_1) + (rate * (src_0 + src_2)));

                // Remember the tail values before we move forward.
                src_0 = src_1;
                src_1 = src_2;
                // trg_iter is again across from src_0.
            }

            // At this point:
            //   src_0 is the 2nd-to-last value in the src.
            //   src_1 is the last value in the src.
            //   trg_iter points to the 2nd-to-last trg.

            // Move trg_iter to point to the last location in trg.
            ++ trg_iter; // trg_0 -> trg_1

            // Calculate the hi end (the last trg). This does not leak heat off the hi edge.
            assign3_functor( *trg_iter, src_1, (carry_edge * src_1) + (rate * src_0));
        }
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// calc_next_generation_forward_difference_2d_..(..)

// _______________________________________________________________________________________________
// calc_next_generation_forward_difference_2d_middle
//  (  damping
//   , rate, rate_side
//   , src_iter, src_iter_limit
//   , src_iter_side_a, src_iter_side_b
//   , trg_iter
//  )

  template
   <  typename RATE_TYPE        // double, float
    , typename SRC_ITER_TYPE    // std::vector< float >::const_iterator
    , typename TRG_ITER_TYPE    // std::vector< float >::iterator
   >
  void
calc_next_generation_forward_difference_2d_middle
 (  RATE_TYPE     const    damping
  , RATE_TYPE     const    rate
  , RATE_TYPE     const    rate_side
  , SRC_ITER_TYPE const &  src_iter         // previous state, not changed, can be same as trg_iter
  , SRC_ITER_TYPE const &  src_iter_limit   // one past the end
  , SRC_ITER_TYPE const &  src_iter_side_a  // previous state, not changed, next to src iter
  , SRC_ITER_TYPE const &  src_iter_side_b  // previous state, not changed, next to src iter
  , TRG_ITER_TYPE const &  trg_iter         // result, same size as src
 )
{
    d_assert( get_no_init_damping_set_value< RATE_TYPE >( ) != damping);
    d_assert( get_no_init_damping_sum_value< RATE_TYPE >( ) != damping);

    typedef typename std::iterator_traits< TRG_ITER_TYPE >::value_type item_type;

    if ( damping == 0 ) {
        calc_forward_diff_2d_middle_
         (  assign3_src_minus_trg_type< item_type >( )
          , rate, rate_side
          , src_iter, src_iter_limit
          , src_iter_side_a
          , src_iter_side_b
          , trg_iter
         );
    } else
    if ( damping == 1 ) {
        calc_forward_diff_2d_middle_
         (  assign3_set_type< item_type >( )
          , rate, rate_side
          , src_iter, src_iter_limit
          , src_iter_side_a
          , src_iter_side_b
          , trg_iter
         );
    } else {
        calc_forward_diff_2d_middle_
         (  assign3_damping_type< item_type, RATE_TYPE >( damping)
          , rate, rate_side
          , src_iter, src_iter_limit
          , src_iter_side_a
          , src_iter_side_b
          , trg_iter
         );
    }
}

// _______________________________________________________________________________________________
// calc_next_generation_forward_difference_2d_edge
//  (  damping, rate, rate_side
//   , src_iter, src_iter_limit
//   , src_iter_side
//   , trg_iter
//  )

  template
   <  typename RATE_TYPE        // double, float
    , typename SRC_ITER_TYPE    // std::vector< float >::const_iterator
    , typename TRG_ITER_TYPE    // std::vector< float >::iterator
   >
  void
calc_next_generation_forward_difference_2d_edge
 (  RATE_TYPE     const    damping
  , RATE_TYPE     const    rate
  , RATE_TYPE     const    rate_side
  , SRC_ITER_TYPE const &  src_iter        // previous state, not changed, can be same as trg_iter
  , SRC_ITER_TYPE const &  src_iter_limit  // one past the end
  , SRC_ITER_TYPE const &  src_iter_side   // previous state, not changed, next to src iter
  , TRG_ITER_TYPE const &  trg_iter        // result, same size as src
 )
{
    d_assert( get_no_init_damping_set_value< RATE_TYPE >( ) != damping);
    d_assert( get_no_init_damping_sum_value< RATE_TYPE >( ) != damping);

    typedef typename std::iterator_traits< TRG_ITER_TYPE >::value_type item_type;

    if ( damping == 0 ) {
        calc_forward_diff_2d_edge_
         (  assign3_src_minus_trg_type< item_type >( )
          , rate, rate_side
          , src_iter, src_iter_limit
          , src_iter_side
          , trg_iter
         );
    } else
    if ( damping == 1 ) {
        calc_forward_diff_2d_edge_
         (  assign3_set_type< item_type >( )
          , rate, rate_side
          , src_iter, src_iter_limit
          , src_iter_side
          , trg_iter
         );
    } else {
        calc_forward_diff_2d_edge_
         (  assign3_damping_type< item_type, RATE_TYPE >( damping)
          , rate, rate_side
          , src_iter, src_iter_limit
          , src_iter_side
          , trg_iter
         );
    }
}

// _______________________________________________________________________________________________
// calc_next_generation_forward_difference_2d_thin_strip
//  (  damping, rate
//   , src_iter, src_iter_limit
//   , trg_iter
//  )

  template
   <  typename RATE_TYPE        // double, float
    , typename SRC_ITER_TYPE    // std::vector< float >::const_iterator
    , typename TRG_ITER_TYPE    // std::vector< float >::iterator
   >
  void
calc_next_generation_forward_difference_2d_thin_strip
 (  RATE_TYPE     const    damping
  , RATE_TYPE     const    rate
  , SRC_ITER_TYPE const &  src_iter        // previous state, not changed, can be same as trg_iter
  , SRC_ITER_TYPE const &  src_iter_limit  // one past the end
  , TRG_ITER_TYPE const &  trg_iter        // result, same size as src
 )
{
    d_assert( get_no_init_damping_set_value< RATE_TYPE >( ) != damping);
    d_assert( get_no_init_damping_sum_value< RATE_TYPE >( ) != damping);

    typedef typename std::iterator_traits< TRG_ITER_TYPE >::value_type item_type;
    RATE_TYPE const base = 1;

    if ( damping == 0 ) {
        calc_forward_diff_thin_strip_
         (  assign3_src_minus_trg_type< item_type >( )
          , base
          , rate
          , src_iter, src_iter_limit
          , trg_iter
         );
    } else
    if ( damping == 1 ) {
        calc_forward_diff_thin_strip_
         (  assign3_set_type< item_type >( )
          , base
          , rate
          , src_iter, src_iter_limit
          , trg_iter
         );
    } else {
        calc_forward_diff_thin_strip_
         (  assign3_damping_type< item_type, RATE_TYPE >( damping)
          , base
          , rate
          , src_iter, src_iter_limit
          , trg_iter
         );
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// calc_next_generation_forward_difference_1d
//  (  rate
//   , src_iter, src_iter_limit
//   , trg_iter
//  )

  template
   <  typename RATE_TYPE      // double, float
    , typename SRC_ITER_TYPE  // std::vector< float >::const_iterator
    , typename TRG_ITER_TYPE  // std::vector< float >::iterator
   >
  void
calc_next_generation_forward_difference_1d
 (  RATE_TYPE     const    rate            // 0 < rate <= 0.5
  , SRC_ITER_TYPE const &  src_iter        // previous state
  , SRC_ITER_TYPE const &  src_iter_limit  // one past the end
  , TRG_ITER_TYPE const &  trg_iter        // result, as big as src
 )
{
    typedef typename std::iterator_traits< TRG_ITER_TYPE >::value_type item_type;

    // For each value in src, calculate a value and store it in trg (where r is the rate):
    //   trg[i] <- (1 - 2r)src[i] + r(src[i-1] + src[i+1])
    // Treat the rate values off the ends as zero, so as not to spill heat off the edges.
    //   trg[0] <- (1 - r)src[0] + r(src[1])
    //   trg[count-1] <- (1 - r)src[count-1] + r(src[count-2])
    RATE_TYPE const base = 1;
    calc_forward_diff_thin_strip_
     (  assign3_set_type< item_type >( )
      , base
      , rate
      , src_iter, src_iter_limit
      , trg_iter
     );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// calc_matrix_diagonal( base, rate, src_iter, src_iter_limit, diag_iter)

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename DIAG_ITER_TYPE
   >
  size_t /* returns count */
calc_matrix_diagonal
 (  RATE_TYPE const  base           // 1 (for backward diff) or 2 (for central diff)
  , RATE_TYPE const  rate           // 0 < rate
  , SRC_ITER_TYPE    src_iter       // previous state
  , SRC_ITER_TYPE    src_iter_limit // one past the end
  , DIAG_ITER_TYPE   diag_iter      // return, as big as (src_iter_limit - src_iter)
 )
{
    // carry_middle (1 + 2r) is all the middle values of the diagonal.
    // carry_edge (1 + r) is used at the two ends, which are floating and insulated and conserves energy.
    RATE_TYPE const carry_edge   = base + rate;
    RATE_TYPE const carry_middle = carry_edge + rate;

    // Fill in the diagonal, and count at the same time.
    // Set the first and last values in the diagonal to the edge values.
    // If we had the count we'd do this instead:
    //   if ( count > 1 ) {
    //     (*diag_iter) = carry_edge;
    //     if ( count > 2 ) {
    //       std::fill_n( diag_iter + 1, count - 2, carry_middle);
    //       (*(diag_iter + (count - 1))) = carry_edge;
    //   } }
    //
    // If rate was a vector of count-2 values (one value for each boundaries between cells),
    // we'd still use (- rate) for both the super- and sub-diagonals, and the diagonal values
    // would be:
    //   diag[ 0 ] = 1 + r[ 0 ]
    //   diag[ i ] = 1 + r[ i-1 ] + r[ i ]
    //   diag[ count-1 ] = 1 + r[ count-2 ]
    //
    // Speedup: The caller could pre-calc the diag, along with the count.
    // But we'd still have to copy the diag to a scratch buffer, so the savings is slight.
    // And remember rate should really be a vector and not a single value.
    // Of course if rate is a vector, we could pre-calculate the carry edges for every cell border
    // in the entire sheet. (Rates only apply to cell borders, not to the cells themselves.)
    size_t count = 0;
    if ( src_iter < src_iter_limit ) {
        (*diag_iter) = carry_edge;
        ++ count;
        ++ src_iter;
        ++ diag_iter;

        if ( src_iter < src_iter_limit ) {
            SRC_ITER_TYPE const src_iter_max = src_iter_limit - 1;
            while ( src_iter != src_iter_max ) {
                (*diag_iter) = carry_middle;
                ++ count;
                ++ src_iter;
                ++ diag_iter;
            }

            (*diag_iter) = carry_edge;
            ++ count;
        }
    }
    d_assert( (src_iter + 1) == src_iter_limit);
    return count;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// solve_matrix_destructive( assign2_functor, count, rate, diag, src, trg)

  template
   <  typename ASSIGN2_FUNCTOR_TYPE
    , typename RATE_TYPE
    , typename DIAG_ITER_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
   >
  void
solve_matrix_destructive
 (  ASSIGN2_FUNCTOR_TYPE    assign_functor
  , RATE_TYPE      const    rate        // negated sub- and super-diagonal
  , size_t         const    count
  , DIAG_ITER_TYPE const &  diag_iter   // matrix diagonal, values are destroyed
  , SRC_ITER_TYPE  const &  src_iter    // previous state, values are destroyed
  , TRG_ITER_TYPE  const &  trg_iter    // return value, next state
 )
{
    // Solve this as a linear equation.
    if ( rate >= 0 ) {
        // Use accurate solver which assumes the rate is reasonable.
        linear_algebra::
          solve_tridiagonal_destructive(
            assign_functor, count, - rate, diag_iter, - rate, src_iter, trg_iter);
    } else {
        // Use robust solver that won't choke on anything.
        linear_algebra::
          solve_tridiagonal_destructive_extra_careful(
            assign_functor, count, - rate, diag_iter, - rate, src_iter, trg_iter);
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// init_damping_and_solve_matrix_destructive(..)

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename TEMP_ITER_TYPE
   >
  void
init_damping_and_solve_matrix_destructive
 (  RATE_TYPE      const    damping
  , RATE_TYPE      const    rate
  , size_t         const    count
  , SRC_ITER_TYPE  const &  src_iter
  , TRG_ITER_TYPE  const &  trg_iter
  , TEMP_ITER_TYPE const &  srcX_iter
  , TEMP_ITER_TYPE const &  diagX_iter
 )
{
    typedef typename std::iterator_traits< TRG_ITER_TYPE >::value_type item_type;

    if ( get_no_init_damping_set_value< RATE_TYPE >( ) == damping ) {
        solve_matrix_destructive
         (  util::assign_set_type< item_type >( )
          , rate, count, diagX_iter, srcX_iter, trg_iter
         );
    } else {
        if ( get_no_init_damping_sum_value< RATE_TYPE >( ) != damping ) {
            init_wave_damping( damping, count, src_iter, trg_iter);
        }
        solve_matrix_destructive
         (  util::assign_sum_type< item_type >( )
          , rate, count, diagX_iter, srcX_iter, trg_iter
         );
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Backward difference

// _______________________________________________________________________________________________
// calc_next_generation_backward_difference_1d
//  (  assign2_functor
//   , rate
//   , src_iter, src_iter_limit
//   , trg_iter
//   , temp_iter_a, temp_iter_b
//  )
//
//   See the Implict Method section of the following:
//     http://en.wikipedia.org/wiki/Finite_difference_method
//
//   This assumes the same boundary conditions as calc_next_generation_forward_difference_1d(..).
//   It doesn't leak heat off the sides.

  template
   <  typename RATE_TYPE       // std::iterator_traits< SRC_ITER_TYPE >::reference
    , typename SRC_ITER_TYPE   // std::vector< float >::const_iterator
    , typename TRG_ITER_TYPE   // std::vector< float >::iterator
    , typename TEMP_ITER_TYPE  // we use 2 temp buffers
   >
  void
calc_next_generation_backward_difference_1d
 (  RATE_TYPE      const    damping         // usually between 0..1
  , RATE_TYPE      const    rate            // 0 < rate
  , SRC_ITER_TYPE  const &  src_iter        // previous state, values not changed, can be same as trg
  , SRC_ITER_TYPE  const &  src_iter_limit  // one past the end
  , TRG_ITER_TYPE  const &  trg_iter        // result, as big as src
  , TEMP_ITER_TYPE const &  srcX_iter       // temp, as big as src
  , TEMP_ITER_TYPE const &  diagX_iter      // temp, as big as src
 )
{
#   if ASSERT_REASONABLE_RATES
    d_assert( rate > 0);
#   endif
    // OK if (src_iter == trg_iter).
    // Except that many values of damping (but not 1.0) use the current value of trg to calc the
    // next value. So if src and trg are the same the only damping that is possible is (damping==1).
    //
    // trg_iter and the two buffers must all be distinct.

    d_assert( src_iter <= src_iter_limit);
    if ( src_iter < src_iter_limit ) {

        // Copy src to srcX. We will pass srcX to the destructive tri-diag solver which will then write
        // over the values in srcX (and in diagX_iter) and use them as a scratch pad.
        std::copy( src_iter, src_iter_limit, srcX_iter);

        // Calculate the diagonal to use in matrix solve.
        // The middle values are assigned (1 + 2r).
        // The end values are assigned (1 + r).
        RATE_TYPE const base = 1;
        size_t const count = calc_matrix_diagonal( base, rate, src_iter, src_iter_limit, diagX_iter);

        // Solve as a linear equation.
        // Improve: We could replace either diagX_iter or srcX_iter with trg_iter, using trg_iter
        // as one of the scratch buffers. Then we'd need only one buffer instead of two.
        init_damping_and_solve_matrix_destructive( damping, rate, count, src_iter, trg_iter, srcX_iter, diagX_iter);
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Central difference

// _______________________________________________________________________________________________
// calc_next_generation_central_difference_1d
//  (  assign2_functor
//   , rate
//   , src_iter, src_iter_limit
//   , trg_iter
//   , temp_iter_a, temp_iter_b
//  )
//
//   This uses the Crank-Nicolson method to calculate the next state.

  template
   <  typename RATE_TYPE
    , typename SRC_ITER_TYPE
    , typename TRG_ITER_TYPE
    , typename TEMP_ITER_TYPE
   >
  void
calc_next_generation_central_difference_1d
 (  RATE_TYPE      const    damping         // usually between 0..1
  , RATE_TYPE      const    rate            // 0 < rate
  , SRC_ITER_TYPE  const &  src_iter        // previous state, values not changed, can be same as trg
  , SRC_ITER_TYPE  const &  src_iter_limit  // one past the end
  , TRG_ITER_TYPE  const &  trg_iter        // result, as big as src
  , TEMP_ITER_TYPE const &  srcX_iter       // temp, as big as src
  , TEMP_ITER_TYPE const &  diagX_iter      // temp, as big as src
 )
{
    typedef typename std::iterator_traits< TRG_ITER_TYPE >::value_type item_type;

#   if ASSERT_REASONABLE_RATES
    d_assert( rate > 0);
    // This probably works if rate==0 too.
#   endif
    // OK if (src_iter == trg_iter).
    // Except that many values of damping (but not 1.0) use the current value of trg to calc the
    // next value. So if src and trg are the same the only damping that is possible is (damping==1).
    //
    // trg_iter and the two buffers must all be distinct.

    d_assert( src_iter <= src_iter_limit);
    if ( src_iter < src_iter_limit ) {

        // For each value in src, calculate a value and store it in srcX (where r is the rate):
        //   srcX[i] <- (2 - 2r)src[i] + r(src[i-1] + src[i+1])
        // Treat the rate values off the ends as zero, so as not to spill heat off the edges.
        //   srcX[0] <- (2 - r)src[0] + r(src[1])
        //   srcX[count-1] <- (2 - r)src[count-1] + r(src[count-2])
        //
        // This similar to a forward-diff solve.
        //   central-diff: (2 - 2r)p(1) + r(p[i-1] + p[i+1])
        //   forward-diff: (1 - 2r)p(1) + r(p[i-1] + p[i+1])
        //
        // Another way to express it:
        //   srcX = src + solve_forward_diff( rate, src)
        // Or
        //   calc_next_generation_forward_difference_1d( rate, src_iter, src_iter_limit, srcX_iter);
        //   iterate thru src and srcX {
        //      (*srcX_iter) += (*src_iter);
        //   }
        RATE_TYPE const base = 2;
        calc_forward_diff_thin_strip_
         (  assign3_set_type< item_type >( )
          , base
          , rate
          , src_iter, src_iter_limit
          , srcX_iter /* temp trg */
         );

        // Calculate the diagonal to use in matrix solve.
        // This is very similar to what we do when we solve using backward_diff, except we fill
        // the main diagonal with slightly different values:
        //   diagonal values for backward diff: 1 + 2r
        //   diagonal values for central  diff: 2 + 2r
        //RATE_TYPE const base = 2;
        size_t const count = calc_matrix_diagonal( base, rate, src_iter, src_iter_limit, diagX_iter);

        // Solve as a linear equation.
        init_damping_and_solve_matrix_destructive( damping, rate, count, src_iter, trg_iter, srcX_iter, diagX_iter);
    }
}

} /* end namespace finite_difference */

// _______________________________________________________________________________________________
// Older version of calc_next_generation_forward_difference_1d(..)

# if 0
  template< typename ITEM_TYPE, typename COUNT_TYPE, typename STRIDE_TYPE >
  void
calc_next_generation_forward_difference_1d(
    ITEM_TYPE   const   rate    // 0 < rate <= 0.5
  , COUNT_TYPE  const   count   // number of items in prev/next vectors
  , ITEM_TYPE   const * prev    // previous state - vector of count items
  , ITEM_TYPE         * next    // result - vector of count items
  , STRIDE_TYPE const   stride  // stride for prev and next
  )
{
    d_assert( rate  >  0);
    d_assert( (rate * 2) <= 1);
    d_assert( count >= 2);
    d_assert( prev != 0);
    d_assert( next != 0);

    // carry is how much heat is carried across the boundary between two cells.
    // carry_edge is used at the two ends, which are floating and insulated.
    ITEM_TYPE const carry_edge = 1 - rate;
    ITEM_TYPE const carry      = carry_edge - rate;
    d_assert( carry >= 0);
    d_assert( carry < carry_edge);

    // [p0, p1, p2] will iterate a 3-wide window over prev.
    // prev_limit is one past the end of prev.
    ITEM_TYPE const * p0         = prev;
    ITEM_TYPE         q0         = *p0;
    ITEM_TYPE const * p1         = p0 + stride;
    ITEM_TYPE         q1         = *p1;
    ITEM_TYPE const * p2         = p1 + stride;
    ITEM_TYPE const * prev_limit = prev + (count * stride);

    // n will iterate over next. We assign calculated values to *n.
    ITEM_TYPE       * n          = next;

    // Calculate the lo end.
    *n = (carry_edge * q0) + (rate * q1);

    // Iterate, calculating all the middle values.
    while ( p2 < prev_limit ) {
        ITEM_TYPE const q2 = *p2;

        n += stride;
        *n = (carry * q1) + (rate * (q0 + q2));

        // We carry both a tail of pointers [p0, p1, p2] and values [q0, q1, q2].
        // We do not need the pointer tail. We only use it for debugging (in the asserts).
        // If we did not keep the value tail but used the pointer tail instead, then this
        // algorithm would not work in-place.
        p0 = p1; q0 = q1;
        p1 = p2; q1 = q2;
        p2 += stride;
    }
    d_assert( n  == (next + ((count - 2) * stride)));
    d_assert( p0 == (prev + ((count - 2) * stride)));
    d_assert( p1 == (prev + ((count - 1) * stride)));

    // Calculate the hi end.
    *(n + stride) = (carry_edge * q1) + (rate * q0);
}
# endif

// _______________________________________________________________________________________________
// Older version of calc_next_generation_backward_difference_1d(..)

# if 0
  template< typename ITEM_TYPE >
  void
calc_next_generation_backward_difference_1d(
    ITEM_TYPE      const   rate      // 0 < rate
  , std::size_t    const   count     // number of items in prev/next vectors
  , ITEM_TYPE            * prev      // previous state - vector of count items
  , std::ptrdiff_t const   p_stride  // stride for prev
  , ITEM_TYPE            * next      // result - array of count items
  , std::ptrdiff_t const   n_stride  // stride for next
  , ITEM_TYPE            * temp      // array to use for diagonal (assume stride 1)
  )
{
    d_assert( rate  >  0);
    d_assert( count >= 1);
    d_assert( prev != 0);
    d_assert( next != 0);
    d_assert( temp != 0);
    d_assert( prev != next);
    d_assert( prev != temp);
    d_assert( next != temp);

    // Solve this as a linear equation.
    // This version doesn't conserve energy, like the one above.
    std::fill_n( temp, count, 1 + (2 * rate)); /* stride for temp is 1 */
    linear_algebra::
      solve_tridiagonal_destructive(
        count, - rate, temp, - rate, prev, p_stride, next, n_stride);
}
# endif


// _______________________________________________________________________________________________
// Older version of calc_next_generation_forward_difference_1d(..)

# if 0
  template< typename ITEM_TYPE >
  void
calc_next_generation_central_difference_1d(
    ITEM_TYPE      const   rate      // 0 < rate
  , std::size_t    const   count     // number of items in prev/next vectors
  , ITEM_TYPE            * prev      // previous state - vector of count items
  , std::ptrdiff_t const   p_stride  // stride for prev
  , ITEM_TYPE            * next      // result - array of count items
  , std::ptrdiff_t const   n_stride  // stride for next
  , ITEM_TYPE            * temp      // array to use for diagonal
  )
{
    d_assert( rate  >  0);
    d_assert( count >= 1);
    d_assert( prev != 0);
    d_assert( next != 0);
    d_assert( temp != 0);
    d_assert( prev != next);
    d_assert( prev != temp);
    d_assert( next != temp);

    // -----------------------------------------------------------------------------
    // The following is very similar to calc_next_generation_forward_difference_1d(..).
    // The end points are treated differently, and the the sustitution equation is
    //   (2 - 2r)p(1) + r(p[i-1] + p[i+1]) instead of
    //   (1 - 2r)p(1) + r(p[i-1] + p[i+1]) (forward diff).
    //
    // The following section adjusts the values in prev so they are no longer
    // just the values of the previous state.
    // Instead, each values is replaced by (2 - 2r)p(i) + r(p[i-1] + p[i+1]).
    // The prev values past the ends are set to zero, so the p(0) is set to
    // (2 - 2r)p(0) + r(0 + p(1)).

    ITEM_TYPE const carry = 2 * (1 - rate);
    // What happens if rate is big? Should we assert the following?
    // d_assert( carry >= 0);

    // [p0, p1, p2] will iterate a 3-wide window over prev.
    // prev_limit is one past the end of prev.
    ITEM_TYPE       * p0         = prev;
    ITEM_TYPE         q0         = *p0;
    ITEM_TYPE       * p1         = p0 + p_stride;
    ITEM_TYPE         q1         = *p1;
    ITEM_TYPE       * p2         = p1 + p_stride;
    ITEM_TYPE const * prev_limit = prev + (count * p_stride);

    // Calculate the lo end.
    *p0 = (carry * q0) + (rate * q1);

    // Iterate, calculating all the middle values.
    while ( p2 < prev_limit ) {
        ITEM_TYPE const q2 = *p2;
        *p1 = (carry * q1) + (rate * (q0 + q2));

        p0 = p1; q0 = q1;
        p1 = p2; q1 = q2;
        p2 += p_stride;
    }
    d_assert( p0 == (prev + ((count - 2) * p_stride)));
    d_assert( p1 == (prev + ((count - 1) * p_stride)));

    // Calculate the hi end.
    *p1 = (carry * q1) + (rate * q0);

    // ---------------------------------------------------------------------------------
    // The following section is similar to calc_next_generation_backward_difference_1d(..).
    // The value filling the main diagonal is (2 + 2r) instead of (1 + 2r).
    // The values in prev (the product vector) have been adjusted above so they are no
    // longer just the previous state.

    std::fill_n( temp, count, 2 * (1 + rate)); /* stride for temp is 1 */
    linear_algebra::
      solve_tridiagonal_destructive(
        count, - rate, temp, - rate, prev, p_stride, next, n_stride);
}
# endif

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef FINITE_DIFF_H
//
// finite_diff.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
