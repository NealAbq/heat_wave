// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// draw_sheet_base.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef DRAW_SHEET_BASE_H
# define DRAW_SHEET_BASE_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "debug.h"
# include "pt3.h"
# include "sheet.h"
# include "gl_env_type_primitives.h"
# include "gl_env_type_colors.h"

// _______________________________________________________________________________________________

namespace draw_sheet {

// _______________________________________________________________________________________________
// Classes defined below

class base_type;
class window_iter_type;

// _______________________________________________________________________________________________
// Typedefs

// Coords are always draw coords.
// The only time this ever deals with sheet coords is when it translates them into draw coords.
typedef float                                      coord_type           ;
typedef gl_env::float_type                         color_component_type ;

// Ratios are pure numbers with no units.
typedef float                                      ratio_type           ;
// Coord-scale is used to convert between coord systems. They may have units.
typedef float                                      coord_scale_type     ;

typedef sheet_type::yx_const_range_type            yx_const_range_type  ;
typedef sheet_type::yx_const_iter_type             yx_const_iter_type   ;
typedef sheet_type::x_const_range_type             x_const_range_type   ;
typedef sheet_type::x_const_iter_type              x_const_iter_type    ;

typedef sheet_type::size_type                      count_type           ;

typedef pt3< coord_type >                          pt_type              ;
typedef gl_env::rgba_type< color_component_type >  color_type           ;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  extern
  pt_type /* never returns zero-length vector, even if triangle is ill-formed */
calc_normal_any_length
 (  pt_type const &  a  /* (a,b,c) form a ccw triangle */
  , pt_type const &  b
  , pt_type const &  c
 );

  extern
  pt_type /* returns unit vector even if input is zero vector */
calc_vector_unit_length
 (  pt_type const &  vector__random_length
 );

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
window_iter_type
{
  // ---------------------------------------------------------------------------------------
  public:
    /* ctor */       window_iter_type( )            ;

  // ---------------------------------------------------------------------------------------
  public:
    coord_type       convert__sheet_to_draw__z_coord( coord_type z_sheet_coord)
                                              const ;
  private:
    void             calc__sheet_to_draw__z_coord_factors( base_type const &)
                                                    ;
    void             calc_xy_cell_start_and_size
                      (  base_type  const &  base
                       , sheet_type const &  sheet
                      )                             ;
  private:
    bool             is_active( )             const { return z_sheet_to_draw_ratio_ != 0; }
    void             set_inactive( )                { z_sheet_to_draw_ratio_ = 0;
                                                      d_assert( ! is_active( ));
                                                    }

  // ---------------------------------------------------------------------------------------
  // Iterator interface
  friend class base_type;
  protected:
    bool             init_yx
                      (  base_type  const &  base
                       , sheet_type const &  sheet
                      )                             ;
    bool             next_yx( )                     ;
    bool             init_x( )                      ;
    bool             next_x( )                      ;
  private:
    void             setup_hi_edge( )               ;
    void             setup_lead_edge( )             ;

  // ---------------------------------------------------------------------------------------
  // Getters (normals)
  public:
    // Normals for the 4 quad corners:
    pt_type          get_normal_trail_lo__any_length( )                     const ;
    pt_type          get_normal_trail_hi__any_length( )                     const ;
    pt_type          get_normal_lead_lo__any_length(  )                     const ;
    pt_type          get_normal_lead_hi__any_length(  )                     const ;
    // Unit length:
    pt_type          get_normal_trail_lo__unit_length( )                    const ;
    pt_type          get_normal_trail_hi__unit_length( )                    const ;
    pt_type          get_normal_lead_lo__unit_length(  )                    const ;
    pt_type          get_normal_lead_hi__unit_length(  )                    const ;

    // Strip average normals:
    pt_type          get_normal_lead_lo__strip_average__any_length( )       const ;
    pt_type          get_normal_lead_hi__strip_average__any_length( )       const ;

    // Area average normals:
    pt_type          get_normal_trail_lo__half_area_average__any_length( )  const ;
    pt_type          get_normal_trail_hi__half_area_average__any_length( )  const ;
    pt_type          get_normal_lead_lo__half_area_average__any_length( )   const ;
    pt_type          get_normal_lead_hi__half_area_average__any_length( )   const ;
    pt_type          get_normal_lead_lo__area_average__any_length( )        const ;
    pt_type          get_normal_lead_hi__area_average__any_length( )        const ;

  // ---------------------------------------------------------------------------------------
  // Getters (direct)
  public:
    coord_type       get_x_lo__all_cells( )             const { return x_lo__all_cells_; }
    coord_type       get_y_lo__all_cells( )             const { return y_lo__all_cells_; }
    coord_type       get_x_delta__single_cell( )        const { return x_delta__single_cell_; }
    coord_type       get_y_delta__single_cell( )        const { return y_delta__single_cell_; }

    coord_type       get_z_lo__draw_coords( )           const { return z_lo__draw_coords_   ; }
    coord_type       get_z_delta__draw_coords( )        const { return z_delta__draw_coords_; }

    bool             is_trail_edge( )                   const { return is_trail_edge_; }
    bool             is_lead_edge( )                    const { return is_lead_edge_; }
    bool             is_lo_edge( )                      const { return is_lo_edge_; }
    bool             is_hi_edge( )                      const { return is_hi_edge_; }

    pt_type const &  get_pt_trail_lo( )                 const { return pt_trail_lo_; }
    pt_type const &  get_pt_trail_hi( )                 const { return pt_trail_hi_; }
    pt_type const &  get_pt_lead_lo( )                  const { return pt_lead_lo_ ; }
    pt_type const &  get_pt_lead_hi( )                  const { return pt_lead_hi_ ; }

    pt_type const &  get_pt_trail_lo_lo( )              const { return pt_trail_lo_lo_; }
    pt_type const &  get_pt_trail_hi_hi( )              const { return pt_trail_hi_hi_; }
    pt_type const &  get_pt_lead_lo_lo( )               const { return pt_lead_lo_lo_ ; }
    pt_type const &  get_pt_lead_hi_hi( )               const { return pt_lead_hi_hi_ ; }

    pt_type const &  get_pt_lead_lead_lo( )             const { return pt_lead_lead_lo_; }
    pt_type const &  get_pt_lead_lead_hi( )             const { return pt_lead_lead_hi_; }

  // ---------------------------------------------------------------------------------------
  // Member vars
  //   These are all draw coords, not sheet coords.
  private:
    coord_type          x_lo__all_cells_       ;
    coord_type          y_lo__all_cells_       ;
    coord_type          x_delta__single_cell_  ;
    coord_type          y_delta__single_cell_  ;

    coord_scale_type    z_sheet_to_draw_ratio_ ;
    coord_type          z_lo__sheet_coords_    ;
    coord_type          z_lo__draw_coords_     ;
    coord_type          z_delta__draw_coords_  ;

    yx_const_iter_type  yx_iter_lo_lo_         ;
    yx_const_iter_type  yx_iter_lo_            ;
    yx_const_iter_type  yx_iter_hi_            ;
    yx_const_iter_type  yx_iter_hi_hi_         ;
    yx_const_iter_type  yx_iter_max_           ;

    x_const_iter_type   x_iter_lo_lo_          ;
    x_const_iter_type   x_iter_lo_             ;
    x_const_iter_type   x_iter_hi_             ;
    x_const_iter_type   x_iter_hi_hi_          ;
    x_const_iter_type   x_iter_lo_max_         ;

    bool                is_trail_edge_         ;
    bool                is_lead_edge_          ;
    bool                is_lo_edge_            ;
    bool                is_hi_edge_            ;

    pt_type             pt_trail_lo_           ;
    pt_type             pt_trail_hi_           ;
    pt_type             pt_lead_lo_            ;
    pt_type             pt_lead_hi_            ;

    pt_type             pt_trail_lo_lo_        ;
    pt_type             pt_trail_hi_hi_        ;
    pt_type             pt_lead_lo_lo_         ;
    pt_type             pt_lead_hi_hi_         ;

    pt_type             pt_lead_lead_lo_       ;
    pt_type             pt_lead_lead_hi_       ;

}; /* end type window_iter_type */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
base_type
  //
  // Pure virtual subtype.
{
  // ---------------------------------------------------------------------------------------
  // Ctor/dtor
  public:
    /* ctor */      base_type( )                                   ;
    virtual         ~base_type( )                                  { }

  // Disable copy
  private:
    /* copy */      base_type(  base_type const &)                 ; // no implementation
    void            operator =( base_type const &)                 ; // no implementation

  // ---------------------------------------------------------------------------------------
  // Framework
  protected:
    // Supertype calls this to kick things off.
    void            draw_surface
                     (  sheet_type const &  sheet
                      , gl_env::enum_type   vertex_entry_mode
                     )                                             ;

    // Supertype overrides to implement draw function.
    virtual void    continue_strip( window_iter_type const &)      = 0; /* pure virtual */

  private:
    void            draw_surface_row
                     (  gl_env::enum_type   vertex_entry_mode
                      , window_iter_type &  window_iter
                     )                                             ;

  // ---------------------------------------------------------------------------------------
  // Setters
  //   Don't set these while drawing.
  public:
    void            set_x_lo__draw_coords__all_cells( coord_type)  ;
    void            set_y_lo__draw_coords__all_cells( coord_type)  ;

    void            set_x_hi__draw_coords__all_cells( coord_type)  ;
    void            set_y_hi__draw_coords__all_cells( coord_type)  ;

    void            set_xy_draw_ratio__single_cell( ratio_type)    ;

    void            set_z_lo__draw_coords( coord_type)             ;
    void            set_z_hi__draw_coords( coord_type)             ;

    void            set_z_lo__sheet_coords( coord_type)            ;
    void            set_z_hi__sheet_coords( coord_type)            ;

  // ---------------------------------------------------------------------------------------
  // Getters
  public:
    coord_type      get_x_lo__draw_coords__all_cells( )      const { return extent_lo__draw_coords_.get_x( ); }
    coord_type      get_y_lo__draw_coords__all_cells( )      const { return extent_lo__draw_coords_.get_y( ); }

    coord_type      get_x_hi__draw_coords__all_cells( )      const { return extent_hi__draw_coords_.get_x( ); }
    coord_type      get_y_hi__draw_coords__all_cells( )      const { return extent_hi__draw_coords_.get_y( ); }

    ratio_type      get_xy_draw_ratio__single_cell( )        const { return xy_draw_ratio__single_cell_; }

    coord_type      get_z_lo__draw_coords( )                 const { return extent_lo__draw_coords_.get_z( ); }
    coord_type      get_z_hi__draw_coords( )                 const { return extent_hi__draw_coords_.get_z( ); }

    coord_type      get_z_lo__sheet_coords( )                const { return z_lo__sheet_coords_; }
    coord_type      get_z_hi__sheet_coords( )                const { return z_hi__sheet_coords_; }

  // ---------------------------------------------------------------------------------------
  // Member vars

  // This defines the cube (in draw coords) into which we draw the entire sheet.
  // This is before scaling by xy_draw_ratio__single_cell_, so the actual box we draw into
  // may be smaller.
  // These corners are only changed at init [and are usually just left as (-1,-1,-1) (+1,+1,+1)].
  private:
    pt_type     extent_lo__draw_coords_     ;
    pt_type     extent_hi__draw_coords_     ;

  // This defines the size of each cell in x/y.
  // For a square cell this is 1.
  // For a wide cell, where x-width is twice y-height, this is 2.
  // For a narrow cell, where x-width is half y-height, this is 0.5.
  // This is always > 0.
  private:
    ratio_type  xy_draw_ratio__single_cell_ ; // no units (draw-coord-size / draw-coord-size)

  // This defines the lo and hi coords of the z-axis in the sheet (in sheet coords).
  // We fit this into the [z_extent_lo__draw_coords_ .. z_extent_hi__draw_coords_]
  // coords when we draw.
  // The x and y components of this box are defined by the size and aspect of the sheet.
  private:
    coord_type  z_lo__sheet_coords_         ;
    coord_type  z_hi__sheet_coords_         ;

} /* end class draw_sheet_base_type */ ;

// _______________________________________________________________________________________________

} /* end namespace draw_sheet */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef DRAW_SHEET_BASE_H
//
// draw_sheet_base.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
