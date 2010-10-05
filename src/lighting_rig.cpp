// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// lighting_rig.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "all.h"
# include "lighting_rig.h"
# include "gl_env_global.h"

# include <QtGui/QAbstractButton>
# include <QtGui/QAbstractSlider>

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Initial values
// _______________________________________________________________________________________________

light_type::color_type const  black_color(     0   , 0   , 0   );
light_type::color_type const  dark_gray_color( 0.2f, 0.2f, 0.2f);
light_type::color_type const  lite_gray_color( 0.7f, 0.7f, 0.7f);
light_type::color_type const  white_color(     1.0f, 1.0f, 1.0f);

light_type::color_type const  dark_green_color( 0.2f, 0.3f, 0.2f);
light_type::color_type const  medi_green_color( 0.3f, 0.7f, 0.3f);
light_type::color_type const  lite_green_color( 0.4f, 1.0f, 0.4f);

light_type::color_type const  dark_orange_color( 0.3f, 0.3f, 0.2f);
light_type::color_type const  medi_orange_color( 0.7f, 0.7f, 0.3f);
light_type::color_type const  lite_orange_color( 1.0f, 1.0f, 0.4f);

light_type::color_type const  dark_blue_color( 0.2f, 0.2f, 0.3f);
light_type::color_type const  medi_blue_color( 0.3f, 0.3f, 0.7f);
light_type::color_type const  lite_blue_color( 0.4f, 0.4f, 1.0f);

QString const  choose_overall_ambient_str(  QObject::tr( "Choose an overall ambient color"));
QString const  choose_ambient_str(          QObject::tr( "Choose an ambient color"));
QString const  choose_diffuse_str(          QObject::tr( "Choose a diffuse color"));
QString const  choose_specular_str(         QObject::tr( "Choose a specular color"));

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// light_type
// _______________________________________________________________________________________________

  /* ctor */
  light_type::
light_type
 (  QObject *           p_parent
  , gl_type *           p_gl_light
  , bool                init_is_on
  , color_type const &  init_ambient_color
  , color_type const &  init_diffuse_color
  , color_type const &  init_specular_color
  , int                 init_spin_angle
  , int                 init_rise_angle
 )
  : QObject                   ( p_parent   )
  , p_gl_light_               ( p_gl_light )
  , p_is_on_                  ( new bool_holder(  this, init_is_on))
  , p_is_displaying_arrow_    ( new bool_holder(  this, false))
  , p_is_ambient_on_          ( new bool_holder(  this, false))
  , p_is_diffuse_on_          ( new bool_holder(  this, true))
  , p_is_specular_on_         ( new bool_holder(  this, true))
  , p_ambient_color_          ( new color_holder( this, init_ambient_color , choose_ambient_str ))
  , p_diffuse_color_          ( new color_holder( this, init_diffuse_color , choose_diffuse_str ))
  , p_specular_color_         ( new color_holder( this, init_specular_color, choose_specular_str))
  , p_spin_angle_             ( new angle_holder( this, init_spin_angle, angle_arc_type( angle_arc_type::e_360)))
  , p_is_move_with_eye__spin_ ( new bool_holder(  this, false))
  , p_rise_angle_             ( new angle_holder( this, init_rise_angle, angle_arc_type( angle_arc_type::e_180)))
  , p_is_move_with_eye__rise_ ( new bool_holder(  this, false))
{
    // The code assume the gl_light object is specified.
    d_assert( p_gl_light_);

    // Move all the values from the holders to the gl_light.
    move_from_holder_to_gl__on_off( )         ;
    move_from_holder_to_gl__arrow_display( )  ;
    move_from_holder_to_gl__ambient_color( )  ;
    move_from_holder_to_gl__diffuse_color( )  ;
    move_from_holder_to_gl__specular_color( ) ;
    move_from_holder_to_gl__spin_angle( )     ;
    move_from_holder_to_gl__rise_angle( )     ;

    // Connect all the holders to slots in this object.
    // These slots will copy changes in the holder to the gl_light object, and the
    // emit the has_changed( ) signal.

    // Watch the on/off button.
    d_assert( p_is_on_);
    d_verify( connect(
        p_is_on_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__on_off( ))
    ));

    // Watch the button that turns the arrow display on/off.
    d_assert( p_is_displaying_arrow_);
    d_verify( connect(
        p_is_displaying_arrow_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__arrow_display( ))
    ));

    // Watch the controls that affect ambient color.
    d_assert( p_is_ambient_on_);
    d_assert( p_ambient_color_);
    d_verify( connect(
        p_is_ambient_on_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__ambient_color( ))
    ));
    d_verify( connect(
        p_ambient_color_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__ambient_color( ))
    ));

    // Watch the controls that affect diffuse color.
    d_assert( p_is_diffuse_on_);
    d_assert( p_diffuse_color_);
    d_verify( connect(
        p_is_diffuse_on_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__diffuse_color( ))
    ));
    d_verify( connect(
        p_diffuse_color_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__diffuse_color( ))
    ));

    // Watch the controls that affect specular color.
    d_assert( p_is_specular_on_);
    d_assert( p_specular_color_);
    d_verify( connect(
        p_is_specular_on_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__specular_color( ))
    ));
    d_verify( connect(
        p_specular_color_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__specular_color( ))
    ));

    // Watch the controls that affect spin angle.
    d_assert( p_spin_angle_);
    d_assert( p_is_move_with_eye__spin_);
    d_verify( connect(
        p_spin_angle_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__spin_angle( ))
    ));
    d_verify( connect(
        p_is_move_with_eye__spin_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__spin_angle( ))
    ));

    // Watch the controls that affect rise angle.
    d_assert( p_rise_angle_);
    d_assert( p_is_move_with_eye__rise_);
    d_verify( connect(
        p_rise_angle_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__rise_angle( ))
    ));
    d_verify( connect(
        p_is_move_with_eye__rise_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__rise_angle( ))
    ));
}

// _______________________________________________________________________________________________
// move_from_holder_to_gl__.. methods

  void
  light_type::
move_from_holder_to_gl__on_off( )
{
    if ( p_is_on_->get_value( ) ) {
        p_gl_light_->request_on( );
    } else {
        p_gl_light_->set_off( );
    }
}

  void
  light_type::
move_from_holder_to_gl__arrow_display( )
{
    p_gl_light_->set_displaying_arrow(
        p_is_displaying_arrow_->get_value( ));
}

  void
  light_type::
move_from_holder_to_gl__ambient_color( )
{
    p_gl_light_->set_ambient_color(
        p_is_ambient_on_->get_value( ) ? p_ambient_color_->get_color( ) : black_color);
}

  void
  light_type::
move_from_holder_to_gl__diffuse_color( )
{
    p_gl_light_->set_diffuse_color(
        p_is_diffuse_on_->get_value( ) ? p_diffuse_color_->get_color( ) : black_color);
}

  void
  light_type::
move_from_holder_to_gl__specular_color( )
{
    p_gl_light_->set_specular_color(
        p_is_specular_on_->get_value( ) ? p_specular_color_->get_color( ) : black_color);
}

  void
  light_type::
move_from_holder_to_gl__spin_angle( )
{
    p_gl_light_->set_move_with_eye__spin( p_is_move_with_eye__spin_->get_value( ));
    p_gl_light_->set_spin__degrees( p_spin_angle_->get_angle__degrees( ));
}

  void
  light_type::
move_from_holder_to_gl__rise_angle( )
{
    p_gl_light_->set_move_with_eye__rise( p_is_move_with_eye__rise_->get_value( ));
    p_gl_light_->set_rise__degrees( p_rise_angle_->get_angle__degrees( ));
}

// _______________________________________________________________________________________________
// has_changed__.. slots

  /* slot */
  void
  light_type::
has_changed__on_off( )
{
    move_from_holder_to_gl__on_off( );
    emit has_changed( );
}

  /* slot */
  void
  light_type::
has_changed__arrow_display( )
{
    move_from_holder_to_gl__arrow_display( );
    emit has_changed( );
}

  /* slot */
  void
  light_type::
has_changed__ambient_color( )
{
    move_from_holder_to_gl__ambient_color( );
    emit has_changed( );
}

  /* slot */
  void
  light_type::
has_changed__diffuse_color( )
{
    move_from_holder_to_gl__diffuse_color( );
    emit has_changed( );
}

  /* slot */
  void
  light_type::
has_changed__specular_color( )
{
    move_from_holder_to_gl__specular_color( );
    emit has_changed( );
}

  /* slot */
  void
  light_type::
has_changed__spin_angle( )
{
    move_from_holder_to_gl__spin_angle( );
    emit has_changed( );
}

  /* slot */
  void
  light_type::
has_changed__rise_angle( )
{
    move_from_holder_to_gl__rise_angle( );
    emit has_changed( );
}

// _______________________________________________________________________________________________
// Attach methods
//
//   Attaching a holder to a control moves the value from the holder to the control.

  void
  light_type::
attach_on_off_button( QAbstractButton * p_check_button)
{
    p_is_on_->attach( p_check_button);
}

  void
  light_type::
attach_arrow_display_button( QAbstractButton * p_check_button)
{
    p_is_displaying_arrow_->attach( p_check_button);
}

  void
  light_type::
attach_ambient_color_ctrls( QAbstractButton * p_is_on_check_button, QAbstractButton * p_color_button)
{
    p_is_ambient_on_->attach( p_is_on_check_button);
    p_ambient_color_->attach( p_color_button);
}

  void
  light_type::
attach_diffuse_color_ctrls( QAbstractButton * p_is_on_check_button, QAbstractButton * p_color_button)
{
    p_is_diffuse_on_->attach( p_is_on_check_button);
    p_diffuse_color_->attach( p_color_button);
}

  void
  light_type::
attach_specular_color_ctrls( QAbstractButton * p_is_on_check_button, QAbstractButton * p_color_button)
{
    p_is_specular_on_->attach( p_is_on_check_button);
    p_specular_color_->attach( p_color_button);
}

  void
  light_type::
attach_spin_angle_ctrls( QAbstractButton * p_is_eye_move_check_button, QAbstractSlider * p_slider)
{
    p_is_move_with_eye__spin_->attach( p_is_eye_move_check_button);
    p_spin_angle_->attach( p_slider);
}

  void
  light_type::
attach_rise_angle_ctrls( QAbstractButton * p_is_eye_move_check_button, QAbstractSlider * p_slider)
{
    p_is_move_with_eye__rise_->attach( p_is_eye_move_check_button);
    p_rise_angle_->attach( p_slider);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// lighting_rig_type
// _______________________________________________________________________________________________

  /* ctor */
  lighting_rig_type::
lighting_rig_type( QObject * p_parent)
  : QObject          ( p_parent   )
  , gl_lighting_     ( )
  , light_0_         ( this, gl_lighting_.add_light( ), true , dark_green_color , lite_orange_color, lite_blue_color ,  40,  20)
  , light_1_         ( this, gl_lighting_.add_light( ), false, dark_orange_color, lite_blue_color  , lite_green_color, -30, -10)
  , p_is_ambient_on_ ( new bool_holder(  this, true))
  , p_ambient_color_ ( new color_holder( this, dark_blue_color, choose_overall_ambient_str))
{
    // Initialize the gl lighting_type object.
    move_from_holder_to_gl__ambient_color( );

    // Watch the controls that affect overall ambient color.
    d_assert( p_is_ambient_on_);
    d_assert( p_ambient_color_);
    d_verify( connect(
        p_is_ambient_on_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__ambient_color( ))
    ));
    d_verify( connect(
        p_ambient_color_, SIGNAL( has_changed( )),
        this, SLOT( has_changed__ambient_color( ))
    ));

    // Funnel has_changed( ) signals from the individual lights.
    d_verify( connect(
        & light_0_, SIGNAL( has_changed( )),
        this, SIGNAL( has_changed( ))
    ));
    d_verify( connect(
        & light_1_, SIGNAL( has_changed( )),
        this, SIGNAL( has_changed( ))
    ));
}

  void
  lighting_rig_type::
move_from_holder_to_gl__ambient_color( )
{
    gl_lighting_.set_global_ambient_color(
        p_is_ambient_on_->get_value( ) ? p_ambient_color_->get_color( ) : black_color);
}

  /* slot */
  void
  lighting_rig_type::
has_changed__ambient_color( )
{
    move_from_holder_to_gl__ambient_color( );
    emit has_changed( );
}

  void
  lighting_rig_type::
attach_ambient_color_ctrls__overall
 (  QAbstractButton *  p_is_on_check_button
  , QAbstractButton *  p_color_button
 )
{
    p_is_ambient_on_->attach( p_is_on_check_button);
    p_ambient_color_->attach( p_color_button);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  with_shader_lighting::
with_shader_lighting( )
  : was_disabled__overall_ ( ! gl_env::global::is_overall_lighting_enabled( ))
  , was_disabled__0_       ( ! gl_env::global::is_light_on( 0))
  , was_disabled__1_       ( ! gl_env::global::is_light_on( 1))
{
    if ( was_disabled__overall_ ) {
        gl_env::global::enable_overall_lighting( );
    }
    if ( was_disabled__0_ ) {
        gl_env::global::set_light_on( 0);
        gl_env::global::set_light_ambient_color( 0, black_color);
        gl_env::global::set_light_diffuse_color( 0, black_color);
        gl_env::global::set_light_specular_color( 0, black_color);
    }
    if ( was_disabled__1_ ) {
        gl_env::global::set_light_on( 1);
        gl_env::global::set_light_ambient_color( 1, black_color);
        gl_env::global::set_light_diffuse_color( 1, black_color);
        gl_env::global::set_light_specular_color( 1, black_color);
    }
}

  with_shader_lighting::
~with_shader_lighting( )
{
    if ( was_disabled__1_ ) {
        gl_env::global::set_light_off( 1);
    }
    if ( was_disabled__0_ ) {
        gl_env::global::set_light_off( 0);
    }
    if ( was_disabled__overall_ ) {
        gl_env::global::disable_overall_lighting( );
    }
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// lighting_rig.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
