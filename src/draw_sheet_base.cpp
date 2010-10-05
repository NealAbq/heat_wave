// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// draw_sheet_base.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Additions:
//   Provide a way to draw the sheet quickly, skipping over cells and rows. Use line_walker_type.
//
//   Provide a way to draw some sections roughly and others in high detail.
//   Provide a way to draw gaps between the strips or between the cells in a strip?
//
//   Eventually we will want several of these drawing-objects, to draw different aspects:
//     Contour (drawing below)
//       Flat landscape (drawing from above)
//     Isotherms
//       As lines on landscape.
//       As enclosing paths from above (on flat landscape)
//     Temperature graph of line through the data (line doesn't have to be straight)
//       Graphs over time instead of over distance
//     Histogram of temperature buckets
//       Statistics like hi/lo temp, highest gradient
//     Graph other functions:
//       Gradient of Temperature (spacial derivative) (vector field)
//       Magnitudes of grad
//       Div-grad (lapacian)
//
//   When we start modeling materials (instead of just a uniform sheet like we do now):
//     Displays showing more model features:
//       Show embedded objects or object borders
//       Heat capacity of different regions
//       Heat conductivity between different regions
//       Textures (and bumps) to show materials and material properties
//
// Design:
//   We should walk 4 strips at once when we are providing area normal.
//   Although we only need 2 strips for simpler normals.
//
//   These are the strips we can use:
//     pt_type        draw_point      - x and y values are the same for every strip
//     color_type     color           - mapped to z coord, sometimes not used
//     pt_type        simple normals  - 4 for each vertex, but we don't ever need to save these
//     pt_type        strip_averaged_normals
//                                    - 2 for each vertex
//     pt_type        area_averaged_normals
//                                    - 1 for each vertex
// _______________________________________________________________________________________________

# include "all.h"
# include "draw_sheet_base.h"
# include "gl_env_global.h"

// _______________________________________________________________________________________________

namespace draw_sheet {

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  pt_type
calc_normal_any_length( pt_type const & a, pt_type const & b, pt_type const & c)
  //
  // Calculates a normal vector to the triangle using the right-hand rule.
  // The pts (a, b, c) define a triangle with ccw wrapping.
  //
  // Never returns a zero vector.
{
    pt_type const  to_a   = a - b;
    pt_type const  to_c   = c - b;
    // Calculate a normal vector with unknown-length.
    pt_type const  cross  = cross_product( to_c, to_a);

    // The cross-product will be zero if either vector is zero or if the two vectors are parallel.
    if ( cross.is_zero( ) ) {
        // This should never happen. Although maybe we could end up rounding to zero if the z values
        // are very very large.
        d_assert( false);
        // Probably some mixture of x- and y-values would be more accurate, but if this ever happens
        // the drawing is probably so messed up it doesn't matter what normal we choose here.
        return pt_type( 0, 0, coord_type( 1));
    }
    return cross;
}

  pt_type
calc_vector_unit_length( pt_type const & vector__random_length)
  //
  // Calculates a unit length vector parallel to vector__random_length.
{
    // Find out how long the vector is.
    coord_scale_type const
        magnitude =
            get_magnitude( vector__random_length);

    // If the vector is zero we cannot return a unit vector.
    // Should we return a zero vector, or an arbitrary unit vector?
    if ( 0 == magnitude ) {
        d_assert( false);
        // Return an arbitrary unit vector. The caller can assume this never returns a zero vector.
        return pt_type( 0, 0, coord_type( 1));
    }
    d_assert( magnitude > 0);

    // Change the random-length vector into a parallel unit-length vector.
    pt_type const
        normal__unit_length =
            vector__random_length / magnitude;
    // The magnitude of the unit_length vector should be close to 1.
    d_assert( normal__unit_length.not_zero( ));
    return normal__unit_length;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* constructor */
  window_iter_type::
window_iter_type( )
  : x_lo__all_cells_       ( 0)
  , y_lo__all_cells_       ( 0)
  , x_delta__single_cell_  ( 0)
  , y_delta__single_cell_  ( 0)
  , z_sheet_to_draw_ratio_ ( 0)
  , z_lo__sheet_coords_    ( 0)
  , z_lo__draw_coords_     ( 0)
  , z_delta__draw_coords_  ( 0)

  , yx_iter_lo_lo_         ( )
  , yx_iter_lo_            ( )
  , yx_iter_hi_            ( )
  , yx_iter_hi_hi_         ( )
  , yx_iter_max_           ( )

  , x_iter_lo_lo_          ( )
  , x_iter_lo_             ( )
  , x_iter_hi_             ( )
  , x_iter_hi_hi_          ( )
  , x_iter_lo_max_         ( )

  , is_trail_edge_         ( false)
  , is_lead_edge_          ( false)
  , is_lo_edge_            ( false)
  , is_hi_edge_            ( false)

  , pt_trail_lo_           ( 0, 0, 0)
  , pt_trail_hi_           ( 0, 0, 0)
  , pt_lead_lo_            ( 0, 0, 0)
  , pt_lead_hi_            ( 0, 0, 0)

  , pt_trail_lo_lo_        ( 0, 0, 0)
  , pt_trail_hi_hi_        ( 0, 0, 0)
  , pt_lead_lo_lo_         ( 0, 0, 0)
  , pt_lead_hi_hi_         ( 0, 0, 0)

  , pt_lead_lead_lo_       ( 0, 0, 0)
  , pt_lead_lead_hi_       ( 0, 0, 0)
{
}

// _______________________________________________________________________________________________

  void
  window_iter_type::
calc__sheet_to_draw__z_coord_factors( base_type const & base)
  //
  // Calculates:
  //   z_sheet_to_draw_ratio_
  //   z_lo__sheet_coords_
  //   z_lo__draw_coords_
  //   z_delta__draw_coords_
{
    // The lo Z in sheet and draw coords.
    z_lo__sheet_coords_    = base.get_z_lo__sheet_coords( );
    z_lo__draw_coords_     = base.get_z_lo__draw_coords( );
    z_delta__draw_coords_  = base.get_z_hi__draw_coords( ) - z_lo__draw_coords_;
    d_assert( z_delta__draw_coords_ > 0);

    // The height in sheet coords.
    coord_type const height_sheet_coords = base.get_z_hi__sheet_coords( ) - z_lo__sheet_coords_;
    d_assert( height_sheet_coords > 0);

    // Calc the sheet_to_draw_coords ratio. This is used to transform from z-sheet to z-draw coords.
    // Unit: draw-coords / sheet-coords
    z_sheet_to_draw_ratio_ = z_delta__draw_coords_ / height_sheet_coords;
    d_assert( z_sheet_to_draw_ratio_ > 0);
}

  coord_type /* returns a Z draw-coord */
  window_iter_type::
convert__sheet_to_draw__z_coord( coord_type z_sheet_coord) const
{
    d_assert( z_sheet_to_draw_ratio_ > 0);
    return z_lo__draw_coords_ +
        (z_sheet_to_draw_ratio_ * (z_sheet_coord - z_lo__sheet_coords_));
}

// _______________________________________________________________________________________________

  void
  window_iter_type::
calc_xy_cell_start_and_size
 (  base_type  const &  base
  , sheet_type const &  sheet
 )
{
    // Find out how many cells (quads) we are drawing.
    // Remember that each sheet values is a corner of a drawn cell, so a 2x2 sheet is drawn as
    // a single cell quad.
    d_assert( sheet.get_x_count( ) > 1); /* the smallest sheet we can draw is 2x2 */
    d_assert( sheet.get_y_count( ) > 1);
    count_type const  x_count  = sheet.get_x_count( ) - 1;
    count_type const  y_count  = sheet.get_y_count( ) - 1;

    // Find out how big the entire drawing will be.
    coord_type const  x_delta__draw_coords__all_cells
                               = base.get_x_hi__draw_coords__all_cells( ) -
                                 base.get_x_lo__draw_coords__all_cells( ) ;
    d_assert( x_delta__draw_coords__all_cells > 0);
    coord_type const  y_delta__draw_coords__all_cells
                               = base.get_y_hi__draw_coords__all_cells( ) -
                                 base.get_y_lo__draw_coords__all_cells( ) ;
    d_assert( y_delta__draw_coords__all_cells > 0);

    // Find the corner where we start drawing the sheet (in draw coords).
    x_lo__all_cells_ = base.get_x_lo__draw_coords__all_cells( );
    y_lo__all_cells_ = base.get_y_lo__draw_coords__all_cells( );

    // Find the size of a single cell in the sheet in draw coords.
    x_delta__single_cell_ = x_delta__draw_coords__all_cells / coord_type( x_count);
    y_delta__single_cell_ = y_delta__draw_coords__all_cells / coord_type( y_count);
    d_assert( x_delta__single_cell_ > 0);
    d_assert( y_delta__single_cell_ > 0);

    // The corner/delta values just calculated will now cause the drawing to fill the entire XY box.
    // But we want to control the xy shape of the cells in the drawing, so we may have to shrink
    // the drawing, either along the x or along the y direction.
    // This ratio has no unit (draw-coord / draw-coord).
    ratio_type const xy_ratio_uncorrected = x_delta__single_cell_ / y_delta__single_cell_;
    d_assert( xy_ratio_uncorrected > 0);

    // get_xy_draw_ratio__single_cell( ) is 2 if we want to draw the cells twice as wide as high.
    d_assert( base.get_xy_draw_ratio__single_cell( ) > 0);

    // Find out which (x or y) size we have to shrink.
    if ( base.get_xy_draw_ratio__single_cell( ) < xy_ratio_uncorrected ) {
        // xy_ratio_uncorrected is too big. Decrease the drawing size in the x direction.
        ratio_type const shrinker = base.get_xy_draw_ratio__single_cell( ) / xy_ratio_uncorrected;
        d_assert( (0 < shrinker) && (shrinker < 1));

        // Decrease x-size.
        x_delta__single_cell_ *= shrinker;
        // Shift x-lo so the sheet will still be centered.
        x_lo__all_cells_ += (1 - shrinker) * (base.get_x_hi__draw_coords__all_cells( ) - x_lo__all_cells_) / 2;
    } else
    if ( xy_ratio_uncorrected < base.get_xy_draw_ratio__single_cell( ) ) {
        // xy_ratio_uncorrected is too small. Decrease the drawing size in the y direction.
        ratio_type const shrinker = xy_ratio_uncorrected / base.get_xy_draw_ratio__single_cell( );
        d_assert( (0 < shrinker) && (shrinker < 1));

        // Decrease y-size.
        y_delta__single_cell_ *= shrinker;
        // Shift y-lo so the sheet will still be centered.
        y_lo__all_cells_ += (1 - shrinker) * (base.get_y_hi__draw_coords__all_cells( ) - y_lo__all_cells_) / 2;
    }
}

// _______________________________________________________________________________________________

  bool
  window_iter_type::
init_yx
 (  base_type  const &  base
  , sheet_type const &  sheet
 )
{
    // Init all the sheet-to-draw coordinate conversions.
    calc_xy_cell_start_and_size( base, sheet);
    calc__sheet_to_draw__z_coord_factors( base);
    d_assert( is_active( ));

    // We can draw any xy_ or yx_range. Since we know we are always drawing lo-to-hi coords, we can
    // use this to control back-to-front drawing order, as long as we know where the camera is.
    // Although back-to-front ordering by row ordering will be messed up if the camera's set up with
    // a large perspective angle.
    yx_const_range_type const yx_range = sheet.get_range_yx( );
    if ( yx_range.get_count( ) <= 1 ) {
        set_inactive( );
        return false;
    }

    // Setup the initial iterators.
    // If we didn't check (yx_range.get_count( ) <= 1) above we'd want to do this:
    //   if ( yx_iter_lo >= yx_iter_max ) return false;
    // before setting yx_iter_hi.
    yx_iter_lo_  = yx_range.get_iter_lo( );
    yx_iter_hi_  = yx_iter_lo_ + 1;
    yx_iter_max_ = yx_range.get_iter_hi( );
    d_assert( yx_iter_lo_ <  yx_iter_max_);
    d_assert( yx_iter_hi_ <= yx_iter_max_);

    // Reset the lo_lo values. These are not used until the 2nd iteration.
    yx_iter_lo_lo_.set( );
    pt_trail_lo_lo_.set_y( coord_type( 1000000));  /* not valid until 2nd iteration */
    pt_lead_lo_lo_ .set_y( coord_type( 1000000));  /* not valid until 2nd iteration */

    // Prepare the lo-row values.
    {   coord_type const y_next = get_y_lo__all_cells( );
        pt_trail_lo_    .set_y( y_next);
        pt_lead_lo_     .set_y( y_next);
        pt_lead_lead_lo_.set_y( y_next);
    }

    // Prepare the hi-row values.
    {   coord_type const y_next = get_y_lo__all_cells( ) + get_y_delta__single_cell( );
        pt_trail_hi_    .set_y( y_next);
        pt_lead_hi_     .set_y( y_next);
        pt_lead_lead_hi_.set_y( y_next);
    }

    // Setup the edges.
    is_lo_edge_ = true;
    setup_hi_edge( );

    d_assert( is_active( ));
    return true;
}

// _______________________________________________________________________________________________

  bool
  window_iter_type::
next_yx( )
{
    d_assert( is_active( ));

    // We are done when we finish the hi edge.
    if ( is_hi_edge_ ) {
        // Reset all the iterators into the sheet.
        yx_iter_lo_lo_.set( );
        yx_iter_lo_   .set( );
        yx_iter_hi_   .set( );
        yx_iter_hi_hi_.set( );
        yx_iter_max_  .set( );
        set_inactive( );
        return false;
    }

    // Move lo into lo_lo.
    yx_iter_lo_lo_ = yx_iter_lo_;
    {   coord_type const y_next = pt_trail_lo_.get_y( );
        pt_trail_lo_lo_  .set_y( y_next);
        pt_lead_lo_lo_   .set_y( y_next);
    }

    // Move hi into lo.
    yx_iter_lo_ = yx_iter_hi_;
    {   coord_type const y_next = pt_trail_hi_.get_y( );
        pt_trail_lo_     .set_y( y_next);
        pt_lead_lo_      .set_y( y_next);
        pt_lead_lead_lo_ .set_y( y_next);
    }

    // Move hi_hi into hi.
    yx_iter_hi_ = yx_iter_hi_hi_;
    {   coord_type const y_next = pt_trail_hi_hi_.get_y( );
        pt_trail_hi_     .set_y( y_next);
        pt_lead_hi_      .set_y( y_next);
        pt_lead_lead_hi_ .set_y( y_next);
    }

    // Setup the edges.
    is_lo_edge_ = false;
    setup_hi_edge( );

    d_assert( is_active( ));
    return true;
}

// _______________________________________________________________________________________________

  void
  window_iter_type::
setup_hi_edge( )
{
    d_assert( yx_iter_hi_ <= yx_iter_max_);
    is_hi_edge_ = (yx_iter_hi_ == yx_iter_max_);
    if ( is_hi_edge_ ) {
        // Reset the hi_hi values. These are not used if this is the last iteration.
        yx_iter_hi_hi_.set( );
        pt_trail_hi_hi_.set_y( coord_type( 1000000));  /* not valid during last iteration */
        pt_lead_hi_hi_ .set_y( coord_type( 1000000));  /* not valid during last iteration */
    } else {
        d_assert( yx_iter_hi_ < yx_iter_max_);

        yx_iter_hi_hi_ = yx_iter_hi_ + 1;
        d_assert( yx_iter_hi_hi_ <= yx_iter_max_);

        coord_type const y_next = pt_trail_hi_.get_y( ) + get_y_delta__single_cell( );
        pt_trail_hi_hi_.set_y( y_next);
        pt_lead_hi_hi_ .set_y( y_next);
    }
}

// _______________________________________________________________________________________________

  bool
  window_iter_type::
init_x( )
{
    // Get the middle two ranges.
    x_const_range_type const x_range_lo = *yx_iter_lo_;
    x_const_range_type const x_range_hi = *yx_iter_hi_;
    d_assert( x_range_lo.get_count( ) == x_range_hi.get_count( ));
    // We need at least two columns to draw anything.
    if ( x_range_lo.get_count( ) <= 1 ) return false;

    // Prepare the middle two row iterators.
    // We iterate over 2 adjacent rows (lo and hi). We also iterator over the two outside
    // rows (lo_lo and hi_hi) unless those are past the edge of the sheet.
    x_iter_hi_     = x_range_hi.get_iter_lo( );
    x_iter_lo_     = x_range_lo.get_iter_lo( );
    x_iter_lo_max_ = x_range_lo.get_iter_hi( );

    // We can assume this because we've already checked (x_range_lo.get_count( ) <= 1).
    d_assert( x_iter_lo_ < x_iter_lo_max_);

    // Set the XZ values for the trailing edge of the middle two rows.
    coord_type const x_trail = get_x_lo__all_cells( );
    pt_trail_lo_.set_x( x_trail).set_z( convert__sheet_to_draw__z_coord( *x_iter_lo_));
    pt_trail_hi_.set_x( x_trail).set_z( convert__sheet_to_draw__z_coord( *x_iter_hi_));

    // Move up to the next column.
    ++ x_iter_hi_;
    ++ x_iter_lo_;

    // Set the XZ values for the leading edge of the middle two rows.
    coord_type const x_lead = x_trail + get_x_delta__single_cell( );
    pt_lead_lo_.set_x( x_lead).set_z( convert__sheet_to_draw__z_coord( *x_iter_lo_));
    pt_lead_hi_.set_x( x_lead).set_z( convert__sheet_to_draw__z_coord( *x_iter_hi_));

    // Prepare the outer row iterators and trailing XZ values if they are available.
    if ( ! is_lo_edge( ) ) {
        x_iter_lo_lo_ = (*yx_iter_lo_lo_).get_iter_lo( );
        pt_trail_lo_lo_.set_x( x_trail).set_z( convert__sheet_to_draw__z_coord( *x_iter_lo_lo_));
        ++ x_iter_lo_lo_;
        pt_lead_lo_lo_ .set_x( x_lead ).set_z( convert__sheet_to_draw__z_coord( *x_iter_lo_lo_));
    }
    if ( ! is_hi_edge( ) ) {
        x_iter_hi_hi_ = (*yx_iter_hi_hi_).get_iter_lo( );
        pt_trail_hi_hi_.set_x( x_trail).set_z( convert__sheet_to_draw__z_coord( *x_iter_hi_hi_));
        ++ x_iter_hi_hi_;
        pt_lead_hi_hi_ .set_x( x_lead ).set_z( convert__sheet_to_draw__z_coord( *x_iter_hi_hi_));
    }

    is_trail_edge_ = true; /* true for the first iteration */
    setup_lead_edge( );

    d_assert( is_active( ));
    return true;
}

// _______________________________________________________________________________________________

  bool
  window_iter_type::
next_x( )
{
    d_assert( is_active( ));

    // We are done when we finish the lead edge.
    if ( is_lead_edge_ ) {
        // Reset all the iterators into the sheet.
        x_iter_lo_lo_ .set( );
        x_iter_lo_    .set( );
        x_iter_hi_    .set( );
        x_iter_hi_hi_ .set( );
        x_iter_lo_max_.set( );
        return false;
    }

    // Move the lead values into the trail points.
    coord_type const new_x_trail = pt_lead_lo_.get_x( );
    pt_trail_lo_.set_x( new_x_trail).set_z( pt_lead_lo_.get_z( ));
    pt_trail_hi_.set_x( new_x_trail).set_z( pt_lead_hi_.get_z( ));

    // Move the lead-lead values into the lead points.
    coord_type const new_x_lead = pt_lead_lead_lo_.get_x( );
    pt_lead_lo_.set_x( new_x_lead).set_z( pt_lead_lead_lo_.get_z( ));
    pt_lead_hi_.set_x( new_x_lead).set_z( pt_lead_lead_hi_.get_z( ));

    // If we're not at the lo edge, move the lo-lo lead into the trail, and set the lo-lo lead.
    if ( ! is_lo_edge( ) ) {
        pt_trail_lo_lo_.set_x( new_x_trail).set_z( pt_lead_lo_lo_.get_z( ));
        pt_lead_lo_lo_ .set_x( new_x_lead ).set_z( convert__sheet_to_draw__z_coord( *x_iter_lo_lo_));
    }

    // If we're not at the hi edge, move the hi-hi lead into the trail, and set the hi-hi lead.
    if ( ! is_hi_edge( ) ) {
        pt_trail_hi_hi_.set_x( new_x_trail).set_z( pt_lead_hi_hi_.get_z( ));
        pt_lead_hi_hi_ .set_x( new_x_lead ).set_z( convert__sheet_to_draw__z_coord( *x_iter_hi_hi_));
    }

    // Fix the lead edge.
    is_trail_edge_ = false; /* only true for the first iteration */
    setup_lead_edge( );

    d_assert( is_active( ));
    return true;
}

// _______________________________________________________________________________________________

  void
  window_iter_type::
setup_lead_edge( )
{
    d_assert( x_iter_lo_ <= x_iter_lo_max_);
    is_lead_edge_ = (x_iter_lo_ == x_iter_lo_max_); /* true for the last iteration */
    if ( is_lead_edge_ ) {
        // Reset the lead_lead values. These are not used if this is the last iteration.
        pt_lead_lead_lo_.set_x( coord_type( 1000000)).set_z( 0);  /* not valid */
        pt_lead_lead_hi_.set_x( coord_type( 1000000)).set_z( 0);  /* not valid */
    } else {
        d_assert( x_iter_lo_ < x_iter_lo_max_);
        ++ x_iter_lo_;
        ++ x_iter_hi_;

        coord_type const x_next = pt_lead_lo_.get_x( ) + get_x_delta__single_cell( );
        pt_lead_lead_lo_.set_x( x_next).set_z( convert__sheet_to_draw__z_coord( *x_iter_lo_));
        pt_lead_lead_hi_.set_x( x_next).set_z( convert__sheet_to_draw__z_coord( *x_iter_hi_));

        if ( ! is_lo_edge( ) ) {
            ++ x_iter_lo_lo_;
        }
        if ( ! is_hi_edge( ) ) {
            ++ x_iter_hi_hi_;
        }
    }
}

// _______________________________________________________________________________________________

  pt_type
  window_iter_type::
get_normal_trail_lo__any_length( ) const
{
    return
      calc_normal_any_length
       (  get_pt_trail_hi( )
        , get_pt_trail_lo( )
        , get_pt_lead_lo(  )
       );
}

  pt_type
  window_iter_type::
get_normal_trail_hi__any_length( ) const
{
    return
      calc_normal_any_length
       (  get_pt_lead_hi(  )
        , get_pt_trail_hi( )
        , get_pt_trail_lo( )
       );
}

  pt_type
  window_iter_type::
get_normal_lead_lo__any_length( ) const
{
    return
      calc_normal_any_length
       (  get_pt_trail_lo( )
        , get_pt_lead_lo(  )
        , get_pt_lead_hi(  )
       );
}

  pt_type
  window_iter_type::
get_normal_lead_hi__any_length( ) const
{
    return
      calc_normal_any_length
       (  get_pt_lead_lo(  )
        , get_pt_lead_hi(  )
        , get_pt_trail_hi( )
       );
}

// _______________________________________________________________________________________________

  pt_type
  window_iter_type::
get_normal_trail_lo__unit_length( ) const
{
    return
      calc_vector_unit_length(
        get_normal_trail_lo__any_length( ));
}

  pt_type
  window_iter_type::
get_normal_trail_hi__unit_length( ) const
{
    return
      calc_vector_unit_length(
        get_normal_trail_hi__any_length( ));
}

  pt_type
  window_iter_type::
get_normal_lead_lo__unit_length( ) const
{
    return
      calc_vector_unit_length(
        get_normal_lead_lo__any_length( ));
}

  pt_type
  window_iter_type::
get_normal_lead_hi__unit_length( ) const
{
    return
      calc_vector_unit_length(
        get_normal_lead_hi__any_length( ));
}

// _______________________________________________________________________________________________

  pt_type
  window_iter_type::
get_normal_lead_lo__strip_average__any_length( ) const
  //
  // This is similar to get_normal_lead_lo__half_area_average__any_length( ) except that function
  // is turned 90 degrees from this one. This one averages along the x axis, that one averages
  // along the y axis.
{
    d_assert( ! is_lead_edge( )); /* cannot be at the sheet's xlo edge */

    pt_type const corner_normal_unit_length = get_normal_lead_lo__unit_length( );
    pt_type const next_normal_any_length    = calc_normal_any_length( get_pt_lead_hi( ), get_pt_lead_lo( ), pt_lead_lead_lo_);
    pt_type const next_normal_unit_length   = calc_vector_unit_length( next_normal_any_length);
    pt_type const sum_unit_normals          = corner_normal_unit_length + next_normal_unit_length;

    // This is probably about length 2 since the two adjacent normals are usually close to parallel.
    // But it can have any length in the range 0..2 inclusive.
    // Zero-length is probably not possible, except due to rounding in extreme cases.
    return sum_unit_normals.not_zero( ) ? sum_unit_normals : pt_type( 0, 0, 1);
}

  pt_type
  window_iter_type::
get_normal_lead_hi__strip_average__any_length( ) const
  //
  // Averages along the x axis.
  // Also used as an area average along the lo (bottom) and hi (top) edges.
{
    d_assert( ! is_lead_edge( )); /* cannot be at the sheet's xhi edge */

    pt_type const corner_normal_unit_length = get_normal_lead_hi__unit_length( );
    pt_type const next_normal_any_length    = calc_normal_any_length( pt_lead_lead_hi_, get_pt_lead_hi( ), get_pt_lead_lo( ));
    pt_type const next_normal_unit_length   = calc_vector_unit_length( next_normal_any_length);
    pt_type const sum_unit_normals          = corner_normal_unit_length + next_normal_unit_length;

    return sum_unit_normals.not_zero( ) ? sum_unit_normals : pt_type( 0, 0, 1);
}

// _______________________________________________________________________________________________

  pt_type
  window_iter_type::
get_normal_trail_lo__half_area_average__any_length( ) const
  //
  // Averages along the y-axis.
{
    d_assert( ! is_lo_edge( )); /* cannot be at the sheet's ylo edge */

    pt_type const corner_normal_unit_length = get_normal_trail_lo__unit_length( );
    pt_type const next_normal_any_length    = calc_normal_any_length( get_pt_lead_lo( ), get_pt_trail_lo( ), pt_trail_lo_lo_);
    pt_type const next_normal_unit_length   = calc_vector_unit_length( next_normal_any_length);
    pt_type const sum_unit_normals          = corner_normal_unit_length + next_normal_unit_length;

    return sum_unit_normals.not_zero( ) ? sum_unit_normals : pt_type( 0, 0, 1);
}

  pt_type
  window_iter_type::
get_normal_trail_hi__half_area_average__any_length( ) const
  //
  // Averages along the y-axis.
{
    d_assert( ! is_hi_edge( )); /* cannot be at the sheet's yhi edge */

    pt_type const corner_normal_unit_length = get_normal_trail_hi__unit_length( );
    pt_type const next_normal_any_length    = calc_normal_any_length( pt_trail_hi_hi_, get_pt_trail_hi( ), get_pt_lead_hi( ));
    pt_type const next_normal_unit_length   = calc_vector_unit_length( next_normal_any_length);
    pt_type const sum_unit_normals          = corner_normal_unit_length + next_normal_unit_length;

    return sum_unit_normals.not_zero( ) ? sum_unit_normals : pt_type( 0, 0, 1);
}

  pt_type
  window_iter_type::
get_normal_lead_lo__half_area_average__any_length( ) const
  //
  // Averages along the y-axis.
  // Use this along the sheets trailing (xlo) edge.
{
    d_assert( ! is_lo_edge( )); /* cannot be at the sheet's ylo edge */

    pt_type const corner_normal_unit_length = get_normal_lead_lo__unit_length( );
    pt_type const next_normal_any_length    = calc_normal_any_length( pt_lead_lo_lo_, get_pt_lead_lo( ), get_pt_trail_lo( ));
    pt_type const next_normal_unit_length   = calc_vector_unit_length( next_normal_any_length);
    pt_type const sum_unit_normals          = corner_normal_unit_length + next_normal_unit_length;

    return sum_unit_normals.not_zero( ) ? sum_unit_normals : pt_type( 0, 0, 1);
}

  pt_type
  window_iter_type::
get_normal_lead_hi__half_area_average__any_length( ) const
  //
  // Averages along the y-axis.
  // Use this along the sheet's leading (xhi) edge.
{
    d_assert( ! is_hi_edge( )); /* cannot be at the sheet's yhi edge */

    pt_type const corner_normal_unit_length = get_normal_lead_hi__unit_length( );
    pt_type const next_normal_any_length    = calc_normal_any_length( get_pt_trail_hi( ), get_pt_lead_hi( ), pt_lead_hi_hi_);
    pt_type const next_normal_unit_length   = calc_vector_unit_length( next_normal_any_length);
    pt_type const sum_unit_normals          = corner_normal_unit_length + next_normal_unit_length;

    return sum_unit_normals.not_zero( ) ? sum_unit_normals : pt_type( 0, 0, 1);
}

// _______________________________________________________________________________________________

  pt_type
  window_iter_type::
get_normal_lead_lo__area_average__any_length( ) const
{
    d_assert( ! is_lead_edge( ));
    d_assert( ! is_lo_edge( ));

    // Get the 4 normals around the lead_lo corner.
    // We only want unit normals since we have to add them together.
    pt_type const n0 = get_normal_lead_lo__unit_length( );
    pt_type const n1 = calc_vector_unit_length(
                         calc_normal_any_length(
                           pt_lead_lo_lo_, get_pt_lead_lo( ), get_pt_trail_lo( )));
    pt_type const n2 = calc_vector_unit_length(
                         calc_normal_any_length(
                           pt_lead_lead_lo_, get_pt_lead_lo( ), pt_lead_lo_lo_));
    pt_type const n3 = calc_vector_unit_length(
                         calc_normal_any_length(
                           get_pt_lead_hi( ), get_pt_lead_lo( ), pt_lead_lead_lo_));

    // Add the normals together. The final length is probably a little less than 3, although
    // it can be anywhere from 0..4 inclusive.
    pt_type const sum_unit_normals = n0 + n1 + n2 + n3;

    // Return the sum. Make sure we don't return zero.
    return sum_unit_normals.not_zero( ) ? sum_unit_normals : pt_type( 0, 0, 1);
}

  pt_type
  window_iter_type::
get_normal_lead_hi__area_average__any_length( ) const
{
    d_assert( ! is_lead_edge( ));
    d_assert( ! is_hi_edge( ));

    pt_type const n0 = get_normal_lead_hi__unit_length( );
    pt_type const n1 = calc_vector_unit_length(
                         calc_normal_any_length(
                           get_pt_trail_hi( ), get_pt_lead_hi( ), pt_lead_hi_hi_));
    pt_type const n2 = calc_vector_unit_length(
                         calc_normal_any_length(
                           pt_lead_hi_hi_, get_pt_lead_hi( ), pt_lead_lead_hi_));
    pt_type const n3 = calc_vector_unit_length(
                         calc_normal_any_length(
                           pt_lead_lead_hi_, get_pt_lead_hi( ), get_pt_lead_lo( )));

    pt_type const sum_unit_normals = n0 + n1 + n2 + n3;
    return sum_unit_normals.not_zero( ) ? sum_unit_normals : pt_type( 0, 0, 1);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* constructor */
  base_type::
base_type( )
  : extent_lo__draw_coords_     ( -1, -1, -1)
  , extent_hi__draw_coords_     ( +1, +1, +1)
  , xy_draw_ratio__single_cell_ ( 1)
  , z_lo__sheet_coords_         ( -1)
  , z_hi__sheet_coords_         ( +1)
{
}

// _______________________________________________________________________________________________

  /* protected */
  void
  base_type::
draw_surface
 (  sheet_type const &  sheet
  , gl_env::enum_type   vertex_entry_mode
 )
{
    d_assert( gl_env::is_vertex_entry_mode( vertex_entry_mode));

    window_iter_type window_iter;
    if ( window_iter.init_yx( *this, sheet) ) {
        do {
            // Draw a single row in the surface.
            draw_surface_row( vertex_entry_mode, window_iter);
        }
        while ( window_iter.next_yx( ) );
    }
}

// _______________________________________________________________________________________________

  /* protected */
  void
  base_type::
draw_surface_row
 (  gl_env::enum_type   vertex_entry_mode
  , window_iter_type &  window_iter
 )
{
    if ( window_iter.init_x( ) ) {
        // Draw a strip of shapes - quads, triangles, lines, points, etc.
        gl_env::global::with_active_vertex_entry av( vertex_entry_mode);
        do {
            // Drop the next two points. This will define 2 triangles, or one quad, or normal lines or dots.
            continue_strip( window_iter); /* virtual, usually overridden */
        }
        while ( window_iter.next_x( ) );
    }
}

// _______________________________________________________________________________________________

  void
  base_type::
set_x_lo__draw_coords__all_cells( coord_type x_lo)
{
    extent_lo__draw_coords_.set_x( x_lo);
}

  void
  base_type::
set_y_lo__draw_coords__all_cells( coord_type y_lo)
{
    extent_lo__draw_coords_.set_y( y_lo);
}

  void
  base_type::
set_x_hi__draw_coords__all_cells( coord_type x_hi)
{
    extent_hi__draw_coords_.set_x( x_hi);
}

  void
  base_type::
set_y_hi__draw_coords__all_cells( coord_type y_hi)
{
    extent_hi__draw_coords_.set_y( y_hi);
}

  void
  base_type::
set_xy_draw_ratio__single_cell( ratio_type xy_ratio)
{
    d_assert( xy_ratio > 0);
    xy_draw_ratio__single_cell_ = xy_ratio;
}

  void
  base_type::
set_z_lo__draw_coords( coord_type z_lo)
{
    extent_lo__draw_coords_.set_z( z_lo);
}

  void
  base_type::
set_z_hi__draw_coords( coord_type z_hi)
{
    extent_hi__draw_coords_.set_z( z_hi);
}

  void
  base_type::
set_z_lo__sheet_coords( coord_type z_lo)
{
    z_lo__sheet_coords_ = z_lo;
}

  void
  base_type::
set_z_hi__sheet_coords( coord_type z_hi)
{
    z_hi__sheet_coords_ = z_hi;
}

// _______________________________________________________________________________________________

} /* end namespace draw_sheet */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// draw_sheet_base.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
