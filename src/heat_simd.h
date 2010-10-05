// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// heat_simd.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
//
# pragma once
# ifndef HEAT_SIMD_H
# define HEAT_SIMD_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "out_of_date_ui.h"
# include "ui_heat_simd.h"
# include <QtGui/QMainWindow>

class animate_ui_type;

// _______________________________________________________________________________________________

  class
heat_wave_main_window_type
  : public QMainWindow
{
  Q_OBJECT

  // -------------------------------------------------------------------------------------------
  // Types
  public:
    typedef Ui::heat_wave_main_window  scafold_type     ;
    typedef heat_solver::rate_type     rate_type        ;
    typedef heat_solver::control_type  heat_solver_type ;

    enum hilite_status_type {
        e_normal
      , e_caution
      , e_alert
    };

  // -------------------------------------------------------------------------------------------
  // Constructor / Destructor
  public:
    /* ctor */            heat_wave_main_window_type( )                                   ;

  // -------------------------------------------------------------------------------------------
  // Navigators
  public:
    heat_widget_type   *  get_heat_widget( )                                        const ;
    sheet_control_type *  get_sheet_control( )                                      const ;
    heat_solver_type   *  get_heat_solver( )                                        const ;
    animate_type       *  get_animate( )                                            const ;

  // -------------------------------------------------------------------------------------------
  // Slots
  protected slots:
    void                  propagate_fps( int)                                             ;

    void                  update_draw_stats( )                                            ;
    void                  clear_draw_stats( )                                             ;
    void                  update_animation_stats( )                                       ;
    void                  clear_animation_stats( )                                        ;
    void                  update_solve_stats( )                                           ;
    void                  clear_solve_stats( )                                            ;

    void                  set_sheet_size( )                                               ;
    void                  set_solve_rates( )                                              ;
    void                  set_rate_x_from_ui( double)                                     ;
    void                  set_rate_y_from_ui( double)                                     ;
    void                  set_rate_background_colors( )                                   ;

    void                  set_xy_draw_size_limit( )                                       ;
    void                  set_damping_from_ui( double)                                    ;

    void                  set_window_size_labels( )                                       ;

    void                  set_shader_interpolate( bool)                                   ;
    void                  save_image_file( )                                              ;

  // -------------------------------------------------------------------------------------------
  // Init
  protected:
    void                  init_sliders_and_scrollbars( )                                  ;
    void                  init_clipping_controls( )                                       ;
    void                  init_color_controls( )                                          ;
    void                  init_show_grid_button( )                                        ;
    void                  init_full_screen_button( )                                      ;
    void                  init_draw_stats( )                                              ;
    void                  init_animation_buttons( )                                       ;
    void                  init_animation_stats( )                                         ;
    void                  init_auto_solve_buttons( )                                      ;
    void                  init_auto_solve_drawing_update( )                               ;
    void                  init_solve_stats( )                                             ;
    void                  init_heat_buttons( )                                            ;
    void                  init_sheet_size( )                                              ;
    void                  init_solve_pass_count( )                                        ;
    void                  init_solve_rate( )                                              ;
    void                  init_solve_technique( )                                         ;
    void                  init_solve_method( )                                            ;
    void                  init_draw_limits( )                                             ;
    void                  init_window_size_labels( )                                      ;
    void                  init_lighting( )                                                ;
    void                  init_face_properties( )                                         ;
    void                  init_bristle_properties( )                                      ;
    void                  init_isotherm_properties( )                                     ;
    void                  init_save_image_file( )                                         ;

  // -------------------------------------------------------------------------------------------
  // Display
  protected:
    void                  update_display_generation(             sheet_control_type *)    ;
    bool                  update_display_worker_thread_stats(    sheet_control_type *)    ;
    bool                  update_display_control_thread_stats(   sheet_control_type *)    ;
    bool                  update_display_auto_solve_cycle_stats( sheet_control_type *)    ;

    void                  set_label_to_number( QLabel *, float)                           ;
    void                  set_label_to_number_inverse( QLabel *, float, float = 1.0)      ;

    hilite_status_type    calc_rate_status( rate_type, rate_type)                         ;
    void                  set_damping_background_color( )                                 ;

  // -------------------------------------------------------------------------------------------
  // Member vars
  private:
    // Auto generated class/object that holds all the widgets and sub-widgets.
    scafold_type            ui                              ;

    // Object that manages the list of animators.
    animate_ui_type *       p_animate_ui                    ;

    hilite_status_type      rate_x_hilite_status_label_     ; // label (sync'ed with solve-control)
    hilite_status_type      rate_x_hilite_status_spinb_     ; // spin-box
    hilite_status_type      rate_y_hilite_status_label_     ; // label (sync'ed with solve-control)
    hilite_status_type      rate_y_hilite_status_spinb_     ; // spin-box
    hilite_status_type      damping_hilite_status_          ; // spin-box

    // During auto-solve, how often should we update the drawing?
    out_of_date_ui_type *   p_delay_auto_solve_updates_     ;

    // During auto-solve, how often should we update the solve stats?
    out_of_date_ui_type *   p_delay_solve_stats_            ;

    // During auto-solve and animation, how often should we update the draw stats?
    out_of_date_ui_type *   p_delay_draw_stats_             ;

    // During animation, how often should we update achieved FPS?
    out_of_date_ui_type *   p_delay_animation_stats_        ;

    // Persistent vars for "Save Image File".
    QString                 last_save_image_file_name_      ;
    QString                 last_save_image_format_for_dlg_ ;

}; /* end class heat_wave_main_window_type */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef HEAT_SIMD_H
//
// heat_simd.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
