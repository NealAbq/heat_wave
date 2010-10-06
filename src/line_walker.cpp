// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// line_walker.cpp
//
//   Copyright (c) Neal Binnendyk 2009, 2010.
//     <nealabq@gmail.com>
//     <http://nealabq.com/>
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
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Improve:
//   Add an iterator wrapper to scale the result. More notes in stride_iter.cpp.
//
//   Examples:
//     Draw simple line.
//     Draw anti-alias line.
//     Draw line while changing resolution (pixel size).
//
//     Paint a thick path with a bunch of diagonal strokes.
//
//     Copy line graph changing resolution.
//     Copy 2D sheet graph (height map) changing resolution.
//     Create an error summing object for integer accumulators, so you can carry over
//       division errors (rems) to the next calculation.
// _______________________________________________________________________________________________

# include "all.h"
# include "line_walker.h"

// _______________________________________________________________________________________________

  /* static */
  line_walker_type::sint_type
  line_walker_type::
calc_dec_pivot( uint_type src_count, uint_type trg_count)
{
    d_assert( src_count > 0);
    sint_type const  src_max     = src_count / 2;
    sint_type const  src_llimit  = src_max - src_count; /* negative number */

    d_assert( (src_llimit < 0) && (0 <= src_max) && (static_cast< uint_type >( src_max) < src_count));

    d_assert( trg_count > 0);
    sint_type const  trg_max     = trg_count / 2;
    sint_type const  trg_llimit  = trg_max - trg_count; /* negative number */

    d_assert( (trg_llimit < 0) && (0 <= trg_max) && (static_cast< uint_type >( trg_max) < trg_count));

    return trg_llimit - src_llimit;
}

  /* static */
  line_walker_type::sint_type
  line_walker_type::
calc_inc_pivot( uint_type src_count, uint_type trg_count)
{
    sint_type const  src_max  = src_count / 2;
    sint_type const  trg_max  = trg_count / 2;
    return trg_max - src_max;
}

  /* constructor */
  line_walker_type::
line_walker_type( uint_type src_c, uint_type trg_c, alignment_selector alignment /* = e_mid */)
  : src_count_  ( src_c)
  , trg_count_  ( trg_c)
  , dec_pivot_  ( calc_dec_pivot( src_c, trg_c))
  , inc_pivot_  ( calc_inc_pivot( src_c, trg_c))
  , modulo_     ( get_modulo_for_alignment( alignment))
{
    d_assert( get_src_count( ) > 0);
    d_assert( get_trg_count( ) > 0);

    // You have to be able to add the two counts without overflowing the unsigned int.
    d_assert( (get_src_count( ) + get_trg_count( )) > get_src_count( ));
    d_assert( (get_src_count( ) + get_trg_count( )) > get_trg_count( ));

    // Pivot assumption.
    d_assert( (dec_pivot_ + get_trg_count( )) == (inc_pivot_ + get_src_count( )));

    // Check assumptions about modulo_ min/max.
  # ifndef NDEBUG
    sint_type const  modulo_min   = get_modulo_min( );
    sint_type const  modulo_max   = get_modulo_max( );
    sint_type const  modulo_sum   = modulo_max + modulo_min;
    uint_type const  modulo_diff  = modulo_max - modulo_min;
    d_assert( modulo_min <= modulo_max);
    d_assert( (-1 <= modulo_sum) && (modulo_sum <= +1));
    d_assert( modulo_diff == ((get_src_count( ) + get_trg_count( )) - 2));

    // d_assert( modulo_min == ((trg_llimit + 1) - src_max));
    // d_assert( modulo_max == (trg_max - (src_llimit + 1)));
  # endif
}

// _______________________________________________________________________________________________

  line_walker_type::src_trg_selector
  line_walker_type::
inc( )
{
    d_assert( modulo_ >= get_modulo_min( ));
    d_assert( modulo_ <= get_modulo_max( ));

    if ( modulo_ < inc_pivot_ /* is_src_spilling_hi( ) */ ) {
        d_assert( is_src_spilling_hi( ) && ! is_trg_spilling_hi( ));
        modulo_ += get_trg_width( );
        d_assert( modulo_ <= get_modulo_max( ));
        d_assert( modulo_ > dec_pivot_);
        d_assert( is_src_spilling_lo( ) && ! is_trg_spilling_lo( ));
        return e_trg;
    }

    if ( modulo_ > inc_pivot_ /* is_trg_spilling_hi( ) */ ) {
        d_assert( is_trg_spilling_hi( ) && ! is_src_spilling_hi( ));
        modulo_ -= get_src_width( );
        d_assert( modulo_ >= get_modulo_min( ));
        d_assert( modulo_ < dec_pivot_);
        d_assert( is_trg_spilling_lo( ) && ! is_src_spilling_lo( ));
        return e_src;
    }

    /* modulo_ == inc_pivot_ */
    d_assert( is_aligned_hi( ));
    d_assert( (! is_src_spilling_hi( )) && (! is_trg_spilling_hi( )));
    modulo_ = dec_pivot_;
    d_assert( (! is_src_spilling_lo( )) && (! is_trg_spilling_lo( )));
    d_assert( is_aligned_lo( ));
    return e_src_and_trg;
}

  line_walker_type::src_trg_selector
  line_walker_type::
dec( )
{
    d_assert( modulo_ >= get_modulo_min( ));
    d_assert( modulo_ <= get_modulo_max( ));

    if ( modulo_ > dec_pivot_ /* is_src_spilling_lo( ) */ ) {
        d_assert( is_src_spilling_lo( ) && ! is_trg_spilling_lo( ));
        modulo_ -= get_trg_width( );
        d_assert( modulo_ >= get_modulo_min( ));
        d_assert( modulo_ < inc_pivot_);
        d_assert( is_src_spilling_hi( ) && ! is_trg_spilling_hi( ));
        return e_trg;
    }

    if ( modulo_ < dec_pivot_ /* is_trg_spilling_lo( ) */ ) {
        d_assert( is_trg_spilling_lo( ) && ! is_src_spilling_lo( ));
        modulo_ += get_src_width( );
        d_assert( modulo_ <= get_modulo_max( ));
        d_assert( modulo_ > inc_pivot_);
        d_assert( is_trg_spilling_hi( ) && ! is_src_spilling_hi( ));
        return e_src;
    }

    /* modulo_ == dec_pivot_ */
    d_assert( is_aligned_lo( ));
    d_assert( (! is_src_spilling_lo( )) && (! is_trg_spilling_lo( )));
    modulo_ = inc_pivot_;
    d_assert( (! is_src_spilling_hi( )) && (! is_trg_spilling_hi( )));
    d_assert( is_aligned_hi( ));
    return e_src_and_trg;
}

// _______________________________________________________________________________________________

// The following notes describe the design of a src-to-trg iterator based on line_walker_type.
# if 0

  template< typename SRC_ITER_T >
  class
line_walker_iter_type
  //
  // This is an iter. It needs:
  //   ctor()
  //   ctor( init-args)
  //   init( init-args)
  //   copy ctor/assop
  //   reset
  //   inc/dec
  //   way to test if is at/past end
  //     past-end (and before-start) states
  //   get trg value [deref operator *()]
  //   typedefs (value_type etc)
{
  // -------------------------------------------------------------------------------------------------
  // Typedefs
  private:
    typedef line_walker_iter_type< SRC_ITER_T >  this_type     ;
  public:
    typedef line_walker_type::sint_type          sint_type     ;
    typedef line_walker_type::uint_type          uint_type     ;
    typedef SRC_ITER_T                           src_iter_type ;

  private:
    src_iter_type  src_iter_  ;
    uint_type      src_index_ ;
    uint_type      trg_index_ ;
};

// Iterators:
//   Inc/dec are from the trg point of view.
//   Iterator over src values, accumulating them as trg values.
//     Accumulator (usually sum, preserves sum)
//     Scaled accumulate (preserves area or average, scale can be provided or calculated)
//     Or you can just pick one (middle) value from all the src values and use that.
//       Or highest, or lowest.
//
//   We want a separate set of iterators for setting values.

(see static method line_walker_type::copy(..) in line_walker.h to see how to implement inc/dec.

    // Take care of the easy linear-copy case. This is faster.
    if ( src_count == trg_count ) {
        copy_n( assign_fn, src_iter, src_count, trg_iter);
        return true;
    }

    // The line_walker_type class requires the following:
    if ( (src_count > 0) && (trg_count > 0) && ((src_count + trg_count) > 0) ) {
        line_walker_type walker( src_count, trg_count, line_walker_type::e_lo);

        d_assert( walker.is_aligned_lo( ));
        d_assert( ! walker.is_trg_spilling_lo( ));
        d_assert( ! walker.is_src_spilling_lo( ));

        uint_type src_countdown = src_count;
        uint_type trg_countdown = trg_count;
        bool is_new_src = true;
        bool is_new_trg = true;
        do {
            assign_fn( walker, src_iter, trg_iter, is_new_src, is_new_trg);

            src_trg_selector const inc_code = walker.inc( );
            is_new_src = includes_src( inc_code);
            is_new_trg = includes_trg( inc_code);
            d_assert( is_new_src || is_new_trg /* one or both are true */);

            if ( is_new_src ) {
                ++ src_iter;
                -- src_countdown;
            }
            if ( is_new_trg ) {
                ++ trg_iter;
                -- trg_countdown;
            }
        }
        while ( src_countdown && trg_countdown );
        d_assert( is_new_trg);
        d_assert( (0 == src_countdown) && (0 == trg_countdown));

        d_assert( ! walker.is_src_spilling_lo( ));
        d_assert( ! walker.is_trg_spilling_lo( ));
        d_assert( walker.is_aligned_lo( ));

# endif

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// line_walker.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
