// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// heat_widget.h
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
# pragma once
# ifndef HEAT_WIDGET_H
# define HEAT_WIDGET_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "debug.h"

# include "full_screen.h"
# include "animate.h"
# include "lighting_rig.h"
# include "face_properties_style.h"
# include "bristle_properties_style.h"
# include "isotherm_properties_style.h"
# include "out_of_date.h"

# include "bool_holder.h"
# include "pack_holder.h"
# include "angle_holder.h"
# include "color_gradient_holder.h"

# include "solve_control.h"

# include <QtOpenGL/QGLWidget>
class QScrollBar;

// _______________________________________________________________________________________________

  class
heat_widget_type
  : public QGLWidget
{
  Q_OBJECT

  // -------------------------------------------------------------------------------------------
  // Typedefs
  public:
    typedef date_time::second_type         second_type        ; /* seconds duration type */
    typedef date_time::millisecond_type    millisecond_type   ; /* milliseconds duration type */
    typedef date_time::tick_point_type     tick_point_type    ;
    typedef date_time::tick_duration_type  tick_duration_type ;

    typedef double                         angle_type         ;
    typedef color_holder::color_type       color_type         ;

  // -------------------------------------------------------------------------------------------
  // Contructor
  public:
    /* ctor */        heat_widget_type( QWidget * p_parent = 0)      ;
    virtual           ~heat_widget_type( )                           ;

  // -------------------------------------------------------------------------------------------
  // Overridden from QGLWidget
  protected:
    virtual void      resizeGL( int x_size, int y_size)              ;
    virtual void      initializeGL( )                                ;
    virtual void      paintGL( )                                     ;

  // -------------------------------------------------------------------------------------------
  // Overridden event handlers from QWidget
  protected:
    virtual void      mousePressEvent( QMouseEvent *)                ;
    virtual void      keyPressEvent( QKeyEvent *)                    ;
    virtual void      paintEvent( QPaintEvent *)                     ;
    virtual void      resizeEvent( QResizeEvent *)                   ;

  // -------------------------------------------------------------------------------------------
  // Drawing slots and signals
  protected slots:
    void              maybe_update( )                                ;
    void              maybe_update_after_sheet_change( )             ;
    void              maybe_update_after_auto_solve_stop( bool)      ;
  signals:
    void              draw_finished( )                               ;
    void              size_changed( )                                ;

  // -------------------------------------------------------------------------------------------
  // Full screen
  public:
    void              init_full_screen( QAbstractButton *)           ;
  public slots:
    void              enter_full_screen( )                           ;
    void              exit_full_screen( )                            ;

  // -------------------------------------------------------------------------------------------
  // Solve
  public:
    void              init_solve_control( )                          ;
    sheet_control_type *
                      get_sheet_control( )                     const ;
    void              set_auto_solve_update_delay( out_of_date_type *)
                                                                     ;

  // -------------------------------------------------------------------------------------------
  // Save image file (PNG, JPG, etc)
  public:
    bool              is_painting_off_screen( )                const { return
                                                                         is_drawing_to_fbo_ ||
                                                                         is_drawing_to_pbo_ ||
                                                                         is_drawing_to_pixmap_ ;
                                                                     }

    void              save_image_to_file
                       (  QString const &  save_image_file_name
                        , char    const *  p_format
                       )                                             ;

    QImage            get_snapshot__as_image__using_fbo
                       (  int  x_size_pixels  = 0
                        , int  y_size_pixels  = 0
                       )                                             ;
    QPixmap           get_snapshot__as_pixmap__using_fbo
                       (  int  x_size_pixels  = 0
                        , int  y_size_pixels  = 0
                       )                                             ;

    QImage            get_snapshot__as_image__using_pbo
                       (  int  x_size_pixels  = 0
                        , int  y_size_pixels  = 0
                       )                                             ;
    QPixmap           get_snapshot__as_pixmap__using_pbo
                       (  int  x_size_pixels  = 0
                        , int  y_size_pixels  = 0
                       )                                             ;

    QImage            get_snapshot__as_image__using_render
                       (  int  x_size_pixels  = 0
                        , int  y_size_pixels  = 0
                       )                                             ;
    QPixmap           get_snapshot__as_pixmap__using_render
                       (  int  x_size_pixels  = 0
                        , int  y_size_pixels  = 0
                       )                                             ;

  // -------------------------------------------------------------------------------------------
  // Grid
  public:
    bool              is_showing_grid( )                       const ;
    void              attach_showing_grid_button( QAbstractButton *) ;

  // -------------------------------------------------------------------------------------------
  // Clipping
  public:
    bool              is_slice_clipping( )                     const ;
    double            get_clipping_center( )                   const ;
    double            get_clipping_spread( )                   const ;
    void              attach_clipping_ensemble
                       (  QAbstractButton *  is_button
                        , QScrollBar      *  center_scrollbar
                        , QAbstractSlider *  spread_slider
                       )                                             ;
  private slots:
    void              redraw_if_clipping( )                          ;
    void              recalc_clipping_spread( )                      ;

  // -------------------------------------------------------------------------------------------
  // Colors and materials
  protected:
    void              init_colors( )                                 ;
  public:
    color_holder   *  get_color_holder_background( )           const { return d_verify_not_zero( p_color_holder_background_); }
    color_gradient_holder
                   *  get_color_gradient_holder( )             const { return d_verify_not_zero( p_color_gradient_holder_); }

  // -------------------------------------------------------------------------------------------
  // Animators
  protected:
    void                   init_animation( )                         ;
  public:
    animate_type        *  get_animate( )                      const { return d_verify_not_zero( p_animate_); }

  // -------------------------------------------------------------------------------------------
  // Lighting rig
  protected:
    void                   init_lighting_rig( )                      ;
  public:
    lighting_rig_type   *  get_lighting_rig( )                 const { return d_verify_not_zero( p_lighting_rig_); }

  // -------------------------------------------------------------------------------------------
  // Face properties
  protected:
    void              init_face_properties( )                        ;
  public:
    face_properties_style_type *
                      get_face_properties( )                   const { return d_verify_not_zero( p_face_properties_); }

  // -------------------------------------------------------------------------------------------
  // Bristle properties
  protected:
    void              init_bristle_properties( )                     ;
  public:
    bristle_properties_style_type *
                      get_bristle_properties( )                const { return d_verify_not_zero( p_bristle_properties_); }

  // -------------------------------------------------------------------------------------------
  // Isotherm properties
  protected:
    void              init_isotherm_properties( )                    ;
  public:
    isotherm_properties_style_type *
                      get_isotherm_properties( )               const { return d_verify_not_zero( p_isotherm_properties_); }

  // -------------------------------------------------------------------------------------------
  // Viewing controls - rotations, zooms, scrolls
  protected:
    void              construct_slider_holders( )                    ;

  public:
    angle_type        get_z_axis_rotation_angle__degrees( )    const ;
    angle_type        get_z_axis_rotation_angle__radians( )    const ;

    angle_type        get_perspective_angle__degrees( )        const ;
    angle_type        get_perspective_angle__radians( )        const ;

    angle_type        get_rise_angle__degrees( )               const ;
    angle_type        get_rise_angle__radians( )               const ;

    double            get_model_scale( )                       const ;
    double            get_z_scale( )                           const ;
    double            get_zoom_scale( )                        const ;

    double            get_eye_right_translation( )             const ;
    double            get_eye_up_translation( )                const ;

  public:
    angle_holder *    get_perspective_angle_holder( )          const { return d_verify_not_zero( p_perspective_angle_); }
    angle_holder *    get_rise_angle_holder( )                 const { return d_verify_not_zero( p_rise_angle_); }
    angle_holder *    get_z_axis_rotation_angle_holder( )      const { return d_verify_not_zero( p_z_axis_rotation_angle_); }

    pack_range_steps_holder *  get_zoom_scale_holder( )        const { return d_verify_not_zero( p_zoom_scale_); }
    pack_range_steps_holder *  get_model_scale_holder( )       const { return d_verify_not_zero( p_model_scale_); }
    pack_range_steps_holder *  get_z_scale_holder( )           const { return d_verify_not_zero( p_z_scale_); }

    double_slide_holder *  get_eye_right_translation_holder( ) const { return d_verify_not_zero( p_eye_right_translation_); }
    double_slide_holder *  get_eye_up_translation_holder( )    const { return d_verify_not_zero( p_eye_up_translation_); }

    double_slide_holder *  get_clipping_center_holder( )       const { return d_verify_not_zero( p_clipping_center_); }
    double_slide_holder *  get_clipping_spread_holder( )       const { return d_verify_not_zero( p_clipping_spread_); }

    bool_holder *     get_is_showing_grid( )                   const { return d_verify_not_zero( p_is_showing_grid_); }
    bool_holder *     get_is_clipping( )                       const { return d_verify_not_zero( p_is_clipping_); }

  protected:
    void              setup_broadcast_scroll_steps_changed( )  const ;
  protected slots:
    void              update_scroll_steps( )                         ;

  // -------------------------------------------------------------------------------------------
  // Draw stats
  public:
    int               get_draw_count( )                        const { return draw_count_; }
    bool              is_available_duration_draw( )            const ;
    second_type       get_average_duration_draw_seconds( )     const ;
    millisecond_type  get_average_duration_draw_mseconds( )    const ;
    void              clear_draw_stats( )                            ;

  // -------------------------------------------------------------------------------------------
  // OpenGL drawing
  protected:
    void              confirm_viewport( )                            ;
    static double     get_viewport_xy_size_ratio( )                  ;
    static void       get_viewport_size( int & x_size, int & y_size) ;
  signals:
    void              viewport_changed( )                            ;

  protected:
    static void       setup_drawing_properties( )                    ;

  protected:
    bool              is_ortho_projection( )                   const ;

    void              clear_background( )                      const ;
    double            set_the_projection( )                    const ;
    void              set_the_lookat( double)                  const ;
    void              paint_model( )                                 ;
    void              maybe_draw_back_grid( )                  const ;
    void              draw_sheet( )                                  ;
    void              draw_sheet_surface( )                          ;
    void              set_gl_face_fill_and_cull( )                   ;
    void              set_gl_face_shininess( gl_env::enum_type face, float value)
                                                                     ;
    void              prepare_slice_clipping( double z_half)   const ;

  public:
    void              calc_orthos
                       (  double &  x_ortho
                        , double &  y_ortho
                       )                                       const ;
    void              calc_model_half_sizes_in_rotated_model_coords
                       (  double &  horz
                        , double &  vert
                        , double &  eye
                       )                                       const ;

  // -------------------------------------------------------------------------------------------
  // Member vars
  private:
    QeFullScreenChild              *  p_full_screen_               ;
    animate_type                   *  p_animate_                   ;
    lighting_rig_type              *  p_lighting_rig_              ;
    face_properties_style_type     *  p_face_properties_           ;
    bristle_properties_style_type  *  p_bristle_properties_        ;
    isotherm_properties_style_type *  p_isotherm_properties_       ;
    sheet_control_type             *  p_sheet_control_             ;
    out_of_date_type               *  p_auto_solve_update_delay_   ;

  private:
    // This is true while we are painting. It's a recursion guard.
    bool                              is_update_being_suppressed_  ;
    bool                              is_before_gl_init_           ;
    bool                              is_painting_                 ;
    bool                              is_drawing_to_fbo_           ; /* off-screen */
    bool                              is_drawing_to_pbo_           ; /* off-screen */
    bool                              is_drawing_to_pixmap_        ; /* off-screen */
    moving_sum< tick_duration_type >  start_finish_durations_draw_ ;

    // draw_count_ is signed because the UI (widgets) expect a signed value, and we want to be able
    // to pass around a -1 (invalid) value sometimes.
    int                               draw_count_                  ;

  private:
    bool_holder                    *  p_is_showing_grid_           ;

    bool_holder                    *  p_is_clipping_               ;
    double_slide_holder            *  p_clipping_center_           ; // -1 .. 1
    double_slide_holder            *  p_clipping_spread_           ; // 0 .. 1, clip-all .. clip-none

    color_holder                   *  p_color_holder_background_   ;
    color_gradient_holder          *  p_color_gradient_holder_     ;

  private:
    angle_holder                   *  p_perspective_angle_         ;
    angle_holder                   *  p_rise_angle_                ;
    angle_holder                   *  p_z_axis_rotation_angle_     ;

    pack_range_steps_holder        *  p_zoom_scale_                ;
    pack_range_steps_holder        *  p_model_scale_               ;
    pack_range_steps_holder        *  p_z_scale_                   ;

    double_slide_holder            *  p_eye_right_translation_     ;
    double_slide_holder            *  p_eye_up_translation_        ;

} /* end class heat_widget_type */;

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef HEAT_WIDGET_H */
//
// heat_widget.H - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
