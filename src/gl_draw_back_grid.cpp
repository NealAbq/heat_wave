// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_draw_back_grid.cpp
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
# include "gl_draw_back_grid.h"
# include "gl_env_global.h"

// _______________________________________________________________________________________________
//
namespace gl_env {
namespace background {
// _______________________________________________________________________________________________

  /* private class */
  class
draw_back_grid_algorithm
{
  public:
    /* ctor */  draw_back_grid_algorithm( bool is_ortho, color_gradient_holder const * p_cg)
                  : is_ortho_projection( is_ortho)
                  , p_color_gradient( p_cg)
                  {
                    d_assert( p_color_gradient);
                  }

  public:
    void        draw_x_axis_quads
                 (  float_type  x0
                  , float_type  x1
                  , float_type  y
                  , float_type  corner_alpha
                  , float_type  middle_alpha
                 )                                                                   const ;
    void        draw_y_axis_quads
                 (  float_type  x
                  , float_type  y0
                  , float_type  y1
                  , float_type  corner_alpha
                  , float_type  middle_alpha
                 )                                                                   const ;

    void        draw_back_grid_x_plane( float_type x0, float_type x1, float_type y)  const ;
    void        draw_back_grid_y_plane( float_type x, float_type y0, float_type y1)  const ;
    void        draw_back_grid_corner
                 (  float_type  x_corner
                  , float_type  y_corner
                  , float_type  x_reach
                  , float_type  y_reach
                 )                                                                   const ;

  private:
    void        draw_x_axis_lines( float_type x0, float_type x1, float_type y)       const ;
    void        draw_y_axis_lines( float_type x, float_type y0, float_type y1)       const ;
    void        draw_z_axis_line( float_type x, float_type y)                        const ;

  private:
    void        color_and_vertex( float_type x, float_type y, int color_index)       const ;
    void        color_and_line_xxy
                 (  float_type  x0
                  , float_type  x1
                  , float_type  y
                  , int         color_index
                 )                                                                   const ;
    void        color_and_line_xyy
                 (  float_type  x
                  , float_type  y0
                  , float_type  y1
                  , int         color_index
                 )                                                                   const ;

    void        color_half_quad_xxy
                 (  float_type  x0
                  , float_type  x1
                  , float_type  y
                  , int         color_index
                  , float_type  alpha0
                  , float_type  alpha1
                 )                                                                   const ;
    void        color_half_quad_xyy
                 (  float_type  x
                  , float_type  y0
                  , float_type  y1
                  , int         color_index
                  , float_type  alpha0
                  , float_type  alpha1
                 )                                                                   const ;

    float_type  get_z_coord( int color_index)                                        const ;
    void        drop_color( int color_index)                                         const ;
    void        drop_color( int color_index, float_type alpha)                       const ;

  private:
    bool                  const    is_ortho_projection ;
    color_gradient_holder const *  p_color_gradient    ;
};

// _______________________________________________________________________________________________

  /* export function */
  void
draw_grid
 (  float_type                     angle
  , bool_type                      is_ortho_projection
  , color_gradient_holder const *  p_color_gradient
 )
{
    // Set the normal once, and leave it alone.
    gl_env::global::setup_normal_vector( 0.0f, 0.0f, 1.0f);

    // Leave material colors, color-tracking, lighting, etc the way we found it.
    gl_env::global::with_saved_server_attributes
        at( GL_ENABLE_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT | GL_LINE_BIT /* GL_HINT_BIT */ );

    // Disable lighting and just use straight colors. We are just drawing lines.
    gl_env::global::disable_overall_lighting( );          // GL_LIGHTING_BIT, GL_ENABLE_BIT

    // Setup polygon drawing for the backgrid.
    gl_env::global::set_shade_model_smooth( );            // GL_LIGHTING_BIT
    gl_env::global::set_polygon_mode( GL_FRONT, GL_FILL); // GL_POLYGON_BIT
    gl_env::global::enable_face_culling( );               // GL_POLYGON_BIT, GL_ENABLE_BIT
    gl_env::global::set_cull_face( GL_BACK);              // GL_POLYGON_BIT

    // Setup default line properties.
    gl_env::global::disable( GL_LINE_SMOOTH);             // GL_LINE_BIT, GL_ENABLE_BIT
    //gl_env::global::set_hint( GL_LINE_SMOOTH_HINT, GL_NICEST);  // GL_HINT_BIT
    gl_env::global::set_line_width( 1);                   // GL_LINE_BIT
    gl_env::global::set_line_stipple( 2, 0x8888);         // GL_LINE_BIT
    gl_env::global::enable_line_stipple( );               // GL_LINE_BIT, GL_ENABLE_BIT

    float_type const  angle_spread  = 10.0f; /* 10 degree angle on both sides of the axes */
    // Consider making angle_spread smaller as get_perspective_angle( ) gets close to zero.
    // get_perspective_angle( ) / 5?

    // Improve: These assume the model is drawn in a [-1 .. +1] box on the xy-plane.
    // We should get the real numbers and use separate x_lo, y_lo, x_hi, y_hi.
    float_type const  line_pad  = 0.02f;
    float_type const  x_hi      = 1 + line_pad; /* the model spans [-1 .. +1] */
    float_type const  y_hi      = x_hi;
    float_type const  x_lo      = - x_hi;
    float_type const  y_lo      = - y_hi;
    float_type const  dx        = x_hi - x_lo;
    float_type const  dy        = y_hi - y_lo;

    draw_back_grid_algorithm alg( is_ortho_projection, p_color_gradient);

    // Draw the grid lines. Only draw the back lines.
    // Instead of testing angle here we could set up a clip plane, although since the modelview matrix is already
    // set to model space we'd want to push a modelview identity when setting the clip-plane equations.
    if ( angle <= (-180.0 + angle_spread) ) { alg.draw_back_grid_x_plane( x_hi, x_lo, y_lo);           } else
    if ( angle <  ( -90.0 - angle_spread) ) { alg.draw_back_grid_corner(  x_lo,       y_lo, +dx, +dy); } else
    if ( angle <= ( -90.0 + angle_spread) ) { alg.draw_back_grid_y_plane( x_lo, y_lo, y_hi);           } else
    if ( angle <  (   0.0 - angle_spread) ) { alg.draw_back_grid_corner(  x_lo,       y_hi, +dx, -dy); } else
    if ( angle <= (   0.0 + angle_spread) ) { alg.draw_back_grid_x_plane( x_lo, x_hi, y_hi);           } else
    if ( angle <  ( +90.0 - angle_spread) ) { alg.draw_back_grid_corner(  x_hi,       y_hi, -dx, -dy); } else
    if ( angle <= ( +90.0 + angle_spread) ) { alg.draw_back_grid_y_plane( x_hi, y_hi, y_lo);           } else
    if ( angle <  (+180.0 - angle_spread) ) { alg.draw_back_grid_corner(  x_hi,       y_lo, -dx, +dy); } else
                                            { alg.draw_back_grid_x_plane( x_hi, x_lo, y_lo);           }

    float_type const  wall_pad      = 0.001f; /* in addition to line_pad */
    float_type const  corner_alpha  = 0.6f  ; /* the color gets more transparent from corner to center */
    float_type const  middle_alpha  = 0.1f  ;

    // Should we gl_env::global::freeze_depth_buffer( )? It's not necessary because this is the last drawing.
    gl_env::global::enable_blending( );

    // Draw the translucent walls. Draw all 4 walls and cull back faces.
    // Assume this is the last drawing since we should only draw translucent after all opaque drawing is done.
    alg.draw_x_axis_quads( x_lo, x_hi, y_hi + wall_pad, corner_alpha, middle_alpha);
    alg.draw_x_axis_quads( x_hi, x_lo, y_lo - wall_pad, corner_alpha, middle_alpha);
    alg.draw_y_axis_quads( x_lo - wall_pad, y_lo, y_hi, corner_alpha, middle_alpha);
    alg.draw_y_axis_quads( x_hi + wall_pad, y_hi, y_lo, corner_alpha, middle_alpha);
}

// _______________________________________________________________________________________________

  void
  draw_back_grid_algorithm::
draw_back_grid_x_plane( float_type x0, float_type x1, float_type y) const
{
    draw_z_axis_line( x0, y);
    draw_z_axis_line( x1, y);

    draw_x_axis_lines( x0, x1, y);
    if ( ! is_ortho_projection ) {
        draw_y_axis_lines( x0, y, /* yMid */ 0);
        draw_y_axis_lines( x1, y, /* yMid */ 0);
    }
}

  void
  draw_back_grid_algorithm::
draw_back_grid_y_plane( float_type x, float_type y0, float_type y1) const
{
    draw_z_axis_line( x, y0);
    draw_z_axis_line( x, y1);

    draw_y_axis_lines( x, y0, y1);
    if ( ! is_ortho_projection ) {
        draw_x_axis_lines( x, /* xMid */ 0, y0);
        draw_x_axis_lines( x, /* xMid */ 0, y1);
    }
}

  void
  draw_back_grid_algorithm::
draw_back_grid_corner
 (  float_type  x_corner
  , float_type  y_corner
  , float_type  x_reach
  , float_type  y_reach
 ) const
{
    draw_z_axis_line( x_corner, y_corner);
    draw_x_axis_lines( x_corner, x_corner + x_reach, y_corner);
    draw_y_axis_lines( x_corner, y_corner, y_corner + y_reach);
}

// _______________________________________________________________________________________________

  void
  draw_back_grid_algorithm::
draw_x_axis_lines( float_type x0, float_type x1, float_type y) const
{
    gl_env::global::enable_line_stipple( );
    gl_env::global::with_active_vertex_entry_lines av;

    color_and_line_xxy( x0, x1, y, color_gradient_holder::e_hot  );
    color_and_line_xxy( x0, x1, y, color_gradient_holder::e_warm );
    color_and_line_xxy( x0, x1, y, color_gradient_holder::e_tepid);
    color_and_line_xxy( x0, x1, y, color_gradient_holder::e_cool );
    color_and_line_xxy( x0, x1, y, color_gradient_holder::e_cold );
}

  void
  draw_back_grid_algorithm::
draw_y_axis_lines( float_type x, float_type y0, float_type y1) const
{
    gl_env::global::enable_line_stipple( );
    gl_env::global::with_active_vertex_entry_lines av;

    color_and_line_xyy( x, y0, y1, color_gradient_holder::e_hot  );
    color_and_line_xyy( x, y0, y1, color_gradient_holder::e_warm );
    color_and_line_xyy( x, y0, y1, color_gradient_holder::e_tepid);
    color_and_line_xyy( x, y0, y1, color_gradient_holder::e_cool );
    color_and_line_xyy( x, y0, y1, color_gradient_holder::e_cold );
}

  void
  draw_back_grid_algorithm::
draw_z_axis_line( float_type x, float_type y) const
{
    gl_env::global::disable_line_stipple( );
    gl_env::global::with_active_vertex_entry_line_strip av;

    color_and_vertex( x, y, color_gradient_holder::e_hot  );
    color_and_vertex( x, y, color_gradient_holder::e_warm );
    color_and_vertex( x, y, color_gradient_holder::e_tepid);
    color_and_vertex( x, y, color_gradient_holder::e_cool );
    color_and_vertex( x, y, color_gradient_holder::e_cold );
}

// _______________________________________________________________________________________________

  void
  draw_back_grid_algorithm::
draw_x_axis_quads
 (  float_type  x0
  , float_type  x1
  , float_type  y
  , float_type  corner_alpha
  , float_type  middle_alpha
 ) const
{
    float_type const xm = (x0 + x1) / 2;

    {   gl_env::global::with_active_vertex_entry_quad_strip qs;

        color_half_quad_xxy( x1, xm, y, color_gradient_holder::e_hot  , corner_alpha, middle_alpha);
        color_half_quad_xxy( x1, xm, y, color_gradient_holder::e_warm , corner_alpha, middle_alpha);
        color_half_quad_xxy( x1, xm, y, color_gradient_holder::e_tepid, corner_alpha, middle_alpha);
        color_half_quad_xxy( x1, xm, y, color_gradient_holder::e_cool , corner_alpha, middle_alpha);
        color_half_quad_xxy( x1, xm, y, color_gradient_holder::e_cold , corner_alpha, middle_alpha);
    }
    {   gl_env::global::with_active_vertex_entry_quad_strip qs;

        color_half_quad_xxy( xm, x0, y, color_gradient_holder::e_hot  , middle_alpha, corner_alpha);
        color_half_quad_xxy( xm, x0, y, color_gradient_holder::e_warm , middle_alpha, corner_alpha);
        color_half_quad_xxy( xm, x0, y, color_gradient_holder::e_tepid, middle_alpha, corner_alpha);
        color_half_quad_xxy( xm, x0, y, color_gradient_holder::e_cool , middle_alpha, corner_alpha);
        color_half_quad_xxy( xm, x0, y, color_gradient_holder::e_cold , middle_alpha, corner_alpha);
    }
}

  void
  draw_back_grid_algorithm::
draw_y_axis_quads
 (  float_type  x
  , float_type  y0
  , float_type  y1
  , float_type  corner_alpha
  , float_type  middle_alpha
 ) const
{
    float_type const ym = (y0 + y1) / 2;

    {   gl_env::global::with_active_vertex_entry_quad_strip qs;

        color_half_quad_xyy( x, y1, ym, color_gradient_holder::e_hot  , corner_alpha, middle_alpha);
        color_half_quad_xyy( x, y1, ym, color_gradient_holder::e_warm , corner_alpha, middle_alpha);
        color_half_quad_xyy( x, y1, ym, color_gradient_holder::e_tepid, corner_alpha, middle_alpha);
        color_half_quad_xyy( x, y1, ym, color_gradient_holder::e_cool , corner_alpha, middle_alpha);
        color_half_quad_xyy( x, y1, ym, color_gradient_holder::e_cold , corner_alpha, middle_alpha);
    }
    {   gl_env::global::with_active_vertex_entry_quad_strip qs;

        color_half_quad_xyy( x, ym, y0, color_gradient_holder::e_hot  , middle_alpha, corner_alpha);
        color_half_quad_xyy( x, ym, y0, color_gradient_holder::e_warm , middle_alpha, corner_alpha);
        color_half_quad_xyy( x, ym, y0, color_gradient_holder::e_tepid, middle_alpha, corner_alpha);
        color_half_quad_xyy( x, ym, y0, color_gradient_holder::e_cool , middle_alpha, corner_alpha);
        color_half_quad_xyy( x, ym, y0, color_gradient_holder::e_cold , middle_alpha, corner_alpha);
    }
}

// _______________________________________________________________________________________________

  void
  draw_back_grid_algorithm::
color_and_vertex( float_type x, float_type y, int color_index) const
{
    d_assert( gl_env::global::is_vertex_entry_line_strip( ));

    drop_color( color_index);
    gl_env::global::drop_vertex( x, y, get_z_coord( color_index));
}

  void
  draw_back_grid_algorithm::
color_and_line_xxy( float_type x0, float_type x1, float_type y, int color_index) const
{
    d_assert( gl_env::global::is_vertex_entry_lines( ));

    float_type const z = get_z_coord( color_index);

    drop_color( color_index);
    gl_env::global::drop_vertex( x0, y, z);
    gl_env::global::drop_vertex( x1, y, z);
}

  void
  draw_back_grid_algorithm::
color_and_line_xyy( float_type x, float_type y0, float_type y1, int color_index) const
{
    d_assert( gl_env::global::is_vertex_entry_lines( ));

    float_type const z = get_z_coord( color_index);

    drop_color( color_index);
    gl_env::global::drop_vertex( x, y0, z);
    gl_env::global::drop_vertex( x, y1, z);
}

  void
  draw_back_grid_algorithm::
color_half_quad_xxy
 (  float_type  x0
  , float_type  x1
  , float_type  y
  , int         color_index
  , float_type  alpha0
  , float_type  alpha1
 ) const
{
    d_assert( gl_env::global::is_vertex_entry_quad_strip( ));

    float_type const z = get_z_coord( color_index);

    drop_color( color_index, alpha0);
    gl_env::global::drop_vertex( x0, y, z);
    drop_color( color_index, alpha1);
    gl_env::global::drop_vertex( x1, y, z);
}

  void
  draw_back_grid_algorithm::
color_half_quad_xyy
 (  float_type  x
  , float_type  y0
  , float_type  y1
  , int         color_index
  , float_type  alpha0
  , float_type  alpha1
 ) const
{
    d_assert( gl_env::global::is_vertex_entry_quad_strip( ));

    float_type const z = get_z_coord( color_index);

    drop_color( color_index, alpha0);
    gl_env::global::drop_vertex( x, y0, z);
    drop_color( color_index, alpha1);
    gl_env::global::drop_vertex( x, y1, z);
}

// _______________________________________________________________________________________________

  float_type
  draw_back_grid_algorithm::
get_z_coord( int color_index) const
{
    // Improve. This assumes the model fits in the z coords [-1 .. +1].
    // The calculation should be:
    //   float_type const z = z_lo + (normal_mid * (z_hi - z_lo))
    return (p_color_gradient->get_normal_mid_at( color_index) * 2) - 1;
}

  void
  draw_back_grid_algorithm::
drop_color( int color_index) const
{
    rgba_type< float_type > const &
        color = p_color_gradient->get_color_holder_at( color_index)->get_color( );
    gl_env::global::drop_color( color);
}

  void
  draw_back_grid_algorithm::
drop_color( int color_index, float_type alpha) const
{
    rgba_type< float_type >
        color = p_color_gradient->get_color_holder_at( color_index)->get_color( );
    color.set_a( alpha);
    gl_env::global::drop_color( color);
}

// _______________________________________________________________________________________________
//
} /* end namespace background */
} /* end namespace gl_env */
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_draw_back_grid.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
