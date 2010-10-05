// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// sheet.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Improve:
//   All of these should work on slices/projections and rectangles.
//   sheet_type< bool, 2 >
//     Like a 1-bit bitmap
//   Transforms:
//     reflect, rotate, shear?
//     reverse individual rows and columns.
//     negate all values
//   Swizzles and extractions:
//     transform sheets from xy to yx (column-major to row-major)
//     extract rectangles, etc
//   Slices (like multi-dimensional iterators):
//     we need to be able to pull out planes and lines, even if they are at an angle
//
// 2D sheet iterator thoughts:
//   get_x( ) -> index_type (or size_type)
//   get_y( ) -> index_type
//   get( )   -> const_reference (same as operator *)
//
//   ref_x( ) or aspect_x( )
//     Return a thing that remains attached to the orignal iterator and provides all the
//     iterator operators, except they only travel on the x line.
//     It would be better if this were a range. That way you'd be getting a line or a column.
//
//   iter_x( ) and iter_y( )
//     Return an iterator-like thing with all the operators.
//     Independent of the original 2D operator?
//
//   is_at_end_x( )
//   is_at_end_y( )
//   crlf( ) which is the same as go_lo_x( ) and inc_y( )
//
//   Class assumes row-major order.
//   Members of this 2D sheet iter class:
//     ptr_first_column
//     ptr_last_column
//     ptr_first_row_in_column
//     ptr_last_row_in_column
//     ptr_element
//
//     ptr_first_column <=
//       ptr_first_row_in_column <=
//         ptr_element <=
//       ptr_last_row_in_column <=
//     ptr_last_column
//
//   Values that could be derived:
//     x_index, y_index, x_count, y_count
//     x_stride (always 1?)
//     y_stride (x_count)
//
//   This is an inner iterator. It does not naturally have a past-the-end state.
//   It does have a natural 1D iter interface - row-major traversal.
//
//   You could add a corner offset and this could define a rectangular view into a
//   sheet, like a window.
//   You could also add a stencil or some other kind of filter.
//   You could even provide this with transformations, like this was a pixmap.
//
//   If we drop the ptr_element member-var, then this is more of a view into a
//   sheet (a slice or filter) than an iter itself.
//   Then you could provide a set of linear iterators on top of this.
//
//   x_varia_iter_type:
//     Given an x_varia_iter_type, get begin_y_varia_iter_type?
//     To do this x_varia_iter_type has to know both strides and the begin/end of the other axis.
//     So we really have 2 x_varia_iter_types:
//       A simple pair { pointer; stride; }. You cannot get a y_varia_iter_type from this.
//       A simple 2D iterator in x_varia_iter_type mode { pointer; stride; other_stride; }
//     We also need a range object { ptr_lo, ptr_hi, stride }
//     We also need a range iter, or maybe a range-range object.
//
//   We have to think about how we handle is_past_end states.
//
//   Or we could have a two-layer kind of iterator:
//     array.get_x_iter( ) -> x_iter
//     x_iter.get_y_iter( ) -> yy_iter
//     yy_iter.get( ) -> value
//
//   The sheet should keep track of min/max values.
// _______________________________________________________________________________________________

# include "all.h"
# include "sheet.h"
# include "util.h"
# include "line_walker.h"
# include "angle_holder.h" /* for angle_holder::pi */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Wrapping functors

  namespace /* anonymous */ {
  template< typename FUNCT_A, typename FUNCT_B >
  struct
sum_transform_functor_type
{
    typedef sheet_type::size_type   size_type  ;
    typedef sheet_type::value_type  value_type ;

    sum_transform_functor_type( FUNCT_A const & f_a, FUNCT_B const & f_b)
      : funct_a_( f_a), funct_b_( f_b) { }
      FUNCT_A funct_a_;
      FUNCT_B funct_b_;

      value_type
    operator ()
     (  value_type  old_value_z
      , size_type   x
      , size_type   y
     )
    {
        return funct_a_( old_value_z, x, y) + funct_b_( old_value_z, x, y);
    }
};
  } /* end namespace anonymous */

  namespace /* anonymous */ {
  template< typename FUNCT_A, typename FUNCT_B >
  struct
product_transform_functor_type
{
    typedef sheet_type::size_type   size_type  ;
    typedef sheet_type::value_type  value_type ;

    product_transform_functor_type( FUNCT_A const & f_a, FUNCT_B const & f_b)
      : funct_a_( f_a), funct_b_( f_b) { }
      FUNCT_A funct_a_;
      FUNCT_B funct_b_;

      value_type
    operator ()
     (  value_type  old_value_z
      , size_type   x
      , size_type   y
     )
    {
        return funct_a_( old_value_z, x, y) * funct_b_( old_value_z, x, y);
    }
};
  } /* end namespace anonymous */

  namespace /* anonymous */ {
  template< typename FUNCT_A, typename FUNCT_B >
  struct
compose_transform_functor_type
{
    typedef sheet_type::size_type   size_type  ;
    typedef sheet_type::value_type  value_type ;

    compose_transform_functor_type( FUNCT_A const & f_a, FUNCT_B const & f_b)
      : funct_a_( f_a), funct_b_( f_b) { }
      FUNCT_A funct_a_;
      FUNCT_B funct_b_;

      value_type
    operator ()
     (  value_type  old_value_z
      , size_type   x
      , size_type   y
     )
    {
        return funct_a_( funct_b_( old_value_z, x, y), x, y);
    }
};
  } /* end namespace anonymous */

// _______________________________________________________________________________________________
// Functions to deliver the wrapping functors, to save you from complicated type decls.

  template< typename FUNCT_A, typename FUNCT_B >
  sum_transform_functor_type< FUNCT_A, FUNCT_B >
make_sum_transform_functor( FUNCT_A const & f_a, FUNCT_B const & f_b)
{
    return sum_transform_functor_type< FUNCT_A, FUNCT_B >( f_a, f_b);
}

  template< typename FUNCT_A, typename FUNCT_B >
  product_transform_functor_type< FUNCT_A, FUNCT_B >
make_product_transform_functor( FUNCT_A const & f_a, FUNCT_B const & f_b)
{
    return product_transform_functor_type< FUNCT_A, FUNCT_B >( f_a, f_b);
}

  template< typename FUNCT_A, typename FUNCT_B >
  compose_transform_functor_type< FUNCT_A, FUNCT_B >
make_compose_transform_functor( FUNCT_A const & f_a, FUNCT_B const & f_b)
{
    return compose_transform_functor_type< FUNCT_A, FUNCT_B >( f_a, f_b);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  // Non-member helper function
  void
swap( sheet_type & a, sheet_type & b)
  //
  // Use this to implement r-value assignment (destructive assignment).
{
    boost::swap( a.x_count_, b.x_count_);
    boost::swap( a.y_count_, b.y_count_);
    boost::swap( a.ref_inner( ), b.ref_inner( ));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  // Default constructor
  sheet_type::
sheet_type( )
  : x_count_ ( size_type( 0))
  , y_count_ ( size_type( 0))
  , array_   ( )
{
    d_assert( is_reset( ));
    assert_valid( );
}

  // Copy constructor
  sheet_type::
sheet_type( this_type const & src)
  : x_count_ ( src.get_x_count( ))
  , y_count_ ( src.get_y_count( ))
  , array_   ( src.get_inner( ))
{
    d_assert( is_reset( ));
    assert_valid( );
}

  // Copy assignment
  sheet_type &
  sheet_type::
operator =( this_type const & src)
{
    x_count_ = src.get_x_count( );
    y_count_ = src.get_y_count( );
    array_   = src.get_inner( );
    return *this;
}

// _______________________________________________________________________________________________

  bool
  sheet_type::
maybe_add_in( sheet_type const & src_sheet)
  //
  // This fails and returns false if src is not exactly the same size as this sheet.
{
    util::assign_sum_type< value_type > assign_functor;
    return combine_with( assign_functor, src_sheet);
}

  bool
  sheet_type::
maybe_subtract_out( sheet_type const & src_sheet)
{
    util::assign_subtract_out_type< value_type > assign_functor;
    return combine_with( assign_functor, src_sheet);
}

// _______________________________________________________________________________________________

  // Public method
# ifndef NDEBUG
  void
  sheet_type::
assert_valid( ) const
  //
  // This is rudimentary and not complete. There are many checks that can be added.
{
    d_assert( get_x_count( ) <= get_max_x_count( ));
    d_assert( get_y_count( ) <= get_max_y_count( ));

    // The following assert is not valid because std::vector<T>.clear( ) does not release internal memory.
    // Later: Now we run the dtor to release memory, so this assert now works:
    d_assert( get_inner( ).size( ) == get_inner( ).capacity( ));

    if ( is_reset( ) ) {
        d_assert( get_x_count(  ) == 0);
        d_assert( get_y_count(  ) == 0);
        d_assert( get_xy_count( ) == 0);

        d_assert( 0 == get_inner( ).size( ));
    } else {
        d_assert( get_x_count(  ) > 0);
        d_assert( get_y_count(  ) > 0);
        d_assert( get_xy_count( ) > get_x_count( ));
        d_assert( get_xy_count( ) > get_y_count( ));

        d_assert( get_xy_count( ) == get_inner( ).size( ));

        d_assert( begin( ) < end( ));
        d_assert( (begin( ) + get_xy_count( )) == end( ));
        d_assert( begin( ) == (end( ) - get_xy_count( )));
    }
}
# endif

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Reset

  // Public method
  void
  sheet_type::
reset( )
{
    if ( not_reset( ) ) {
        x_count_ = size_type( 0);
        y_count_ = size_type( 0);

        // You'd think that this would release all the memory:
        //   ref_inner( ).clear( );
        //   d_assert( ref_inner( ).capacity( ) == 0);
        // But it doesn't, and the assert fails (with the microsoft std::vector<>).
        // If clear( ) doesn't free memory, the only way I know to force it is to run the destructor.
        util::apply_dtor( & ref_inner( ));
        util::apply_default_ctor( & ref_inner( ));
        d_assert( ref_inner( ).capacity( ) == 0);
    }
    d_assert( is_reset( ));
    assert_valid( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Setting the sheet size (xy counts, or x-size and y-size)

  // Public method
  bool
  sheet_type::
set_xy_counts_raw_values( size_type x_count, size_type y_count)
{
    // Do nothing if the request is illegal.
    if ( (x_count > get_max_x_count( )) || (y_count > get_max_y_count( )) ) {
        d_assert( false);
        return false;
    }

    // It's not legal to ask for a zero and non-zero dimension together.
    if ( (x_count && ! y_count) || (y_count && ! x_count) ) {
        d_assert( false);
        return false;
    }

    // Throw everything away.
    reset( );

    // It's better to call reset( ) if you want to set the counts to zero.
    if ( x_count && y_count ) {

        // Remember the new counts.
        x_count_ = x_count;
        y_count_ = y_count;

        // Allocate space.
        ref_inner( ).resize( get_xy_count( ));
    }
    return true;
}

  // Public method
  bool
  sheet_type::
set_xy_counts( size_type x_count, size_type y_count, value_type init_value)
{
    if ( set_xy_counts_raw_values( x_count, y_count) ) {
        fill_sheet( init_value);
        return true;
    }
    return false;
}

  // Public method
  bool
  sheet_type::
change_xy_counts( size_type x_count, size_type y_count)
{
    if ( is_reset( ) ) {
        return set_xy_counts( x_count, y_count, 0);
    }

    this_type other;
    if ( ! other.set_xy_counts_raw_values( x_count, y_count) ) {
        return false;
    }
    if ( other.not_reset( ) && ! copy_preserve_heights( *this, other) ) {
        return false;
    }

    // Swap the guts of the two sheets.
    swap( *this, other);
    return true;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Ranges

  sheet_type::xy_varia_range_type
  sheet_type::
get_range_xy( )
{
    return
        xy_varia_range_type(
            get_x_count( ), get_x_stride( ),
            get_y_count( ), get_y_stride( ),
            begin( ));
}

  sheet_type::xy_const_range_type
  sheet_type::
get_range_xy( ) const
{
    return
        xy_const_range_type(
            get_x_count( ), get_x_stride( ),
            get_y_count( ), get_y_stride( ),
            begin( ));
}

  sheet_type::yx_varia_range_type
  sheet_type::
get_range_yx( )
{
    return
        yx_varia_range_type(
            get_y_count( ), get_y_stride( ),
            get_x_count( ), get_x_stride( ),
            begin( ));
}

  sheet_type::yx_const_range_type
  sheet_type::
get_range_yx( ) const
{
    return
        yx_const_range_type(
            get_y_count( ), get_y_stride( ),
            get_x_count( ), get_x_stride( ),
            begin( ));
}

// _______________________________________________________________________________________________

  sheet_type::yx_const_range_type
  sheet_type::
get_range_yx
 (  size_type  x_lo
  , size_type  x_hi_plus
  , size_type  y_lo
  , size_type  y_hi_plus
 ) const
{
    // Check to make sure this is legal. Otherwise return a blank range.
    if ( not_reset( ) ) {
        yx_const_range_type yx_range = get_range_yx( );
        d_assert( yx_range.                  get_count( ) == get_y_count( ));
        d_assert( yx_range.get_next_range( ).get_count( ) == get_x_count( ));

        if ( yx_range.                  restrict_range_by_index( y_lo, y_hi_plus) &&
             yx_range.ref_next_range( ).restrict_range_by_index( x_lo, x_hi_plus) )
        {
            return yx_range;
        }
    }
    return yx_const_range_type( );
}

  sheet_type::yx_varia_range_type
  sheet_type::
get_range_yx
 (  size_type  x_lo
  , size_type  x_hi_plus
  , size_type  y_lo
  , size_type  y_hi_plus
 )
  // Exactly the same as const version above except for the substitution:
  //   yx_const_range_type ->
  //   yx_varia_range_type
{
    // Check to make sure this is legal. Otherwise return a blank range.
    if ( not_reset( ) ) {
        yx_varia_range_type yx_range = get_range_yx( );
        d_assert( yx_range.                  get_count( ) == get_y_count( ));
        d_assert( yx_range.get_next_range( ).get_count( ) == get_x_count( ));

        if ( yx_range.                  restrict_range_by_index( y_lo, y_hi_plus) &&
             yx_range.ref_next_range( ).restrict_range_by_index( x_lo, x_hi_plus) )
        {
            return yx_range;
        }
    }
    return yx_varia_range_type( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Setting values

  bool
  sheet_type::
fill_sheet( value_type new_value)
{
    // Fill in the entire sheet with value.
    if ( not_reset( ) ) {
        std::fill( begin( ), end( ), new_value);
        return true;
    }
    return false;
}

  bool
  sheet_type::
set_value_at( value_type new_value, size_type x, size_type y)
{
    if ( not_reset( ) &&
        /* (0 <= x) && */ (x < get_x_count( )) &&
        /* (0 <= y) && */ (y < get_y_count( )) )
    {
        yx_varia_range_type range_yx = get_range_yx( x, x + 1, y, y + 1);
        d_assert( range_yx.get_count( ) == 1);
        d_assert( range_yx.get_next_range( ).get_count( ) == 1);
        *(range_yx.get_leaf_iter( )) = new_value;
        return true;
    }
    return false;
}

// _______________________________________________________________________________________________

  /* private functor */
  namespace /* anonymous */ {
  struct
fill_functor_type
{
    fill_functor_type( sheet_type::value_type new_value)
      : value_( new_value) { }
      sheet_type::value_type const value_;

      sheet_type::value_type
    operator ()
     (  sheet_type::value_type  /* old_value */
      , sheet_type::size_type   /* x */
      , sheet_type::size_type   /* y */
     )
    {
        return value_;
    }
};
  } /* end namespace anonymous */

  bool
  sheet_type::
fill_rectangle_coords
 (  value_type  new_value
  , size_type   x_lo
  , size_type   x_hi_plus
  , size_type   y_lo
  , size_type   y_hi_plus
 )
{
    fill_functor_type op_inst( new_value);
    return transform_rectangle( op_inst, x_lo, x_hi_plus, y_lo, y_hi_plus);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* private functor */
  namespace /* anonymous */ {
  struct
scale_functor_type
{
    scale_functor_type( sheet_type::value_type scale)
      : scale_( scale) { }
      sheet_type::value_type const scale_;

      sheet_type::value_type
    operator ()
     (  sheet_type::value_type  old_value
      , sheet_type::size_type   /* x */
      , sheet_type::size_type   /* y */
     )
    {
        return old_value * scale_;
    }
};
  } /* end namespace anonymous */

  bool
  sheet_type::
scale_rectangle_coords
 (  value_type  scale
  , size_type   x_lo
  , size_type   x_hi_plus
  , size_type   y_lo
  , size_type   y_hi_plus
 )
{
    scale_functor_type op_inst( scale);
    return transform_rectangle( op_inst, x_lo, x_hi_plus, y_lo, y_hi_plus);
}

  bool
  sheet_type::
scale_sheet( value_type scale)
{
    return scale_rectangle_coords( scale, 0, get_x_count( ), 0, get_y_count( ));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Min/max

  namespace /* anonymous */ {
  struct
find_min_max_functor_type
{
    find_min_max_functor_type( )
      : is_found_( false)
      { }
      bool                    is_found_ ;
      sheet_type::value_type  min_value_;
      sheet_type::value_type  max_value_;

      bool
    operator ()
     (  sheet_type::value_type  value
      , sheet_type::size_type
      , sheet_type::size_type
     )
    {
        // Keep track of min/max.
        if ( ! is_found_ ) {
            is_found_ = true;
            min_value_ = value;
            max_value_ = value;
        } else
        if ( value < min_value_ ) {
            min_value_ = value;
        } else
        if ( max_value_ < value ) {
            max_value_ = value;
        }

        // Return true to keep scanning until we've looked at everything.
        return true;
    }
};
  } /* end namespace anonymous */

  sheet_type::value_type
  sheet_type::
get_min_value
 (  size_type  x_lo
  , size_type  x_hi_plus
  , size_type  y_lo
  , size_type  y_hi_plus
 ) const
{
    find_min_max_functor_type min_max_op;
    scan_rectangle( min_max_op,
        x_lo, x_hi_plus,
        y_lo, y_hi_plus);
    d_assert( min_max_op.is_found_);
    return min_max_op.min_value_;
}

  sheet_type::value_type
  sheet_type::
get_min_value( ) const
{
    return get_min_value( 0, get_x_count( ), 0, get_y_count( ));
}

  bool
  sheet_type::
get_min_max_values
 (  size_type     x_lo
  , size_type     x_hi_plus
  , size_type     y_lo
  , size_type     y_hi_plus
  , value_type &  return_min_value
  , value_type &  return_max_value
 ) const
{
    find_min_max_functor_type min_max_op;
    scan_rectangle( min_max_op,
        x_lo, x_hi_plus,
        y_lo, y_hi_plus);
    if ( min_max_op.is_found_ ) {
        return_min_value = min_max_op.min_value_;
        return_max_value = min_max_op.max_value_;
        return true;
    }
    return false;
}

  bool
  sheet_type::
get_min_max_values
 (  value_type &  return_min_value
  , value_type &  return_max_value
 ) const
{
    return
        get_min_max_values(
            0, get_x_count( ),
            0, get_y_count( ),
            return_min_value,
            return_max_value);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Normalize (and functor)

  namespace /* anonymous */ {
  struct
normalize_functor_type
  //
  // This could be a chain operator. The result could either be calculated on the way down (change_src)
  // on on the way back up (change_result).
{
    typedef sheet_type::value_type  value_type;
    typedef sheet_type::size_type   size_type ;

    normalize_functor_type
     (  value_type  src_lo
      , value_type  src_hi
      , value_type  trg_lo  = -1
      , value_type  trg_hi  = +1
     )
      : src_lo_( src_lo), src_hi_( src_hi)
      , trg_lo_( trg_lo), trg_hi_( trg_hi)
      , trg_src_ratio_( 0)
      {
        ctor_tail( );
      }

    normalize_functor_type
     (  sheet_type const &  sheet
      , value_type          trg_lo  = -1
      , value_type          trg_hi  = +1
     )
      : src_lo_( 0), src_hi_( 0)
      , trg_lo_( trg_lo)
      , trg_hi_( trg_hi) // only value not changed below
      , trg_src_ratio_( 0)
      {
        sheet.get_min_max_values( src_lo_, src_hi_);
        ctor_tail( );
      }

    normalize_functor_type
     (  sheet_type const &  sheet
      , size_type           x_lo
      , size_type           x_hi_plus
      , size_type           y_lo
      , size_type           y_hi_plus
      , value_type          trg_lo  = -1
      , value_type          trg_hi  = +1
     )
      : src_lo_( 0), src_hi_( 0)
      , trg_lo_( trg_lo)
      , trg_hi_( trg_hi) // only value not changed below
      , trg_src_ratio_( 0)
      {
        sheet.get_min_max_values( x_lo, x_hi_plus, y_lo, y_hi_plus, src_lo_, src_hi_);
        ctor_tail( );
      }

      // Sets trg_lo_ and trg_src_ratio_.
      // Call this at the end of every ctor.
      void
    ctor_tail( )
      {  if ( src_lo_ == src_hi_ ) {
          trg_src_ratio_ = 0; // zero means always return trg_lo
          trg_lo_ = (trg_lo_ + trg_hi_) / 2; // flat return value
        } else {
          // Same with or without commented-out if.. half-statement.
          /*if ( trg_lo_ == trg_hi_ ) {
            trg_src_ratio_ = 0; // always return trg_lo (which is the same as trg_hi)
          } else*/ {
            trg_src_ratio_ = (trg_hi_ - trg_lo_) / (src_hi_ - src_lo_);
          }
        }
      }

      // Get an internal value after the normalize operation is done.
      value_type
    get_trg_src_ratio( ) const
      { return trg_src_ratio_; }

      // Member vars
      value_type src_lo_, src_hi_;
      value_type trg_lo_, trg_hi_;
      value_type trg_src_ratio_;

      // functor operator()
      value_type
    operator ()
     (  value_type v
      , size_type
      , size_type
     )
      {
        if ( ! trg_src_ratio_ ) return trg_lo_;
        if ( v <= src_lo_ ) return trg_lo_;
        if ( v >= src_hi_ ) return trg_hi_;
        return ((v - src_lo_) * trg_src_ratio_) + trg_lo_;
      }
};
  } /* end namespace anonymous */

  bool
  sheet_type::
normalize
 (  size_type     x_lo
  , size_type     x_hi_plus
  , size_type     y_lo
  , size_type     y_hi_plus
  , value_type    trg_lo           /* = -1 */
  , value_type    trg_hi           /* = +1 */
  , value_type *  p_trg_src_ratio  /* =  0 */
 )
  // It's OK if (trg_lo > trg_hi). It will negate the values.
  // It's OK if (trg_lo == trg_hi). It will set all values the same.
{
    // First set this to zero for the following cases:
    //   We are flattening the sheet (in which case the delta-after/delta-before ratio is zero).
    //   The sheet is already flat (delta-before is zero). In this case we also flatten the trg
    //     (the after values) so delta-after/delta-before is 0/0 which is undefined.
    //   We run into an error (a mis-sized rectangle) and return false. In this case the values
    //     in the sheet are not changed.
    if ( p_trg_src_ratio ) {
        *p_trg_src_ratio = 0;
    }

    // Check the special case where trg_lo == trg_hi.
    // The function would work the same without this test; this is just a speedup.
    if ( trg_lo == trg_hi ) {
        // Return false if there's something wrong with the rectangle coords.
        return fill_rectangle_coords( trg_lo, x_lo, x_hi_plus, y_lo, y_hi_plus);
    }

    // Find min/max so we can scale.
    value_type min_value;
    value_type max_value;
    if ( get_min_max_values( x_lo, x_hi_plus, y_lo, y_hi_plus, min_value, max_value) ) {
        if ( min_value < max_value ) {
            normalize_functor_type functor( min_value, max_value, trg_lo, trg_hi);
            d_verify(
                transform_rectangle(
                    functor,
                    x_lo, x_hi_plus,
                    y_lo, y_hi_plus));
            if ( p_trg_src_ratio ) {
                *p_trg_src_ratio = functor.get_trg_src_ratio( );
            }
        } else {
            // If min==max, we cannot spread the values out between trg_lo and trg_hi.
            // So instead just set them all to the mid point.
            d_assert( min_value == max_value);
            d_verify(
                fill_rectangle_coords( (trg_lo + trg_hi) / 2, x_lo, x_hi_plus, y_lo, y_hi_plus));
        }
        // If get_min_max_value(..) didn't fail, then we know the rectangle coords are OK.
        return true;
    }

    // Return false if there's something wrong with the rectangle coords.
    return false;
}

  bool
  sheet_type::
normalize
 (  value_type    trg_lo           /* = -1 */
  , value_type    trg_hi           /* = +1 */
  , value_type *  p_trg_src_ratio  /* =  0 */
 )
{
    return
        normalize(
            0, get_x_count( ),
            0, get_y_count( ),
            trg_lo, trg_hi, p_trg_src_ratio);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Copy (with line_walker_type so you can copy between sheets with different sizes)

  namespace /* anonymous */ {
  /* functor struct (provides two functions for line_walker_type copy templates) */
  /* this should be part of stride_iter so we can always copy one stride_range to another */
  struct
copy_preserve_heights_assign_functor
{
    typedef sheet_type::size_type           size_type          ;
    typedef sheet_type::value_type          value_type         ;
    typedef sheet_type::yx_const_iter_type  yx_const_iter_type ;
    typedef sheet_type::yx_varia_iter_type  yx_varia_iter_type ;
    typedef sheet_type::x_const_range_type  x_const_range_type ;
    typedef sheet_type::x_varia_range_type  x_varia_range_type ;
    typedef sheet_type::x_varia_iter_type   x_varia_iter_type  ;
    typedef sheet_type::x_const_iter_type   x_const_iter_type  ;

    /* functor function (for line_walker_type copy templates) */
    void
  operator ()
   (  line_walker_type   const &  walker
    , yx_const_iter_type const &  src_yx_iter
    , yx_varia_iter_type const &  trg_yx_iter
    , bool                     /* is_new_src */
    , bool                        is_new_trg
   )
    {   x_const_range_type const  src_x_range  = *src_yx_iter;
        x_varia_range_type const  trg_x_range  = *trg_yx_iter;

        x_const_iter_type  const  src_x_iter   = src_x_range.get_iter_lo( );
        x_varia_iter_type  const  trg_x_iter   = trg_x_range.get_iter_lo( );

        size_type          const  src_x_count  = src_x_range.get_count( );
        size_type          const  trg_x_count  = trg_x_range.get_count( );

        // This is sort of ugly, but it saves us from having to divide the same two numbers over and
        // over again. This is based on the logic in line_walker_type::get_trg_value_preserve_area(..).
        // If we had a way of assigning a src-iterator to a trg-iterator and scaling at the same time
        // (and using line-walker when the iterators were different sizes), then we could use that
        // and the line-walker templates.
        if ( walker.is_trg_width_fully_covered( ) ) {
            d_verify( is_new_trg ?
              line_walker_type::copy_preserve_area(       src_x_iter, src_x_count, trg_x_iter, trg_x_count) :
              line_walker_type::accumulate_preserve_area( src_x_iter, src_x_count, trg_x_iter, trg_x_count) );
        } else
        /* we have to scale */ {
            value_type const scale_factor = walker.get_trg_overlap_ratio< value_type >( );
            d_verify( is_new_trg ?
              line_walker_type::scaled_copy_preserve_area(       scale_factor, src_x_iter, src_x_count, trg_x_iter, trg_x_count) :
              line_walker_type::scaled_accumulate_preserve_area( scale_factor, src_x_iter, src_x_count, trg_x_iter, trg_x_count) );
        }
    }

    /* functor function (for line_walker_type copy templates) */
    void
  operator ()
   (  yx_const_iter_type const &  src_yx_iter
    , yx_varia_iter_type const &  trg_yx_iter
   )
    {   x_const_range_type const  src_x_range  = *src_yx_iter;
        x_varia_range_type const  trg_x_range  = *trg_yx_iter;

        x_const_iter_type  const  src_x_iter   = src_x_range.get_iter_lo( );
        x_varia_iter_type  const  trg_x_iter   = trg_x_range.get_iter_lo( );

        size_type          const  src_x_count  = src_x_range.get_count( );
        size_type          const  trg_x_count  = trg_x_range.get_count( );

        line_walker_type::copy_preserve_area( src_x_iter, src_x_count, trg_x_iter, trg_x_count);
    }
};
  } /* end namespace anonymous */

// _______________________________________________________________________________________________

  /* static method */
  bool
  sheet_type::
copy_preserve_heights
 (  this_type const &  src_sheet
  , this_type       &  trg_sheet
 )
{
    size_type const  src_x_count  = src_sheet.get_x_count( );
    size_type const  trg_x_count  = trg_sheet.get_x_count( );
    size_type const  src_y_count  = src_sheet.get_y_count( );
    size_type const  trg_y_count  = trg_sheet.get_y_count( );

    if ( (src_y_count <= 0) &&
         (trg_y_count <= 0) &&
         (src_x_count <= 0) &&
         (trg_x_count <= 0) )
    {
        return true;
    }

    if ( (src_y_count <= 0) ||
         (trg_y_count <= 0) ||
         (src_x_count <= 0) ||
         (trg_x_count <= 0) )
    {
        return false;
    }

    yx_const_range_type  src_range  = src_sheet.get_range_yx( );
    yx_varia_range_type  trg_range  = trg_sheet.get_range_yx( );

    return
      line_walker_type::copy
       (  copy_preserve_heights_assign_functor( )
        , src_range.get_iter_lo( ), src_range.get_count( )
        , trg_range.get_iter_lo( ), trg_range.get_count( )
       );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Template methods to test mapping (transform) functions

  template< typename CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE >
  bool
  sheet_type::
fill_6_param_functor_1( value_type dist_fraction)
  //
  // This sums a 6-param functor on top of existing (flattened) sheet values.
{
    value_type const  z_max          = +1.05f;
    value_type const  z_min          = - z_max;
    value_type const  z_squeeze_max  = +0.1f;
    value_type const  z_squeeze_min  = - z_squeeze_max;

    size_type  const  x_count        = get_x_count( );
    size_type  const  y_count        = get_y_count( );
    value_type const  x_count_f      = static_cast< value_type >( x_count);
    value_type const  y_count_f      = static_cast< value_type >( y_count);
    return
        transform_rectangle
         (  make_sum_transform_functor
             (  normalize_functor_type( -1, +1, z_squeeze_min, z_squeeze_max)
              , CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE
                 (  x_count_f / 2
                  , y_count_f / 2
                  , x_count_f * dist_fraction
                  , y_count_f * dist_fraction
                  , z_min, z_max
                 )
             )
          , 0, x_count
          , 0, y_count
         ) &&
        normalize( );
}

  template< typename CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE >
  bool
  sheet_type::
fill_6_param_functor_2( value_type dist_fraction)
  //
  // Sums on top of existing sheet values.
{
    value_type const  z_max          = +1.05f;
    value_type const  z_min          = - z_max;
    value_type const  z_squeeze_max  = +0.1f;
    value_type const  z_squeeze_min  = - z_squeeze_max;

    size_type  const  x_count        = get_x_count( );
    size_type  const  y_count        = get_y_count( );
    value_type const  x_count_f      = static_cast< value_type >( x_count);
    value_type const  y_count_f      = static_cast< value_type >( y_count);
    return
        transform_rectangle
         (  make_sum_transform_functor
             (  normalize_functor_type( -1, +1, z_squeeze_min, z_squeeze_max)
              , make_sum_transform_functor
                 (  CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE
                     (  x_count_f / 4
                      , y_count_f / 4
                      , x_count_f * dist_fraction
                      , y_count_f * dist_fraction
                      , z_min, z_max
                     )
                  , CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE
                     (  x_count_f * 3 / 4
                      , y_count_f * 3 / 4
                      , x_count_f * dist_fraction
                      , y_count_f * dist_fraction
                      , z_max, z_min
                     )
                 )
             )
          , 0, x_count
          , 0, y_count
         ) &&
        normalize( );
}

  template< typename CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE >
  bool
  sheet_type::
fill_6_param_functor_4( value_type dist_fraction)
  //
  // Sums on top of existing sheet values.
{
    value_type const  z_max          = +1.05f;
    value_type const  z_min          = - z_max;
    value_type const  z_squeeze_max  = +0.1f;
    value_type const  z_squeeze_min  = - z_squeeze_max;

    size_type  const  x_count        = get_x_count( );
    size_type  const  y_count        = get_y_count( );
    value_type const  x_count_f      = static_cast< value_type >( x_count);
    value_type const  y_count_f      = static_cast< value_type >( y_count);
    return
        transform_rectangle
         (  make_sum_transform_functor
             (  normalize_functor_type( -1, +1, z_squeeze_min, z_squeeze_max)
              , make_sum_transform_functor
                 (  make_sum_transform_functor
                     (  CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE
                         (  x_count_f / 4
                          , y_count_f / 4
                          , x_count_f * dist_fraction
                          , y_count_f * dist_fraction
                          , z_min, z_max
                         )
                      , CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE
                         (  x_count_f * 3 / 4
                          , y_count_f * 3 / 4
                          , x_count_f * dist_fraction
                          , y_count_f * dist_fraction
                          , z_min, z_max
                         )
                     )
                  , make_sum_transform_functor
                     (  CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE
                         (  x_count_f / 4
                          , y_count_f *3 / 4
                          , x_count_f * dist_fraction
                          , y_count_f * dist_fraction
                          , z_max, z_min
                         )
                      , CALC_NEW_VALUE_6_PARAM_FUNCTOR_TYPE
                         (  x_count_f * 3 / 4
                          , y_count_f / 4
                          , x_count_f * dist_fraction
                          , y_count_f * dist_fraction
                          , z_max, z_min
                         )
                     )
                 )
             )
          , 0, x_count
          , 0, y_count
         ) &&
        normalize( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Bell curve (experiments)

  bool
  sheet_type::
fill_bell_curve
 (  value_type  center_x    // value_type instead of size_type so it can be 50.5
  , value_type  center_y
  , value_type  std_dev_x
  , value_type  std_dev_y
  , value_type  value_z_lo  /* = static_cast< value_type >( -1) */
  , value_type  value_z_hi  /* = static_cast< value_type >( +1) */
 )
  // This does not sum on top of existing src values.
{
    return
        transform_sheet
         (  bell_curve_functor_type
             (  center_x, center_y
              , std_dev_x, std_dev_y
              , value_z_lo, value_z_hi
             )
         );
}

  bool
  sheet_type::
fill_bell_curve_1( )
  //
  // This sums a bell curve on top of existing sheet values.
{
    return fill_6_param_functor_1< bell_curve_functor_type >( 1 / static_cast< value_type >( 5));
}

  bool
  sheet_type::
fill_bell_curve_2( )
  //
  // Sums on top of existing sheet values.
{
    return fill_6_param_functor_2< bell_curve_functor_type >( 1 / static_cast< value_type >( 5));
}

  bool
  sheet_type::
fill_bell_curve_4( )
  //
  // Sums on top of existing sheet values.
{
    return fill_6_param_functor_4< bell_curve_functor_type >( 1 / static_cast< value_type >( 7));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// sin(x)/x (experiments)

  namespace /* anonymous */ {
  struct
sin_over_dist_functor_type
  //
  // Also called a normal distribution.
{
    typedef sheet_type::size_type   size_type  ;
    typedef sheet_type::value_type  value_type ;

      // Constructor
    sin_over_dist_functor_type
     (  value_type  center_x    // value_type instead of size_type so it can be 50.5
      , value_type  center_y
      , value_type  dist_x      // always positive, never zero (or negative)
      , value_type  dist_y      // always positive, never zero (or negative)
      , value_type  value_z_lo  = -1
      , value_type  value_z_hi  = +1
     )
      : center_x_( center_x)
      , center_y_( center_y)
      , pi_over_dist_x_( angle_holder::pi / dist_x)
      , pi_over_dist_y_( angle_holder::pi / dist_y)
      , z_lo_( value_z_lo + 1)
      , z_delta_( (value_z_hi - value_z_lo) / 2)
      {
        d_assert( dist_x > 0);
        d_assert( dist_y > 0);
      }
      // Member vars
      value_type const  center_x_       ;
      value_type const  center_y_       ;
      value_type const  pi_over_dist_x_ ;
      value_type const  pi_over_dist_y_ ;
      value_type const  z_lo_           ;
      value_type const  z_delta_        ;

      // Functor operator()
      value_type
    operator ()
     (  value_type  /* old_value_z - not used */
      , size_type   x
      , size_type   y
     )
    {
        value_type const delta_x = (x - center_x_) * pi_over_dist_x_;
        value_type const delta_y = (y - center_y_) * pi_over_dist_y_;
        value_type const delta_r = std::sqrt( (delta_x * delta_x) + (delta_y * delta_y));
        value_type const sin_over_x = (delta_r < 0.0001) ? 1 : (std::sin( delta_r) / delta_r);
        return z_lo_ + (z_delta_ * sin_over_x);
    }
};
  } /* end namespace anonymous */

// _______________________________________________________________________________________________

  bool
  sheet_type::
fill_sin_over_dist
 (  value_type  center_x    // value_type instead of size_type so it can be 50.5
  , value_type  center_y
  , value_type  dist_x
  , value_type  dist_y
  , value_type  value_z_lo  /* = static_cast< value_type >( -1) */
  , value_type  value_z_hi  /* = static_cast< value_type >( +1) */
 )
  // This does not sum on top of existing src values.
{
    return
        transform_sheet
         (  sin_over_dist_functor_type
             (  center_x, center_y
              , dist_x, dist_y
              , value_z_lo, value_z_hi
             )
         );
}

  bool
  sheet_type::
fill_sin_over_dist_1( )
{
    return fill_6_param_functor_1< sin_over_dist_functor_type >( 1 / static_cast< value_type >( 8));
}

  bool
  sheet_type::
fill_sin_over_dist_2( )
{
    return fill_6_param_functor_2< sin_over_dist_functor_type >( 1 / static_cast< value_type >( 10));
}

  bool
  sheet_type::
fill_sin_over_dist_4( )
{
    return fill_6_param_functor_4< sin_over_dist_functor_type >( 1 / static_cast< value_type >( 12));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Delta

  void
  sheet_type::
set_delta( )
  //
  // A standard kind of transformation that you'd apply to an image to identify edges.
  // Also a simple & crude first derivative (except for the hi edges).
  //
  // This fills in the entire sheet.
  //
  // We need a window iterator, and this should be functor over a 2x2 window.
  // This can easily operate by copying from a src to a trg sheet. Do we want transforms
  // to do this or work in-place?
{
    // L-shaped window.
    if ( (get_x_count( ) > 0) && (get_y_count( ) > 0) ) {
        yx_varia_range_type const  yx_range_trg    = get_range_yx( );  // trg sheet
        yx_varia_iter_type         yx_iter_trg     = yx_range_trg.get_iter_lo( );

        yx_const_range_type const  yx_range_src    = yx_range_trg; // src sheet
        yx_const_iter_type         yx_iter_src     = yx_range_src.get_iter_lo( );
        yx_const_iter_type  const  yx_iter_src_hi  = yx_range_src.get_iter_hi( );

        // Loop thru the rows, one by one.
        while ( yx_iter_src != yx_iter_src_hi ) {
            x_varia_range_type const  x_range_trg     = yx_iter_trg.get_range( );
            x_varia_iter_type         x_iter_trg      = x_range_trg.get_iter_lo( );

            yx_const_iter_type const  yx_iter_src_up  = yx_iter_src + 1;
            x_const_range_type const  x_range_src     = yx_iter_src   .get_range( );
            x_const_range_type const  x_range_src_up  = yx_iter_src_up.get_range( );

            x_const_iter_type         x_iter_src      = x_range_src   .get_iter_lo( );
            x_const_iter_type         x_iter_src_up   = x_range_src_up.get_iter_lo( );
            x_const_iter_type  const  x_iter_src_hi   = x_range_src   .get_iter_hi( );

            // Loop thru the cells in each row.
            while ( x_iter_src != x_iter_src_hi ) {
                x_const_iter_type const x_iter_src_over = x_iter_src + 1;

                // Set all the cells in this row (except the last cell).
                *x_iter_trg = (*x_iter_src_over + *x_iter_src_up) - (2 * (*x_iter_src));

                x_iter_src = x_iter_src_over;
                ++ x_iter_src_up;
                ++ x_iter_trg;
            }
            // Set the last cell in the row.
            if ( get_x_count( ) == 1 ) {
                *x_iter_trg = (*x_iter_src_up - *x_iter_src);
            } else
            if ( get_x_count( ) == 2 ) {
                *x_iter_trg = *(x_iter_trg - 1);
            } else {
                // We could keep going with this. If x_count>3 we could smooth out the
                // 2nd derivative, if >4 we could smooth the 3rd derivative, etc.
                *x_iter_trg = *(x_iter_trg - 1) + *(x_iter_trg - 1) - *(x_iter_trg - 2);
            }

            yx_iter_src = yx_iter_src_up;
            ++ yx_iter_trg;
        } /* done looping thru the rows */

        // We have not assigned values to the top row yet.
        //   yx_iter_trg is set to the top row
        //   yx_iter_src is set to the top row
        x_varia_range_type const  x_range_trg  = yx_iter_trg.get_range( );
        x_varia_iter_type         x_iter_trg   = x_range_trg.get_iter_lo( );

        // If there is only one row in the sheet ...
        if ( get_y_count( ) == 1 ) {
            // Nx1 sheet (one row).
            // Assign the values according to the source.
            x_const_range_type const  x_range_src    = yx_iter_src.get_range( );
            x_const_iter_type         x_iter_src     = x_range_src.get_iter_lo( );
            x_const_iter_type  const  x_iter_src_hi  = x_range_src.get_iter_hi( );

            while ( x_iter_src != x_iter_src_hi ) {
                x_const_iter_type const  x_iter_src_over = x_iter_src + 1;

                // Set all the cells (except the last) in the last row.
                *x_iter_trg = (*x_iter_src_over - *x_iter_src);

                x_iter_src = x_iter_src_over;
                ++ x_iter_trg;
            }

            // Fill in the last corner.
            if ( get_x_count( ) == 1 ) {
                // Copy from src to trg.
                // This would be the right thing to do if src and trg were different.
                // Since they're the same (in this case) we don't have to do anything here.
                *x_iter_trg = *x_iter_src;
            } else
            if ( get_x_count( ) == 2 ) {
                // Copy from the TRG cell before.
                *x_iter_trg = *(x_iter_trg - 1);
            } else {
                // Copy from the 2 TRG cells before.
                d_assert( get_x_count( ) > 2);
                *x_iter_trg = *(x_iter_trg - 1) + *(x_iter_trg - 1) - *(x_iter_trg - 2);
            }
        } else
        // If there are two rows ...
        if ( get_y_count( ) == 2 ) {
            // Nx2 sheet (two rows).
            // Copy the row below to this row.
            // We could also calculate the row in a 1-d fashion, but if the user wants this he
            // can make an Nx1 sheet.
            x_const_iter_type  const  x_iter_trg_hi     = x_range_trg.get_iter_hi( );
            x_const_range_type const  x_range_trg_down  = (yx_iter_trg - 1).get_range( );
            x_const_iter_type         x_iter_trg_down   = x_range_trg_down.get_iter_lo( );

            while ( x_iter_trg != x_iter_trg_hi ) {
                *x_iter_trg = *x_iter_trg_down;
                ++ x_iter_trg;
                ++ x_iter_trg_down;
            }
            // Fill in the last corner.
            *x_iter_trg = *x_iter_trg_down;
        } else
        /* y_count > 2 */ {
            // MxN, more than 2 rows (although we may have 2 or fewer columns).
            d_assert( get_y_count( ) > 2);

            // Fill in the row to smooth the slope at the edge.
            x_const_iter_type  const  x_iter_trg_hi   = x_range_trg.get_iter_hi( );
            x_const_range_type const  x_range_trg_d   = (yx_iter_trg - 1).get_range( );
            x_const_range_type const  x_range_trg_dd  = (yx_iter_trg - 2).get_range( );
            x_const_iter_type         x_iter_trg_d    = x_range_trg_d.get_iter_lo( );
            x_const_iter_type         x_iter_trg_dd   = x_range_trg_dd.get_iter_lo( );

            // Smooth the slope.
            while ( x_iter_trg != x_iter_trg_hi ) {
                *x_iter_trg = *x_iter_trg_d + *x_iter_trg_d - *x_iter_trg_dd;
                ++ x_iter_trg;
                ++ x_iter_trg_d;
                ++ x_iter_trg_dd;
            }

            // What do we want to do with the last corner?
            // We could just fill it in like all the other cells in the row, in order to smooth
            // slope going up.
            // Or we could smooth the slope in the x direction, like all the cells below the corner.
            // Or we could take an average value.

            // First store the value that smooths the slope in the y direction.
            *x_iter_trg = *x_iter_trg_d + *x_iter_trg_d - *x_iter_trg_dd;

            // If there are two or more previous values in the x direction ...
            if ( get_x_count( ) > 1 ) {
                // Calculate the value that would smooth the slope in the x direction.
                value_type const x_smooth = *(x_iter_trg - 1) + *(x_iter_trg - 1) - *(x_iter_trg - 2);

                // Set the corner to the average of the two smooth values.
                *x_iter_trg = (*x_iter_trg + x_smooth) / 2;
            }
        }
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

// For now we don't need or use these values.
// And if we have them we have to maintain them when outside callers change values in the sheet.
# if 0
// Do this so min and max work whether or not <WinDef.h> is included.
// Windows defines _cpp_min() and _cpp_max(), which are the same as std::min() and std::max(),
// but that's useless.
// To disable the generation of the min and max macros in Visual C++, #define NOMINMAX
using std::min;
using std::max;

  /* Private method */
  void
  sheet_type::
set_min_max_values( )
{
    if ( is_reset( ) ) {
        z_min_ = value_type(  0);
        z_max_ = value_type( -1);
    } else {
        // Inefficient: loops thru twice.
        const_iterator const iter_begin = begin( );
        const_iterator const iter_end   = end  ( );
        z_min_ = *(std::min_element( iter_begin, iter_end));
        z_max_ = *(std::max_element( iter_begin, iter_end));
    }
}
# endif

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// sheet.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
