// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// heat_simd.cpp
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
//   Look at drawing as 2D grid which we then add 3rd D to in vertex shader.
//     This might speed up drawing a lot.
//
//   Select what to show using:
//     Z-axis values (scalar)
//     Colors (can be a 3D vector, maybe more with texture)
//     Bristles
//     Animation (4th dimension)
//   What to show:
//     Values (what we do now).
//     Momentum.
//     Gradient absolute, in 1 direction, 2D gradient.
//     Curl (from a 3D sheet), or any 2D/3D vector field.
//
//   GLSL shader bug:
//     Does not draw isotherms or other textures.
//
//   Multi-solve (multi-step) (extra-pass) in worker thread bugs:
//     Edge freeze
//     Center sink (and other sources/sinks)
//
//     Early exit:
//       Early (discard) exit works great.
//       There are really several exits:
//         Get out immediately, don't worry about incomplete results. Existing early (shutdown) exit.
//         Get out now if we're in a multi-step, discard what we're working on now but try to save
//           the last result. This is very similar to the existing early exit, except we make sure
//           we return enough info so the caller can pull out the last completed solve (although
//           they'll probably lose history).
//         Get out after the next complete multistep instead of going thru all the multisteps.
//
//     Timing control:
//     Tell other thread to multi-step solve for a fixed amount of time, not a fixed number of
//     iterations. I'm not sure we really need this though -- we could estimates the number of
//     steps from the time before launching the worker thread.
//
//   Draw isotherms even if face display is turned off.
//   Transparent drawing, even if it's imperfect.
//     Detect transparency mistakes with a depth buffer (updated while drawing transparent).
//     We can make this work with enough passes.
//   Screen-door drawing, with gaps between rows, and with a screen-door shader?
//
//   Heat sources and sinks. Points and edges.
//   Buttons for centering and fitting to viewport.
//   Split screen. Slices into 3D solid.
//
//   Priority:
//     The user can make the solve thread a lower-priority thread.
//     Make the draw thread (draw to a bitmap) a lower-priority thread.
//     Another approach is to ask for:
//       Duty cycle: Make sure thread is idle XX% of the time.
//       Max frequency: Do not solve more than XX times/minute. Animation does this.
//       Fixed pause: After each solve wait XX millisecs before starting the next.
//
//   Colors:
//     Show a color gradient bar.
//     Add/remove colors.
//     Edit colors:
//       Width, center, mixing with next color.
//     Allow isotherms in color bars, individual isotherms.
//       Isotherms have sharp edges and don't mix.
//       Isotherms sit on top of the colors and can be switched on/off.
//     Each color bar can be composed of sub-bars.
//     Each color bar can have individual and repeating isotherms.
//
//     Color controls:
//       HSV and RGB sliders and spin boxes. Also transparency?
//       A vertical color bar along the side?
//       Maybe this should be on the main display and part of the back grid.
//     Color transition/mixing ctrls.
//     Color center and widths on both sides.
//     Ways to add isotherms (single, repeat) to a single bar.
//     The entire color display is also just a bar (with end repeat/reflect/gradient).
//       Each bar can be composed of smaller bars.
//
//   Add r-click.
//     Start/stop simulation
//     Start/stop animation
//     Start/stop full-screen
//     Root of all UI
//     About
//
//   Take some of UI off tabs and move into dialogs. Particularly to do with stat updates.
//     Or put it in toolboxes.
//     Transparent hierarcy (tree-like) UI.
//     Mix animation into the UI.
//     Allow display of each UI piece:
//       Tray, along bottom full size (like the sliders are now), etc.
//       Transparency and fade options. Only after mouse over-and-out?
//     Single main UI button, maybe attached to r-click?
//
//   Work on aspect ratios, to make lo-res draw same ratio as hi-res.
//   Allow non-rectangular cells.
//   Get the ratio for drawing from two places:
//     The lo-resolution sheet may have a slightly different aspect ratio than the full-res sheet.
//   Options (for the full-res sheet):
//     Square sheet
//     Square cells
//     Custom: enables sliders
//     Slider to adjust sheet ratio
//     Slider to adjust cell ratio
//   Better interface:
//     Slider: Adjust sheet ratio
//     Spin double: Show current ratio
//     Button: Square the sheet (set ratio to 1)
//     And then a 2nd triple (slider, spin, button) for adjusting cell size.
//   Also display the actual size of the lo-res copy sheet, if it's being used.
//
//   Show the fine grid as a mesh (or dots) on top of the coarse grid. And vice versa.
//     Raise (or lower) the mesh slightly so you can see it. Control raise/lower amount?
//     You can compare any two sheets this way.
//     Compare sheets that have been solved with different algorithms. Or compare generation
//     N and (N - 1).
//       Show differences as color.
//
//   Draw: Draw a course grid that's a multiple of actual grid. That way we don't have
//   to make a lo-res copy of the sheet. Re-work line_walker to make this easier, although
//   we could just skip intermediate values instead of summing them.
//   If we skip intermediate values we probably want to make the trailing edges of quads
//   a different size. Or use line_walker to distribute the smaller quads throughout the sheet.
//
//   Keep a long history of sheets and draw them all as a 3D grid. Or flat sheets stacked.
//
//   Buttons to double and half resolution? Both directions or just 1 direction?
//   Mode to display 1D "sheet" instead of 2D.
//     As ribbon, as square tube, as round tube. As mesh?
//   Way to allow a 1D display of an arbitrary line thru a 2D sheet. Not just along an axis,
//     but through any diagonal.
//
//   Round and elliptical sheets.
//
//   Change inner type to double.
//   Change inner type to non-exponential ranged float (32 or 64 bit).
//
//   Add mouse selection. Report location, temperature, material, and object labels under mouse.
//     Let us add heat to a location, or to an object we've selected.
//
//   xy (model) clip. Allow us to show just a small portion of the sheet.
//     Show only what's in rectangle ((400,350),(450,375)).
//
//   Fix add-heat buttons to work with wave equation, so we don't mess up history.
//     Don't destroy deltas between last two sheets.
//   Sheet-changing functions:
//     Move to worker thread.
//     Allow them to change the history so we don't destroy momentum.
//
//   Move drawing into other thread. Make it interuptable.
//
//   Add movie generation.
//
//   Moving averages should be resetable and length adjustable.
//   Can we set up headless time/accuracy tests? We have scripting hooks available thru Qt.
//
//   Display gradient of surface (vector), or div-grad which is scalar and so should be easier.
//
//   Add heat src - where? use rotation and rise? negative rise could be heat sink.
//   Add heat sink
//     Really 3 degrees of freedom:
//       x,y location (rotation and radius)
//       height
//
//   Allow circular sheet and other shapes. Stencil.
//
//   UI to change the shape of the squares (rectangles)
//     This might change some of the draw-frame calculations also.
//     The sheet drawing code is already coded for this, but the framing assumes the model
//     is contained in a cube. We might want to generalize this, and maybe even allow circular
//     cutouts.
//
//   Different solvers:
//     Larger window forward diff.
//     Wave equation. 3rd derivative equation (d3u/dt3 = div-grad u)
//     Mixed schemes (like abba). 2D and 1D schemes.
//     Allow the solve thread to make several iterations on its own, maybe alternating schemes?
//
//   Add timing (governor) to solve, to limit:
//     Duty cycle, or
//       Duty cycle wrt the worker thread? Or the control thread?
//     Solves / second
//       Attach this to a slider, like we do for animation.
//
//   Give user control over how to display stats.
//     Which stats to show.
//     Where to show them.
//       Separate window, on screen (opengl overlay), on side.
//   Provide toolboxes instead of tabs.
//   Button to hide the tabs (but not the sliders/scrollbars).
//     Along with a menubar or r-click menus for all the controls.
//
//   Drawing should be to a bitmap in a worker thread (probably not the solve thread).
//
//   Rates UI:
//     Change both rates at once (slider). Lock rates together.
//     Buttons to pick reasonable values for method/technique.
//
//   Give the user an update button that would update stats and the drawing.
//     Pushing that button interfers the stats around it.
//     It's already easy to do this. We don't need another button.
//
//   Add mixed materials
//     Change drawing so you can choose to draw only some materials
//   Improve drawing so we don't pass over thin stripes
//   Experiment with different kinds of solvers - variable resolutions/rates
//   Add edge conditions.
//   Add cutouts (for non-rectangular).
//
//   All UI should be available in toolboxes/toolbars as well as in side tabs.
//   Maybe also from menus?
//   And from full-screen mode (maybe with detached toolbars).
//
//   Should we add tab awareness so we don't update stats when the stat is shown in a
//   tab that is hidden? This is not hard to do. We know which tab (index) is on top,
//   and we can find out the tab-index for any widget in the tabs. But maybe this gives
//   us no discerable speedup.
//
//   Import/export formats.
//   More separation between model and display. Allow 2D view into 3D model.
//
//   Compile with GCC.
//   Compile/run under Linux.
//   Parallelize with OpenMP. Try MPI (MPICH).
//
//   Translate rates back to real time units (millisecs), sizes, and real material heat capacities.
// _______________________________________________________________________________________________

# include "all.h"

// In Windows you can include QDir below, with the other Qt headers.
// But in Linux (Qt 4.5, gcc -Wall) QDir must be included before "heat_simd.h".
# include <QtCore/QDir>

# include "heat_simd.h"
# include "solve_control.h"
# include "color_gradient_holder.h"
# include "bool_holder.h"
# include "angle_holder.h"
# include "animate_ui.h"
# include "solve_control.h"
# include "shader.h"

# include <QtGui/QFileDialog>
# include <QtGui/QImageWriter>
# include <QtGui/QMessageBox>

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  bool
is_small_negative_corrected_recursively( QDoubleSpinBox * const p_dspin_box)
  //
  // QDoubleSpinBox seems to like slightly negative values.
  // It often gives us a negative zero value when we flip down with the arrows.
  // This function gets rid of that.
{
    d_assert( p_dspin_box);
    double const value_from_ui = p_dspin_box->value( );

    // Return value.
    bool did_we_recurse = false;

    // This is a simple function, made long by a lot of recursion-watch code.
    // It is much easier to understand if you just get rid of it.

    # ifndef NDEBUG
      static int  debug_recurse_depth   = 0;
      static bool debug_expect_recurse  = false;
      static bool debug_flip_on_recurse = false;
      d_assert
       (  ((0 == debug_recurse_depth) && ! debug_expect_recurse) ||
          ((1 == debug_recurse_depth) &&   debug_expect_recurse)
       );
      debug_recurse_depth += 1;
    # endif

    // This should also test for negative zero -- is_negative_zero(..) -- but we don't have a reliable test.
    if ( (-0.001 < value_from_ui) && (value_from_ui < 0) ) {
        # ifndef NDEBUG
          d_assert( 1 == debug_recurse_depth);
          d_assert( ! debug_expect_recurse);
          d_assert( ! debug_flip_on_recurse);
          debug_expect_recurse = true;
          debug_flip_on_recurse = true;
        # endif

        // Set the UI value to zero. This will recurse back around and call this function with
        // a value of zero.
        p_dspin_box->setValue( 0);
        did_we_recurse = true;

        # ifndef NDEBUG
          d_assert( debug_expect_recurse);
          d_assert( ! debug_flip_on_recurse);
          debug_expect_recurse = false;
        # endif
    } else {
        // We may or may not be recursing when we get here.
        # ifndef NDEBUG
          d_assert
           (  ((1 == debug_recurse_depth) && (! debug_expect_recurse) && (! debug_flip_on_recurse)) ||
              ((2 == debug_recurse_depth) &&    debug_expect_recurse  &&    debug_flip_on_recurse && (0 == value_from_ui))
           );
          debug_flip_on_recurse = false;
        # endif
    }

    # ifndef NDEBUG
      debug_recurse_depth -= 1;
      d_assert( (0 == debug_recurse_depth) || (1 == debug_recurse_depth));
    # endif

    return did_we_recurse;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
set_background_normal( QWidget * const p_widget)
{
    d_assert( p_widget);
    QString const default_style; /* empty string */
    p_widget->setStyleSheet( default_style);
}

  void
set_background_caution( QWidget * const p_widget)
{
    d_assert( p_widget);

    QString const dspinb_caution_style( QString::fromUtf8(
        "QDoubleSpinBox { background-color : #ffc000; selection-background-color : #c04000 }"));
    QString const label_caution_style( QString::fromUtf8(
        "QLabel { background-color : #ffc000 }"));

    QString const * p_caution_style = 0;
    if ( qobject_cast< QDoubleSpinBox const * >( p_widget) ) {
        p_caution_style = & dspinb_caution_style;
    } else
    if ( qobject_cast< QLabel const * >( p_widget) ) {
        p_caution_style = & label_caution_style;
    }

    if ( p_caution_style ) {
        p_widget->setStyleSheet( * p_caution_style);
    } else {
        d_assert( false);
    }
}

  void
set_background_alert( QWidget * const p_widget)
{
    d_assert( p_widget);

    QString const dspinb_alert_style( QString::fromUtf8(
        "QDoubleSpinBox { background-color : #ff8080; selection-background-color : #ff2020 }"));
    QString const label_alert_style( QString::fromUtf8(
        "QLabel { background-color : #ff8080 }"));

    QString const * p_alert_style = 0;
    if ( qobject_cast< QDoubleSpinBox const * >( p_widget) ) {
        p_alert_style = & dspinb_alert_style;
    } else
    if ( qobject_cast< QLabel const * >( p_widget) ) {
        p_alert_style = & label_alert_style;
    }

    if ( p_alert_style ) {
        p_widget->setStyleSheet( * p_alert_style);
    } else {
        d_assert( false);
    }
}

  bool
set_background_status
 (  QWidget                                      * const  p_widget
  , heat_wave_main_window_type::hilite_status_type     &  old_value
  , heat_wave_main_window_type::hilite_status_type const  new_value
 )
{
    if ( new_value == old_value ) return false;
    switch ( new_value ) {
        case heat_wave_main_window_type::e_alert   : set_background_alert(   p_widget); break;
        case heat_wave_main_window_type::e_caution : set_background_caution( p_widget); break;
        default:
            d_assert( new_value == heat_wave_main_window_type::e_normal);
            set_background_normal( p_widget);
            break;
    }
    old_value = new_value;
    return true;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* constructor */
  heat_wave_main_window_type::
heat_wave_main_window_type( )
  : QMainWindow                     ( 0, Qt::WindowSystemMenuHint    |
                                         Qt::WindowCloseButtonHint   |
                                         Qt::WindowMinMaxButtonsHint )
  , ui                              ( )
  , p_animate_ui                    ( 0)
  , rate_x_hilite_status_label_     ( e_normal)
  , rate_x_hilite_status_spinb_     ( e_normal)
  , rate_y_hilite_status_label_     ( e_normal)
  , rate_y_hilite_status_spinb_     ( e_normal)
  , damping_hilite_status_          ( e_normal)
  , p_delay_auto_solve_updates_     ( 0)
  , p_delay_solve_stats_            ( 0)
  , p_delay_draw_stats_             ( 0)
  , p_delay_animation_stats_        ( 0)
  , last_save_image_file_name_      ( )
  , last_save_image_format_for_dlg_ ( )
{
    // Construct and initialize all the child widgets.
    ui.setupUi( this);

    // Attach the UI widgets and the controls.
    init_sliders_and_scrollbars( );
    init_clipping_controls( );
    init_color_controls( );
    init_show_grid_button( );
    init_full_screen_button( );
    init_draw_stats( );
    init_animation_buttons( );
    init_animation_stats( );
    init_auto_solve_buttons( );
    init_auto_solve_drawing_update( );
    init_solve_stats( );
    init_heat_buttons( );
    init_sheet_size( );
    init_solve_pass_count( );
    init_solve_rate( );
    init_solve_technique( );
    init_solve_method( );
    init_draw_limits( );
    init_window_size_labels( );
    init_lighting( );
    init_face_properties( );
    init_bristle_properties( );
    init_isotherm_properties( );
    init_save_image_file( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  heat_widget_type *
  heat_wave_main_window_type::
get_heat_widget( ) const
  //
  // You can get pointers to the children either by name or thru the ui object.
  // For example: findChild< heat_widget_type * >( QString::fromUtf8( "p_heat_widget_"))
{
    return d_verify_not_zero( ui.p_heat_wave_widget);
}

  sheet_control_type *
  heat_wave_main_window_type::
get_sheet_control( ) const
{
    return d_verify_not_zero( get_heat_widget( )->get_sheet_control( ));
}

  heat_wave_main_window_type::heat_solver_type *
  heat_wave_main_window_type::
get_heat_solver( ) const
{
    return d_verify_not_zero( get_sheet_control( )->get_heat_solver( ));
}

  animate_type *
  heat_wave_main_window_type::
get_animate( ) const
{
    return d_verify_not_zero( get_heat_widget( )->get_animate( ));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_sliders_and_scrollbars( )
{
    heat_widget_type * const p_hw = get_heat_widget( );

    // Attach slider/scrollbars to value-holders.
    # define ATTACH_SLIDER( TYPE, NAME )                           \
               p_hw->get_ ## NAME ## _holder( )->                  \
                  attach( ui.p_ ## TYPE ## _ ## NAME ## _)  /* end macro */

    ATTACH_SLIDER( slider   , z_axis_rotation_angle  );
    ATTACH_SLIDER( slider   , perspective_angle      );
    ATTACH_SLIDER( slider   , rise_angle             );
    ATTACH_SLIDER( slider   , model_scale            );
    ATTACH_SLIDER( slider   , z_scale                );
    ATTACH_SLIDER( slider   , zoom_scale             );
    ATTACH_SLIDER( scrollbar, eye_right_translation  );
    ATTACH_SLIDER( scrollbar, eye_up_translation     );

    # undef ATTACH_SLIDER

    // Keep the widths of the scrollbar thumbs updated as the model changes size.
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_clipping_controls( )
{
    heat_widget_type * const p_hw = get_heat_widget( );

    // Attach the clipping controls to the heat-widget.
    p_hw->attach_clipping_ensemble
       (  ui.p_button_is_clipping_
        , ui.p_scrollbar_clipping_center_
        , ui.p_slider_clipping_spread_
       );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_animation_buttons( )
{
    heat_widget_type * const  p_hw       = get_heat_widget( );
    animate_type     * const  p_animate  = p_hw->get_animate( );

    // Button clicks that start/stop/step the animation.
    p_animate->attach__on_off_button( ui.p_button_animation_toggle_      );
    p_animate->attach__on_off_button( ui.p_button_animation_local_toggle_);
    p_animate->attach__step_button(   ui.p_button_animation_local_step_  );

    // Attach the FPS slider to the animate object.
    // This also sets the FPS requested label.
    // Get the initial state from the slider (not the animate object).
    d_verify( connect(
        ui.p_slider_animation_fps_, SIGNAL( valueChanged( int)),
        this, SLOT( propagate_fps( int))
    ));
    // Make the animate object agree with the slider's initial state.
    propagate_fps( ui.p_slider_animation_fps_->value( ));

    d_assert( ! p_animate_ui);
    p_animate_ui = new animate_ui_type( this, & ui, p_animate, p_hw);
}

// _______________________________________________________________________________________________

  /* slot */
  void
  heat_wave_main_window_type::
propagate_fps( int value_from_slider_log_0_1000)
{
    d_assert( ui.p_slider_animation_fps_->value( ) == value_from_slider_log_0_1000);

    animate_type * const p_animate = get_animate( );
    p_animate->request_frames_per_second_log_0_1000( value_from_slider_log_0_1000);

    float const requested_fps = p_animate->get_requested_frames_per_second( );
    d_assert( requested_fps >= 0);
    if ( requested_fps > 0 ) {
        set_label_to_number( ui.p_label_animation_fps_requested_, p_animate->get_requested_frames_per_second( ));
    } else {
        ui.p_label_animation_fps_requested_->setText( QObject::tr( "no limit"));
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_auto_solve_buttons( )
{
    sheet_control_type * const p_sctrl = get_sheet_control( );

    // Button clicks that start and stop solving.
    d_verify( connect(
        ui.p_button_auto_solve_toggle_, SIGNAL( toggled( bool)),
        p_sctrl, SLOT( start_stop_auto_solving( bool))));
    d_verify( connect(
        ui.p_button_solve_single_step_, SIGNAL( clicked( )),
        p_sctrl, SLOT( single_step_solve( ))));

    // Signals from the solve-control telling us that solving is started or stopped.
    d_verify( connect(
        p_sctrl , SIGNAL( auto_solving_started( bool)),
        ui.p_button_auto_solve_toggle_, SLOT( setChecked( bool))));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_auto_solve_drawing_update( )
{
    d_assert( ! p_delay_auto_solve_updates_);
    p_delay_auto_solve_updates_ = new out_of_date_ui_type( );
    // Stick with the defaults:
    //    p_delay->ref_delay( ).set_option( out_of_date_type::always);
    //    p_delay->ref_delay( ).set_seconds_max_out_of_date( 1.0);
    //    p_delay_->ref_delay( ).set_gen_max_out_of_date( 9);

    p_delay_auto_solve_updates_->set_button_always(     ui.p_radio_auto_update_always_   );
    p_delay_auto_solve_updates_->set_button_never(      ui.p_radio_auto_update_never_    );
    p_delay_auto_solve_updates_->set_button_watch_gen(  ui.p_radio_auto_update_gen_      );
    p_delay_auto_solve_updates_->set_spinbox_gen(       ui.p_spinb_auto_update_gen_      );
    p_delay_auto_solve_updates_->set_button_watch_tick( ui.p_radio_auto_update_seconds_  );
    p_delay_auto_solve_updates_->set_spinbox_seconds(   ui.p_spinb_auto_update_seconds_  );

    p_delay_auto_solve_updates_->set_ui_from_delay( );

    heat_widget_type * const p_hw = get_heat_widget( );
    p_hw->set_auto_solve_update_delay( & p_delay_auto_solve_updates_->ref_delay( ));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_solve_stats( )
{
    d_assert( ! p_delay_solve_stats_);
    p_delay_solve_stats_ = new out_of_date_ui_type( );
    // Stick with the defaults:
    //    p_delay->ref_delay( ).set_option( out_of_date_type::always);
    //    p_delay->ref_delay( ).set_seconds_max_out_of_date( 1.0);
    //    p_delay_->ref_delay( ).set_gen_max_out_of_date( 9);

    p_delay_solve_stats_->set_button_always(     ui.p_radio_sim_stats_update_always_  );
    p_delay_solve_stats_->set_button_never(      ui.p_radio_sim_stats_update_never_   );
    p_delay_solve_stats_->set_button_watch_gen(  ui.p_radio_sim_stats_update_gen_     );
    p_delay_solve_stats_->set_spinbox_gen(       ui.p_spinb_sim_stats_update_gen_     );
    p_delay_solve_stats_->set_button_watch_tick( ui.p_radio_sim_stats_update_seconds_ );
    p_delay_solve_stats_->set_spinbox_seconds(   ui.p_spinb_sim_stats_update_seconds_ );

    p_delay_solve_stats_->set_ui_from_delay( );

    // Button to clear the stats.
    d_verify( connect(
        ui.p_button_clear_solve_stats_, SIGNAL( clicked( )),
        this, SLOT( clear_solve_stats( ))
    ));

    // Signal from the solve-control telling us the solve generation has changed.
    sheet_control_type * const p_sctrl = get_sheet_control( );
    d_verify( connect(
        p_sctrl , SIGNAL( sheet_is_changed( )),
        this, SLOT( update_solve_stats( ))
    ));
}

// _______________________________________________________________________________________________

  /* slot */
  void
  heat_wave_main_window_type::
clear_solve_stats( )
{
    // The statistics come from the solve control.
    sheet_control_type * const p_sctrl = get_sheet_control( );
    p_sctrl->clear_solve_stats( );

    update_display_generation( p_sctrl);
    ui.p_value_wt_msecs_    ->setText( QObject::tr( ""));
    ui.p_value_wt_per_sec_  ->setText( QObject::tr( ""));
    ui.p_value_ct_msecs_    ->setText( QObject::tr( ""));
    ui.p_value_ct_per_sec_  ->setText( QObject::tr( ""));
    ui.p_value_auto_msecs_  ->setText( QObject::tr( ""));
    ui.p_value_auto_per_sec_->setText( QObject::tr( ""));

    d_assert( p_delay_solve_stats_);
    p_delay_solve_stats_->ref_delay( ).update( -1);
}

  /* slot */
  void
  heat_wave_main_window_type::
update_solve_stats( )
  //
  // Called when the heat_widget tells us the generation count has changed.
{
    d_assert( p_delay_solve_stats_);

    // The statistics come from the solve control.
    sheet_control_type * const p_sctrl = get_sheet_control( );

    // Update the statistics.
    if ( (! p_sctrl->is_auto_solving( )) ||
         p_delay_solve_stats_->get_delay( ).is_out_of_date( p_sctrl->get_sheet_generation( )) )
    {
        // These are almost always updated the first time thru (unless the option is never).
        // We want these non-blank as soon as possible.
        update_display_generation( p_sctrl);

        bool const is_ok1 = update_display_worker_thread_stats(    p_sctrl);
        bool const is_ok2 = update_display_control_thread_stats(   p_sctrl);
        bool const is_ok3 = update_display_auto_solve_cycle_stats( p_sctrl);

        // Only update if all displays were OK.
        if ( is_ok1 && is_ok2 && is_ok3 ) {
            p_delay_solve_stats_->ref_delay( ).update( p_sctrl->get_sheet_generation( ));
        }
    }
}

// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
update_display_generation( sheet_control_type * p_sctrl)
  //
  // Display the generation number.
  // It's too bad this is so slow. It's nice when you can show it for every solve.
{
    int const gen = p_sctrl->get_sheet_generation( );
    d_assert( gen >= 0);

    // The following is surprisingly slow. It seems to take about as long as solving a 200x200 sheet.
    ui.p_value_generation_->setNum( gen);
}

  bool
  heat_wave_main_window_type::
update_display_worker_thread_stats( sheet_control_type * p_sctrl)
  //
  // There are two stats:
  //   worker-thread average duration per solve
  //   worker-thread potential solves per second
{
    if ( p_sctrl->is_available_duration_worker_thread( ) ) {
        float const
            solve_duration_msecs =
                p_sctrl->get_average_duration_worker_thread_mseconds( );
        d_assert( solve_duration_msecs >= 0);
        set_label_to_number(         ui.p_value_wt_msecs_  , solve_duration_msecs);
        set_label_to_number_inverse( ui.p_value_wt_per_sec_, solve_duration_msecs, 1000);
        return true;
    }
    return false;
}

  bool
  heat_wave_main_window_type::
update_display_control_thread_stats( sheet_control_type * p_sctrl)
  //
  // There are two stats:
  //   control-thread average duration per solve
  //   control-thread potential solves per second
{
    if ( p_sctrl->is_available_duration_control_thread( ) ) {
        float const
            solve_duration_msecs =
                p_sctrl->get_average_duration_control_thread_mseconds( );
        d_assert( solve_duration_msecs >= 0);
        set_label_to_number(         ui.p_value_ct_msecs_  , solve_duration_msecs);
        set_label_to_number_inverse( ui.p_value_ct_per_sec_, solve_duration_msecs, 1000);
        return true;
    }
    return false;
}

  bool
  heat_wave_main_window_type::
update_display_auto_solve_cycle_stats( sheet_control_type * p_sctrl)
  //
  // There are two stats:
  //   auto-solve average start-start duration per solve
  //   auto-solve potential solves per second
{
    if ( p_sctrl->is_available_duration_auto_solve_cycle( ) ) {
        float const
            solve_duration_msecs =
                p_sctrl->get_average_duration_auto_solve_cycle_mseconds( );
        d_assert( solve_duration_msecs >= 0);
        set_label_to_number(         ui.p_value_auto_msecs_  , solve_duration_msecs);
        set_label_to_number_inverse( ui.p_value_auto_per_sec_, solve_duration_msecs, 1000);
        return true;
    }
    return false;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_draw_stats( )
{
    d_assert( ! p_delay_draw_stats_);
    p_delay_draw_stats_ = new out_of_date_ui_type( );
    // Stick with the defaults:
    //    p_delay->ref_delay( ).set_option( out_of_date_type::always);
    //    p_delay->ref_delay( ).set_seconds_max_out_of_date( 1.0);
    //    p_delay_->ref_delay( ).set_gen_max_out_of_date( 9);

    p_delay_draw_stats_->set_button_always(     ui.p_radio_draw_stats_update_always_  );
    p_delay_draw_stats_->set_button_never(      ui.p_radio_draw_stats_update_never_   );
    p_delay_draw_stats_->set_button_watch_gen(  ui.p_radio_draw_stats_update_gen_     );
    p_delay_draw_stats_->set_spinbox_gen(       ui.p_spinb_draw_stats_update_gen_     );
    p_delay_draw_stats_->set_button_watch_tick( ui.p_radio_draw_stats_update_seconds_ );
    p_delay_draw_stats_->set_spinbox_seconds(   ui.p_spinb_draw_stats_update_seconds_ );

    p_delay_draw_stats_->set_ui_from_delay( );

    // Button to clear the stats.
    d_verify( connect(
        ui.p_button_clear_draw_stats_, SIGNAL( clicked( )),
        this, SLOT( clear_draw_stats( ))
    ));

    // Signal from the heat-widget telling us it has just finished drawing.
    heat_widget_type * const p_hw = get_heat_widget( );
    d_verify( connect( p_hw , SIGNAL( draw_finished( )), this, SLOT( update_draw_stats( ))));
}

  /* slot */
  void
  heat_wave_main_window_type::
clear_draw_stats( )
{
    // The statistics come from the heat control.
    get_heat_widget( )->clear_draw_stats( );

    d_assert( p_delay_draw_stats_);
    p_delay_draw_stats_->ref_delay( ).update( -1);

    ui.p_value_draw_count_  ->setText( QObject::tr( ""));
    ui.p_value_draw_seconds_->setText( QObject::tr( ""));
    ui.p_value_draw_rate_   ->setText( QObject::tr( ""));

    update_draw_stats( );
}

  /* slot */
  void
  heat_wave_main_window_type::
update_draw_stats( )
  //
  // Called when the heat_widget tells us it has drawn.
{
    d_assert( p_delay_draw_stats_);

    // The heat widget supplies statistics.
    heat_widget_type const * const p_hw = get_heat_widget( );

    // We only delay when we are either animating, or auto-solving, or both.
    bool const
        is_animated_or_auto =
            get_animate( )->is_animating( ) || get_sheet_control( )->is_auto_solving( );

    // Update the statistics.
    if ( (! is_animated_or_auto) ||
         p_delay_draw_stats_->get_delay( ).is_out_of_date( p_hw->get_draw_count( )) )
    {
        int const draw_count = p_hw->get_draw_count( );
        d_assert( draw_count >= 0);
        ui.p_value_draw_count_->setNum( draw_count);

        if ( p_hw->is_available_duration_draw( ) ) {
            float const seconds = p_hw->get_average_duration_draw_seconds( );
            d_assert( seconds >= 0);

            // Flaw: This displays the number of seconds, but milliseconds are a more
            // natural unit here. Change the UI and then change it here.
            set_label_to_number(         ui.p_value_draw_seconds_, seconds);
            set_label_to_number_inverse( ui.p_value_draw_rate_   , seconds);

            p_delay_draw_stats_->ref_delay( ).update( p_hw->get_draw_count( ));
        }
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_animation_stats( )
{
    d_assert( ! p_delay_animation_stats_);
    p_delay_animation_stats_ = new out_of_date_ui_type( );
    // Stick with the defaults:
    //    p_delay->ref_delay( ).set_option( out_of_date_type::always);
    //    p_delay->ref_delay( ).set_seconds_max_out_of_date( 1.0);
    //    p_delay_->ref_delay( ).set_gen_max_out_of_date( 9);

    p_delay_animation_stats_->set_button_always(    ui.p_radio_animation_fps_always_);
    p_delay_animation_stats_->set_button_never(     ui.p_radio_animation_fps_never_ );
    p_delay_animation_stats_->set_button_watch_gen( ui.p_radio_animation_fps_gen_   );
    p_delay_animation_stats_->set_spinbox_gen(      ui.p_spinb_animation_fps_gen_   );
    //p_delay_animation_stats_->set_button_watch_tick( ... );
    //p_delay_animation_stats_->set_spinbox_seconds(   ... );

    p_delay_animation_stats_->set_ui_from_delay( );

    // Button to clear the stats.
    d_verify( connect(
        ui.p_button_clear_animation_fps_achieved_, SIGNAL( clicked( )),
        this, SLOT( clear_animation_stats( ))
    ));

    // Signal from the heat-widget telling us it has just finished drawing.
    heat_widget_type * const p_hw = get_heat_widget( );
    d_verify( connect( p_hw , SIGNAL( draw_finished( )), this, SLOT( update_animation_stats( ))));
}

  /* slot */
  void
  heat_wave_main_window_type::
clear_animation_stats( )
{
    // The statistics come from the animation engine.
    get_animate( )->clear_duration_stats( );

    d_assert( p_delay_animation_stats_);
    p_delay_animation_stats_->ref_delay( ).update( -1);
    ui.p_label_animation_fps_achieved_->setText( QObject::tr( ""));

    // We don't display p_animate->get_move_count( ), but if we did we'd clear
    // the display-ctrl here.

    update_animation_stats( );
}

  /* slot */
  void
  heat_wave_main_window_type::
update_animation_stats( )
  //
  // Called when the heat_widget tells us it has drawn.
{
    d_assert( p_delay_animation_stats_);

    // The animation driver supplies statistics.
    animate_type const * const p_animate = get_animate( );

    // Update the statistics.
    if ( p_animate->is_animating( ) &&
         p_delay_animation_stats_->get_delay( ).is_out_of_date( p_animate->get_move_count( )) )
    {
        if ( p_animate->is_available_duration( ) ) {
            float const seconds = p_animate->get_average_duration_seconds( );
            d_assert( seconds >= 0);
            set_label_to_number_inverse( ui.p_label_animation_fps_achieved_, seconds);
            p_delay_animation_stats_->ref_delay( ).update( p_animate->get_move_count( ));
        }
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
set_label_to_number( QLabel * p_label, float value)
{
    d_assert( p_label);
    if ( value <= -49.5 ) {
        p_label->setNum( static_cast< int >( value - 0.5));
    } else
    if ( value <  +49.5 ) {
        p_label->setNum( value /* as double */);
    } else
    /* if value >= +49.5 */ {
        p_label->setNum( static_cast< int >( value + 0.5));
    }
}

  void
  heat_wave_main_window_type::
set_label_to_number_inverse( QLabel * p_label, float value, float numerator /* = 1.0 */)
{
    if ( value == 0 ) {
        // Avoid dividing by zero.
        p_label->setText(  QObject::tr( "xxxxxx"));
    } else
    /* if value != 0 */ {
        set_label_to_number( p_label, numerator / value);
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_color_controls( )
{
    heat_widget_type      * const  p_hw    = get_heat_widget( );
    color_gradient_holder * const  p_grad  = p_hw->get_color_gradient_holder( );

    // Button to edit the background color.
    p_hw->get_color_holder_background( )->attach( ui.p_button_color_back_);

    // Buttons to edit the strip of model colors (hot, warm, tepid, cool, and cold).
    p_grad->get_color_holder_cold(  )->attach( ui.p_button_color_cold_ );
    p_grad->get_color_holder_cool(  )->attach( ui.p_button_color_cool_ );
    p_grad->get_color_holder_tepid( )->attach( ui.p_button_color_tepid_);
    p_grad->get_color_holder_warm(  )->attach( ui.p_button_color_warm_ );
    p_grad->get_color_holder_hot(   )->attach( ui.p_button_color_hot_  );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_show_grid_button( )
{
    heat_widget_type * const p_hw = get_heat_widget( );

    // Attach the button that shows the grid to the bool holder in the heat-widget.
    p_hw->attach_showing_grid_button( ui.p_button_is_showing_grid_);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_full_screen_button( )
{
    get_heat_widget( )->init_full_screen( ui.p_button_full_screen_);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_heat_buttons( )
{
    sheet_control_type * const p_sctrl = get_sheet_control( );

    // Buttons to change the heat on the sheet.
    d_verify( connect(
        ui.p_button_heat_normalize_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_normalize_sheet( ))));
    d_verify( connect(
        ui.p_button_heat_flatten_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_flatten_sheet( ))));
    d_verify( connect(
        ui.p_button_heat_delta_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_delta( ))));

    d_verify( connect(
        ui.p_button_heat_init_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_set_init_test( ))));
    d_verify( connect(
        ui.p_button_heat_stair_steps_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_stair_steps( ))));
    d_verify( connect(
        ui.p_button_heat_random_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_set_sheet_random_noise( ))));

    d_verify( connect(
        ui.p_button_heat_bell_1a_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_bell_curve_1( ))));
    d_verify( connect(
        ui.p_button_heat_bell_2a_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_bell_curve_2( ))));
    d_verify( connect(
        ui.p_button_heat_bell_3a_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_bell_curve_4( ))));

    d_verify( connect(
        ui.p_button_heat_bell_1b_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_bell_corner_1( ))));
    d_verify( connect(
        ui.p_button_heat_bell_2b_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_bell_corner_2( ))));
    d_verify( connect(
        ui.p_button_heat_bell_3b_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_bell_corner_3( ))));

    d_verify( connect(
        ui.p_button_heat_bell_1c_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_bell_corner_4( ))));
    d_verify( connect(
        ui.p_button_heat_bell_2c_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_raindrop_up( ))));
    d_verify( connect(
        ui.p_button_heat_bell_3c_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_raindrop_down( ))));

    d_verify( connect(
        ui.p_button_heat_sin_over_dist_1_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_sin_over_dist_1( ))));
    d_verify( connect(
        ui.p_button_heat_sin_over_dist_2_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_sin_over_dist_2( ))));
    d_verify( connect(
        ui.p_button_heat_sin_over_dist_3_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_sin_over_dist_4( ))));

    d_verify( connect(
        ui.p_button_heat_ramp_1_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_ramp_corner_to_corner( ))));
    d_verify( connect(
        ui.p_button_heat_ramp_2_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_ramp_2_corners( ))));
    d_verify( connect(
        ui.p_button_heat_ramp_3_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_ramp_4_corners( ))));

    d_verify( connect(
        ui.p_button_heat_flatten_values_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_flatten_values( ))));
    d_verify( connect(
        ui.p_button_heat_flatten_momentum_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_flatten_momentum( ))));
    d_verify( connect(
        ui.p_button_heat_half_values_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_half_values( ))));
    d_verify( connect(
        ui.p_button_heat_half_momentum_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_half_momentum( ))));
    d_verify( connect(
        ui.p_button_heat_half_both_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_half_both( ))));

    d_verify( connect(
        ui.p_button_heat_reverse_wave_, SIGNAL( clicked( )),
        p_sctrl, SLOT( request_reverse_wave( ))));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_sheet_size( )
{
    sheet_control_type * const p_sctrl = get_sheet_control( );

    // Button to change the sheet size.
    d_verify( connect( ui.p_button_set_sheet_size_, SIGNAL( clicked( )), this, SLOT( set_sheet_size( ))));

    int const init_sheet_size_x = p_sctrl->get_x_size( );
    int const init_sheet_size_y = p_sctrl->get_y_size( );

    ui.p_spinb_sheet_size_x_->setValue( init_sheet_size_x);
    ui.p_spinb_sheet_size_y_->setValue( init_sheet_size_y);

    ui.p_value_sheet_size_x_->setNum( init_sheet_size_x);
    ui.p_value_sheet_size_y_->setNum( init_sheet_size_y);
}

  /* slot */
  void
  heat_wave_main_window_type::
set_sheet_size( )
{
    sheet_control_type * const p_sctrl = get_sheet_control( );

    int const x_size = ui.p_spinb_sheet_size_x_->value( );
    int const y_size = ui.p_spinb_sheet_size_y_->value( );

    p_sctrl->request_set_xy_sizes( x_size, y_size);

    // We'd like to do this here, but the sheet size isn't changed yet if we are solving.
    // This might be a problem later if the request fails.
    //   ui.p_value_sheet_size_x_->setNum( p_sctrl->get_x_size( ));
    //   ui.p_value_sheet_size_y_->setNum( p_sctrl->get_y_size( ));
    ui.p_value_sheet_size_x_->setNum( x_size);
    ui.p_value_sheet_size_y_->setNum( y_size);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_solve_pass_count( )
{
    heat_solver_type * const p_hsolv = get_heat_solver( );

    // Spin box to set the extra-pass count.
    d_verify( connect(
        ui.p_spinb_pass_count_, SIGNAL( valueChanged( int)),
        p_hsolv, SLOT( set_pass_count( int))
    ));

    // Get the default value from the solve control and set it in the UI.
    int const pass_count = static_cast< int >( p_hsolv->get_pass_count( ));
    ui.p_spinb_pass_count_->setValue( pass_count);
}

  void
  heat_wave_main_window_type::
init_solve_rate( )
{
    heat_solver_type * const p_hsolv = get_heat_solver( );

    // Button to change the heat/diffusion rates.
    d_verify( connect(
        ui.p_button_set_solve_rates_, SIGNAL( clicked( )),
        this, SLOT( set_solve_rates( ))
    ));

    // Get the initial values from the solve control.
    double const init_rate_x = p_hsolv->get_rate_x( );
    double const init_rate_y = p_hsolv->get_rate_y( );

    // Set them in the spin boxes.
    ui.p_spinb_rate_x_->setValue( init_rate_x);
    ui.p_spinb_rate_y_->setValue( init_rate_y);

    // Set them in the labels.
    ui.p_value_rate_x_->setNum( init_rate_x);
    ui.p_value_rate_y_->setNum( init_rate_y);

    // Update the background colors of both the spin-boxes and labels.
    set_rate_background_colors( );

    // Update the colors whenever the spin-boxes spin. Also get rid of small negative values.
    d_verify( connect(
        ui.p_spinb_rate_x_, SIGNAL( valueChanged( double)),
        this, SLOT( set_rate_x_from_ui( double))));
    d_verify( connect(
        ui.p_spinb_rate_y_, SIGNAL( valueChanged( double)),
        this, SLOT( set_rate_y_from_ui( double))));
}

  /* slot */
  void
  heat_wave_main_window_type::
set_solve_rates( )
  //
  // Signaled when set-rates button is pushed.
{
    heat_solver_type * const p_hsolv = get_heat_solver( );

    // Get values from the spin-boxes.
    rate_type const x_rate = static_cast< rate_type >( ui.p_spinb_rate_x_->value( ));
    rate_type const y_rate = static_cast< rate_type >( ui.p_spinb_rate_y_->value( ));

    // Set the values in both the solve control and in the labels.
    p_hsolv->set_rates( x_rate, y_rate);
    ui.p_value_rate_x_->setNum( x_rate);
    ui.p_value_rate_y_->setNum( y_rate);

    // Maybe update the colors in the labels (should not affect spin boxes).
    set_rate_background_colors( );
}

  /* slot */
  void
  heat_wave_main_window_type::
set_rate_x_from_ui( double rate_x_from_ui)
{
    maybe_used_only_for_debug( rate_x_from_ui);
    d_assert( ui.p_spinb_rate_x_->value( ) == rate_x_from_ui);

    if ( ! is_small_negative_corrected_recursively( ui.p_spinb_rate_x_) ) {
        set_rate_background_colors( );
    }
}

  /* slot */
  void
  heat_wave_main_window_type::
set_rate_y_from_ui( double rate_y_from_ui)
{
    maybe_used_only_for_debug( rate_y_from_ui);
    d_assert( ui.p_spinb_rate_y_->value( ) == rate_y_from_ui);

    if ( ! is_small_negative_corrected_recursively( ui.p_spinb_rate_y_) ) {
        set_rate_background_colors( );
    }
}

  /* slot */
  void
  heat_wave_main_window_type::
set_rate_background_colors( )
  //
  // Signaled or called when:
  //   UI is initialized.
  //   Rate spin-boxes change (should never affect labels).
  //   Set-rate button is pushed (should never affect spin boxes).
  //   Method is changed.
  //   Technique is changed.
{
    // Get values from spin boxes. These values can be different from what's in the solve-control and labels.
    {   rate_type const  rate_x_from_ui  = ui.p_spinb_rate_x_->value( );
        rate_type const  rate_y_from_ui  = ui.p_spinb_rate_y_->value( );
        set_background_status( ui.p_spinb_rate_x_, rate_x_hilite_status_spinb_,
            calc_rate_status( rate_x_from_ui, rate_y_from_ui));
        set_background_status( ui.p_spinb_rate_y_, rate_y_hilite_status_spinb_,
            calc_rate_status( rate_y_from_ui, rate_x_from_ui));
    }

    // Get values from solve-control, which should be the same as the values in the labels.
    // Assumes labels and solve-control are sync'ed up.
    {   heat_solver_type * const p_hsolv = get_heat_solver( );
        set_background_status( ui.p_value_rate_x_, rate_x_hilite_status_label_,
            calc_rate_status( p_hsolv->get_rate_x( ), p_hsolv->get_rate_y( )));
        set_background_status( ui.p_value_rate_y_, rate_y_hilite_status_label_,
            calc_rate_status( p_hsolv->get_rate_y( ), p_hsolv->get_rate_x( )));
    }
}

  /* not a slot */
  heat_wave_main_window_type::hilite_status_type
  heat_wave_main_window_type::
calc_rate_status( rate_type rate, rate_type other_rate)
{
    // A negative rate is always illegal.
    if ( rate < 0 ) return e_alert;

    // Backward-diff works with any positive rate.
    heat_solver_type * const p_hsolv = get_heat_solver( );

    if ( p_hsolv->is_method__backward_diff( ) ) return e_normal;

    // Forward and central diff need small positive rates.
    if ( p_hsolv->is_method__forward_diff( ) && ! p_hsolv->is_technique__ortho_interleave( ) ) {
        // Forward diff that is not interleave uses a 2d algorithm and looks at both x- and y- neighbors
        // when it calcs the next value. Thus the rates are constrained to smaller positive values.
        rate_type const adjust_rate = rate - (1 / static_cast< rate_type >( 10000));
        if ( adjust_rate <= (1 / static_cast< rate_type >( 5)) ) return e_normal;
        if ( adjust_rate <= ((1 - (2 * other_rate)) / 3) ) return e_normal;

        if ( adjust_rate <= (1 / static_cast< rate_type >( 4)) ) return e_caution;
        if ( (2 * (adjust_rate + other_rate)) < 1 ) return e_caution;
    } else {
        if ( rate <= (1 / static_cast< rate_type >( 3)) ) return e_normal;
        if ( rate <  (1 / static_cast< rate_type >( 2)) ) return e_caution;
    }
    return e_alert;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_solve_technique( )
{
    heat_solver_type * const p_hsolv = get_heat_solver( );

    // Setup the radio buttons.
    QAbstractButton * const p_radio_orth = ui.p_radio_technique_ortho_interleave_ ;
    QAbstractButton * const p_radio_simu = ui.p_radio_technique_simultaneous_     ;
    QAbstractButton * const p_radio_wave = ui.p_radio_technique_wave_             ;

    // Set the init state of the radio buttons from the solve object.
    p_radio_orth->setChecked( p_hsolv->is_technique__ortho_interleave(  ));
    p_radio_simu->setChecked( p_hsolv->is_technique__simultaneous_2d(   ));
    p_radio_wave->setChecked( p_hsolv->is_technique__wave_with_damping( ));

    // Tell the radio buttons to update the solve object.
    d_verify( connect(
        p_radio_orth, SIGNAL( toggled( bool)),
        p_hsolv, SLOT( set_technique__ortho_interleave(  bool))));
    d_verify( connect(
        p_radio_simu, SIGNAL( toggled( bool)),
        p_hsolv, SLOT( set_technique__simultaneous_2d(   bool))));
    d_verify( connect(
        p_radio_wave, SIGNAL( toggled( bool)),
        p_hsolv, SLOT( set_technique__wave_with_damping( bool))));

    // The alert/caution color on the rates depends on the technique.
    d_verify( connect(
        p_hsolv, SIGNAL( technique_is_changed( )),
        this, SLOT( set_rate_background_colors( ))));

    // Setup the spinbox with the wave-damping value.
    QDoubleSpinBox * const p_damp_spinb = ui.p_dspinb_damping_;
    p_damp_spinb->setValue( p_hsolv->get_damping( ));
    set_damping_background_color( );
    d_verify( connect(
        p_damp_spinb, SIGNAL( valueChanged( double)),
        this, SLOT( set_damping_from_ui( double))));

    // How to connect the spinbox directly to the solve control:
    //   d_verify( connect(
    //       p_damp_spinb, SIGNAL( valueChanged( double)),
    //       p_hsolv, SLOT( set_damping( double))));
    //
    // Instead of SIGNAL( valueChanged(..)) we could use SIGNAL( editingFinished( )).
    // The problem with this is the user has to click somewhere else to get the change enacted.
}

  /* slot */
  void
  heat_wave_main_window_type::
set_damping_from_ui( double damping_from_ui)
{
    d_assert( ui.p_dspinb_damping_->value( ) == damping_from_ui);

    if ( ! is_small_negative_corrected_recursively( ui.p_dspinb_damping_) ) {
        get_heat_solver( )->set_damping( damping_from_ui);
        set_damping_background_color( );
    }
}

  /* not a slot */
  void
  heat_wave_main_window_type::
set_damping_background_color( )
{
    heat_solver_type * const p_hsolv = get_heat_solver( );
    set_background_status( ui.p_dspinb_damping_, damping_hilite_status_,
        ((0 <= p_hsolv->get_damping( )) && (p_hsolv->get_damping( ) <= 1)) ? e_normal : e_alert);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_solve_method( )
{
    sheet_control_type * const p_sctrl = get_sheet_control( );
    heat_solver_type   * const p_hsolv = get_heat_solver( );

    QAbstractButton * const p_radio_forw = ui.p_radio_method_forward_diff_ ;
    QAbstractButton * const p_radio_back = ui.p_radio_method_backward_diff_;
    QAbstractButton * const p_radio_cent = ui.p_radio_method_central_diff_ ;

    QAbstractButton * const p_check_para = ui.p_check_method_parallel_     ;

    QAbstractButton * const p_check_fix  = ui.p_check_fix_edges_           ;
    QAbstractButton * const p_check_sink = ui.p_check_sink_center_         ;
    QAbstractButton * const p_check_vort = ui.p_check_vortex_              ;

    // Set the init state of the UI from the state of the solve control object.
    p_radio_forw->setChecked( p_hsolv->is_method__forward_diff(  ));
    p_radio_back->setChecked( p_hsolv->is_method__backward_diff( ));
    p_radio_cent->setChecked( p_hsolv->is_method__central_diff(  ));

    p_check_para->setChecked( p_hsolv->is_method_parallel(      ));

    p_check_fix ->setChecked( p_sctrl->are_edges_fixed(         ));
    p_check_sink->setChecked( p_sctrl->is_center_frozen(        ));
    p_check_vort->setChecked( p_sctrl->is_vortex_on(            ));

    // Radio buttons for solve method.
    d_verify( connect(
        p_radio_forw, SIGNAL( toggled( bool)),
        p_hsolv, SLOT( set_method__forward_diff( bool))));
    d_verify( connect(
        p_radio_back, SIGNAL( toggled( bool)),
        p_hsolv, SLOT( set_method__backward_diff( bool))));
    d_verify( connect(
        p_radio_cent, SIGNAL( toggled( bool)),
        p_hsolv, SLOT( set_method__central_diff( bool))));

    // The alert/caution color on the rates depends on the method.
    d_verify( connect(
        p_hsolv, SIGNAL( method_is_changed( )),
        this, SLOT( set_rate_background_colors( ))));

    // Checkbox for parallel solve.
    d_verify( connect(
        p_check_para, SIGNAL( toggled( bool)),
        p_hsolv, SLOT( set__is_method_parallel( bool))));

    // Checkbox for fixed edges.
    d_verify( connect(
        p_check_fix, SIGNAL( toggled( bool)),
        p_sctrl, SLOT( set__are_edges_fixed( bool))));

    // Checkbox for sunken point in center of sheet.
    d_verify( connect(
        p_check_sink, SIGNAL( toggled( bool)),
        p_sctrl, SLOT( set__is_center_frozen( bool))));

    // Checkbox for fixed edges.
    d_verify( connect(
        p_check_vort, SIGNAL( toggled( bool)),
        p_sctrl, SLOT( set__is_vortex_on( bool))));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

// If we were reporting on:
//   p_sctrl->get_x_draw_size_limit( )
//   p_sctrl->get_y_draw_size_limit( )
//
// Then we should be listening for:
//   p_sctrl SIGNAL( draw_size_limit_is_changed( ))

  void
  heat_wave_main_window_type::
init_draw_limits( )
{
    sheet_control_type * const p_sctrl              = get_sheet_control( );
    QAbstractButton    * const p_checkb_is_limited  = ui.p_checkb_is_draw_size_limited_;
    QSpinBox           * const p_spinb_xy_thousands = ui.p_spinb_xy_draw_size_limit_;

    p_checkb_is_limited->setChecked( p_sctrl->is_draw_size_limited( ));
    p_spinb_xy_thousands->setValue( p_sctrl->get_xy_draw_size_limit( ) / 1000);

    d_verify( connect(
        p_checkb_is_limited, SIGNAL( toggled( bool)),
        p_sctrl, SLOT( set__is_draw_size_limited( bool))));

    d_verify( connect(
        p_spinb_xy_thousands, SIGNAL( editingFinished( )),
        this, SLOT( set_xy_draw_size_limit( ))));
}

  /* slot */
  void
  heat_wave_main_window_type::
set_xy_draw_size_limit( )
{
    int const new_limit_in_thousands = ui.p_spinb_xy_draw_size_limit_->value( );
    get_sheet_control( )->set_xy_draw_size_limit( new_limit_in_thousands * 1000);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_window_size_labels( )
{
    heat_widget_type * const p_hw = get_heat_widget( );
    d_verify( connect(
        p_hw, SIGNAL( size_changed( )),
        this, SLOT( set_window_size_labels( ))));

    set_window_size_labels( );
}

  /* slot */
  void
  heat_wave_main_window_type::
set_window_size_labels( )
{
    d_assert( ui.p_heat_wave_widget_and_scrolls);
    d_assert( ui.p_label_window_size_scroll_area_x);
    d_assert( ui.p_label_window_size_scroll_area_y);
    ui.p_label_window_size_scroll_area_x->setNum( ui.p_heat_wave_widget_and_scrolls->geometry( ).width( ));
    ui.p_label_window_size_scroll_area_y->setNum( ui.p_heat_wave_widget_and_scrolls->geometry( ).height( ));

    heat_widget_type * const p_hw = get_heat_widget( );

    d_assert( ui.p_label_window_size_graphics_area_x);
    d_assert( ui.p_label_window_size_graphics_area_y);
    ui.p_label_window_size_graphics_area_x->setNum( p_hw->width( ));
    ui.p_label_window_size_graphics_area_y->setNum( p_hw->height( ));

    // p_top_widget is zero if we are fullscreen
    QWidget * p_top_widget = p_hw->parentWidget( );
    if ( p_top_widget ) {
        for ( ; ; ) {
            QWidget * const p_top_widget_next = p_top_widget->parentWidget( );
            if ( ! p_top_widget_next ) break;
            p_top_widget = p_top_widget_next;
        }

        d_assert( ui.p_label_window_size_client_x);
        d_assert( ui.p_label_window_size_client_y);
        ui.p_label_window_size_client_x->setNum( p_top_widget->width( ));
        ui.p_label_window_size_client_y->setNum( p_top_widget->height( ));

        d_assert( ui.p_label_window_size_desktop_x);
        d_assert( ui.p_label_window_size_desktop_y);
        ui.p_label_window_size_desktop_x->setNum( p_top_widget->frameGeometry( ).width( ));
        ui.p_label_window_size_desktop_y->setNum( p_top_widget->frameGeometry( ).height( ));
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_lighting( )
{
    heat_widget_type  *  p_hw   = get_heat_widget( );
    lighting_rig_type *  p_lr   = p_hw->get_lighting_rig( );
    light_type        *  p_li0  = p_lr->get_light_0( );
    light_type        *  p_li1  = p_lr->get_light_1( );

    p_lr->attach_ambient_color_ctrls__overall(
        ui.p_check_is_overall_ambient_on_, ui.p_color_button_overall_ambient_);

    p_li0->attach_on_off_button( ui.p_check_is_light1_on_);
    p_li0->attach_arrow_display_button( ui.p_check_is_light1_arrow_on_);
    p_li0->attach_ambient_color_ctrls(  ui.p_check_is_light1_ambient_on_ , ui.p_color_button_light1_ambient_ );
    p_li0->attach_diffuse_color_ctrls(  ui.p_check_is_light1_diffuse_on_ , ui.p_color_button_light1_diffuse_ );
    p_li0->attach_specular_color_ctrls( ui.p_check_is_light1_specular_on_, ui.p_color_button_light1_specular_);
    p_li0->attach_spin_angle_ctrls( ui.p_check_light1_spin_is_move_with_eye_, ui.p_slider_light1_spin_angle_);
    p_li0->attach_rise_angle_ctrls( ui.p_check_light1_rise_is_move_with_eye_, ui.p_slider_light1_rise_angle_);

    p_li1->attach_on_off_button( ui.p_check_is_light2_on_);
    p_li1->attach_arrow_display_button( ui.p_check_is_light2_arrow_on_);
    p_li1->attach_ambient_color_ctrls(  ui.p_check_is_light2_ambient_on_ , ui.p_color_button_light2_ambient_ );
    p_li1->attach_diffuse_color_ctrls(  ui.p_check_is_light2_diffuse_on_ , ui.p_color_button_light2_diffuse_ );
    p_li1->attach_specular_color_ctrls( ui.p_check_is_light2_specular_on_, ui.p_color_button_light2_specular_);
    p_li1->attach_spin_angle_ctrls( ui.p_check_light2_spin_is_move_with_eye_, ui.p_slider_light2_spin_angle_);
    p_li1->attach_rise_angle_ctrls( ui.p_check_light2_rise_is_move_with_eye_, ui.p_slider_light2_rise_angle_);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_face_properties( )
{
    face_properties_style_type *  p_face_props  = get_heat_widget( )->get_face_properties( );

    p_face_props->get_top_face_style( )->attach
     (  ui.p_radio_top_face_filled_
      , ui.p_radio_top_face_lined_
      , ui.p_radio_top_face_dotted_
      , ui.p_radio_top_face_invisible_
     );
    p_face_props->get_under_face_style( )->attach
     (  ui.p_radio_under_face_filled_
      , ui.p_radio_under_face_lined_
      , ui.p_radio_under_face_dotted_
      , ui.p_radio_under_face_invisible_
     );

    p_face_props->get_top_face_shininess(   )->attach( ui.p_slider_top_face_shininess_);
    p_face_props->get_under_face_shininess( )->attach( ui.p_slider_under_face_shininess_);

    p_face_props->get_shading_style( )->attach
     (  ui.p_radio_shader_block_
      , ui.p_radio_shader_simple_
      , ui.p_radio_shader_strip_
      , ui.p_radio_shader_area_
     );
    p_face_props->get_is_triangle_mesh( )->attach( ui.p_check_mesh_triangles_);

    // Instead of this:
    //   p_face_props->get_is_interpolate( )->attach( ui.p_check_shader_interpolate_normals_);
    // We have a custom way of doing this:
    ui.p_check_shader_interpolate_normals_->setChecked( false);
    d_verify( connect(
        ui.p_check_shader_interpolate_normals_, SIGNAL( toggled( bool)),
        this, SLOT( set_shader_interpolate( bool))
    ));
    // We should also connect back the other way, so that if the holder changes it will change
    // the checkbox display. But that isn't necessary right now.

    p_face_props->get_pixel_width( )->attach( ui.p_spinb_face_pixel_width_);
    p_face_props->get_antialias_style( )->attach
     (  ui.p_radio_face_antialias_best_
      , ui.p_radio_face_antialias_fastest_
      , ui.p_radio_face_antialias_none_
     );
}

  /* slot */
  void
  heat_wave_main_window_type::
set_shader_interpolate( bool is_on)
{
    if ( is_on ) {
        // We only need to do this the first time, to get warning dialogs out of the way.
        init_shader_program__blinn_phong( );
    }
    // If the shader is not available maybe we should not allow this to turn on.
    // We'd have to ui.p_check_shader_interpolate_normals_->setChecked( false).
    get_heat_widget( )->get_face_properties( )->get_is_interpolate( )->set_value( is_on);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_bristle_properties( )
{
    bristle_properties_style_type *  p_bristle_props  = get_heat_widget( )->get_bristle_properties( );

    p_bristle_props->get_style( )->attach
     (  ui.p_radio_bristle_style_lines_
      , ui.p_radio_bristle_style_dots_
      , ui.p_radio_bristle_style_none_
     );

    p_bristle_props->get_top_length(   )->attach( ui.p_slider_bristle_top_length_  );
    p_bristle_props->get_under_length( )->attach( ui.p_slider_bristle_under_length_);

    p_bristle_props->get_top_color(   )->attach( ui.p_push_bristle_top_color_  );
    p_bristle_props->get_under_color( )->attach( ui.p_push_bristle_under_color_);

    p_bristle_props->get_position( )->attach
     (  ui.p_radio_bristle_pos_triangle_center_
      , ui.p_radio_bristle_pos_quad_center_
      , ui.p_radio_bristle_pos_quad_quads_
      , ui.p_radio_bristle_pos_simple_corners_
      , ui.p_radio_bristle_pos_strip_corners_
      , ui.p_radio_bristle_pos_area_corners_
      , ui.p_radio_bristle_pos_area_penta_
     );

    p_bristle_props->get_pixel_width( )->attach( ui.p_spinb_bristle_pixel_width_);

    p_bristle_props->get_antialias_style( )->attach
     (  ui.p_radio_bristle_antialias_best_
      , ui.p_radio_bristle_antialias_fastest_
      , ui.p_radio_bristle_antialias_none_
     );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_wave_main_window_type::
init_isotherm_properties( )
{
    isotherm_properties_style_type *  p_isotherm_props  = get_heat_widget( )->get_isotherm_properties( );

    p_isotherm_props->get_is_showing( )->attach( ui.p_check_is_showing_isotherms_);

    p_isotherm_props->get_width(   )->attach( ui.p_slider_isotherm_width_ );
    p_isotherm_props->get_spacing( )->attach( ui.p_slider_isotherm_space_ );
    p_isotherm_props->get_offset(  )->attach( ui.p_slider_isotherm_offset_);

    p_isotherm_props->get_color(   )->attach( ui.p_push_isotherm_color_);
    p_isotherm_props->get_opacity( )->attach( ui.p_slider_isotherm_opacity_);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Save Image File
//
//   This should have two data structures:
//     A list of all supported image formats (list of format_info).
//     This is initialized once and read-only (const) after that.
//     This is shared by all image-file savers.
//
//     A pair< file_name, format_info > that remembers the last saved file. Each kind of
//     image saver can have one of these, and it should be possible to make these cookie-
//     persistent across sessions.
// _______________________________________________________________________________________________

// File scope:
namespace /* anonymous */ {

# ifdef Q_WS_WIN
  QString const save_image_format_for_dlg__all = QObject::tr( "All Files (*.*)");
# else
  QString const save_image_format_for_dlg__all = QObject::tr( "All Files (*)");
# endif

  struct
image_format_info_type
{
    /* ctor */  image_format_info_type( QByteArray const &) ;

    QByteArray  as_bytes                                    ;
    QString     as_string                                   ;
    QString     as_string_file_extension                    ;
    QString     as_string_for_dlg                           ;
};

  /* ctor */
  image_format_info_type::
image_format_info_type( QByteArray const & as_byte_array)
  : as_bytes ( as_byte_array)
  , as_string( QString::fromLocal8Bit( as_byte_array.data( )))
  , as_string_file_extension
             ( QObject::tr( ".") + as_string)
  , as_string_for_dlg
             ( as_string.toUpper( ) + QObject::tr( " (*.") +
               as_string.toLower( ) + QObject::tr( ")"   )  )
  //
  // This converts a byte-array using QString::fromLocal8Bit(..).
  // I'm not sure that's right. The choices are:
  //   QString::fromLocal8Bit( byte_array.data( ))
  //   QString::fromAscii(     byte_array.data( ))
  //   QString::fromLatin1(    byte_array.data( ))
  //   QString::fromUtf8(      byte_array.data( ))
{ }

QList< image_format_info_type > save_image_format_info_list   ;
QString                         save_image_format_list_string ;

  char const *
find_image_format_from_format_for_dlg
 (  QString const &  format_for_dlg
  , QString       &  last_format_for_dlg
 )
{
    // Use "int" instead of "size_t" because Qt interface is written that way.
    for ( int
            index = 0
          ; index < save_image_format_info_list.size( )
          ; ++ index )
    {
        image_format_info_type const &
            item = save_image_format_info_list.at( index);
        if ( item.as_string_for_dlg == format_for_dlg ) {
            last_format_for_dlg = item.as_string_for_dlg;
            return item.as_bytes.constData( );
        }
    }
    return 0;
}

} /* end namespace anonymous */

  void
  heat_wave_main_window_type::
init_save_image_file( )
{
    // Set up where we save the first file.
    // This is the same dir where the executable file lives and is not a good default.
    // We should store this location as a (persistent) registry cookie between session.
    last_save_image_file_name_ = QDir::currentPath( );

    // Build a list of image formats we can easily write.
    QList< QByteArray > image_formats = QImageWriter::supportedImageFormats( );
    while ( ! image_formats.isEmpty( ) ) {
        image_format_info_type const image_format_info( image_formats.takeFirst( ));
        save_image_format_info_list.append( image_format_info);

        if ( ! save_image_format_list_string.isEmpty( ) ) {
            save_image_format_list_string += tr( ";;");
        }
        save_image_format_list_string += image_format_info.as_string_for_dlg;
    }
    if ( ! save_image_format_list_string.isEmpty( ) ) {
        // End the string with:
        //   ";;All Files (*)"    - linux
        //   ";;All Files (*.*)"  - windows
        save_image_format_list_string += tr( ";;");
        save_image_format_list_string += save_image_format_for_dlg__all;
    }

    // Attach the button.
    d_verify( connect(
        ui.p_button_save_image_file_, SIGNAL( clicked( )),
        this, SLOT( save_image_file( ))
    ));
}

  /* slot */
  void
  heat_wave_main_window_type::
save_image_file( )
{
    if ( save_image_format_list_string.isEmpty( ) ) {
        // If QImageWriter::supportedImageFormats( ) cannot find any supported image formats ...
        QMessageBox::warning( this,
          QObject::tr( "Missing File-Save Support"),
          QObject::tr( "Cannot save image file because no supported image-formats were discovered. Press OK to close."));
    } else {
        // Ask the user for an image save-file name.
        // This will warn you if the file already exists (and you will write over it).
        QString const caption = tr( "Choose an Image Save-File Name");
        QString       save_image_format_for_dlg = last_save_image_format_for_dlg_; /* might be empty string */
        QString const save_image_file_name =
            QFileDialog::getSaveFileName
             (  this                            /* parent window */
              , caption                         /* default is "Save As" */
              , last_save_image_file_name_      /* either directory or file (full-path) */
              , save_image_format_list_string   /* items separated by ";;" */
              , & save_image_format_for_dlg     /* format string, like "JPG (*.jpg)" or "BMP (*.bmp)" */
             );

        // If the user cancels the return string is empty.
        if ( ! save_image_file_name.isEmpty( ) ) {
            // We could pick this file-name apart:
            //   QFileInfo file_info = save_image_file_name;

            // Save the file-name so we start there the next time we ask for an image save-file name.
            last_save_image_file_name_ = save_image_file_name;

            // The format is useless if it is "All (*.*)".
            char const *
                p_format =
                    find_image_format_from_format_for_dlg
                     (  save_image_format_for_dlg
                      , last_save_image_format_for_dlg_
                     );

            // Create an image and save it to the chosen file.
            get_heat_widget( )->save_image_to_file( save_image_file_name, p_format);
        }
    }
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// heat_simd.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
