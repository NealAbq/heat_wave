// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// draw_sheet_surface.cpp
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
// Additions:
//   The caller sets up one of these:
//     Non-smooth (block) shading
//     Gouraud smooth shading
//     Phong smooth shading (need shaders)
//
//   Add strip-averaged normals.
//   Add area-averaged normals.
//
//   Specify texture coord instead of color. No, we'll probably auto-gen texture coords.
//     So we just need a way to skip colors.
//
//   The members of this class could include:
//     Temporary structures?
//       For now we sweep thru each row of cells twice, each time computing the 3-vertex and
//       color for each cell. With a buffer consisting of sheet.get_x_count( ) of colors and
//       vertices we wouldn't have to calculate them again, but the calculations are cheap so
//       I doubt this is worth much.
//       We cannot save on normal-vector calculations because these are calculated only once.
//     Display list
//       If we're drawing into a display list, that may be stored as part of this structure?
//
//   We could try drawing with smoothing (bezier spline) if we have a coarse grid.
// _______________________________________________________________________________________________

# include "all.h"
# include "draw_sheet_surface.h"
# include "gl_env_global.h"

// _______________________________________________________________________________________________

  /* ctor */
  draw_sheet_surface_type::
draw_sheet_surface_type
 (  color_gradient_holder const *   p_grad
  , bool                            is_triangles
  , shading_style_type::value_type  shading_style
 )
  : draw_sheet::base_type ( )
  , p_window_iter_        ( 0)
  , p_color_gradient_     ( p_grad)
  , is_drawing_triangles_ ( is_triangles)
  , shading_style_        ( shading_style)
{
    d_assert( p_color_gradient_);
}

// _______________________________________________________________________________________________

  /* public */
  void
  draw_sheet_surface_type::
draw( sheet_type const & sheet)
{
    if ( sheet.is_reset( ) ) {
        // The background is already clear, so draw nothing.
    } else
    if ( (sheet.get_x_count( ) < 2) || (sheet.get_y_count( ) < 2) ) {
        // This drawing function cannot draw a single cell, or a single row or column.
        // If we ever need to draw this add the code here.
    } else {
        draw_surface( sheet, is_drawing_triangles( ) ? GL_TRIANGLE_STRIP : GL_QUAD_STRIP);
    }
}

// _______________________________________________________________________________________________

  /* overridden pure virtual, framework */
  void
  draw_sheet_surface_type::
continue_strip( window_iter_type const & window_iter)
{
    d_assert( 0 == p_window_iter_);
    p_window_iter_ = & window_iter;

    if ( is_shading_strip_average( ) ) {
        continue_strip__strip_average( );
    } else
    if ( is_shading_area_average( ) ) {
        continue_strip__area_average( );
    } else
    if ( is_drawing_triangles( ) ) {
        if ( is_shading_block( ) ) {
            continue_strip__triangle_block( );
        } else
        /* is_shading_simple( ) */ {
            continue_strip__triangle_simple( );
        }
    } else
    /* drawing quads */ {
        if ( is_shading_block( ) ) {
            continue_strip__quad_block( );
        } else
        /* is_shading_simple( ) */ {
            continue_strip__quad_simple( );
        }
    }

    p_window_iter_ = 0;
}

// _______________________________________________________________________________________________

  void
  draw_sheet_surface_type::
continue_strip__triangle_block( ) const
  //
  // Assumes smooth shade is OFF.
  // Simple and block triangles have the same normals and only differ in the colors.
  // In block the vertex colors are set according to the average z-coord of the triangle.
{
    d_assert( p_window_iter_);
    d_assert( is_drawing_triangles( ));
    d_assert( is_shading_block( ));

    drop_normal_any_length( p_window_iter_->get_normal_trail_hi__any_length( ));
    drop_color(
        ( p_window_iter_->get_pt_trail_hi( ).get_z( ) +
          p_window_iter_->get_pt_trail_lo( ).get_z( ) +
          p_window_iter_->get_pt_lead_hi(  ).get_z( ) ) / coord_type( 3));

    if ( p_window_iter_->is_trail_edge( ) ) {
        drop_vertex_no_color( p_window_iter_->get_pt_trail_hi( )); /* 1st point in strip */
        drop_vertex_no_color( p_window_iter_->get_pt_trail_lo( )); /* 2nd point in strip */
    }
    drop_vertex_no_color( p_window_iter_->get_pt_lead_hi( )); /* defines a triangle */

    drop_normal_any_length( p_window_iter_->get_normal_lead_lo__any_length( ));
    drop_color(
        ( p_window_iter_->get_pt_lead_hi(  ).get_z( ) +
          p_window_iter_->get_pt_trail_lo( ).get_z( ) +
          p_window_iter_->get_pt_lead_lo(  ).get_z( ) ) / coord_type( 3));
    drop_vertex_no_color( p_window_iter_->get_pt_lead_lo( )); /* defines a triangle */
}

  void
  draw_sheet_surface_type::
continue_strip__triangle_simple( ) const
  //
  // Assumes smooth shade.
  // Simple and block triangles have the same normals and only differ in the colors.
  // In simple the vertex colors are set according to z-coord and interpolated.
  //
  // The normals are not very good because adjacent triangles share the vertices and the normals.
  // So the trailing two vertices for a triangle have normals from the last two triangles.
  // Improvements:
  //  - Use the triangle normal for the middle vertex, so a triangle has normals from the previous
  //    and next triangle, not from the previous two.
  //  - Draw the triangles as triangles so they don't have to share normals.
{
    d_assert( p_window_iter_);
    d_assert( is_drawing_triangles( ));
    d_assert( is_shading_simple( ));

    // For triangles, the first 3 vertices in the strip have the same normal.
    drop_normal_any_length( p_window_iter_->get_normal_trail_hi__any_length( ));

    if ( p_window_iter_->is_trail_edge( ) ) {
        drop_vertex( p_window_iter_->get_pt_trail_hi( )); /* 1st point in strip */
        drop_vertex( p_window_iter_->get_pt_trail_lo( )); /* 2nd point in strip */
    }
    drop_vertex( p_window_iter_->get_pt_lead_hi( )); /* defines a triangle */

    drop_normal_any_length( p_window_iter_->get_normal_lead_lo__any_length( ));
    drop_vertex( p_window_iter_->get_pt_lead_lo( )); /* defines a triangle */
}

// _______________________________________________________________________________________________

  void
  draw_sheet_surface_type::
continue_strip__quad_block( ) const
  //
  // In quad-block the normal is the average of the 4 simple corner normals.
  // And the color is from the average z-value of the 4 corners.
{
    d_assert( p_window_iter_);
    d_assert( ! is_drawing_triangles( ));
    d_assert( is_shading_block( ));

    drop_normal_any_length(
        p_window_iter_->get_normal_trail_hi__unit_length( ) +
        p_window_iter_->get_normal_trail_lo__unit_length( ) +
        p_window_iter_->get_normal_lead_hi__unit_length(  ) +
        p_window_iter_->get_normal_lead_lo__unit_length(  ) );
    drop_color(
        ( p_window_iter_->get_pt_trail_hi( ).get_z( ) +
          p_window_iter_->get_pt_trail_lo( ).get_z( ) +
          p_window_iter_->get_pt_lead_hi(  ).get_z( ) +
          p_window_iter_->get_pt_lead_lo(  ).get_z( ) ) / coord_type( 4));

    if ( p_window_iter_->is_trail_edge( ) ) {
        drop_vertex_no_color( p_window_iter_->get_pt_trail_hi( ));
        drop_vertex_no_color( p_window_iter_->get_pt_trail_lo( ));
    }
    drop_vertex_no_color( p_window_iter_->get_pt_lead_hi( ));
    drop_vertex_no_color( p_window_iter_->get_pt_lead_lo( ));
}

  void
  draw_sheet_surface_type::
continue_strip__quad_simple( ) const
  //
  // In quad-block, each corner gets the simple normal, and the colors are from simple z values.
  //
  // These are badly done normals. The leading-edge for the quad has correct normals, and the
  // trailing edge has normals from the last quad.
  // Improvements:
  //  - Make the middle two vertices have this quad's normals. The first normal can be from the
  //    previous quad, and the last can be from the next quad.
  //  - Draw the quads separately and not in a strip.
{
    d_assert( p_window_iter_);
    d_assert( ! is_drawing_triangles( ));
    d_assert( is_shading_simple( ));

    if ( p_window_iter_->is_trail_edge( ) ) {
        drop_normal_any_length( p_window_iter_->get_normal_trail_hi__any_length( ));
        drop_vertex( p_window_iter_->get_pt_trail_hi( ));

        drop_normal_any_length( p_window_iter_->get_normal_trail_lo__any_length( ));
        drop_vertex( p_window_iter_->get_pt_trail_lo( ));
    }

    drop_normal_any_length( p_window_iter_->get_normal_lead_hi__any_length( ));
    drop_vertex( p_window_iter_->get_pt_lead_hi( ));

    drop_normal_any_length( p_window_iter_->get_normal_lead_lo__any_length( ));
    drop_vertex( p_window_iter_->get_pt_lead_lo( ));
}

// _______________________________________________________________________________________________

  void
  draw_sheet_surface_type::
continue_strip__strip_average( ) const
{
    d_assert( p_window_iter_);
    d_assert( is_shading_strip_average( ));

    if ( p_window_iter_->is_trail_edge( ) ) {
        drop_normal_any_length( p_window_iter_->get_normal_trail_hi__any_length( ));
        drop_vertex( p_window_iter_->get_pt_trail_hi( ));

        drop_normal_any_length( p_window_iter_->get_normal_trail_lo__any_length( ));
        drop_vertex( p_window_iter_->get_pt_trail_lo( ));
    }

    if ( p_window_iter_->is_lead_edge( ) ) {
        drop_normal_any_length( p_window_iter_->get_normal_lead_hi__any_length( ));
        drop_vertex( p_window_iter_->get_pt_lead_hi( ));

        drop_normal_any_length( p_window_iter_->get_normal_lead_lo__any_length( ));
        drop_vertex( p_window_iter_->get_pt_lead_lo( ));
    } else {
        drop_normal_any_length( p_window_iter_->get_normal_lead_hi__strip_average__any_length( ));
        drop_vertex( p_window_iter_->get_pt_lead_hi( ));

        drop_normal_any_length( p_window_iter_->get_normal_lead_lo__strip_average__any_length( ));
        drop_vertex( p_window_iter_->get_pt_lead_lo( ));
    }
}

  void
  draw_sheet_surface_type::
continue_strip__area_average( ) const
{
    d_assert( p_window_iter_);
    d_assert( is_shading_area_average( ));

    if ( p_window_iter_->is_trail_edge( ) ) {
        drop_normal_any_length(
            p_window_iter_->is_hi_edge( ) ?
                p_window_iter_->get_normal_trail_hi__any_length( ) :
                p_window_iter_->get_normal_trail_hi__half_area_average__any_length( ));
        drop_vertex( p_window_iter_->get_pt_trail_hi( ));

        drop_normal_any_length(
            p_window_iter_->is_lo_edge( ) ?
                p_window_iter_->get_normal_trail_lo__any_length( ) :
                p_window_iter_->get_normal_trail_lo__half_area_average__any_length( ));
        drop_vertex( p_window_iter_->get_pt_trail_lo( ));
    }

    if ( p_window_iter_->is_lead_edge( ) ) {
        drop_normal_any_length(
            p_window_iter_->is_hi_edge( ) ?
                p_window_iter_->get_normal_lead_hi__any_length( ) :
                p_window_iter_->get_normal_lead_hi__half_area_average__any_length( ));
        drop_vertex( p_window_iter_->get_pt_lead_hi( ));

        drop_normal_any_length(
            p_window_iter_->is_lo_edge( ) ?
                p_window_iter_->get_normal_lead_lo__any_length( ) :
                p_window_iter_->get_normal_lead_lo__half_area_average__any_length( ));
        drop_vertex( p_window_iter_->get_pt_lead_lo( ));
    } else {
        drop_normal_any_length(
            p_window_iter_->is_hi_edge( ) ?
                p_window_iter_->get_normal_lead_hi__strip_average__any_length( ) :
                p_window_iter_->get_normal_lead_hi__area_average__any_length( ));
        drop_vertex( p_window_iter_->get_pt_lead_hi( ));

        drop_normal_any_length(
            p_window_iter_->is_lo_edge( ) ?
                p_window_iter_->get_normal_lead_lo__strip_average__any_length( ) :
                p_window_iter_->get_normal_lead_lo__area_average__any_length( ));
        drop_vertex( p_window_iter_->get_pt_lead_lo( ));
    }
}

// _______________________________________________________________________________________________

  void
  draw_sheet_surface_type::
drop_color( coord_type z_draw) const
{
    d_assert( p_window_iter_);
    gl_env::global::drop_color(
      p_color_gradient_->get_color_at_normal(
        (z_draw - p_window_iter_->get_z_lo__draw_coords( )) /
        p_window_iter_->get_z_delta__draw_coords( )
    ) );
}

  /* static */
  void
  draw_sheet_surface_type::
drop_vertex_no_color( pt_type const & pt)
{
    gl_env::global::drop_vertex( pt.get_x( ), pt.get_y( ), pt.get_z( ));
}

  void
  draw_sheet_surface_type::
drop_vertex( pt_type const & pt) const
{
    d_assert( p_window_iter_);
    gl_env::global::drop_color(
      p_color_gradient_->get_color_at_normal(
        (pt.get_z( ) - p_window_iter_->get_z_lo__draw_coords( )) /
        p_window_iter_->get_z_delta__draw_coords( )
    ) );

    gl_env::global::drop_vertex( pt.get_x( ), pt.get_y( ), pt.get_z( ));
}

// _______________________________________________________________________________________________

  // static
  void
  draw_sheet_surface_type::
drop_normal_any_length( pt_type const & normal)
  //
  // The normal vector should be unit vectors (magnitude 1), after the normal vector is transformed
  // into the "eye" coordinate system. You can scale the normals yourself, you you can have OpenGL
  // do it.
{
    // A zero vector is the only illegal value here.
    // Do we need to test for negative zeros? No, the test takes care of this.
    d_assert( normal.not_zero( ));
    gl_env::global::drop_normal_vector( normal.get_x( ), normal.get_y( ), normal.get_z( ));
}

// _______________________________________________________________________________________________

# if 0
  // static
  void
  draw_sheet_surface_type::
glNormal_ccw_do_not_scale_to_unit_length( pt_type const & a, pt_type const & b, pt_type const & c)
  //
  // Sets a normal vector for a vertex composed of points a, b, and c, so b is the point of
  // the vertex.
  // a, b, and c should be three points in a triangle/quad/polygon.
{
    pt_type const to_a  = a - b;
    pt_type const to_c  = c - b;
    pt_type const cross = cross_product( to_c, to_a);

    // The cross-product will be zero if either vector is zero or if the
    // two vectors are parallel.
    d_assert( cross.not_zero( ));

    glNormal_any_length( cross);
}

  // static
  void
  draw_sheet_surface_type::
glNormal_ccw_scale_to_unit_length( pt_type const & a, pt_type const & b, pt_type const & c)
  //
  // Sets a normal vector for a vertex composed of points a, b, and c, so b is the point of
  // the vertex.
  // a, b, and c should be three points in a triangle/quad/polygon.
{
    pt_type const to_a  = a - b;
    pt_type const to_c  = c - b;
    pt_type const cross = cross_product( to_c, to_a);

    // The cross-product will be zero if either vector is zero or if the
    // two vectors are parallel.
    d_assert( cross.not_zero( ));

    glNormal_scale_to_unit_length( cross);
}

  // static
  void
  draw_sheet_surface_type::
glNormal_scale_to_unit_length( pt_type const & normal)
  //
  // Use this if you want the your normal vectors to be unit vectors (in the current
  // coord system).
{
    coord_type const mag = get_magnitude( normal);
    // I don't think this ever happens, but test anyway.
    if ( mag ) {
        glNormal_is_unit_length( normal / mag);
    } else {
        d_assert( false);
    }
}

  // static
  void
  draw_sheet_surface_type::
glNormal_is_unit_length( pt_type const & unit_normal)
  //
  // Use this if your normal vector is of unit length.
{
  # ifndef NDEBUG
    // Magnitude of a vector in draw coords.
    // Instead of float this could be coord_type, or maybe another type like magnitude_type (in case
    // coord_type was an integer type).
    // These asserts have failed with very large sheet values (overflowing float values). In that case
    // we got a magnitude of zero. Maybe we should correct that here and change the unit normal to
    // (1,0,0) instead of passing along (0,0,0).
    { float const  debug_magnitude  = get_magnitude( unit_normal);
      d_assert( 0.99f < debug_magnitude);
      d_assert( 1.01f > debug_magnitude);
    }
  # endif

    glNormal_any_length( unit_normal);
}
# endif

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// draw_sheet_surface.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
