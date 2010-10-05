// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// heat_widget.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Notes:
//
//   Is glutFullScreen( ) smoother than the full-screen we have now?
//
//   Is there any advantage to glutReshapeWindow( 500, 500) and glutPositionWindow( 50, 50)
//   over the QT stuff?
//
//   Draw stats (text, stats like FPS) transparently on the front.
//     This is a way to report stats, instead of in the sidebar.
//     Can also do it in toolbar windows and stay-on-top popups.
//     Or in the title/status bar for the window.
//
//   Split screens. Overview on left, zoom on the right?
//
//   Material model:
//     String of z values, each associated with a color.
//     Two more colors, for above the highest and below the lowest? No, probably better if
//       the highest and lowest colors are just used for any overflows.
//
//     You can have separate front/back properties.
//     You can make the material semi-transparent with the (diffuse) alpha value.
//       But we'll have to draw front-to-back with any transparency.
//       First draw all the opaque materials normally, lock the depth buffer, and draw
//       the transparent stuff back-to-front.
//
//   Flat vs spherical projections:
//     Your eye projects onto your spherically shaped retina.
//     But the computer screen is flat, and OpenGL does a flat projection.
//     A perfect lens (a pinhole) can project correctly onto either kind of surface.
//     You cannot project more than 180 degree angle onto a flat sheet, and you'd need
//     an infinite sheet to get to 180 degrees. But s spherical sheet is not restricted
//     in the same way (except by the lens of course).
//     Fisheye lenses that project more than 180 degrees onto a flat sheet are not
//     linear projections. You cannot describe them with the transformation matrices,
//     and so you cannot fisheye in OpenGL.
//
//     Think of a 3D pinhole in a 4D space.
// _______________________________________________________________________________________________

# include "all.h"
# include "heat_widget.h"
# include "gl_env_global.h"
# include "gl_draw_back_grid.h"
# include "draw_sheet_surface.h"
# include "draw_sheet_bristles.h"
# include "shader.h"

# include <QtGui/QMessageBox>
# include <QtGui/QScrollBar>
# include <QtOpenGL/QGLFormat>
# include <QtOpenGL/QGLPixelBuffer>
# include <QtOpenGL/QGLFramebufferObject>

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Constructor
// _______________________________________________________________________________________________

# if 0
  void
debug__examine_gl_format( QGLFormat const & format)
  //
  // Use this in the debugger to examine GL formats.
{
    // Notes:
    //   samples( ) means multi-sampling
    //   swapInterval( ) means sync with vertical refresh
    //   directRendering( ) means use hw acceleration.
    //     Set false for software rendering (like you get when you render directly to a bitmap in Windows).

    // We cannot disable the accumulator buffer on our system.
    // We cannot turn off alpha when we're rending to a pbo, but we can with a window.

    // The columns in the end-of-line comments show:
    //   Default values, from QGLFormat::defaultFormat( )
    //   Default-widget values, from this widget when it's created with no format specs
    //   Actual widget values, with stencil and accum turned off. Although apparently you cannot
    //     turn accum off, at least not on my system.
    //   Value for pbo that is passed format( ) from widget. I tried turning accum off here
    //     also, but my system would not let me. I also could not turn alpha off.
    int  const  i0  = format.depthBufferSize(   );  // -1, 24, 24, 24
    int  const  i1  = format.accumBufferSize(   );  // -1, 64, 64, 64
    int  const  i2  = format.redBufferSize(     );  // -1,  8,  8,  8
    int  const  i3  = format.greenBufferSize(   );  // -1,  8,  8,  8
    int  const  i4  = format.blueBufferSize(    );  // -1,  8,  8,  8
    int  const  i5  = format.alphaBufferSize(   );  // -1, -1, -1,  8
    int  const  i6  = format.stencilBufferSize( );  // -1,  8, -1, -1
    int  const  i7  = format.samples(           );  // -1, -1, -1, -1
    int  const  i8  = format.swapInterval(      );  // -1,  1,  1, -1
    int  const  i9  = format.plane(             );  //  0,  0,  0,  0

    // The only place where the default-widget value is different from the
    // default value is ->accum( ), which is false by default.
    bool const  b0  = format.sampleBuffers(     );  // f, f, f, f
    bool const  b1  = format.doubleBuffer(      );  // t, t, t, f
    bool const  b2  = format.depth(             );  // t, t, t, t
    bool const  b3  = format.rgba(              );  // t, t, t, t
    bool const  b4  = format.alpha(             );  // f, f, f, t  - cannot turn this off for pbo
    bool const  b5  = format.accum(             );  // f, t, t, t  - cannot turn this off
    bool const  b6  = format.stencil(           );  // t, t, f, f
    bool const  b7  = format.stereo(            );  // f, f, f, f
    bool const  b8  = format.directRendering(   );  // t, t, t, t
    bool const  b9  = format.hasOverlay(        );  // f, f, f, f
}
# endif


  /* ctor */
  heat_widget_type::
heat_widget_type( QWidget * p_parent /* = 0 */)

  : QGLWidget                    (  QGLFormat
                                     ( QGL::NoAccumBuffer   |  /* ignored on my Windows system */
                                       QGL::NoStencilBuffer
                                     )
                                  , p_parent
                                 )

  , p_full_screen_               ( 0)
  , p_animate_                   ( 0)
  , p_lighting_rig_              ( 0)
  , p_face_properties_           ( 0)
  , p_bristle_properties_        ( 0)
  , p_isotherm_properties_       ( 0)
  , p_sheet_control_             ( 0)
  , p_auto_solve_update_delay_   ( 0)

  , is_update_being_suppressed_  ( false)
  , is_before_gl_init_           ( true )
  , is_painting_                 ( false)
  , is_drawing_to_fbo_           ( false)
  , is_drawing_to_pbo_           ( false)
  , is_drawing_to_pixmap_        ( false)

  , start_finish_durations_draw_ ( 32)
  , draw_count_                  ( 0)

  , p_is_showing_grid_           ( 0)
  , p_is_clipping_               ( 0)
  , p_clipping_center_           ( 0)
  , p_clipping_spread_           ( 0)
  , p_color_holder_background_   ( 0)
  , p_color_gradient_holder_     ( 0)

  , p_perspective_angle_         ( 0)
  , p_rise_angle_                ( 0)
  , p_z_axis_rotation_angle_     ( 0)
  , p_zoom_scale_                ( 0)
  , p_model_scale_               ( 0)
  , p_z_scale_                   ( 0)
  , p_eye_right_translation_     ( 0)
  , p_eye_up_translation_        ( 0)
{
    d_assert( QGLFormat::hasOpenGL( ));
    d_assert( isValid( ));

    // Since this is a widget, the title doesn't show and is mostly used when searching child windows.
    setWindowTitle( QObject::tr( "heat widget"));

    construct_slider_holders( );
    setup_broadcast_scroll_steps_changed( );

    init_animation( );
    init_lighting_rig( );
    init_face_properties( );
    init_bristle_properties( );
    init_isotherm_properties( );
    init_colors( );
    init_solve_control( );
}

// _______________________________________________________________________________________________

  /* virtual dtor */
  heat_widget_type::
~heat_widget_type( )
{
    // Release the shaders and lights. Do it while the appropriate OpenGL context is bound.
    // This is optional since the gl-context will soon be destroyed (along with the shaders etc).
    d_assert( ! is_painting_);
    d_assert( ! is_painting_off_screen( ));
    if ( ! is_before_gl_init_ ) {
        release_shader_program__blinn_phong( );  /* optional */
        get_lighting_rig( )->detach_from_gl( );  /* optional */
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Update slots
// _______________________________________________________________________________________________

  /* slot */
  void
  heat_widget_type::
maybe_update( )
{
    if ( ! is_update_being_suppressed_ ) {
        update( );
    }
}

  /* slot */
  void
  heat_widget_type::
maybe_update_after_sheet_change( )
{
    if ( ! p_sheet_control_->is_auto_solving( ) ) {
        maybe_update( );
    } else
    if ( ! p_auto_solve_update_delay_ ) {
        maybe_update( );
    } else
    if ( p_auto_solve_update_delay_->is_out_of_date( p_sheet_control_->get_sheet_generation( )) ) {
        maybe_update( );
    }
}

  /* slot */
  void
  heat_widget_type::
maybe_update_after_auto_solve_stop( bool is_started)
{
    // We only delay if p_auto_solve_update_delay_ is not zero.
    if ( (! is_started) &&
         p_auto_solve_update_delay_ &&
         p_auto_solve_update_delay_->is_out_of_date_gen( p_sheet_control_->get_sheet_generation( )) )
    {
        if ( p_sheet_control_->is_next_solve_pending( ) ) {
            // When auto-solve is going full-tilt (with no delays between the finish and the
            // next start) then there is always a next-solve pending when we get here.

            // Since a solve is pending, we can wait for it to finish and get updated then.
            // If the solve is going to be very long however we may want to update now.
            // And if the solve is short there's a good chance the worker thread is done and
            // the is-done message will be queued and processed before we actually get around
            // to drawing. In which case we'll still draw only once.
            // But if the solve is not quite done and we call maybe_update( ) here, we'll get
            // a quick double draw.

            // maybe_update( ) ?
            // maybe we should set a timer? 200 msecs?
        } else {
            maybe_update( );
        }
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// QWidget overrides
// _______________________________________________________________________________________________

  /* overridden virtual from QWidget */
  void
  heat_widget_type::
mousePressEvent( QMouseEvent *)
{
    exit_full_screen( );
}

  /* overridden virtual from QWidget */
  void
  heat_widget_type::
keyPressEvent( QKeyEvent *) 
{
    exit_full_screen( );
}

// _______________________________________________________________________________________________

  /* overridden virtual from QGLWidget and QWidget */
  void
  heat_widget_type::
paintEvent( QPaintEvent * p_paint_event)
  //
  // This is called for most painting, which is usually driven by messages in the message queue.
  // But this is NOT called for synchronous painting, when paintGL( ) is called directly.
  // Search for paintGL( ), updateGL( ), renderPixmap( ), etc to see where that happens.
{
    // Is this ever clipping, or is it always a full-screen update?
    // We assume a full-screen update because we draw the full screen every time, although that
    // could change if we ever start drawing to a bitmap and only bliting forward when done.
    // This is not exactly the same as double buffering but it's close.

    // This sometimes gets called before GL is setup. Specifically, it happens when we are animating
    // (which is constantly painting) and we switch to full-screen mode. The window gets paint messages
    // before it gets a chance to init GL.
    if ( is_before_gl_init_ ) {
        // Post another paint message for later. These are low-priority messages.
        maybe_update( );
        return;
    }

    // This can happen if we post a dialog during paintGL( ).
    if ( is_painting_ ) {
        // This is pretty ugly.
        return;
    }
    // is_painting_off_screen( ) might be true here. That's OK as long as is_painting_ is not
    // true (as long as we're not inside PaintGL( ).

    // Start timing the draw.
    tick_point_type const  tick_start_draw  = date_time::get_tick_now( );

    // If we are animating and solving at the same time, we might want to skip this update if
    //   This is a full-window draw.
    //   We are animating.
    //   We are auto-solving or just expecting a solve to finish soon.
    //   We are expecting the next solve to finish before the next animation draw.
    //
    // tick_duration_type tick_count_to_next_animation_draw = ...
    // if ( p_solve_control->is_sheet_change_expected_soon( tick_count_to_next_animation_draw) )
    //   skip this refresh
    //   tell auto-solve-draw-update to draw the next solve and disregard any delay
    // }

    // Supress calls to update( ) since are about to redraw everything anyway.
    // This assert(..) can fail if we assert(..) while in the middle of a draw.
    d_assert( ! is_update_being_suppressed_);
    is_update_being_suppressed_ = true;
    try {
        // The context must be set before we call maybe_move( ). This is a bug. The animated rig should
        // not depend on having an active GL context.
        // The context must be for this widget if we're here. When we draw into a pixmap or pbo context,
        // we call paintGL( ) directly. We do not go thru here.
        this->makeCurrent( );  /* it's probably already set, but make extra sure */
        d_assert( context( ) && (context( ) == QGLContext::currentContext( )));

        // If we are animating, let the animate object move stuff before we paint.
        // We should not call this if the paint is clipped, only if the paint is full-window.
        d_assert( p_animate_);
        p_animate_->maybe_move( );

        // Call the supertype. This in turn calls paintGL(..).
        QGLWidget::paintEvent( p_paint_event);
        // At this point we'd like to call ::ValidateRect( HWND, 0), but that's not available cross-platform.

        // Increment draw-count. Avoid wrapping into negative numbers.
        draw_count_ += 1;
        if ( draw_count_ < 0 ) {
            draw_count_ = 0;
        }
    }
    catch ( ... ) {
        is_update_being_suppressed_ = false;
        throw;
    }
    d_assert( is_update_being_suppressed_);
    is_update_being_suppressed_ = false;

    // Record how long the draw took.
    if ( date_time::is_valid_tick_pt( tick_start_draw) ) {
        tick_point_type const tick_finish_draw = date_time::get_tick_now( );
        if ( tick_finish_draw >= tick_start_draw ) {
            start_finish_durations_draw_.record_next( tick_finish_draw - tick_start_draw);
        }
    }

    // Tell the delay object we've just updated.
    if ( p_auto_solve_update_delay_ ) {
        p_auto_solve_update_delay_->update( p_sheet_control_->get_sheet_generation( ));
    }

    // Let anyone know who is displaying draw stats.
    emit draw_finished( );
}

// _______________________________________________________________________________________________

  /* overridden virtual from QGLWidget and QWidget */
  void
  heat_widget_type::
resizeEvent( QResizeEvent * p_resize_event)
{
    QWidget * const p_parent = parentWidget( );
    if ( p_parent &&
        (! p_parent->isHidden( )) && (! p_parent->isMinimized( )) &&
        (! isHidden( )) && (! isFullScreen( )) )
    {
        emit size_changed( );
    }

    // Call the supertype.
    QGLWidget::resizeEvent( p_resize_event);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Full screen
// _______________________________________________________________________________________________

  void
  heat_widget_type::
init_full_screen( QAbstractButton * p_full_screen_button)
  //
  // Call this function to hook-up a full-screen push-button.
{
    // Setup full-screen display of the heat widget.
    d_assert( ! p_full_screen_);
    p_full_screen_ = new QeFullScreenChild( this);
    d_assert( p_full_screen_);

    d_verify( connect(
        p_full_screen_button, SIGNAL( clicked( )),
        this, SLOT( enter_full_screen( ))
    ));
}

  /* slot */
  void
  heat_widget_type::
enter_full_screen( )
{
    d_assert( ! is_painting_);
    d_assert( ! is_painting_off_screen( ));
    if ( ! isFullScreen( ) ) {
        get_isotherm_properties( )->teardown_gl( ); /* unbind and delete 1D texture */

        // Mark this true because in MSWindows Qt creates a new rendering context when it
        // goes into full-screen. But I don't know if this would be true for Unix/Linux/etc
        // even if we did get full-screen working on anything but Windows.
        is_before_gl_init_ = true;

        d_assert( p_full_screen_);
        p_full_screen_->doFullScreen( );
    }
}

  /* slot */
  void
  heat_widget_type::
exit_full_screen( )
  //
  // Call this function to exit full-screen mode.
  // It's safe to call this any time, even when we're not in full-screen mode.
{
    d_assert( ! is_painting_);
    d_assert( ! is_painting_off_screen( ));
    if ( isFullScreen( ) ) {
        get_isotherm_properties( )->teardown_gl( ); /* unbind and delete 1D texture */

        // Mark this true because in MSWindows Qt creates a new rendering context when it
        // leaves full-screen. But I don't know if this would be true for Unix/Linux/etc
        // (full-screen isn't implemented for anything but Windows).
        is_before_gl_init_ = true;

        d_assert( p_full_screen_);
        p_full_screen_->undoFullScreen( );
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Animation
//
//   The animation wakeup() signal triggers slot maybe_update(), which is a simple update.
//   But since the solver and auto-solve also drives a kind of animation, we could weave the two
//   togther more more carefully.
//   (This can be accomplished in the animate object. See notes in animate.cpp.)
// _______________________________________________________________________________________________

  void
  heat_widget_type::
init_animation( )
{
    d_assert( ! p_animate_);
    p_animate_ = new animate_type( this);
    d_assert( p_animate_);

    d_verify( connect(
        p_animate_, SIGNAL( wakeup( )),
        this, SLOT( maybe_update( ))
    ));
    // Is there any way to know, during paint, who asked for the update?
    // We can set a flag when we call update( ), so during paint we can know that animation,
    // for example, asked for the update. But we cannot know if something else, maybe the windowing
    // system, also asked for an update.
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Lighting rig
// _______________________________________________________________________________________________

  void
  heat_widget_type::
init_lighting_rig( )
{
    d_assert( ! p_lighting_rig_);
    p_lighting_rig_ = new lighting_rig_type( this);
    d_assert( p_lighting_rig_);

    d_verify( connect(
        p_lighting_rig_, SIGNAL( has_changed( )),
        this, SLOT( maybe_update( ))
    ));

    // The lighting still has to be attached to the UI controls.
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_widget_type::
init_face_properties( )
{
    d_assert( ! p_face_properties_);
    p_face_properties_ = new face_properties_style_type( this);
    d_assert( p_face_properties_);

    d_verify( connect(
        p_face_properties_, SIGNAL( has_changed( )),
        this, SLOT( maybe_update( ))
    ));
}

  void
  heat_widget_type::
init_bristle_properties( )
{
    d_assert( ! p_bristle_properties_);
    p_bristle_properties_ = new bristle_properties_style_type( this);
    d_assert( p_bristle_properties_);

    d_verify( connect(
        p_bristle_properties_, SIGNAL( has_changed( )),
        this, SLOT( maybe_update( ))
    ));
}

  void
  heat_widget_type::
init_isotherm_properties( )
{
    d_assert( ! p_isotherm_properties_);
    p_isotherm_properties_ = new isotherm_properties_style_type( this);
    d_assert( p_isotherm_properties_);

    d_verify( connect(
        p_isotherm_properties_, SIGNAL( has_changed( )),
        this, SLOT( maybe_update( ))
    ));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Colors
// _______________________________________________________________________________________________

  void
  heat_widget_type::
init_colors( )
{
    // Setup the background color.
    color_type const dark_green( 0, 0.3f, 0.2f);
    p_color_holder_background_ = new color_holder( this, dark_green, QObject::tr( "Choose background color"));
    d_verify( connect( p_color_holder_background_, SIGNAL( has_changed( )), this, SLOT( maybe_update( ))));
    // We should probably have a slot to watch for background-color changes which would:
    //   Set the GL background color
    //   Tell the window to redraw
    // Right now we just set the GL background color every time we draw.

    p_color_gradient_holder_ = new color_gradient_holder( this);
    d_verify( connect( p_color_gradient_holder_, SIGNAL( has_changed( )), this, SLOT( maybe_update( ))));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Draw stats
// _______________________________________________________________________________________________

  bool
  heat_widget_type::
is_available_duration_draw( ) const
{
    return ! start_finish_durations_draw_.is_empty( );
}

  heat_widget_type::second_type
  heat_widget_type::
get_average_duration_draw_seconds( ) const
{
    // Design flaw. This is calculating the average by dividing the sum by the count.
    // Calling .get_average< second_type >( ) works when the tick_duration_type is a type that can
    // be cast to second_type, like when tick_duration_type is clock_t (a primative int). But when
    // tick_duration_type is a boost::posix_time::time_duration this no longer works.
    return start_finish_durations_draw_.get_average__seconds( );
}

  heat_widget_type::millisecond_type
  heat_widget_type::
get_average_duration_draw_mseconds( ) const
{
    return date_time::convert_seconds_to_milliseconds( get_average_duration_draw_seconds( ));
}

  void
  heat_widget_type::
clear_draw_stats( )
{
    draw_count_ = 0;
    start_finish_durations_draw_.set_empty( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Sliders and Scrollbars
//
// Macros to define the following getters:
//   get_model_scale( )
//   get_z_scale( )
//   get_zoom_scale( )
//   get_eye_right_translation( )
//   get_eye_up_translation( )
//
//   get_z_axis_rotation_angle__degrees( )   get_z_axis_rotation_angle__radians( )
//   get_perspective_angle__degrees( )       get_perspective_angle__radians( )
//   get_rise_angle__degrees( )              get_rise_angle__radians( )
// _______________________________________________________________________________________________

# define DEFINE_HOLDER_GETTER( NAME)          \
      double                                  \
      heat_widget_type::                      \
    get_ ## NAME( ) const                     \
      {                                       \
        d_assert( p_ ## NAME ## _);           \
        return p_ ## NAME ## _->get_pack( );  \
      }                                       /* end of macro */

DEFINE_HOLDER_GETTER( model_scale)
DEFINE_HOLDER_GETTER( z_scale)
DEFINE_HOLDER_GETTER( zoom_scale)

# undef DEFINE_HOLDER_GETTER

  double
  heat_widget_type::
get_eye_right_translation( ) const
  {
    d_assert( p_eye_right_translation_);
    return p_eye_right_translation_->get_value( );
  }

  double
  heat_widget_type::
get_eye_up_translation( ) const
  {
    d_assert( p_eye_up_translation_);
    return p_eye_up_translation_->get_value( );
  }

// _______________________________________________________________________________________________

# define DEFINE_ANGLE_HOLDER_GETTERS( NAME)             \
      heat_widget_type::angle_type                      \
      heat_widget_type::                                \
    get_ ## NAME ## __degrees( ) const                  \
      {                                                 \
        d_assert( p_ ## NAME ## _);                     \
        return p_ ## NAME ## _->get_angle__degrees( );  \
      }                                                 \
                                                        \
      heat_widget_type::angle_type                      \
      heat_widget_type::                                \
    get_ ## NAME ## __radians( ) const                  \
      {                                                 \
        d_assert( p_ ## NAME ## _);                     \
        return p_ ## NAME ## _->get_angle__radians( );  \
      }                                                 /* end of macro */

DEFINE_ANGLE_HOLDER_GETTERS( z_axis_rotation_angle)
DEFINE_ANGLE_HOLDER_GETTERS( perspective_angle)
DEFINE_ANGLE_HOLDER_GETTERS( rise_angle)

# undef DEFINE_ANGLE_HOLDER_GETTERS

// _______________________________________________________________________________________________

  void
  heat_widget_type::
construct_slider_holders( )
  //
  // Which can be either model or camera centric?
  //   rise_angle_
  //   z_axis_rotation_angle_
  //   eye_right_translation_
  //   eye_up_translation_
  //     Model  - the model is moving/rotating
  //     Camera - the camera is moving and rotating around the model
  //
  // These can arguably also be either camera or model centric:
  //   zoom_scale_
  //     Closeness - larger values move the camera in (higher zoom) (model centric).
  //     Distance  - larger values move the camera and model farther apart.
  //
  //   perspective_angle_
  //     Distance    - how far away the camera is. Infinity is ortho.
  //     Perspective - how much perspective to apply. Zero is ortho.
  //
  // Scales that only affect parts of the model are model centric.
  // Although you can still invert them to make them agree with overall zoom.
  //   model_scale_
  //   z_scale_
  //     Model size - larger values make the model bigger
  //     Squeeze    - larger values make the model bigger
{
  // ------------------------------------------------------------------------------------------------
  // ------------------------------------------------------------------------------------------------
  # define CONSTRUCT_ANGLE_HOLDER( NAME, INIT, LO, HI )     \
        d_assert( 0 == p_ ## NAME);                         \
        p_ ## NAME = new angle_holder( this, INIT, LO, HI); \
        d_verify( connect(                                  \
            p_ ## NAME, SIGNAL( has_changed( )),            \
            this, SLOT( maybe_update( ))                    \
        ))                                                  /* end macro */

    // Perspective angle
    //   This angle is in degrees, not radians.
    //   This is kept between 0 and +120 inclusive (0 <= angle <= +120).
    //
    //   When this angle is zero, we use an ortho projection instead of a perspective projection.
    //
    //   The math for this works up to an 180 degree angle (angle < 180).
    //   We evaluate this (when angle > 0):
    //     1 / tan( angle / 2)
    //   Since tan( 90 degrees ) is not defined (tan -> sin/cos -> 1/0) we can not evaluate tan,
    //   although we could say 1/tan(90) is zero (the cotangent).
    //   |
    //   If angle >= 180 we are asking for a fish-eye projection. Which isn't supported.
    //
    CONSTRUCT_ANGLE_HOLDER( perspective_angle_, 15, 0, 120);

    // Rise angle
    //   This angle is in degrees, not radians.
    //   This is kept between -90 and +90 inclusive (-90 <= angle <= +90).
    //   -90 means we are looking at the model from underneath, straight up the z-axis.
    //   +90 means we are looking at the model from above, straight down the z-axis.
    //   0 means we are looking at the model from the side, so the z=0 plane is edge-on.
    //
    CONSTRUCT_ANGLE_HOLDER( rise_angle_, -12, -90, +90);

    // Z-axis rotation angle
    //   This angle is in degrees, not radians.
    //   This is kept between 180 and -180 (-180 < angle <= +180).
    //
    CONSTRUCT_ANGLE_HOLDER( z_axis_rotation_angle_, 10, -180, +180);

  # undef CONSTRUCT_ANGLE_HOLDER

  // ------------------------------------------------------------------------------------------------
  // ------------------------------------------------------------------------------------------------
  # define CONSTRUCT_VALUE_HOLDER( NAME, INIT, LO, HI ) \
        d_assert( 0 == p_ ## NAME);                     \
        p_ ## NAME = new                                \
            pack_range_steps_holder(                    \
                this, INIT, LO, HI, true);              \
        d_verify( connect(                              \
            p_ ## NAME, SIGNAL( has_changed( )),        \
            this, SLOT( maybe_update( ))                \
        ))                                              /* end macro */

    // Model scale
    //   In a perspective view, this is the amount of space the middle of the model box
    //   takes up on the screen. It becomes a distance multiplier.
    //   It's the same thing in an ortho view, except during ortho the distance is infinity
    //   so really it's a multiplier. Big number means zoomed out (model appears smaller).
    //
    CONSTRUCT_VALUE_HOLDER( model_scale_, 1.2, 0.2, 10.0);

    // Z scale
    //   Separate value to scale the z value.
    //   Smaller number means a flatter picture.
    //
    CONSTRUCT_VALUE_HOLDER( z_scale_, 1.0, 0.2, 20.0);

    // Zoom scale
    CONSTRUCT_VALUE_HOLDER( zoom_scale_, 1.2, 0.4, 100.0);

  # undef CONSTRUCT_VALUE_HOLDER

    // Eye right translation
    //
    //   Translation tangent to eye sphere.
    //   Positive value translates to the right.
    d_assert( 0 == p_eye_right_translation_);
    p_eye_right_translation_ = new double_slide_holder( this, 0.0, -1.0, +1.0);
    d_verify( connect(
        p_eye_right_translation_, SIGNAL( has_changed( )),
        this, SLOT( maybe_update( ))
    ));

    // Eye up translation
    d_assert( 0 == p_eye_up_translation_);
    p_eye_up_translation_ = new double_slide_holder( this, 0.0, -1.0, +1.0);
    d_verify( connect(
        p_eye_up_translation_, SIGNAL( has_changed( )),
        this, SLOT( maybe_update( ))
    ));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Solve control
// _______________________________________________________________________________________________

  void
  heat_widget_type::
init_solve_control( )
{
    p_sheet_control_ = new sheet_control_type( this);

    // This is not exactly the appropriate slot fot this signal. It does not signal that the
    // sheet is changing, but rather that the way we are drawing the sheet is changing.
    // So maybe the slot should be maybe_update( ) instead, like it is when we change a color
    // or another property that affects drawing.
    d_verify( connect(
        p_sheet_control_, SIGNAL( draw_size_limit_is_changed( )),
        this, SLOT( maybe_update_after_sheet_change( ))));

    d_verify( connect(
        p_sheet_control_, SIGNAL( sheet_is_changed( )),
        this, SLOT( maybe_update_after_sheet_change( ))));

    d_verify( connect(
        p_sheet_control_, SIGNAL( auto_solving_started( bool)),
        this, SLOT( maybe_update_after_auto_solve_stop( bool))));
}

  sheet_control_type *
  heat_widget_type::
get_sheet_control( ) const
{
    d_assert( p_sheet_control_);
    return p_sheet_control_;
}

  void
  heat_widget_type::
set_auto_solve_update_delay( out_of_date_type * p_delay)
{
    // Improve. If this is already set we should check to see if we need an update.
    p_auto_solve_update_delay_ = p_delay;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Grid button
// _______________________________________________________________________________________________

  bool
  heat_widget_type::
is_showing_grid( ) const
{
    return p_is_showing_grid_ ? p_is_showing_grid_->get_value( ) : true;
}

  void
  heat_widget_type::
attach_showing_grid_button( QAbstractButton * p_on_off_button)
{
    // Create the on/off holder and attach it to the button.
    d_assert( ! p_is_showing_grid_);
    p_is_showing_grid_ = new bool_holder( this, true); /* start with this on */
    p_is_showing_grid_->attach( p_on_off_button);
    d_verify( connect( p_is_showing_grid_, SIGNAL( has_changed( )), this, SLOT( maybe_update( ))));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Clipping planes
//
//   A collection of widgets act together to control the clipping slice. They are:
//     An on/off button (checkbox)
//     A slider to control how wide the slice is
//     A scrollbar to move the slice in and out
// _______________________________________________________________________________________________

  bool
  heat_widget_type::
is_slice_clipping( ) const
{
    return p_is_clipping_ ? p_is_clipping_->get_value( ) : false;
}

  double
  heat_widget_type::
get_clipping_center( ) const
{
    return p_clipping_center_ ? p_clipping_center_->get_value( ) : 0.0;
}

  double
  heat_widget_type::
get_clipping_spread( ) const
{
    return p_clipping_spread_ ? p_clipping_spread_->get_value( ) : 1.0;
}

// _______________________________________________________________________________________________

double const  c_slice_clipping_spread_min        = 0.05; // near zero (zero clips away everything)
double const  c_slice_clipping_spread_max        = 1.01; // near 1
double const  c_slice_clipping_spread_value      = 0.10; // init value, changes with slider (from min to max)
double const  c_slice_clipping_center_range_max  = 1.02; // near but > c_slice_clipping_spread_max

  void
  heat_widget_type::
attach_clipping_ensemble
 (  QAbstractButton *  p_on_off_button
  , QScrollBar      *  p_center_scrollbar
  , QAbstractSlider *  p_spread_slider
 )
{
    // These holders are not created until the are attached to controls.

    // Create the on/off holder and attach it to the button.
    d_assert( ! p_is_clipping_);
    p_is_clipping_ = new bool_holder( this, false);
    p_is_clipping_->attach( p_on_off_button);
    d_verify( connect( p_is_clipping_, SIGNAL( has_changed( )), this, SLOT( maybe_update( ))));

    // Create the clip-plane value holders.
    d_assert( ! p_clipping_center_);
    d_assert( ! p_clipping_spread_);

    double const  center_range  = c_slice_clipping_center_range_max - c_slice_clipping_spread_value;
    p_clipping_center_ = new double_slide_holder( this, 0.0, - center_range, + center_range);
    p_clipping_spread_ = new double_slide_holder( this, c_slice_clipping_spread_value,
                                                        c_slice_clipping_spread_min,
                                                        c_slice_clipping_spread_max);

    p_clipping_center_->set_page_step( 2 * c_slice_clipping_spread_value);

    // Attach the scrollbar and slider to the holders.
    p_clipping_center_->attach( p_center_scrollbar);
    p_clipping_spread_->attach( p_spread_slider);

    // Keep the drawing updated.
    d_verify( connect(
        p_clipping_center_, SIGNAL( has_changed( )), this, SLOT( redraw_if_clipping( ))));
    d_verify( connect(
        p_clipping_spread_, SIGNAL( has_changed( )), this, SLOT( redraw_if_clipping( ))));

    // All the slider/scrollbar and holder signals/slots are setup except the one that changes
    // the width of the scrollbar thumb as the slider changes.
    d_verify( connect(
        p_clipping_spread_, SIGNAL( has_changed( )), this, SLOT( recalc_clipping_spread( ))));
}

// _______________________________________________________________________________________________

  /* slot */
  void
  heat_widget_type::
redraw_if_clipping( )
{
    if ( is_slice_clipping( ) ) {
        emit maybe_update( );
    }
}

  /* slot */
  void
  heat_widget_type::
recalc_clipping_spread( )
  //
  // When the clipping-spread value changes, it triggers other changes:
  //   clipping-center range (min and max)
  //   clipping-center value
  //   clipping-center page-step
  //   clipping-center single-step
{
    // Improve:
    //   We need a GUI scrollbar class with slots to set the range (min/max) and step (page/single) values.
    //   The scrollbars we have now only have a slot to set the "value", not the range or steps.

    // We adjust the scrollbar range and page-step to adjust the thumb size.
    // Below we adjust both the range and the steps, but we could get away with just adjusting the range or
    // just the steps if it helped (ie if we had slots for one but not the other).

    d_assert( p_clipping_spread_);
    d_assert( p_clipping_center_);

    double const  new_spread  = p_clipping_spread_->get_value( );
                    d_assert( new_spread >= c_slice_clipping_spread_min);
                    d_assert( new_spread <= c_slice_clipping_spread_max);
                    d_assert( new_spread <  c_slice_clipping_center_range_max);

    double const  old_max     = p_clipping_center_->get_max_value( );
    double const  old_value   = p_clipping_center_->get_value( );

    double const  new_max     = c_slice_clipping_center_range_max - new_spread;
    double const  new_min     = - new_max;
    double const  new_value   = old_max ? (old_value * new_max / old_max) : 0;

    // If the min/max/value haven't changed then neither have the steps.
    p_clipping_center_->set_values( new_value, new_min, new_max, 0, 2 * new_spread);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// 
// Scroll bars
// _______________________________________________________________________________________________

  void
  heat_widget_type::
calc_orthos( double & r_x_ortho, double & r_y_ortho) const
  //
  // Depends on these values
  //   zoom_scale
  //   viewport_xy_ratio (changed when window is resized)
  //
  // Assumes the model is drawn in a [-1..+1] cube.
{
    d_assert( ! is_before_gl_init_);

    // Before model scaling (model_scale and z_scale) is applied, the model fits in
    // a [-1 .. +1] 3D box that can be rotated. The most a corner can stick out from the
    // origin is sqrt( x^2 + y^2 + z^2) or sqrt( 1+1+1).
    static double const fixed_model_radius = std::sqrt( 3.0);

    // Find the sides of the frustum that we'll fit to the viewport.
    // Fit the frustum to the height of the (un-scaled) model, and fit the width to the height.
    // This is consistent with gluPerspective(..).
    // If we fit height and width independently we'd get unwanted stretching.
    r_y_ortho = fixed_model_radius / get_zoom_scale( );
    r_x_ortho = r_y_ortho * get_viewport_xy_size_ratio( );

    // Bug. By calling get_viewport_xy_size_ratio( ) above, we become dependent on having
    // an active GL context. Instead we should either get the viewport size from the widget
    // window, or we should keep it in our own data structure.
}

// _______________________________________________________________________________________________

  void
  heat_widget_type::
calc_model_half_sizes_in_rotated_model_coords
 (  double &  r_x_half_size
  , double &  r_y_half_size
  , double &  r_z_half_size
 ) const
  //
  // x_half_size depends on these values:
  //   z_axis_rotation_angle
  //   model_scale
  //
  // y_half_size and z_half_size depend on all the above and also:
  //   rise_angle
  //   z_scale
  //
  // This is used for scrolling, to calc the width of the scrollbar thumbs and to slide the
  // camera sideways and up/down (tangent) to follow the scrollbars.
{
    static
    double const  sqrt_2_minus_1   = std::sqrt( 2.0) - 1;

    double const  z_axis_rot_rads  = get_z_axis_rotation_angle__radians( );
    double const  horz_spin        = std::abs( std::sin( 2 * z_axis_rot_rads));
    double const  horz_side_bump   = horz_spin * sqrt_2_minus_1;
    double const  x_half_size      = get_model_scale( ) * (1 + horz_side_bump);

    double const  rise_angle_rads  = get_rise_angle__radians( );
    double const  abs_rise_sin     = std::abs( std::sin( rise_angle_rads));
    double const  abs_rise_cos     = std::abs( std::cos( rise_angle_rads));
    // It's not really necessary to take the absolute value of cos( rise-angle) because the
    // rise-angle is in the range [-90..+90] degrees, and the cosine should always be non-negative.
    // But if we allowed a rise angle outside the range [-90 .. +90] then we'd want to take the
    // absolute value.

    double const  vert_correct_1   = x_half_size    * abs_rise_sin;
    double const  vert_correct_2   = get_z_scale( ) * abs_rise_cos;
    double const  y_half_size      = vert_correct_1 + vert_correct_2;

    double const  eye_correct_1    = x_half_size    * abs_rise_cos;
    double const  eye_correct_2    = get_z_scale( ) * abs_rise_sin;
    double const  z_half_size      = eye_correct_1 + eye_correct_2;

    // Return values
    r_x_half_size = x_half_size;
    r_y_half_size = y_half_size;
    r_z_half_size = z_half_size;
}

// _______________________________________________________________________________________________

  void
  heat_widget_type::
setup_broadcast_scroll_steps_changed( ) const
{
  # define CONNECT_SCROLL_STEPS_CHANGED( NAME )        \
            d_assert( p_ ## NAME);                     \
            d_verify( connect(                         \
                p_ ## NAME, SIGNAL( has_changed( )),   \
                this, SLOT( update_scroll_steps( ))))  /* end macro */

    CONNECT_SCROLL_STEPS_CHANGED( z_axis_rotation_angle_);
    CONNECT_SCROLL_STEPS_CHANGED( rise_angle_           );
    CONNECT_SCROLL_STEPS_CHANGED( zoom_scale_           );
    CONNECT_SCROLL_STEPS_CHANGED( model_scale_          );
    CONNECT_SCROLL_STEPS_CHANGED( z_scale_              );

  # undef CONNECT_SCROLL_STEPS_CHANGED

    d_verify( connect(
        this, SIGNAL( viewport_changed( )),
        this, SLOT( update_scroll_steps( ))));
}

  /* slot */
  void
  heat_widget_type::
update_scroll_steps( )
{
    double x_half_size = 0;
    double y_half_size = 0;
    double z_half_size = 0; /* not used except as param */
    calc_model_half_sizes_in_rotated_model_coords( x_half_size, y_half_size, z_half_size);

    double x_ortho = 0;
    double y_ortho = 0;
    calc_orthos( x_ortho, y_ortho);

    double horz_page_step = (2 * x_ortho) / x_half_size;
    double vert_page_step = (2 * y_ortho) / y_half_size;

    // It's possible that the page step could be very very big. Check.
    d_assert( horz_page_step >= 0);
    if ( horz_page_step > 100000 ) {
        horz_page_step = 100000;
    }
    d_assert( vert_page_step >= 0);
    if ( vert_page_step > 100000 ) {
        vert_page_step = 100000;
    }

    p_eye_right_translation_->set_steps( 0, horz_page_step);
    p_eye_up_translation_   ->set_steps( 0, vert_page_step);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Save image to file
// _______________________________________________________________________________________________

  void
  heat_widget_type::
save_image_to_file
 (  QString const &  save_image_file_name
  , char    const *  p_format
 )
{
    // How to save the image to a file:
    //   1  bool is_ok = pixmap.save( filename, p_format_string, quality);
    //   2  bool is_ok = image .save( filename, p_format_string, quality);
    //   3  QImageWriter image_writer( file_name, format_q_byte_array);
    //      image_writer.setCompression(..)
    //      image_writer.setGamma(..)
    //      image_writer.setQuality(..)
    //      image_writer.setText(..)
    //      bool is_ok = image_writer.write( image);
    //      if ( ! is_ok ) {
    //        QString err_str = image_writer.errorString( );
    //        .. report error ..
    //      }
    //
    // We should either have a second dialog, or create a custom choose-image-file dialog,
    // that supports the following:
    //   Scale the image, or set the width/height
    //   Set quality
    //   Set compression
    //   Set gamma
    //   Set key-value string pairs like "Author" "John Doe"
    //   Set other properties? See image_writer.supportsOption(..).
    //   Allow alpha pixels? Allow stencil bitmap (for icons).
    //   Report error when something goes wrong.

    // Improve: When saving we should check for inconsistent types.
    // For example, if save_image_file_name ends in ".png" but the chosen format
    // was for JPG. We should confirm it's not a mistake with a dialog.
    // if ( save_image_file_name.endsWith( ".bmp", Qt::CaseInsensitive) ) ..

    d_assert( ! is_before_gl_init_);

    bool is_saved = false;

    QImage image ;
    d_assert( image.isNull( ));

    // First try to get a snapshot using FBO. This won't work if FBO isn't supported.
    image = get_snapshot__as_image__using_fbo( );
    if ( image.isNull( ) ) {
        image = get_snapshot__as_image__using_pbo( );
    }

    // Next try PBO. This also might not work since PBO is not part of OpenGL 1.1.
    if ( ! image.isNull( ) ) {
        is_saved = image.save( save_image_file_name, p_format);
    }

    // Finally try drawing directly to a bitmap. This should always work (in Windows at least),
    // but the drawing is very slow and very poor quality since it uses the GDI implementation
    // of OpenGL.
    if ( ! is_saved ) {
        QPixmap const pixmap = get_snapshot__as_pixmap__using_render( );
        if ( ! pixmap.isNull( ) ) {
            is_saved = pixmap.save( save_image_file_name, p_format);
        }
    }

    // Let the user know if it didn't work.
    if ( ! is_saved ) {
        QMessageBox::warning( 0,
          QObject::tr( "Image not saved"),
          QObject::tr( "Could not save image. Press OK to close."));
    }
}

// _______________________________________________________________________________________________

  QPixmap
  heat_widget_type::
get_snapshot__as_pixmap__using_fbo
 (  int  x_size_pixels  /* = 0 */
  , int  y_size_pixels  /* = 0 */
 )
  // QImage const image = pixmap.toImage( );
  // QPixmap const pm = QPixmap::fromImage( image);
  // You can paint either one with QPainter:
  //   painter.drawImage(..)
  //   painter.drawPixmap(..)
{
    return
        QPixmap::fromImage
         (  get_snapshot__as_image__using_fbo
             (  x_size_pixels
              , y_size_pixels
             )
         );
}

  QImage
  heat_widget_type::
get_snapshot__as_image__using_fbo
 (  int  x_size_pixels  /* = 0 */
  , int  y_size_pixels  /* = 0 */
 )
  // Returns image.isNull( ) if this fails.
  //
  // Returns image.isNull( ) if QGLFramebufferObject::hasOpenGLFramebufferObjects( )
  // (GL_EXT_framebuffer_object) or QGLFormat::OpenGL_Version_2_0( ) returns false.
  // We could also check ::glCheckFramebufferStatusEXT( ).
  //
  // We only use this if OpenGL 2.0 is supported because that allows us to have textures
  // with sizes that are not restricted to a power-of-2.
{
    // This should never recurse.
    d_assert( ! this->is_painting_off_screen( ));
    d_assert( ! this->is_drawing_to_fbo_);

    // Return value. Starts out null.
    QImage image ;
    d_assert( image.isNull( ));

    // Fix the sizes.
    if ( x_size_pixels <= 0 ) {
        x_size_pixels = this->width( );
    }
    if ( y_size_pixels <= 0 ) {
        y_size_pixels = this->height( );
    }

    // Make the snapshot, assuming the size is correct.
    if ( (x_size_pixels > 0) &&
         (y_size_pixels > 0) &&
         ::gl_env::env_type::is_gl_version_2_0( ) &&
         QGLFramebufferObject::hasOpenGLFramebufferObjects( ) )
    {
        d_assert( QGLFormat::openGLVersionFlags( ) & QGLFormat::OpenGL_Version_2_0);
        d_assert( QGLFormat::openGLVersionFlags( ) & QGLFormat::OpenGL_Version_1_1);
        d_assert( QGLFormat::openGLVersionFlags( ) & QGLFormat::OpenGL_Version_1_2);
        d_assert( QGLFormat::openGLVersionFlags( ) & QGLFormat::OpenGL_Version_1_3);
        d_assert( QGLFormat::openGLVersionFlags( ) & QGLFormat::OpenGL_Version_1_4);
        d_assert( QGLFormat::openGLVersionFlags( ) & QGLFormat::OpenGL_Version_1_5);

        // We can find out the biggest renderbuffer size allowed.
        // But we're going to be rendering to a texture so we probably should
        // check texture size instead.
        //   gl_env::global::get_int( GL_MAX_RENDERBUFFER_SIZE_EXT)
        //   gl_env::global::get_int( GL_MAX_TEXTURE_SIZE)
        //   gl_env::global::get_max_texture_size( )

        // A framebuffer object uses the same context as this widget.
        // You MUST have a current gl-context when you create the FBO.
        this->makeCurrent( );

        // You can only call this when you have an active gl-context:
        //d_assert( ::glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT) == GL_FRAMEBUFFER_COMPLETE_EXT);

        // Create the framebuffer object.
        // Ask for depth but no stencil buffers attached.
        // The color buffer will be a texture (GL_TEXTURE_2D), not a renderbuffer object.
        // Qt isn't set up for renderbuffers.
        //
        // Qt 4.6 provides a QGLFramebufferObjectFormat class thru which you can ask for
        // depth and stencil buffers and multisampling.
        QGLFramebufferObject draw_target( x_size_pixels, y_size_pixels, QGLFramebufferObject::Depth);
        if ( draw_target.isValid( ) ) {

            // Direct drawing away from the widget and into the FBO.
            if ( draw_target.bind( ) ) {
                d_assert( draw_target.isBound( ));

                d_assert( ! this->is_drawing_to_fbo_);
                this->is_drawing_to_fbo_ = true;
                try {
                    // Tell GL to draw over the entire FBO surface.
                    // (We don't have to do this if the size is the same as the widget. Which is usually is.)
                    gl_env::global::with_saved_server_attributes saved_viewport( GL_VIEWPORT_BIT);
                    gl_env::global::set_viewport( 0, 0, x_size_pixels, y_size_pixels);

                    // We can skip initialization because we are using the same context as the widget.
                    // this->initializeGL( );

                    // Draw the widget image into the FBO.
                    this->paintGL( );

                    // Make sure the drawing is finished and all the pixels are rendered.
                    ::glFlush( );
                }
                catch ( ... ) {
                    d_assert( this->is_drawing_to_fbo_);
                    this->is_drawing_to_fbo_ = false;
                    draw_target.release( );
                    throw;
                }
                d_assert( this->is_drawing_to_fbo_);
                this->is_drawing_to_fbo_ = false;

                // Tell GL to stop drawing to the FBO and go back to the widget.
                draw_target.release( );
            }

            // Copy PBO pixels to an image.
            image = draw_target.toImage( );
        }
    }
    return image;
}

// _______________________________________________________________________________________________

  QPixmap
  heat_widget_type::
get_snapshot__as_pixmap__using_pbo
 (  int  x_size_pixels  /* = 0 */
  , int  y_size_pixels  /* = 0 */
 )
{
    return
        QPixmap::fromImage
         (  get_snapshot__as_image__using_pbo
             (  x_size_pixels
              , y_size_pixels
             )
         );
}

  QImage
  heat_widget_type::
get_snapshot__as_image__using_pbo
 (  int  x_size_pixels  /* = 0 */
  , int  y_size_pixels  /* = 0 */
 )
  // Returns image.isNull( ) if this fails.
  //
  // Returns image.isNull( ) if QGLPixelBuffer::hasOpenGLPbuffers( ) returns false.
{
    // This should never recurse.
    d_assert( ! this->is_painting_off_screen( ));
    d_assert( ! this->is_drawing_to_pbo_);

    // Return value. Starts out null.
    QImage image ;
    d_assert( image.isNull( ));

    // Fix the sizes.
    if ( x_size_pixels <= 0 ) {
        x_size_pixels = this->width( );
    }
    if ( y_size_pixels <= 0 ) {
        y_size_pixels = this->height( );
    }

    // Make the snapshot, assuming the size is correct.
    if ( (x_size_pixels > 0) &&
         (y_size_pixels > 0) &&
         QGLPixelBuffer::hasOpenGLPbuffers( ) )
    {
        // Wrap the following in a try..catch.. so we can make sure to this->makeCurrent( )
        // after the PBO is dtor'd.
        try {
            // We can use format( ) from the widget. Double-buffering will be turned off
            // automatically when we're drawing to a PBO.
            // Share textures, display lists, and shaders with the widget.
            QGLPixelBuffer draw_target( x_size_pixels, y_size_pixels, this->format( ), this);
            if ( draw_target.isValid( ) ) {

                d_assert( ! this->is_drawing_to_pbo_);
                this->is_drawing_to_pbo_ = true;
                try {
                    // Tell GL to draw to the PBO, not to the widget window.
                    draw_target.makeCurrent( );

                    // Tell GL to draw over the entire PBO surface.
                    // Since this is a separate context you do not have to do this:
                    //   gl_env::global::with_saved_server_attributes saved_viewport( GL_VIEWPORT_BIT);
                    gl_env::global::set_viewport( 0, 0, x_size_pixels, y_size_pixels);

                    // Call our (overridden virtual) init method.
                    // It knows what to do when is_drawing_to_pbo_ is true.
                    this->initializeGL( );

                    // Draw the widget image into the PBO.
                    this->paintGL( );

                    // Make sure the drawing is finished and all the pixels are rendered.
                    ::glFlush( );

                    // Tell GL to stop drawing to the PBO. We could call this->makeCurrent( ) and direct
                    // drawing to the widget again. But we'll do that later, after the PBO is destroyed,
                    // because it appears that Qt (4.5) messes up the context when deleting a PBO.
                    // We could probably skip this since the QGLPixelBuffer probably does it for us.
                    draw_target.doneCurrent( );
                }
                catch ( ... ) {
                    d_assert( this->is_drawing_to_pbo_);
                    this->is_drawing_to_pbo_ = false;
                    throw;
                }
                d_assert( this->is_drawing_to_pbo_);
                this->is_drawing_to_pbo_ = false;

                // Copy PBO pixels to an image.
                image = draw_target.toImage( );
            }
        }
        catch ( ... ) {
            // Do this AFTER the QGLPixelBuffer is dtor'd.
            this->makeCurrent( );
            throw;
        }
        // Do this AFTER the QGLPixelBuffer is dtor'd.
        this->makeCurrent( );
        // It may seem we're going to a lot of trouble to make sure we set the context after
        // the PBO is dtor'd. But if we don't we have trouble in Qt (4.5).
        // It happens in PaintEvent(..), which gets called with no context set. This is how the
        // framework works, because it doesn't set the context until paintGL( ), but we need the
        // context set in PaintEvent(..) (when animating) because the animator moves things, which
        // causes scrollbars to resize, which needs to know how big the viewport is. Which calls
        // get_viewport_xy_size_ratio( ) which relys on having an active GL context. This is a
        // flawed design however. You can get rid of this extra this->makeCurrent( ) stuff when
        // it is fixed (and animation and scrollbars do not depend on having GL available).
        //
        // All this work to call this->makeCurrent at the end is not necessary now that we call
        // this->makeCurrent( ) in PaintEvent(..).
    }
    return image;
}

// _______________________________________________________________________________________________

  QImage
  heat_widget_type::
get_snapshot__as_image__using_render
 (  int  x_size_pixels  /* = 0 */
  , int  y_size_pixels  /* = 0 */
 )
{
    return
        get_snapshot__as_pixmap__using_render
         (  x_size_pixels
          , y_size_pixels
         ).toImage( );
}

  QPixmap
  heat_widget_type::
get_snapshot__as_pixmap__using_render
 (  int  x_size_pixels  /* = 0 */
  , int  y_size_pixels  /* = 0 */
 )
  // Returns a pixmap.isNull( ) if this fails.
  //
  // On windows this yields a lower quality image than one you'd get using PBO or
  // FBO objects. This draws into a system bitmap using a software (GDI OpenGL 1.1) driver.
  //
  // There are several ways to get the snapshot using render:
  //
  //   1  Use glReadPixels(..) to get image. Fist call glReadBuffer(..) to set src (FBO maybe).
  //      Reading from the front/back buffers is not 100% reliable.
  //      Create image with pixmap.loadFromData(..).
  //      We may have to vertically flip the image or change BGR to RGB.
  //      We may have to look at the hardware byte endian.
  //
  //   2  QImage image = p_gl_widget->grabFrameBuffer( );
  //      Not reliable unless drawing to an FBO or PBO.
  //      Probably equiv to method #1. Albeit easier.
  //
  //   3  QPixmap pixmap = QPixmap::grabWidget( p_widget);
  //
  //   4  QPixmap pixmap = p_gl_widget->renderPixmap( int w = 0, int h = 0);
  //      You can control the size.
  //      Doesn't rely on FBO or PBO support.
  //      Sets up a new context, so it's a little slow.
  //      The context draws into a memory bitmap and does not share with the widget context, so
  //      shaders and textures are complicated.
  //      You get a context that does NOT use hardware acceleration because you're not drawing
  //      into graphics memory. In Windows you get the software-only GDI context which supports
  //      almost nothing beyond OpenGL 1.1, and does a poor job in general.
  //
  // renderPixmap(..) creates a new GL context and temporarily replaces the existing
  // context. The new context does not share from the original context, so shader-programs
  // do not work.
  // This calls initializeGL( ) (with the new context), so we have to be aware of that.
  // The new context is deleted by the time renderPixmap() returns.
{
    // This should never recurse.
    d_assert( ! this->is_painting_off_screen( ));
    d_assert( ! is_drawing_to_pixmap_);

    // Return value. Starts out null.
    QPixmap pixmap ;
    d_assert( pixmap.isNull( ));

    // Fix the sizes.
    if ( x_size_pixels <= 0 ) {
        x_size_pixels = this->width( );
    }
    if ( y_size_pixels <= 0 ) {
        y_size_pixels = this->height( );
    }

    // Make the snapshot, assuming the size is correct.
    if ( (x_size_pixels > 0) && (y_size_pixels > 0) ) {

        // Set is_drawing_to_pixmap_ so drawing functions can adjust.
        d_assert( ! is_drawing_to_pixmap_);
        is_drawing_to_pixmap_ = true;
        try {
            // Create a pixmap and a gl-context that draws into the pixmap.
            // This calls resizeGL(..), initializeGL( ), and paintGL( ).
            // This does NOT go thru PaintEvent(..).
            pixmap = this->renderPixmap( x_size_pixels, y_size_pixels);

            // We can post a warning dialog now if we want, since we're no longer in paintGL( ).
            // If the shaders ran into an error while in paintGL( ), they did not post a dialog
            // but they remembered. We tell them to post the warning now.
            //
            // We cannot do this ahead of time because we can only test while the context exists.
            // However maybe we could test from inside initializeGL( ), although we'd have to be
            // careful to restore the bitmap context (make it current) when we were done.
            maybe_post_warning_dialog__blinn_phong__for_bitmap_context( );
        }
        catch ( ... ) {
            d_assert( is_drawing_to_pixmap_);
            is_drawing_to_pixmap_ = false;
            throw;
        }
        d_assert( is_drawing_to_pixmap_);
        is_drawing_to_pixmap_ = false;
    }

    // Return the pixmap. QPixmap has efficient (ref-count) copy semantics.
    return pixmap;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// OpenGL init
// _______________________________________________________________________________________________

  /* overridden virtual from QGLWidget */
  void
  heat_widget_type::
initializeGL( )
  //
  // initializeGL( ) is an overridden virtual, part of the QT framework.
  // It is run once when the GL context is first created, and once everytime the render context has to be rebuilt.
  // The rendering context is rebuilt when we change to/from fullscreen mode.
  //
  // The QT framework does not provide a corresponding un-init function.
  //
  // This sets up the GL context with default values.
{
    // Qt calls this method once when first initializing the window, and then EVERY TIME the window switches
    // from full-screen to windowed or back. It does not happen when we switch between minimize/restore/maximize
    // tho, only when we enter/exit full-screen.
    //
    // Qt also calls this to initialize a software context created during renderPixmap(..). This context
    // does NOT share with the widget context, so we have to re-create the textures and shader programs.
    //
    // And we call this method ourselves when we create a PBO context. The PBO context shares
    // with the original widget context. We can use the shader programs (and display lists) from the original
    // context without any trouble.
    // We can also use textures from the original context, although we have to bind them.

    d_assert( ! is_painting_); /* we may relax this assumption someday */
    d_assert( ! is_drawing_to_fbo_);
    if ( (! is_drawing_to_pixmap_) && (! is_drawing_to_pbo_) ) {
        // When we switch in/out of full-screen we get a new QGLContext (this QGLWidget doesn't change though).
        // The new QGLContext is created so it "shares" with the old context, and then the old context is deleted.

        // Init the global env the first time thru this method.
        if ( ! ::gl_env::global::is_init( ) ) {
            ::gl_env::global::init( true, true);
        } else {
            // If this is not the first time thru here, we need to detach instead of init.
            // This happens (in Windows) when we switch between full-screen-window and normal-window.
            get_lighting_rig( )->detach_from_gl( );
        }

        // Turn on the lights.
        get_lighting_rig( )->attach_to_gl( );
        get_lighting_rig( )->set_on( );

        // Create and bind 1D texture.
        get_isotherm_properties( )->setup_gl( );

        // We cannot draw until is_before_gl_init_ is marked false.
        // If it's already true then we are risking getting a paint event before GL is ready.
        //
        // is_before_gl_init_ is true when:
        //   This widget is first created.
        //   We switch to full-screen mode (Windows only).
        //   We switch out of full-screen mode (Windows only).
        d_assert( is_before_gl_init_);
        is_before_gl_init_ = false;

    } else {
        d_assert( ::gl_env::global::is_init( ));
        d_assert( ! is_before_gl_init_);

        // Turn on the lights.
        get_lighting_rig( )->copy_to_current_gl_context( );

        // Prepare 1D texture for isotherms.
        // The PBO context already has the texture available. It just needs to be bound.
        // The pixmap context does not share so we have to create a 2nd texture as well as bind it.
        //
        // The texture is not necessary, however, if we're not drawing isotherms.
        // Unless we start drawing z-colors using textures instead of calling glColor(..).
        if ( is_drawing_to_pbo_ ) {
            d_assert( ! is_drawing_to_pixmap_);
            get_isotherm_properties( )->setup_gl_in_shared_context( );
        } else {
            d_assert( is_drawing_to_pixmap_);
            get_isotherm_properties( )->setup_gl_in_clone_context( );
        }
    }

    // Set up lots of other GL properties.
    setup_drawing_properties( );
}

  /* static */
  void
  heat_widget_type::
setup_drawing_properties( )
{
    // The front-faces are wrapped counter-clockwise.
    ::gl_env::global::set_front_face_ccw( );

    // Call enable_blending( ) when drawing translucent.
    ::gl_env::global::set_blending_factors( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Tell GL to normalize all normal vectors so they are unit length.
    ::gl_env::global::enable( GL_NORMALIZE);

    // -- Setup the depth buffer --

    // GL_LESS is the default. GL_LEQUAL is also ok.
    ::gl_env::global::set_depth_test_fn( GL_LESS);

    // [0..1]. One is the maximum value and also the default.
    ::gl_env::global::set_depth_buffer_clear_value( 1.0);

    ::gl_env::global::enable_depth_test( );

    // -- Setup material colors --
    // Later, when we have more than one material, we will do this before we draw the materials, not
    // here during setup.

    // You shouldn't mess with material colors while tracking is enabled. Not even non-tracking materials.
    // Although it should be OK when overall lighting is disabled.
    ::gl_env::global::disable_material_color_tracking( );

    // Tell GL to automatically set material ambient and diffuse colors to be the same as the dropped color.
    ::gl_env::global::set_material_color_tracking( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Set the default material front/tack ambient colors.
    // This is unnecessary however because material ambient and diffuse colors track the dropped colors.
    // gl_env::global::setup_material_ambient_and_diffuse_color( color_type( 1.0f, 1.0f, 1.0f, 1.0f));

    // The specular color is set white. It does not track the drop color, so hilites are white.
    // To change this we could drop_material_specular_color(..) when we are dropping colors.
    ::gl_env::global::setup_material_specular_color( color_type( 1.0f, 1.0f, 1.0f));

    // Turn off emission color. This is unnecessary since it is already the default.
    // gl_env::global::setup_material_glow_color( color_type( 0, 0, 0));

    // We're done changing material colors.
    ::gl_env::global::enable_material_color_tracking( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// OpenGL viewport
// _______________________________________________________________________________________________

  /* overridden virtual from QGLWidget */
  void
  heat_widget_type::
resizeGL( int x_size_window, int y_size_window)
{
    d_assert( ! is_before_gl_init_);

    d_assert( width(  ) == x_size_window);
    d_assert( height( ) == y_size_window);

    // Avoid zeros so we don't have to worry about division.
    if ( 0 == x_size_window ) { x_size_window = 1; }
    if ( 0 == y_size_window ) { y_size_window = 1; }

    // We never see negative numbers here.
    d_assert( x_size_window > 0);
    d_assert( y_size_window > 0);

    // Set the viewport (and emit a signal) if the numbers really have changed.
    gl_env::int_type  x_size_viewport  = 0;
    gl_env::int_type  y_size_viewport  = 0;
    get_viewport_size( x_size_viewport, y_size_viewport);
    if (    (x_size_window != x_size_viewport) ||
            (y_size_window != y_size_viewport) )
    {
        // The window coords we are going to draw into.
        gl_env::global::set_viewport( 0, 0, x_size_window, y_size_window);

        // Tell the world (or anyone who's listening).
        emit viewport_changed( );
    }

    // no need to call supertype::resizeGL(..)
}

// _______________________________________________________________________________________________

  void
  heat_widget_type::
confirm_viewport( )
  //
  // Sometimes the viewport starts out too small even though the window was resized
  // all notifications were processed. I think it's related to double-buffering.
  // The first time we set the viewport it only gets set for the foreground buffer.
  // But it only happens the first time, even as we resize the window (confirmed
  // with a static counter).
{
    // If the current viewport does not agree with the window, fix it. But only if
    // the window is not zero-sized.
    int const  x_size_window  = width( );
    int const  y_size_window  = height( );
    if ( (x_size_window != 0) && (y_size_window != 0) ) {

        // Call resizeGL(..) instead of ::glViewport(..) directly because
        // resizeGL(..) does nothing if the viewport size is already set, and
        // resizeGL(..) emits the viewport_changed( ) signal.
        resizeGL( x_size_window, y_size_window);
    }
}

// _______________________________________________________________________________________________

  /* static */
  double
  heat_widget_type::
get_viewport_xy_size_ratio( ) //const because it's static
  //
  // Ratio of viewport-width / viewport-height, where width and height are pixels. Always > 0.
  //
  // This is < 1 for a tall narrow window (portrait).
  // This is > 1 for a fat short window (landscape).
  //
  // Special cases:
  //   Width or height are negative. Returns absolute value of ratio.
  //   Width and height are zero. Returns 1.
  //   Width is zero. Returns 1/height.
  //   Height is zero. Returns width.
{
    gl_env::int_type  x_size  = 0;
    gl_env::int_type  y_size  = 0;
    get_viewport_size( x_size, y_size);

    // Ratio of width to height. get_viewport_size(..) always returns positive integers,
    // even if the viewport has zero or negative size, so the division below is safe.
    d_assert( (x_size > 0) && (y_size > 0));
    return
        static_cast< double >( x_size) /
        static_cast< double >( y_size);
}

// _______________________________________________________________________________________________

  /* static */
  void
  heat_widget_type::
get_viewport_size( int & x_size, int & y_size) //const because it's static
{
    gl_env::int_type  xlo  = -1, ylo  = -1;
    gl_env::size_type xdel =  0, ydel =  0;
    gl_env::global::get_viewport( xlo, ylo, xdel, ydel);

    // For this application we always set the viewport origin to zero.
    d_assert( 0 == xlo);
    d_assert( 0 == ylo);

    // Change the sizes from size_type to int.
    // These are the return values.
    x_size = xdel;
    y_size = ydel;

    // Avoid zeros and negatives.
    if ( x_size == 0 ) {
        x_size = 1;
    } else
    if ( x_size < 0 ) {
        d_assert( false);
        x_size = - x_size;
    }
    if ( y_size == 0 ) {
        y_size = 1;
    } else
    if ( y_size < 0 ) {
        d_assert( false);
        y_size = - y_size;
    }
    d_assert( x_size > 0);
    d_assert( y_size > 0);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// OpenGL paint
// _______________________________________________________________________________________________

  /* overridden virtual from QGLWidget */
  void
  heat_widget_type::
paintGL( )
{
    d_assert( ! is_before_gl_init_);

    // paintGL( ) can recurse when you post a dialog while here.
    // For example, during paint we sometimes need to compile a shader, and if that fails we
    // may show a warning dialog. This can in turn generate paint messages for this widget.
    if ( is_painting_ ) {
        // Erase the color and depth buffer.
        clear_background( );
        return;
    }

    is_painting_ = true;
    try {
        // Sometimes the viewport starts out too small.
        confirm_viewport( );

        // Erase the color and depth buffer.
        clear_background( );

        // Setup either an ortho or perspective projection.
        double const z_distance_to_the_focus = set_the_projection( );
        d_assert( z_distance_to_the_focus >= 0);

        // Setup the "view" part of modelview. Move the camera/model. Scroll and rotate.
        set_the_lookat( z_distance_to_the_focus);

        // Paint the model in the model coord system.
        paint_model( );
    }
    catch ( ... ) {
        is_painting_ = false;
        throw;
    }
    d_assert( is_painting_);
    is_painting_ = false;

    // Clean up if we are drawing to a temporary GL context.
    // When we are drawing to a pixmap by calling renderPixmap( ), the Qt framework creates a pixmap,
    // binds it to a context, initializes, draws, and then deletes the context before returning.
    // This is the last chance we have to clean up before the context is gone.
    // This hilites a symmetry flaw in the Qt framework: there's an initializeGL(..) but no
    // uninitializeGL(..).
    if ( is_drawing_to_pixmap_ ) {
        // We probably don't have to free this texture here since the context is about to be destroyed
        // and is supposed to destroy the isotherm texture with it.
        // But it helps us keep track of resources on our end.
        get_isotherm_properties( )->teardown_gl_in_clone_context( ); /* unbind and delete 1D texture */

        // Release the shader program (not necessary except for sanity check):
        release_shader_program__blinn_phong__for_bitmap_context( );
    }
}

// _______________________________________________________________________________________________

  /* could be static */
  void
  heat_widget_type::
clear_background( ) const
{
    // Set the background color.
    // We don't have to do this before every draw like we're doing here.
    // Just whenever we change the background color.
    ::gl_env::global::set_color_buffer_clear_color( get_color_holder_background( )->get_color( ));

    // Clear the buffers and get ready for drawing.
    ::gl_env::global::clear_color_and_depth_buffer( );
}

// _______________________________________________________________________________________________

  bool
  heat_widget_type::
is_ortho_projection( ) const
{
    // Treat very small projection angles like ortho projections.
    // We could make this zero and everything works, as we get very close to zero we are moving
    // the model very far away to achieve such a small projection.
    double const min_projection_angle_degrees = 1.0; /* 1 degree arc, arbitrary */

    d_assert( get_perspective_angle__degrees( ) >= 0);
    return get_perspective_angle__degrees( ) <= min_projection_angle_degrees;
}

// _______________________________________________________________________________________________

  double
  heat_widget_type::
set_the_projection( ) const
  //
  // Setup the camera.
  // In other words, set the projection matrix to transform eye space coordinates into clip coordinates.
  //
  // Sets up either an ortho or perspective projection.
  // Returns the amount to move the model along the z-axis in order to put it in the frustum.
  //
  // The projection depends on these values. In the UI all these except the viewport are controlled by sliders.
  //   perspective_angle
  //   zoom_scale
  //   viewport_xy_size_ratio
  //
  //   model_scale (only if perspective (non-ortho) projection)
  //   z_scale     (only if perspective (non-ortho) projection)
{
    // Clear the projection stack matrix.
    // We don't have to do this every time if the projection hasn't changed (and usually it hasn't).
    ::gl_env::global::set_current_matrix_name( GL_PROJECTION);
    ::gl_env::global::reset_current_matrix( );

    // Find the sides of the frustum that we'll fit to the viewport.
    // These values incorporate the overall zoom, but are not affected by rotations.
    double x_ortho = 0;
    double y_ortho = 0;
    calc_orthos( x_ortho, y_ortho);
    d_assert( (x_ortho > 0) && (y_ortho > 0));

    // We do an ortho projection if the angle is zero. Otherwise we do a perspective projection.
    if ( is_ortho_projection( ) ) {
        // Set up a simple orthogonal projection, where the xy plane of the view maps to the viewport.
        // Set the z_clip limits large so none of the model ever gets clipped. But don't set them too
        // large because we then we lose precision in the depth buffer.
        double const z_mid_ortho = 512; /* big enough to avoid clipping, but not too big to lose depth precision */
        gl_env::global::setup_ortho(
            - x_ortho, x_ortho,
            - y_ortho, y_ortho,
            0, (2 * z_mid_ortho));

        // We could return 0 here because we don't have to move the model along the z-axis for an ortho projection.
        // We don't have to restrict our z-near-clipping value to > 0.
        // But we don't so that absolute postioning will work.
        //  o  Are we using positioned (as opposed to directional) lights? In that case the postions of the
        //     camera (at the origin), the lights, and the model all matter and we should move the model down
        //     the z-axis so it'll be out in front of the camera.
        //  o  Are any positioned lights attenuated?
        //  o  Are we using fog? If so the distance between the camera and the model elements matter.
        return z_mid_ortho;
    }

    // Setup a prespective (non-ortho) projection.
    d_assert( get_perspective_angle__degrees( ) > 0);
    d_assert( get_perspective_angle__degrees( ) < 180);

    // The fixed model radius is calculated for a model before asymmetric model-scaling is applied.
    // We also need to know how big the model actually is, after scaling, to avoid clipping planes.
    // We calculate it like this:
    //   sqrt( x_out^2 + y_out^2 + z_out^2) which is
    //   sqrt( model_scale^2 + model_scale^2 + z_scale^2)
    double const  model_scale            = get_model_scale( ); /* asymmetric x & y scale */
    double const  z_scale                = get_z_scale( );     /* asymmetric z scale */
    double const  adjusted_model_radius  = std::sqrt( (2 * model_scale * model_scale) + (z_scale * z_scale));
    d_assert( adjusted_model_radius > 0); /* approx std::sqrt( 3), although maybe 20 times more/less */

    // We use perspective_angle to calculate the focal plane. We choose the mid plane so that the model
    // remains about the same size as we change the perspective, even though this might result in a
    // negative near plane.
    double const  half_persp_angle_rads  = get_perspective_angle__radians( ) / 2;
    double const  z_mid_clipping         = y_ortho / std::tan( half_persp_angle_rads);
    d_assert( z_mid_clipping > 0);
    // z_mid_clipping can be as small as about 1/100 (full zoom-in, max perspective angle), or about
    // as big as 300 (full zoom-out, small-angle (almost ortho) perspective.

    // Make sure the far clipping plane is far enough away so nothing gets clipped.
    double const  z_far_clipping         = z_mid_clipping + adjusted_model_radius + adjusted_model_radius;
    d_assert( z_far_clipping > 0);
    // Setting z_far_clipping to (z_mid_clipping  + adjusted_model_radius) works fine, but I increase it
    // to make sure we don't clip the back-grid display which is slightly behind the model.
    // And if we ever draw stuff farther away in the background (we may want to draw the light sources
    // as lights), we will have to increase z_far_clipping even more.
    //
    // Redesign: Draw the background separate from the model in the foreground.
    // This will let us:
    //   Use tight near/far planes when drawing the model, and
    //   Use the near/far planes to implement slice clippling, which we now do with separate clipping planes.
    // Remember, the near/far planes affect the depths in the depth buffer as well as clip. The depth-buffer
    // cannot be used with different passes if the near/far planes vary. We'd have to draw the foreground
    // and then the background with a stencil, or draw the background first and then the foreground.

    // Calculate the (ideal) near clipping plane. This might be negative.
    double const  z_near_clipping_ideal  = z_mid_clipping - adjusted_model_radius;
    d_assert( z_far_clipping > z_near_clipping_ideal);

    // There are two things to watch for in z_near_clipping_ideal:
    //   It can be zero or negative, which is illegal. The final z_near_clipping value must be positive.
    //   We want to keep the ratio of (z_far_clipping / z_near_clipping) as near to 1 as possible.
    //     It will never be greater than one as long as z_near_clipping_ideal is positive.
    //     We loose about log2(z_far/z_near) precision in the fragment depth calculations (stored
    //     in the depth buffer), which would matter more if the depth buffer stored floats instead
    //     of doubles (ARB_depth_buffer_float). A double has about 53 bits of precision while a float
    //     has about 24.

    // We're willing to give up about 10-bits of precision.
    double const  max_far_near_ratio     = 1024;

    // First closest (smallest) allowable value for z_near_clipping.
    double const  z_near_clipping_min    = z_far_clipping / max_far_near_ratio;
    d_assert( (0 < z_near_clipping_min) && (z_near_clipping_min < z_far_clipping));

    // Make sure the z_near_clipping value we use is big enough. (This is a max(a,b) function.)
    double const  z_near_clipping_final  = (z_near_clipping_ideal > z_near_clipping_min) ?
                                            z_near_clipping_ideal : z_near_clipping_min;
    d_assert( (0 < z_near_clipping_final) && (z_near_clipping_final < z_far_clipping));

    // Project the xy-box on the z_mid plane to the z_near plane.
    double const  near_plane_reduction   = z_near_clipping_final / z_mid_clipping;
    // The original assert(..) is:
    //   d_assert( (0 < near_plane_reduction) && (near_plane_reduction < 1));
    // But when you zoom way in and max all the scales you can get this to around 4.8.
    // Should we allow it, or keep it at 1.0?
    d_assert( (0 < near_plane_reduction) && (near_plane_reduction < 5));

    double const  x_ortho_near_plane     = x_ortho * near_plane_reduction;
    double const  y_ortho_near_plane     = y_ortho * near_plane_reduction;
    d_assert( (x_ortho_near_plane > 0) && (y_ortho_near_plane > 0));

    // Setup the perspective matrix.
    gl_env::global::setup_frustum(
        - x_ortho_near_plane, x_ortho_near_plane,
        - y_ortho_near_plane, y_ortho_near_plane,
        z_near_clipping_final, z_far_clipping);

    // Return the distance down the z-axis where we are focused.
    // We will move the model (down) to be centered there.
    return z_mid_clipping;
}

// _______________________________________________________________________________________________

  void
  heat_widget_type::
set_the_lookat( double z_distance_to_the_focus) const
  //
  // Setup the gimbal and lighting.
  //
  // Moves the model around in front of a stationary camera.
  // This is the same as moving the camera around a stationary model, except all movements have
  // to be reversed.
  //
  // The model-view transform consists of the "model" part, composed with the "view" part.
  // This sets up the view part.
  //
  // Moves the camera from side-to-side and up-down to follow normal scrolling.
  // Rotates the model underneath the camera (or the camera around the model).
  //
  // This function does the same sort of things that gluLookAt(..) does.
  //
  // This also sets up lighting, slice clipping, and the isotherm/color texture.
{
    // Clear the modelview matrix.
    gl_env::global::set_current_matrix_name( GL_MODELVIEW);
    gl_env::global::reset_current_matrix( );

    // At this point the camera is at (0,0,0) looking down at (0,0,-1).
    // Overall zoom-scale and perspective-angle have been accounted for, so now is the time
    // to position lights that move with the camera.
    // We can tex-gen a 1D z-coord texture here to get camera-distance stripes (eye-so-therms?)

    // Move the model to simulate horz/vert scrolling:
    // The corrections here make the width/height of the model bigger when they are turned
    // so their corners are sticking out. They factor in model scaling but not overall zoom
    // and perspective, which we've already taken care of.
    double x_half_size = 0;
    double y_half_size = 0;
    double z_half_size = 0;
    calc_model_half_sizes_in_rotated_model_coords( x_half_size, y_half_size, z_half_size);

    // Move sideways for horizontal scrolling.
    // Also move up/down for vertical scrolling.
    // Also move the model down the z-axis so it's between the near/far clipping planes.
    gl_env::global::translate_current_matrix
       (  - get_eye_right_translation( ) * x_half_size
        , + get_eye_up_translation( )    * y_half_size
        , - z_distance_to_the_focus
       );

    // Set up clipping planes perpendicular to the eye.
    prepare_slice_clipping( z_half_size);

    // Spin the model 90 degrees along its x-axis so we're looking at it from the side.
    gl_env::global::rotate_current_matrix( 90.0, -1, 0, 0);
    // The eye is now on the model's y-axis, looking in the (0,1,0) direction at the origin
    // and the xz plane at y=0.

    // Place the lights in the scene.
    get_lighting_rig( )->place_in_eye_space(
        get_rise_angle__degrees( ), get_z_axis_rotation_angle__degrees( ), z_distance_to_the_focus);

    // Rotate the model by the rise angle.
    // Positive angle raises the front of the model. Negative angle raises the camera.
    gl_env::global::rotate_current_matrix( get_rise_angle__degrees( ), -1, 0, 0);

    // Rotate the model around the z-axis. Positive rotates the model ccw when looking from the top.
    gl_env::global::rotate_current_matrix( get_z_axis_rotation_angle__degrees( ), 0, 0, +1);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  void
  heat_widget_type::
paint_model( )
{
    gl_env::global::assert_current_matrix_name( GL_MODELVIEW);

    // Apply two independent model scales.
    // As z_scale goes up the model gets taller and the bumps are more pronounced.
    gl_env::global::scale_current_matrix( get_model_scale( ), get_model_scale( ), get_z_scale( ));

    // The sheet is the main display.
    draw_sheet( );

    // Draw the back grid without slicing. This is translucent so draw it last.
    maybe_draw_back_grid( );
}

// _______________________________________________________________________________________________

  void
  heat_widget_type::
prepare_slice_clipping( double z_half_size) const
  //
  // Setup the slicing clip planes in an unrotate unscaled modelview environment.
  // The planes will be perpendicular to the eye, in the xy-plane.
  //
  // Although this is fast and easy to calculate, we don't have to set this every time.
  // The dependencies are:
  //    Clipping center (distance)
  //    Clipping spread (distance)
  //    z_half_size (calc_model_half_sizes_in_rotated_model_coords):
  //      rise_angle
  //      z_scale
{
    if ( is_slice_clipping( ) ) {
        double const  center       = get_clipping_center( ) * z_half_size;
        double const  half_spread  = get_clipping_spread( ) * z_half_size;

        // Clip, parallel to the xy-plane, using clip planes 0 and 1.
        gl_env::global::set_clip_plane_equation( 0, 0, 0, +1, half_spread + center);
        gl_env::global::set_clip_plane_equation( 1, 0, 0, -1, half_spread - center);
    }

    // The following shows how to set up the clip planes in a rotated (but unscaled) modelview
    // transform. In the code above this would go right before the line:
    //   gl_env::global::scale_current_matrix( get_model_scale( ), get_model_scale( ), get_z_scale( ))
  # if 0
    // These are factors of the rotation and rise angles.
    double x_half_size = 0;
    double y_half_size = 0; /* not used */
    double z_half_size = 0;
    calc_model_half_sizes_in_normalized_device_coords( x_half_size, y_half_size, z_half_size);

    // Calculate the terms of the clipping equations. We rotate the clipping planes to keep them
    // parallel to the screen and perpendicular to the look-at vector of the camera.

    double const  rise_angle          = get_rise_angle__radians( );
    double const  sin_rise            = std::sin( rise_angle);
    double const  cos_rise            = std::cos( rise_angle);

    double const  sin_rise_corrected  = sin_rise / z_half_size;
    double const  cos_rise_corrected  = cos_rise / x_half_size;

    double const  z_rot_angle         = get_z_axis_rotation_angle__radians( );
    double const  sin_rot             = std::sin( z_rot_angle);
    double const  cos_rot             = std::cos( z_rot_angle);

    // The above values specify the orientation of the clip planes (parallel to the screen).
    // The center and half-spread values below determine the distance from the camera to the
    // clipping planes.
    //
    // The above factors (with corrections) fit the model drawing in a box such that the
    // farthest away parts are at +1 distance, the middle is at 0 distance, and the nearest
    // points are at -1 distance. Rotations are all corrected so the far point is always
    // at +1 distance whether box is parallel-face-on to the camera or pointing a vertex
    // at the camera.
    //
    // What we'll see:
    //   (center == +.9) and (half_width == .1) - Only shows the farthest-away part of the model
    //   (center == -.9) and (half_width == .1) - Only shows the nearest part of the model
    //   (center ==   0) and (half_width == .9) - Show almost all the model. Only the far and
    //                                            near corners are cut off.
    double const  center              = get_clipping_center( ); // -1..+1
    double const  half_spread         = get_clipping_spread( ); // units agree with center

    // Construct a plane-cut that discards the front of the picture and keeps part of the back.
    // If the last value (half_spread - center) is zero, the front half of the picture is cut away.
    // As the last value increases (gets more positive), the clip plane cuts off less and less, and
    // as it decreases (gets more negative) more is cut away.
    double const x_equation = cos_rise_corrected * sin_rot;
    double const y_equation = cos_rise_corrected * cos_rot;
    double const z_equation = sin_rise_corrected;
    gl_env::global::set_clip_plane_equation( 0, x_equation, y_equation, z_equation, half_spread - center);

    // Construct a plane-cut that discards the back of the picture and keeps part of the front.
    // As the last value increases (gets more positive), the clip plane cuts off less and less, and
    // as it decreases (gets more negative) more is cut away.
    //
    // The (ax + by + cz) part of equation_discard_back is the same as equation_discard_front except
    // all the components are negated. This describes a parallel plane, except the front face of the
    // plane is reversed.
    gl_env::global::set_clip_plane_equation( 1, - x_equation, - y_equation, - z_equation, half_spread + center);
  # endif
}

  struct
with_slice_clipping
{
    bool const is_slice_clipping;

    with_slice_clipping( bool is)
      : is_slice_clipping( is)
      { if ( is_slice_clipping ) {
          gl_env::global::enable_clip_plane( 0);
          gl_env::global::enable_clip_plane( 1);
      } }

    ~with_slice_clipping( )
      { if ( is_slice_clipping ) {
          gl_env::global::disable_clip_plane( 1);
          gl_env::global::disable_clip_plane( 0);
      } }
};

// _______________________________________________________________________________________________

  void
  heat_widget_type::
draw_sheet( )
{
    // Slice clipping applies to both the sheet and the bristles.
    with_slice_clipping sl( is_slice_clipping( ));

    draw_sheet_surface( );

    // Draw the bristles if requested.
    if ( ! get_bristle_properties( )->get_style( )->is_none( ) ) {
        gl_env::global::with_saved_server_attributes
            at( GL_ENABLE_BIT | GL_HINT_BIT | GL_LIGHTING_BIT | GL_LINE_BIT | GL_POINT_BIT );

        draw_sheet_bristles_type
         (  get_bristle_properties( )
          , get_model_scale( )
          , get_z_scale( )
         ).draw( p_sheet_control_->get_sheet_for_draw( ));
    }
}

  void
  heat_widget_type::
draw_sheet_surface( )
{
    // Don't draw the sheet surface if both sides are invisible.
    // We could also draw nothing if:
    //   The top face is invisible and we're looking straight down.
    //   The under face is invisible and we're looking straight up.
    if ( (! get_face_properties( )->get_top_face_style(   )->is_invisible( )) ||
         (! get_face_properties( )->get_under_face_style( )->is_invisible( )) )
    {
        gl_env::global::with_saved_server_attributes
            at( GL_ENABLE_BIT | GL_HINT_BIT | GL_LIGHTING_BIT | GL_POLYGON_BIT | GL_LINE_BIT | GL_POINT_BIT );

        set_gl_face_fill_and_cull( );
        { float const shininess_raw = get_face_properties( )->get_top_face_shininess( )->get_value( );
          float const shininess_inv = 128.0f - shininess_raw;
          float const shininess_use = shininess_inv * shininess_inv / 128.0f;
          set_gl_face_shininess( GL_FRONT, shininess_use);
        }
        { float const shininess_raw = get_face_properties( )->get_under_face_shininess( )->get_value( );
          float const shininess_inv = 128.0f - shininess_raw;
          float const shininess_use = shininess_inv * shininess_inv / 128.0f;
          set_gl_face_shininess( GL_BACK, shininess_use);
        }
        get_face_properties( )->get_antialias_style( )->set_gl
         (  get_face_properties( )->get_top_face_style(   )->is_dotted( ) ||
            get_face_properties( )->get_under_face_style( )->is_dotted( )
          , get_face_properties( )->get_top_face_style(   )->is_lined(  ) ||
            get_face_properties( )->get_under_face_style( )->is_lined(  )
         );

        if ( get_face_properties( )->is_smooth_shade( ) ) {
            // Maybe we should never set this if we are interpolating normals?
            gl_env::global::set_shade_model_smooth( ); // GL_LIGHTING_BIT
        } else {
            gl_env::global::set_shade_model_flat( ); // GL_LIGHTING_BIT
        }

        int const width = get_face_properties( )->get_pixel_width( )->get_value( );
        gl_env::global::set_line_width( width); // GL_LINE_BIT
        gl_env::global::set_point_size( width); // GL_POINT_BIT

        with_isotherm_texture it( get_isotherm_properties( ));

        if ( get_face_properties( )->get_is_interpolate( )->get_value( ) ) {
            // Zero the shader lights we don't use.
            // Improve: Do this by passing uniforms to the shader instead of hacking with
            // the built-in uniforms (the lights).
            with_shader_lighting
              shader_lighting_wrapper;

            // Activate the shader.
            with_blinn_phong_shading__wrapper_type
              shader_program_wrapper
               (  true  /* false will turn off the shader */
                , is_drawing_to_pixmap_  /* true will compile a new program */
                , get_isotherm_properties( )->is_showing( )
                , get_isotherm_properties( )->get_shader_z_factor( )
                , get_isotherm_properties( )->get_shader_offset( )
                , get_isotherm_properties( )->get_shader_color( )
               );

            draw_sheet_surface_type
             (  get_color_gradient_holder( )
              , get_face_properties( )->get_is_triangle_mesh( )->get_value( )
              , get_face_properties( )->get_shading_style( )->get_value( )
             ).draw( p_sheet_control_->get_sheet_for_draw( ));
        } else {
            draw_sheet_surface_type
             (  get_color_gradient_holder( )
              , get_face_properties( )->get_is_triangle_mesh( )->get_value( )
              , get_face_properties( )->get_shading_style( )->get_value( )
             ).draw( p_sheet_control_->get_sheet_for_draw( ));
        }
    }
}

  void
  heat_widget_type::
set_gl_face_fill_and_cull( )
{
    face_style_type *  p_top_style    = get_face_properties( )->get_top_face_style( );
    face_style_type *  p_under_style  = get_face_properties( )->get_under_face_style( );

    // One side must be visible.
    d_assert( (! p_top_style->is_invisible( )) || (! p_under_style->is_invisible( )));

    // Only enable culling if one side is invisible.
    if ( p_top_style->is_invisible( ) || p_under_style->is_invisible( ) ) {
        gl_env::global::enable_face_culling( ); // GL_ENABLE_BIT
    } else {
        gl_env::global::disable_face_culling( ); // GL_ENABLE_BIT
    }

    // Set polygon mode and cull face.
    if ( p_top_style->is_filled( ) ) {
        gl_env::global::set_polygon_mode( GL_FRONT, GL_FILL); // GL_POLYGON_BIT
    } else
    if ( p_top_style->is_lined( ) ) {
        gl_env::global::set_polygon_mode( GL_FRONT, GL_LINE); // GL_POLYGON_BIT
    } else
    if ( p_top_style->is_dotted( ) ) {
        gl_env::global::set_polygon_mode( GL_FRONT, GL_POINT); // GL_POLYGON_BIT
    } else
    /* p_top_style->is_invisible( ) */ {
        d_assert( p_top_style->is_invisible( ));
        d_assert( ! p_under_style->is_invisible( ));
        gl_env::global::set_cull_face( GL_FRONT); // GL_POLYGON_BIT
    }

    if ( p_under_style->is_filled( ) ) {
        gl_env::global::set_polygon_mode( GL_BACK, GL_FILL); // GL_POLYGON_BIT
    } else
    if ( p_under_style->is_lined( ) ) {
        gl_env::global::set_polygon_mode( GL_BACK, GL_LINE); // GL_POLYGON_BIT
    } else
    if ( p_under_style->is_dotted( ) ) {
        gl_env::global::set_polygon_mode( GL_BACK, GL_POINT); // GL_POLYGON_BIT
    } else
    /* p_under_style->is_invisible( ) */ {
        d_assert( p_under_style->is_invisible( ));
        d_assert( ! p_top_style->is_invisible( ));
        gl_env::global::set_cull_face( GL_BACK); // GL_POLYGON_BIT
    }
}

  void
  heat_widget_type::
set_gl_face_shininess( gl_env::enum_type face, float value)
{
    if ( value < 0 ) {
        value = 0.0f;
    } else
    if ( value > 128 ) {
        value = 128.0f;
    }
    gl_env::global::setup_material_shininess_128( face, value); // GL_LIGHTING_BIT
}

// _______________________________________________________________________________________________

  void
  heat_widget_type::
maybe_draw_back_grid( ) const
{
    // Draw the grid in the back of the model.
    // Improve: This could be nicer. Smoother, with tick marks and scale numbers.
    if ( is_showing_grid( ) ) {
        bool const is_ortho = is_ortho_projection( );
        if ( is_ortho && (
                (get_rise_angle__degrees( ) == -90) ||
                (get_rise_angle__degrees( ) == +90) ) )
        {
            // Don't show grid if no perspective and we are looking straight down or straight up.
        } else {
            gl_env::background::draw_grid(
              get_z_axis_rotation_angle__degrees( ), is_ortho, get_color_gradient_holder( ));
        }
    }
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// heat_widget.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
