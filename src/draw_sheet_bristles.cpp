// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// draw_sheet_bristles.cpp
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

# include "all.h"
# include "draw_sheet_bristles.h"
# include "gl_env_global.h"

// _______________________________________________________________________________________________

typedef draw_sheet_bristles_type::pt_type pt_type;

// _______________________________________________________________________________________________

  namespace /* anonymous */ {
  float
calc_hair_length( float length_from_ui, bool is_lines)
  //
  // Return hair-length value suitable for drawing.
  // The UI values are [0..100] or [1..100].
  // Adjust them to values that make sense as lengths in the model coord system.
  //
  // Return 0 to stop a dot drawing on this side. Never return a negative value for a dot.
  // Never return 0 if we're drawing a line instead of a dot. For a line length there is only
  // on negative value we ever return. All the other returns are positive.
  //
  // Draw nothing if both dot lengths are zero.
  // Draw nothing if both line lengths are negative.
{
    // Cannot be zero. Return this negated for lines that are zero-length on this side.
    float const min_hair_length      =  0.002f; /* must be positive */
    float const negative_line_length = -0.001f; /* must be negative */

    // If the UI value is not positive, cancel the dot drawing and tell the line to only
    // draw on the other side of the surface.
    if ( length_from_ui <= 0 ) {
        // I doubt the UI lets this happen.
        return is_lines ? negative_line_length : 0;
    }

    // The UI give us a value in the rangel [1..100].
    float length_for_model = length_from_ui / 80.0f; /* normalize the value to be near 1 */
    length_for_model *= length_for_model; /* square the number */

    if ( length_for_model < min_hair_length ) {
        return is_lines ? negative_line_length : 0;
    }
    return length_for_model;

} } /* end namespace anonymous */

// _______________________________________________________________________________________________

  /* ctor */
  draw_sheet_bristles_type::
draw_sheet_bristles_type
 (  bristle_properties_style_type *  p_props
  , coord_scale_type                 model_xy_scale
  , coord_scale_type                 model_z_scale
 )
  : draw_sheet::base_type ( )
  , p_window_iter_        ( 0)
  , position_             ( p_props->get_position(    )->get_value( ))
  , top_color_            ( p_props->get_top_color(   )->get_color( ))
  , under_color_          ( p_props->get_under_color( )->get_color( ))
  , width_                ( p_props->get_pixel_width( )->get_value( ))
  , is_lines_             ( p_props->get_style( )->is_lines( ))
  , hair_top_length_      ( calc_hair_length( p_props->get_top_length(   )->get_value( ), p_props->get_style( )->is_lines( )))
  , hair_under_length_    ( calc_hair_length( p_props->get_under_length( )->get_value( ), p_props->get_style( )->is_lines( )))
  , antialias_style_      ( *(p_props->get_antialias_style( )))
  , model_xy_scale_       ( model_xy_scale)
  , model_z_scale_        ( model_z_scale )
  , one_over_mxy_sqrd_    ( 1 / (model_xy_scale * model_xy_scale))
  , one_over_mz_sqrd_     ( 1 / (model_z_scale  * model_z_scale ))
{
    d_assert( p_props->get_style( )->is_dots( ) || p_props->get_style( )->is_lines( ));
}

// _______________________________________________________________________________________________

  bool
  draw_sheet_bristles_type::
is_drawable( )
{
    if ( is_lines_ ) {
        // When drawing lines, neither length is ever zero.
        // At least one length must be positive for drawing to happen.
        d_assert( 0 != hair_top_length_  );
        d_assert( 0 != hair_under_length_);
        return (0 < hair_top_length_) || (0 < hair_under_length_);
    }

    // When drawing dots, neither length is negative.
    // If both are zero then nothing is drawn.
    d_assert( 0 <= hair_top_length_  );
    d_assert( 0 <= hair_under_length_);
    return (0 != hair_top_length_) || (0 != hair_under_length_);
}

// _______________________________________________________________________________________________

  void
  draw_sheet_bristles_type::
setup_gl( )
{
    if ( is_lines_ ) {
        gl_env::global::set_line_width( width_); // GL_LINE_BIT
        gl_env::global::set_shade_model_smooth( ); // GL_LIGHTING_BIT
        antialias_style_.set_gl( false, true);
    } else {
        gl_env::global::set_point_size( width_); // GL_POINT_BIT
        antialias_style_.set_gl( true, false);
    }

    // Disable lighting and materials and just use straight colors.
    gl_env::global::disable_overall_lighting( ); // GL_ENABLE_BIT, GL_LIGHTING_BIT
}

// _______________________________________________________________________________________________

  /* public */
  void
  draw_sheet_bristles_type::
draw( sheet_type const & sheet)
{
    if ( sheet.is_reset( ) ) {
        // The background is already clear, so draw nothing.
    } else
    if ( (sheet.get_x_count( ) < 2) || (sheet.get_y_count( ) < 2) ) {
        // This drawing function cannot draw a single cell, or a single row or column.
        // If we ever need to draw this add the code here.
    } else {
        if ( is_drawable( ) ) {
            setup_gl( );
            draw_surface( sheet, is_lines_ ? GL_LINES : GL_POINTS);
        }
    }
}

// _______________________________________________________________________________________________

  /* overridden virtual */
  void
  draw_sheet_bristles_type::
continue_strip( window_iter_type const & window_iter)
{
    d_assert( 0 == p_window_iter_);
    p_window_iter_ = & window_iter;

    switch ( position_ ) {
      case bristle_position_type::e_triangle_center : draw_triangle_center_bristles( ); break;
      case bristle_position_type::e_quad_center     : draw_quad_center_bristle(      ); break;
      case bristle_position_type::e_quad_quads      : draw_quad_quad_bristles(       ); break;
      case bristle_position_type::e_simple_corners  : draw_simple_corner_bristles(   ); break;
      case bristle_position_type::e_strip_corners   : draw_strip_corner_bristles(    ); break;
      case bristle_position_type::e_area_corners    : draw_area_corner_bristle(      ); break;
      case bristle_position_type::e_area_penta      : draw_quad_center_bristle(      );
                                                      draw_area_corner_bristle(      ); break;
      default :
        d_assert( false);
    }

    p_window_iter_ = 0;
}

// _______________________________________________________________________________________________

  void
  draw_sheet_bristles_type::
draw_triangle_center_bristles( )
  //
  // Centers of both triangles
  //   Simple normal of big angle (which is what we do here)
  //   Average of 2*big_corner + little_corner_a + little_corner_b
  //   Average of area-averaged normals
  //
  // We could draw the other two triangles too.
{
    d_assert( p_window_iter_);

    // Draw bristles in the middle of the two triangles.
    // Use the same triangles we used for the surface.
    // Use the normals at the triangle right angles.
    draw_top_and_under_bristles
     (  p_window_iter_->get_normal_trail_hi__any_length( )
      , ( p_window_iter_->get_pt_lead_hi(  ) +
          p_window_iter_->get_pt_trail_hi( ) +
          p_window_iter_->get_pt_trail_lo( ) ) / coord_scale_type( 3)
     );
    draw_top_and_under_bristles
     (  p_window_iter_->get_normal_lead_lo__any_length( )
      , ( p_window_iter_->get_pt_trail_lo( ) +
          p_window_iter_->get_pt_lead_lo(  ) +
          p_window_iter_->get_pt_lead_hi(  ) ) / coord_scale_type( 3)
     );
}

  void
  draw_sheet_bristles_type::
draw_quad_center_bristle( )
  //
  // Center of quad
  //   Average normal of 4 corners:
  //     Simple corners (probably the best, this is what we do), or
  //     Strip-average normals (maybe not), or
  //     Area-average normals (maybe not)
  //
  // Draw this alone or with area-averaged corners.
{
    d_assert( p_window_iter_);

    // Draw bristle in the middle of the quad.
    // Use the average normal over all 4 corners of the quad.
    draw_top_and_under_bristles
     (  ( p_window_iter_->get_normal_trail_hi__unit_length( ) +
          p_window_iter_->get_normal_trail_lo__unit_length( ) +
          p_window_iter_->get_normal_lead_hi__unit_length(  ) +
          p_window_iter_->get_normal_lead_lo__unit_length(  ) )

      , ( p_window_iter_->get_pt_lead_hi(  ) +
          p_window_iter_->get_pt_lead_lo(  ) +
          p_window_iter_->get_pt_trail_hi( ) +
          p_window_iter_->get_pt_trail_lo( ) ) / coord_scale_type( 4)
     );
}

  void
  draw_sheet_bristles_type::
draw_quad_quad_bristles( )
  //
  // This is just like draw_triangle_center_bristles( ) except for all 4 possible
  // triangles instead of two, and the middle_pts moved out towards the corners.
  //
  // This is just like draw_simple_corner_bristles( ) except not at the corners but
  // moved in to the quad.
{
    d_assert( p_window_iter_);

    draw_top_and_under_bristles
     (  p_window_iter_->get_normal_trail_hi__any_length( )
      , ( p_window_iter_->get_pt_lead_hi(  ) +
          p_window_iter_->get_pt_trail_lo( ) +
          p_window_iter_->get_pt_trail_hi( ) + /* add this one in twice */
          p_window_iter_->get_pt_trail_hi( ) ) / coord_scale_type( 4)
     );
    draw_top_and_under_bristles
     (  p_window_iter_->get_normal_trail_lo__any_length( )
      , ( p_window_iter_->get_pt_lead_lo(  ) +
          p_window_iter_->get_pt_trail_hi( ) +
          p_window_iter_->get_pt_trail_lo( ) + /* add this one in twice */
          p_window_iter_->get_pt_trail_lo( ) ) / coord_scale_type( 4)
     );
    draw_top_and_under_bristles
     (  p_window_iter_->get_normal_lead_hi__any_length( )
      , ( p_window_iter_->get_pt_trail_hi( ) +
          p_window_iter_->get_pt_lead_lo(  ) +
          p_window_iter_->get_pt_lead_hi(  ) + /* add this one in twice */
          p_window_iter_->get_pt_lead_hi(  ) ) / coord_scale_type( 4)
     );
    draw_top_and_under_bristles
     (  p_window_iter_->get_normal_lead_lo__any_length( )
      , ( p_window_iter_->get_pt_trail_lo( ) +
          p_window_iter_->get_pt_lead_hi(  ) +
          p_window_iter_->get_pt_lead_lo(  ) + /* add this one in twice */
          p_window_iter_->get_pt_lead_lo(  ) ) / coord_scale_type( 4)
     );
}

// _______________________________________________________________________________________________

  void
  draw_sheet_bristles_type::
draw_simple_corner_bristles( )
  //
  // Simple normals.
{
    d_assert( p_window_iter_);

    // All 4 simple corners.
    draw_top_and_under_bristles
     (  p_window_iter_->get_normal_trail_hi__any_length( )
      , p_window_iter_->get_pt_trail_hi( )
     );
    draw_top_and_under_bristles
     (  p_window_iter_->get_normal_trail_lo__any_length( )
      , p_window_iter_->get_pt_trail_lo( )
     );
    draw_top_and_under_bristles
     (  p_window_iter_->get_normal_lead_hi__any_length( )
      , p_window_iter_->get_pt_lead_hi( )
     );
    draw_top_and_under_bristles
     (  p_window_iter_->get_normal_lead_lo__any_length( )
      , p_window_iter_->get_pt_lead_lo( )
     );
}

  void
  draw_sheet_bristles_type::
draw_strip_corner_bristles( )
  //
  // Strip average, lead edge only.
  // This draws the trail edge, but only on the trail edge of the sheet.
{
    d_assert( p_window_iter_);

    if ( p_window_iter_->is_trail_edge( ) ) {
        // Simple corners, not strip averaged.
        draw_top_and_under_bristles
         (  p_window_iter_->get_normal_trail_hi__any_length( )
          , p_window_iter_->get_pt_trail_hi( )
         );
        draw_top_and_under_bristles
         (  p_window_iter_->get_normal_trail_lo__any_length( )
          , p_window_iter_->get_pt_trail_lo( )
         );
    }

    if ( p_window_iter_->is_lead_edge( ) ) {
        // Simple corners, not strip averaged.
        draw_top_and_under_bristles
         (  p_window_iter_->get_normal_lead_hi__any_length( )
          , p_window_iter_->get_pt_lead_hi( )
         );
        draw_top_and_under_bristles
         (  p_window_iter_->get_normal_lead_lo__any_length( )
          , p_window_iter_->get_pt_lead_lo( )
         );
    } else {
        draw_top_and_under_bristles
         (  p_window_iter_->get_normal_lead_hi__strip_average__any_length( )
          , p_window_iter_->get_pt_lead_hi( )
         );
        draw_top_and_under_bristles
         (  p_window_iter_->get_normal_lead_lo__strip_average__any_length( )
          , p_window_iter_->get_pt_lead_lo( )
         );
    }
}

  void
  draw_sheet_bristles_type::
draw_area_corner_bristle( )
  //
  // Area average, lead+hi edge only.
  // This draws the trail and lo edges, but only on the trail and lo edges of the sheet.
{
    d_assert( p_window_iter_);

    if ( p_window_iter_->is_trail_edge( ) ) {
        draw_top_and_under_bristles
         (  p_window_iter_->is_hi_edge( ) ?
                p_window_iter_->get_normal_trail_hi__any_length( ) :
                p_window_iter_->get_normal_trail_hi__half_area_average__any_length( )
          , p_window_iter_->get_pt_trail_hi( )
         );
        draw_top_and_under_bristles
         (  p_window_iter_->is_lo_edge( ) ?
                p_window_iter_->get_normal_trail_lo__any_length( ) :
                p_window_iter_->get_normal_trail_lo__half_area_average__any_length( )
          , p_window_iter_->get_pt_trail_lo( )
         );
    }

    if ( p_window_iter_->is_lead_edge( ) ) {
        draw_top_and_under_bristles
         (  p_window_iter_->is_hi_edge( ) ?
                p_window_iter_->get_normal_lead_hi__any_length( ) :
                p_window_iter_->get_normal_lead_hi__half_area_average__any_length( )
          , p_window_iter_->get_pt_lead_hi( )
         );
        draw_top_and_under_bristles
         (  p_window_iter_->is_lo_edge( ) ?
                p_window_iter_->get_normal_lead_lo__any_length( ) :
                p_window_iter_->get_normal_lead_lo__half_area_average__any_length( )
          , p_window_iter_->get_pt_lead_lo( )
         );
    } else {
        draw_top_and_under_bristles
         (  p_window_iter_->is_hi_edge( ) ?
                p_window_iter_->get_normal_lead_hi__strip_average__any_length( ) :
                p_window_iter_->get_normal_lead_hi__area_average__any_length( )
          , p_window_iter_->get_pt_lead_hi( )
         );
        draw_top_and_under_bristles
         (  p_window_iter_->is_lo_edge( ) ?
                p_window_iter_->get_normal_lead_lo__strip_average__any_length( ) :
                p_window_iter_->get_normal_lead_lo__area_average__any_length( )
          , p_window_iter_->get_pt_lead_lo( )
         );
    }
}

// _______________________________________________________________________________________________

  void
  draw_sheet_bristles_type::
draw_top_and_under_bristles
 (  pt_type const &  normal_vector__any_len
  , pt_type const &  middle_pt
 )
{
    // The normal vector will look right after it is scaled. It's not a normal in the draw coord system
    // we're in currently.
    pt_type const
        normal_vector__unit_len =
            calc_vector__unit_length__scaled_coords( normal_vector__any_len);

    // Do not draw the top point if the length is zero.
    // This design decision also affects the code just below (hair on the under surface) and also
    // earlier when we opt not to draw the dots or lines when both lengths are zero.
    // Maybe instead we should draw the point right on the surface?
    if ( 0 != hair_top_length_ ) {
        pt_type const drop_pt = middle_pt + (normal_vector__unit_len * hair_top_length_);
        gl_env::global::drop_color( top_color_);
        gl_env::global::drop_vertex( drop_pt.get_x( ), drop_pt.get_y( ), drop_pt.get_z( ));
    }

    // Do not draw the bottom point if the length is zero.
    if ( 0 != hair_under_length_ ) {
        pt_type const drop_pt = middle_pt - (normal_vector__unit_len * hair_under_length_);
        gl_env::global::drop_color( under_color_);
        gl_env::global::drop_vertex( drop_pt.get_x( ), drop_pt.get_y( ), drop_pt.get_z( ));
    }

    // Performance: If we are only drawing dots and only on one face we don't have to keep
    // dropping the same color over and over.
}

// _______________________________________________________________________________________________

  pt_type
  draw_sheet_bristles_type::
calc_vector__unit_length__scaled_coords( pt_type const & normal__random_length__local_coords)
  //
  // Converts local coords to scaled coords.
  // Also changes length so returned vector is unit-length (in scaled coords, not local coords).
{
    // normal__random_length__local_coords does not have to be a normal. It could be any vector in
    // the local coord system.

    // Calculate a normal vector that will still be normal after the model is scaled by model_xy_scale
    // and model_z_scale.
    //
    // This is really two transformations composed. Remember cross_vect is in draw-coords, which will be
    // translated, rotated, and scaled before getting to view coords. The translations and rotations are not
    // a problem, but when you scale a normal anisotropically you ruin it -- it no longer sticks out
    // perpendicular to the surface.
    //
    // So we calculate what the perpendicular normal outside the anisotropic scale is, by multiplying the
    // inner normal by the transpose of the inverse of the scale matrix. Since the scale was applied after
    // all rotations in the code leading to here, the scaling matrix is diagonal and easy to invert (and
    // it's its own transpose).
    //
    // That will give us a vector that is normal if applied outside the anisotropic scale.
    // But we have to draw it inside the scale, so we translate it in by multiplying it by the inverse of
    // the scale matrix, which is exactly the same matrix that we used earlier:
    //
    //   Scale matrix: [ sx   0   0  ]
    //                 [  0  sy   0  ]
    //                 [  0   0  sz  ]
    //
    //   Inverse:      [ 1/sx     0     0  ]
    //                 [    0  1/sy     0  ]
    //                 [    0     0  1/sz  ]

    // Apply the above inverse matrix twice: once to get normal into unscaled coords, and once to fix
    // that normal so we can draw it inside the anisotropic transform.
    pt_type const normal_outside_moved_inside
                    (  normal__random_length__local_coords.get_x( ) * one_over_mxy_sqrd_
                     , normal__random_length__local_coords.get_y( ) * one_over_mxy_sqrd_
                     , normal__random_length__local_coords.get_z( ) * one_over_mz_sqrd_
                    );

    // We now have a vector that will be normal once the model scaling is applied. This vector points
    // in the right direction, but it has some random length.
    //
    // We need to divide the vector by its magnitude to make it a unit normal. But we don't want the simple
    // magnitude from get_magnitude(..), because that'll unit-ize the vector in the scaled coord system.
    // Instead we want the magnitude relative to the surface of an ellipsoid defined by the scale factors,
    // so it'll look right after model scaling.
    //
    // Imagine a unit sphere outside the anisotropic-scaled coord system. You can move that sphere into the
    // coord system by transforming it by the inverse of the scaling matrix. You get an ellipsoid.
    // Here we're projecting our random-length normal from the center of that ellipsoid, and setting its
    // length to just touch the surface of the ellipsoid.
    coord_scale_type const elliptical_x         = normal_outside_moved_inside.get_x( ) * model_xy_scale_;
    coord_scale_type const elliptical_y         = normal_outside_moved_inside.get_y( ) * model_xy_scale_;
    coord_scale_type const elliptical_z         = normal_outside_moved_inside.get_z( ) * model_z_scale_ ;
    coord_scale_type const elliptical_magnitude = std::sqrt( (elliptical_x * elliptical_x) +
                                                             (elliptical_y * elliptical_y) +
                                                             (elliptical_z * elliptical_z) );
    d_assert( elliptical_magnitude > 0);

    // Return the vector that will look normal and be the right length after scaling.
    return normal_outside_moved_inside / elliptical_magnitude;
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// draw_sheet_bristles.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
