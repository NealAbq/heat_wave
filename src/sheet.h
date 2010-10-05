// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// sheet.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef SHEET_H
# define SHEET_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// _SECURE_SCL is a Microsoft #define that slows down and restricts iterators.
// The restrictions are too onerous for the following code.
//
// You cannot mix std::vector iterators (from Microsoft) if some of them are _SECURE_SCL and
// some of them are not.
//
# ifndef _SECURE_SCL
#   define _SECURE_SCL 0
# endif
# if _SECURE_SCL != 0
    // The code will NOT run if you get HERE!!
#   pragma error "_SECURE_SCL must be #defined to 0"
#   error "_SECURE_SCL must be #defined to 0"
    //
    //  The 2D iterators break the Microsoft SECURE_SCL rules. They also weigh down the iterators
    //  and are really designed assuming programmers don't know what they're doing.
    //  The best solution in MSVC2008 is to do the following:
    //    Menu ->
    //      Project ->
    //        my_proj Properties... Alt+F7
    //  This will bring up the properties for this project. Choose:
    //    Configuration Properties
    //      C/C++
    //        Preprocessor
    //  On the right will be "Preprocessor Definitions". Choose this and put this text on the front:
    //    _SECURE_SCL=0
    //  Make sure to separate the preprocessor definitions with a comma.
# endif

# include <vector>

# include "debug.h"
# include "util.h"
# include "stride_iter.h"

// _______________________________________________________________________________________________

  class
sheet_type
{
  // -------------------------------------------------------------------------------------------
  // Private Typedef
  private:
    // Make this_type private in case this class is subclassed.
    // We don't want to define this_class incorrectly for a subclass.
    typedef sheet_type                           this_type           ;

  // -------------------------------------------------------------------------------------------
  // Std collection typedefs
  //
  // These are left out:
  //   pointer
  //   const_pointer
  //   allocator_type
  //   reverse_iterator
  //   const_reverse_iterator
  public:
    typedef float                                value_type          ;
    typedef std::vector< value_type >            inner_type          ; // not a std collection type

    typedef inner_type::size_type                size_type           ;
    typedef inner_type::difference_type          difference_type     ;
    typedef inner_type::reference                reference           ;
    typedef inner_type::const_reference          const_reference     ;
    typedef inner_type::iterator                 iterator            ;
    typedef inner_type::const_iterator           const_iterator      ;

        BOOST_MPL_ASSERT(( boost::is_same< inner_type::value_type, value_type >));

  // -------------------------------------------------------------------------------------------
  // 2D iterator types
  public:
    typedef difference_type                      diff_type           ;

    typedef const_iterator                       inner_const_iter    ;
    typedef iterator                             inner_varia_iter    ;

    typedef stride_iter<  inner_varia_iter, 0 >  q_varia_iter_type   ;
    typedef stride_iter<  inner_const_iter, 0 >  q_const_iter_type   ;

    typedef stride_range< inner_varia_iter, 0 >  q_varia_range_type  ;
    typedef stride_range< inner_const_iter, 0 >  q_const_range_type  ;

    typedef stride_iter<  inner_varia_iter, 1 >  qq_varia_iter_type  ;
    typedef stride_iter<  inner_const_iter, 1 >  qq_const_iter_type  ;

    typedef stride_range< inner_varia_iter, 1 >  qq_varia_range_type ;
    typedef stride_range< inner_const_iter, 1 >  qq_const_range_type ;

    // The following type names are sometimes clearer to use, when you know that your
    // range is yx and not xy for example.
    // It would be better if these were glass-subtypes instead of typedefs.

    typedef q_varia_iter_type                    x_varia_iter_type   ;
    typedef q_varia_iter_type                    y_varia_iter_type   ;
    typedef q_const_iter_type                    x_const_iter_type   ;
    typedef q_const_iter_type                    y_const_iter_type   ;
    typedef q_varia_range_type                   x_varia_range_type  ;
    typedef q_varia_range_type                   y_varia_range_type  ;
    typedef q_const_range_type                   x_const_range_type  ;
    typedef q_const_range_type                   y_const_range_type  ;

    typedef qq_varia_iter_type                   yx_varia_iter_type  ;
    typedef qq_varia_iter_type                   xy_varia_iter_type  ;
    typedef qq_const_iter_type                   yx_const_iter_type  ;
    typedef qq_const_iter_type                   xy_const_iter_type  ;
    typedef qq_varia_range_type                  yx_varia_range_type ;
    typedef qq_varia_range_type                  xy_varia_range_type ;
    typedef qq_const_range_type                  yx_const_range_type ;
    typedef qq_const_range_type                  xy_const_range_type ;

  // -------------------------------------------------------------------------------------------
  // Ctors and dtor
  public:
    /* ctor */          sheet_type( )                       ;

  // -------------------------------------------------------------------------------------------
  // Copy
  //   This needs (and does not have) r-value assignment. See swap(..).
  public:
    /* copy */          sheet_type( this_type const &)      ;
    this_type &         operator =( this_type const &)      ;

    bool                maybe_add_in( sheet_type const &)   ;
    this_type &         operator +=( sheet_type const & b)  { d_verify( maybe_add_in( b)); return *this; }

    bool                maybe_subtract_out( sheet_type const &)
                                                            ;
    this_type &         operator -=( sheet_type const & b)  { d_verify( maybe_subtract_out( b)); return *this; }

    this_type &         operator *=( value_type scale)      { d_verify( scale_sheet( scale)); return *this; }

  // -------------------------------------------------------------------------------------------
  // Debug-only
  public:
# ifdef NDEBUG
    void                assert_valid( )               const { }
# else
    void                assert_valid( )               const ;
# endif

  // -------------------------------------------------------------------------------------------
  // Reset
  public:
    bool                is_reset( )                   const { return 0 == get_x_count( ); }
    bool                not_reset( )                  const { return ! is_reset( ); }
    void                reset( )                            ;

  // -------------------------------------------------------------------------------------------
  // Min/max
  public:
    value_type          get_min_value
                         (  size_type  x_lo
                          , size_type  x_hi_plus
                          , size_type  y_lo
                          , size_type  y_hi_plus
                         )                            const ;
    value_type          get_min_value( )              const ;

    bool                get_min_max_values
                         (  size_type     x_lo
                          , size_type     x_hi_plus
                          , size_type     y_lo
                          , size_type     y_hi_plus
                          , value_type &  return_min_value
                          , value_type &  return_max_value
                         )                            const ;
    bool                get_min_max_values
                         (  value_type &  return_min_value
                          , value_type &  return_max_value
                         )                            const ;

  // -------------------------------------------------------------------------------------------
  // Change sheet resolution
  public:
    bool                set_xy_counts_raw_values
                         (  size_type   x_count
                          , size_type   y_count
                         )                                  ;

    bool                set_xy_counts
                         (  size_type   x_count
                          , size_type   y_count
                          , value_type  init_value
                         )                                  ;

    bool                change_xy_counts
                         (  size_type       x_count
                          , size_type       y_count
                         )                                  ;

    static bool         copy_preserve_heights
                         (  this_type const &  src_sheet
                          , this_type       &  trg_sheet
                         )                                  ;

  // -------------------------------------------------------------------------------------------
  // Fillers
  public:
    bool                fill_sheet( value_type)             ;

    bool                set_value_at
                         (  value_type  new_value
                          , size_type   x
                          , size_type   y
                         )                                  ;

    bool                fill_rectangle_coords
                         (  value_type  new_value
                          , size_type   x_lo
                          , size_type   x_hi_plus
                          , size_type   y_lo
                          , size_type   y_hi_plus
                         )                                  ;

    bool                fill_rectangle_margins
                         (  value_type  new_value
                          , size_type   x_lo
                          , size_type   x_hi_margin
                          , size_type   y_lo
                          , size_type   y_hi_margin
                         )                                  { return fill_rectangle_coords( new_value,
                                                                       x_lo, get_x_count( ) - x_hi_margin,
                                                                       y_lo, get_y_count( ) - y_hi_margin);
                                                            }
    bool                fill_rectangle_widths
                         (  value_type  new_value
                          , size_type   x_lo
                          , size_type   x_width
                          , size_type   y_lo
                          , size_type   y_width
                         )                                  { return fill_rectangle_coords( new_value,
                                                                       x_lo, x_lo + x_width,
                                                                       y_lo, y_lo + y_width);
                                                            }

  // -------------------------------------------------------------------------------------------
  // Normalize
  public:
    bool                scale_rectangle_coords
                         (  value_type  scale
                          , size_type   x_lo
                          , size_type   x_hi_plus
                          , size_type   y_lo
                          , size_type   y_hi_plus
                         )                                  ;
    bool                scale_sheet( value_type)            ;

  // -------------------------------------------------------------------------------------------
  // Normalize
  public:
    bool                normalize
                         (  size_type     x_lo
                          , size_type     x_hi_plus
                          , size_type     y_lo
                          , size_type     y_hi_plus
                          , value_type    trg_lo           = -1
                          , value_type    trg_hi           = +1
                          , value_type *  p_trg_src_ratio  =  0
                         )                                  ;
    bool                normalize
                         (  value_type    trg_lo           = -1
                          , value_type    trg_hi           = +1
                          , value_type *  p_trg_src_ratio  =  0
                         )                                  ;

  // -------------------------------------------------------------------------------------------
  public:
                          template< typename COMBINE_FUNCTOR_TYPE >
    bool                combine_with
                         (  COMBINE_FUNCTOR_TYPE &  combine_functor  /* pass by ref */
                          , sheet_type const     &  src_sheet
                         )                                  ;

  protected:
                          template< typename CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE >
    bool                fill_6_param_functor_1( value_type dist_fraction)
                                                            ;
                          template< typename CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE >
    bool                fill_6_param_functor_2( value_type dist_fraction)
                                                            ;
                          template< typename CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE >
    bool                fill_6_param_functor_4( value_type dist_fraction)
                                                            ;

  // -------------------------------------------------------------------------------------------
  // Bell curves (normal distributions)
  public:
    bool                fill_bell_curve
                         (  value_type  center_x    // value_type instead of size_type so it can be 50.5
                          , value_type  center_y
                          , value_type  std_dev_x
                          , value_type  std_dev_y
                          , value_type  value_z_lo  = -1
                          , value_type  value_z_hi  = +1
                         )                                  ;

    bool                fill_bell_curve_1( )                ;
    bool                fill_bell_curve_2( )                ;
    bool                fill_bell_curve_4( )                ;

  // -------------------------------------------------------------------------------------------
  // Sin x over x
  public:
    bool                fill_sin_over_dist
                         (  value_type  center_x    // value_type instead of size_type
                          , value_type  center_y
                          , value_type  dist_x
                          , value_type  dist_y
                          , value_type  value_z_lo  = -1
                          , value_type  value_z_hi  = +1
                         )                                  ;

    bool                fill_sin_over_dist_1( )             ;
    bool                fill_sin_over_dist_2( )             ;
    bool                fill_sin_over_dist_4( )             ;

  // -------------------------------------------------------------------------------------------
  // Delta
  public:
    void                set_delta( )                        ;

  // -------------------------------------------------------------------------------------------
  // Rectangle functor operations
  public:
                          template< typename SCAN_FUNCTOR_TYPE >
    void                scan_rectangle
                         (  SCAN_FUNCTOR_TYPE &  scan_functor
                          , size_type            x_lo
                          , size_type            x_hi_plus
                          , size_type            y_lo
                          , size_type            y_hi_plus
                         )                            const ;

                          template< typename CALC_NEW_VALUE_FUNCTOR_TYPE >
    bool                transform_rectangle
                         (  CALC_NEW_VALUE_FUNCTOR_TYPE  calc_new_value_functor
                          , size_type                    x_lo
                          , size_type                    x_hi_plus
                          , size_type                    y_lo
                          , size_type                    y_hi_plus
                         )                                  ;

                          template
                           <  typename CALC_NEW_VALUE_FUNCTOR_TYPE
                            , typename ASSIGN_FUNCTOR_TYPE
                           >
    bool                transform_rectangle
                         (  CALC_NEW_VALUE_FUNCTOR_TYPE  calc_new_value_functor
                          , ASSIGN_FUNCTOR_TYPE          assign_functor
                          , size_type                    x_lo
                          , size_type                    x_hi_plus
                          , size_type                    y_lo
                          , size_type                    y_hi_plus
                         )                                  ;

  // -------------------------------------------------------------------------------------------
  // Whole-sheet functor operations
  public:
                          template< typename SCAN_FUNCTOR_TYPE >
    void                scan_sheet( SCAN_FUNCTOR_TYPE & scan_functor)
                                                      const { scan_rectangle
                                                               (  scan_functor
                                                                , 0, get_x_count( )
                                                                , 0, get_y_count( )
                                                               );
                                                            }

                          template< typename CALC_NEW_VALUE_FUNCTOR_TYPE >
    bool                transform_sheet( CALC_NEW_VALUE_FUNCTOR_TYPE calc_new_value_functor)
                                                            { return
                                                                transform_rectangle
                                                                 (  calc_new_value_functor
                                                                  , 0, get_x_count( )
                                                                  , 0, get_y_count( )
                                                                 );
                                                            }

                          template
                           <  typename CALC_NEW_VALUE_FUNCTOR_TYPE
                            , typename ASSIGN_FUNCTOR_TYPE
                           >
    bool                transform_sheet
                         (  CALC_NEW_VALUE_FUNCTOR_TYPE  calc_new_value_functor
                          , ASSIGN_FUNCTOR_TYPE          assign_functor
                         )                                  { return
                                                                transform_rectangle
                                                                 (  calc_new_value_functor
                                                                  , assign_functor
                                                                  , 0, get_x_count( )
                                                                  , 0, get_y_count( )
                                                                 );
                                                            }

  // -------------------------------------------------------------------------------------------
  // Flat (1-dimensional) iterators
  public:
    const_iterator      begin( )                      const { return get_inner( ).begin( ); }
    iterator            begin( )                            { return ref_inner( ).begin( ); }
    const_iterator      end( )                        const { return get_inner( ).end( ); }
    iterator            end( )                              { return ref_inner( ).end( ); }

  // -------------------------------------------------------------------------------------------
  // 2D iterators - y iterators
  //   Don't bother with reverse iters.
  public:
    diff_type           get_x_stride( )               const { return 1; }
    diff_type           get_y_stride( )               const { return get_x_count( ); }

    xy_varia_range_type get_range_xy( )                     ;
    xy_const_range_type get_range_xy( )               const ;

    yx_varia_range_type get_range_yx( )                     ;
    yx_const_range_type get_range_yx( )               const ;

    yx_varia_range_type  get_range_yx
                          (  size_type  x_lo
                           , size_type  x_hi_plus
                           , size_type  y_lo
                           , size_type  y_hi_plus
                          )                                 ;

    yx_const_range_type  get_range_yx
                          (  size_type  x_lo
                           , size_type  x_hi_plus
                           , size_type  y_lo
                           , size_type  y_hi_plus
                          )                           const ;

  // -------------------------------------------------------------------------------------------
  // Getters for dimensions
  //   The get_max_..( ) methods could be static (or even const values).
  public:
    size_type           get_x_count( )                const { return x_count_; }
    size_type           get_y_count( )                const { return y_count_; }

    size_type           get_xy_count( )               const { return get_x_count( ) * get_y_count( ); }

    static size_type    get_min_x_count( )        /*const*/ { return 2; }
    static size_type    get_min_y_count( )        /*const*/ { return 2; }
    static size_type    get_max_x_count( )        /*const*/ { return 1 << 15; /* 2^15 or 32K */ }
    static size_type    get_max_y_count( )        /*const*/ { return 1 << 15; /* 2^15 or 32K */ }
                            // Overflow checks:
                            d_static_assert(
                                ( static_cast< size_type >( 1 << 15) *
                                  static_cast< size_type >( 1 << 15) ) == (1 << 30));
                            d_static_assert(
                                ( ( static_cast< size_type >( 1 << 15) *
                                    static_cast< size_type >( 1 << 15) ) / (1 << 15) ) == (1 << 15));

  // -------------------------------------------------------------------------------------------
  // Getters for inner array
  public:
    inner_type const &  get_inner( )                  const { return array_; }
    const_reference     get_at( size_type x, size_type y)
                                                      const { return get_inner( ).at( x + (y * get_x_count( ))); }

  // Setters for inner array
  protected:
    inner_type &        ref_inner( )                        { return array_; }
    reference           ref_at( size_type x, size_type y)   { return ref_inner( ).at( x + (y * get_x_count( ))); }

  // -------------------------------------------------------------------------------------------
  // Member vars
  private:
    size_type   x_count_ ; /* 2D integer vector */
    size_type   y_count_ ;
    inner_type  array_   ; /* Array of z-values */

  // -------------------------------------------------------------------------------------------
  // Friends
  friend void swap( this_type &, this_type &);

}; /* end class sheet_type */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// swap(..)

  void
swap
 (  sheet_type &
  , sheet_type &
 );

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// combine_with(..)

  template< typename COMBINE_FUNCTOR_TYPE >
  bool
  sheet_type::
combine_with
 (  COMBINE_FUNCTOR_TYPE &  combine_functor  /* pass by ref */
  , sheet_type const     &  src_sheet
 )
  // This fails and returns false if src_sheet is not exactly the same size as this sheet.
  // Other actions to consider:
  //   Repeat the sheet
  //   Repeat the sheet mirrored
  //   Repeat the trailing edge
  //   Assume src_sheet is zero everywhere outside the boundaries
  //
  // This should apply a functor like one of these:
  //   functor( value_type & trg, value_type const & src_sheet)
  // The functor can accept const/non-const values and values that auto-cast to other values.
  // The functor can return a bool (early exit).
  // The functor can accept a coord point (.., pt( x, y, z))
  // The functor can accept individual coords (.., x, y, z))
  //
  // This should be a template when sheets are templates, so we can add a sheet_type< int > into a sheet_type< double >.
{
    // Check to make sure the src_sheet sheet matches this sheet.
    if ( (src_sheet.get_x_count( ) != get_x_count( )) ||
         (src_sheet.get_y_count( ) != get_y_count( )) )
    {
        // Size mismatch is failure.
        return false;
    }

    // If the width and/or height is zero there is nothing to copy.
    if ( (get_x_count( ) == 0) || (get_y_count( ) == 0) ) {
        // Nothing to copy. But not failure.
        d_assert( (get_x_count( ) == 0) && (get_y_count( ) == 0));
        d_assert( is_reset( ) && src_sheet.is_reset( ));
        return true;
    }
    d_assert( (! is_reset( )) && (! src_sheet.is_reset( )));

    // Setup the outer iterator vars.
    yx_const_range_type const  src_yx_range    = src_sheet.get_range_yx( );
    yx_const_iter_type         src_yx_iter     = src_yx_range.get_iter_lo( );
    yx_const_iter_type  const  src_yx_iter_hi  = src_yx_range.get_iter_hi( );
    d_assert( src_yx_iter <= src_yx_iter_hi);

    yx_varia_range_type const  trg_yx_range    = get_range_yx( );
    yx_varia_iter_type         trg_yx_iter     = trg_yx_range.get_iter_lo( );
    yx_varia_iter_type  const  trg_yx_iter_hi  = trg_yx_range.get_iter_hi( );
    d_assert( trg_yx_iter <= trg_yx_iter_hi);

    for ( ; ; ) {

        // Setup the inner iterator vars.
        // These should be slices so we can do something like this:
        //   trg_yx_slice.maybe_add_in( src_yx_slice);
        x_const_range_type const  src_x_range    = *src_yx_iter;
        x_const_iter_type         src_x_iter     = src_x_range.get_iter_lo( );
        x_const_iter_type  const  src_x_iter_hi  = src_x_range.get_iter_hi( );
        d_assert( src_x_iter <= src_x_iter_hi);

        x_varia_range_type const  trg_x_range    = *trg_yx_iter;
        x_varia_iter_type         trg_x_iter     = trg_x_range.get_iter_lo( );
        x_varia_iter_type  const  trg_x_iter_hi  = trg_x_range.get_iter_hi( );
        d_assert( trg_x_iter <= trg_x_iter_hi);

        for ( ; ; ) {

            // Do the work -- combine the src value into the trg. Usually something like trg += src.
            combine_functor( *trg_x_iter, *src_x_iter);

            // Move the inner iterators and test for the end.
            if ( src_x_iter_hi == src_x_iter ) break;
            d_assert( trg_x_iter_hi > trg_x_iter);

            ++ src_x_iter;
            ++ trg_x_iter;
        }
        d_assert( trg_x_iter_hi == trg_x_iter);

        // Move the outer iterators and test for the end.
        if ( src_yx_iter_hi == src_yx_iter ) break;
        d_assert( trg_yx_iter_hi > trg_yx_iter);

        ++ src_yx_iter;
        ++ trg_yx_iter;
    }
    d_assert( trg_yx_iter_hi == trg_yx_iter);

    return true;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// scan_rectangle(..)

  template< typename SCAN_FUNCTOR_TYPE >
  void
  sheet_type::
scan_rectangle
 (  SCAN_FUNCTOR_TYPE &  scan_functor  /* pass by ref */
  , size_type            x_lo
  , size_type            x_hi_plus
  , size_type            y_lo
  , size_type            y_hi_plus
 ) const
  //
  // Scans the values in the sheet without changing them.
  // Exits early if functor returns false.
  //
  // The functor looks like this:
  //   op( value, x, y) -> keep_scaning
  // Stops scanning if keep_scaning (return value) is false.
  //
  // The functor signature should be compatible with:
  //     bool
  //   operator ()
  //    (  sheet_type::value_type  value
  //     , sheet_type::size_type   x
  //     , sheet_type::size_type   y
  //    )
{
    // The 2 implementations that follow both work. They demostrate how to use the iters raw and how
    // to use scan_leaves_with_2d_index_early_exit(..).
  # if 1

    yx_const_range_type const yx_range = get_range_yx( x_lo, x_hi_plus, y_lo, y_hi_plus);
    if ( yx_range.get_count( ) && yx_range.get_next_range( ).get_count( ) ) {
        scan_leaves_with_2d_index_early_exit( yx_range, scan_functor, x_lo, y_lo);
    }

  # else
    // The second method just uses the iterators.

    // Check to make sure this is legal. Otherwise return false.
    if ( not_reset( ) &&
        (0 <= x_lo) && (x_lo < get_x_count( )) && (x_lo < x_hi_plus) && (x_hi_plus <= get_x_count( )) &&
        (0 <= y_lo) && (y_lo < get_y_count( )) && (y_lo < y_hi_plus) && (y_hi_plus <= get_y_count( )) )
    {
        yx_const_range_type const  yx_range      = get_range_yx( );
        yx_const_iter_type  const  yx_iter_hi    = yx_range.get_iter_lo( ) + (y_hi_plus - 1);
        yx_const_iter_type         yx_iter_walk  = yx_range.get_iter_lo( ) + y_lo;
        size_type                  y_walk        = y_lo;
        for ( ; ; ) {
            d_assert( yx_iter_walk <= yx_iter_hi);

            x_const_range_type const  x_range      = *yx_iter_walk;
            x_const_iter_type  const  x_iter_hi    = x_range.get_iter_lo( ) + (x_hi_plus - 1);
            x_const_iter_type         x_iter_walk  = x_range.get_iter_lo( ) + x_lo;
            size_type                 x_walk       = x_lo;
            for ( ; ; ) {
                d_assert( x_iter_walk <= x_iter_hi);

                // Apply the functor.
                if ( ! scan_functor( *x_iter_walk, x_walk, y_walk) ) {
                    return;
                }

                if ( x_iter_hi == x_iter_walk ) break;
                ++ x_iter_walk;
                ++ x_walk;
            }
            d_assert( x_walk == (x_hi_plus - 1));

            if ( yx_iter_hi == yx_iter_walk ) break;
            ++ yx_iter_walk;
            ++ y_walk;
        }
        d_assert( y_walk == (y_hi_plus - 1));
    }
  # endif
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// transform_rectangle(..)
//
//   We also need a copy-and-transform mechanism, that copies one sheet to another while
//   transforming the values.

  /* private functor class, used in template method below */
  template
   <  typename CALC_NEW_VALUE_FUNCTOR_TYPE // operator( old_val, x, y) -> new_val
    , typename ASSIGN_FUNCTOR_TYPE         = util::assign_set_type< sheet_type::value_type >
   >
  struct /* functor */
scan_leaves_with_2d_index_functor_type
{
  typedef sheet_type::size_type  size_type ; /* x/y coord type */
  typedef sheet_type::reference  reference ; /* NOT ref-to-const */

    /* constructor */
  scan_leaves_with_2d_index_functor_type
  (  CALC_NEW_VALUE_FUNCTOR_TYPE const &  f  /* we make copies of these functors */
   , ASSIGN_FUNCTOR_TYPE         const &  a  = ASSIGN_FUNCTOR_TYPE( )
  )
    : calc_functor_( f), assign_functor_( a) { }
    CALC_NEW_VALUE_FUNCTOR_TYPE  calc_functor_   ; /* copy of functor, not ref */
    ASSIGN_FUNCTOR_TYPE          assign_functor_ ; /* copy of functor, not ref */

    /* functor */
    void
  operator ()( reference v, size_type x, size_type y)
    { assign_functor_( v, calc_functor_( v, x, y)); }
};

// _______________________________________________________________________________________________

  template
   <  typename CALC_NEW_VALUE_FUNCTOR_TYPE
    , typename ASSIGN_FUNCTOR_TYPE
   >
  bool
  sheet_type::
transform_rectangle
 (  CALC_NEW_VALUE_FUNCTOR_TYPE  calc_new_value_functor
  , ASSIGN_FUNCTOR_TYPE          assign_functor
  , size_type                    x_lo
  , size_type                    x_hi_plus
  , size_type                    y_lo
  , size_type                    y_hi_plus
 )
  // Improve: Provide a parallel version of this function.
  //
  // Instead of just scanning the values in the sheet, this sets them.
  // The functor looks like this:
  //   op( old_value, x, y) -> new_value
  // The sheet is set to the new value at (x, y).
  //
  // The functor signature should be compatible with:
  //     sheet_type::value_type
  //   operator ()
  //    (  value_type  old_value
  //     , size_type   x
  //     , size_type   y
  //    )
{
    yx_varia_range_type const yx_range = get_range_yx( x_lo, x_hi_plus, y_lo, y_hi_plus);
    if ( yx_range.get_count( ) && yx_range.get_next_range( ).get_count( ) ) {
        scan_leaves_with_2d_index_functor_type< CALC_NEW_VALUE_FUNCTOR_TYPE, ASSIGN_FUNCTOR_TYPE >
            wrapper_functor( calc_new_value_functor, assign_functor);
        scan_leaves_with_2d_index( yx_range, wrapper_functor, x_lo, y_lo);
        return true;
    }
    return false;
}

// _______________________________________________________________________________________________

  template< typename CALC_NEW_VALUE_FUNCTOR_TYPE >
  bool
  sheet_type::
transform_rectangle
 (  CALC_NEW_VALUE_FUNCTOR_TYPE  calc_new_value_functor
  , size_type                    x_lo
  , size_type                    x_hi_plus
  , size_type                    y_lo
  , size_type                    y_hi_plus
 )
{
    return
      transform_rectangle
       (  calc_new_value_functor
        , util::assign_set_type< value_type >( )
        , x_lo, x_hi_plus
        , y_lo, y_hi_plus
       );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Example of a functor to use in transform_rectangle(..), above.

  struct /* functor */
bell_curve_functor_type
  //
  // Also called a normal distribution.
{
    typedef sheet_type::size_type   size_type  ;
    typedef sheet_type::value_type  value_type ;

    bell_curve_functor_type
     (  value_type  center_x    // value_type instead of size_type so it can be 50.5
      , value_type  center_y
      , value_type  std_dev_x   // cannot be zero, negative same as positive (this is squared)
      , value_type  std_dev_y   // cannot be zero, negative same as positive
      , value_type  value_z_lo  = -1
      , value_type  value_z_hi  = +1
     )
      : center_x_( center_x)
      , center_y_( center_y)
      , neg2_std_dev_squared_x_( -2 * std_dev_x * std_dev_x)
      , neg2_std_dev_squared_y_( -2 * std_dev_y * std_dev_y)
      , value_z_lo_( value_z_lo)
      , value_z_delta_( value_z_hi - value_z_lo)
      {
        d_assert( neg2_std_dev_squared_x_ < 0);
        d_assert( neg2_std_dev_squared_y_ < 0);
      }
      value_type const  center_x_               ;
      value_type const  center_y_               ;
      value_type const  neg2_std_dev_squared_x_ ;
      value_type const  neg2_std_dev_squared_y_ ;
      value_type const  value_z_lo_             ;
      value_type const  value_z_delta_          ;

      value_type
    operator ()
     (  value_type  /* old_value_z - not used */
      , size_type   x
      , size_type   y
     )
    {
        value_type const delta_x = x - center_x_;
        value_type const delta_y = y - center_y_;
        return value_z_lo_ +
            (value_z_delta_ *
             std::exp(
                ((delta_x * delta_x) / neg2_std_dev_squared_x_) +
                ((delta_y * delta_y) / neg2_std_dev_squared_y_) ));
    }
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef SHEET_H
//
// sheet.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
