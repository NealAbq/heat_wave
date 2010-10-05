// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// color_holder.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// This performs several functions:
//   Keep a color value which is used while drawing.
//
//   Posts the choose-color dialog to change the color. This keeps the title string.
//
//   Signals [emit has_changed( )] when the color is changed. You do not use this to change
//   the color button, but you do use this to update the drawing.
//
//   Keeps a list of buttons.
//     When the button is clicked, the choose-color dialog is shown.
//     When the color is changed, the background/foreground colors for the buttons are changed.
//
// This is somewhat ad-hoc. A better design would be to make the color holder independent of
// GUI and communicate via signals.
// We need a button subtype for choosing colors. It will have slots so that it can attach
// to this holder class using only signals.
// _______________________________________________________________________________________________

# include "all.h"
# include "color_holder.h"

# include <QtGui/QWidget>
# include <QtGui/QPalette>
# include <QtGui/QColorDialog>
# include <QtGui/QAbstractButton>

// _______________________________________________________________________________________________

namespace /* anonymous */ {

typedef unsigned char  ubyte_type;

  int /* unsigned byte */
get_component_as_int( float red_f)
{
    return gl_env::convert_fractional_fixed_point_to< ubyte_type >( red_f);
}

  float
get_component_from_int( int red_i)
{
    ubyte_type const red_ub = static_cast< ubyte_type >( red_i);
    d_assert( static_cast< int >( red_ub) == red_i);
    return gl_env::convert_fractional_fixed_point_to< float >( red_ub);
}

} /* end namespace anonymous */

// _______________________________________________________________________________________________

  /* constructor */
  color_holder::
color_holder
 (  QObject          *  p_parent
  , color_type const &  init_color
  , QString    const &  dlg_title
 )
  : holder_base_type ( p_parent)
  , color_           ( init_color)
  , qcolor_          ( )
  , last_good_hue_   ( 0)

  , dlg_title_       ( dlg_title)
  , buttons_         ( )

  , p_holder_r_      ( 0)
  , p_holder_g_      ( 0)
  , p_holder_b_      ( 0)
  , p_holder_h_      ( 0)
  , p_holder_s_      ( 0)
  , p_holder_v_      ( 0)
{
    ensure_color_is_valid( );
    move_color_to_qcolor( );
}

  /* constructor */
  color_holder::
color_holder
 (  QObject       *  p_parent
  , QColor  const &  init_color
  , QString const &  dlg_title
 )
  : holder_base_type ( p_parent)
  , color_           ( )
  , qcolor_          ( init_color)
  , last_good_hue_   ( 0)

  , dlg_title_       ( dlg_title)
  , buttons_         ( )

  , p_holder_r_      ( 0)
  , p_holder_g_      ( 0)
  , p_holder_b_      ( 0)
  , p_holder_h_      ( 0)
  , p_holder_s_      ( 0)
  , p_holder_v_      ( 0)
{
    ensure_qcolor_is_valid( );
    move_qcolor_to_color( );
}

// _______________________________________________________________________________________________

  /* static */
  bool
  color_holder::
is_color_valid( color_type const & c)
{
    return c.get_a( ) == 1.0;
}

  /* static */
  void
  color_holder::
ensure_color_is_valid( color_type & c)
{
    if ( ! is_color_valid( c) ) {
        c.set_a( 1.0);
    }
    d_assert( is_color_valid( c));
}

  bool
  color_holder::
is_color_valid( ) const
{
    return is_color_valid( color_);
}

  void
  color_holder::
ensure_color_is_valid( )
{
    ensure_color_is_valid( color_);
}

// _______________________________________________________________________________________________

  /* static */
  bool
  color_holder::
is_qcolor_valid( QColor const & qc)
{
    if ( qc.spec( ) == QColor::Hsv ) {
        d_assert( (-1 <= qc.hue(        )) && (qc.hue(        ) <= 359));
        d_assert( ( 0 <= qc.saturation( )) && (qc.saturation( ) <= 255));
        d_assert( ( 0 <= qc.value(      )) && (qc.value(      ) <= 255));
        return qc.alpha( ) == 255;
    }
    return false;
}

  /* static */
  void
  color_holder::
ensure_qcolor_is_valid( QColor & qc)
{
    if ( ! is_qcolor_valid( qc) ) {
        qc = qc.toHsv( );
        qc.setAlpha( 255);
    }
    d_assert( is_qcolor_valid( qc));
}

  bool
  color_holder::
is_qcolor_valid( ) const
{
    return is_qcolor_valid( qcolor_);
}

  void
  color_holder::
ensure_qcolor_is_valid( )
  //
  // Call this after any change to QColor.
{
    ensure_qcolor_is_valid( qcolor_);
    maybe_save_last_good_hue( qcolor_.hue( ));
}

  void
  color_holder::
maybe_save_last_good_hue( int const new_h)
  //
  // We save the hue whenever it is changed in qcolor, as long as it's not -1.
{
    if ( new_h < 0 ) {
        d_assert( new_h == -1);
    } else {
        d_assert( new_h <= 359);
        last_good_hue_ = new_h;
    }
}

// _______________________________________________________________________________________________

  void
  color_holder::
move_qcolor_to_color( )
{
    QRgb const rgb = get_qcolor( ).rgb( );
    // Assume the above call to rgb( ) did not change the QColor.
    d_assert( is_qcolor_valid( ));

    float const rf = get_component_from_int( qRed(   rgb));
    float const gf = get_component_from_int( qGreen( rgb));
    float const bf = get_component_from_int( qBlue(  rgb));
    color_.set( rf, gf, bf);
    d_assert( is_color_valid( ));
}

  void
  color_holder::
move_color_to_qcolor( )
{
    qcolor_.setRgb( get_r_0_255( ), get_g_0_255( ), get_b_0_255( ));
    // Make sure the QColor is HSV and not RGB.
    ensure_qcolor_is_valid( );
}

// _______________________________________________________________________________________________

  /* getter */
  int
  color_holder::
get_r_0_255( ) const
{
    return get_component_as_int( get_color( ).get_r( ));
}

  /* getter */
  int
  color_holder::
get_g_0_255( ) const
{
    return get_component_as_int( get_color( ).get_g( ));
}

  /* getter */
  int
  color_holder::
get_b_0_255( ) const
{
    return get_component_as_int( get_color( ).get_b( ));
}

// _______________________________________________________________________________________________

  /* slot */
  void
  color_holder::
set_r_0_255( int new_r)
{
    d_assert( (0 <= new_r) && (new_r <= 255));
    if ( get_r_0_255( ) != new_r ) {
        color_type new_color( get_color( ));
        new_color.set_r( get_component_from_int( new_r));
        d_assert( is_color_valid( new_color));
        set_color( new_color);
        d_assert( get_r_0_255( ) == new_r);
    }
}

  /* slot */
  void
  color_holder::
set_g_0_255( int new_g)
{
    d_assert( (0 <= new_g) && (new_g <= 255));
    if ( get_g_0_255( ) != new_g ) {
        color_type new_color( get_color( ));
        new_color.set_g( get_component_from_int( new_g));
        d_assert( is_color_valid( new_color));
        set_color( new_color);
        d_assert( get_g_0_255( ) == new_g);
    }
}

  /* slot */
  void
  color_holder::
set_b_0_255( int new_b)
{
    d_assert( (0 <= new_b) && (new_b <= 255));
    if ( get_b_0_255( ) != new_b ) {
        color_type new_color( get_color( ));
        new_color.set_b( get_component_from_int( new_b));
        d_assert( is_color_valid( new_color));
        set_color( new_color);
        d_assert( get_b_0_255( ) == new_b);
    }
}

// _______________________________________________________________________________________________

  /* getter */
  int
  color_holder::
get_h_0_359( ) const
{
    int hue = get_qcolor( ).hue( );
    if ( hue < 0 ) {
        d_assert( hue == -1);
        hue = last_good_hue_;
    }
    d_assert( (0 <= hue) && (hue <= 359));
    return hue;
}

  /* getter */
  int
  color_holder::
get_s_0_255( ) const
{
    int const saturation = get_qcolor( ).saturation( );
    d_assert( (0 <= saturation) && (saturation <= 255));
    return saturation;
}

  /* getter */
  int
  color_holder::
get_v_0_255( ) const
{
    int const value = get_qcolor( ).value( );
    d_assert( (0 <= value) && (value <= 255));
    return value;
}

// _______________________________________________________________________________________________

  /* slot */
  void
  color_holder::
set_h_0_359( int new_h)
{
    d_assert( (0 <= new_h) && (new_h <= 359));
    if ( get_h_0_359( ) != new_h ) {
        QColor new_qcolor( QColor::fromHsv( new_h, get_s_0_255( ), get_v_0_255( )));
        d_assert( is_qcolor_valid( new_qcolor));

        // It's possible the hue in new_qcolor is -1, and we've lost the new_h value.
        // Make sure we save it in last_good_hue_.
        if ( new_qcolor.hue( ) != -1 ) {
            // This shouldn't be necessary.
            d_assert( new_h == new_qcolor.hue( ));
            new_h = new_qcolor.hue( );
        }
        maybe_save_last_good_hue( new_h);

        // Propagate the color change.
        set_qcolor( new_qcolor);
        d_assert( get_h_0_359( ) == new_h);
    }
}

  /* slot */
  void
  color_holder::
set_s_0_255( int new_s)
{
    d_assert( (0 <= new_s) && (new_s <= 255));
    if ( get_s_0_255( ) != new_s ) {
        QColor const new_qcolor = QColor::fromHsv( get_h_0_359( ), new_s, get_v_0_255( ));
        d_assert( is_qcolor_valid( new_qcolor));
        set_qcolor( new_qcolor);
        d_assert( get_s_0_255( ) == new_s);
    }
}

  /* slot */
  void
  color_holder::
set_v_0_255( int new_v)
{
    d_assert( (0 <= new_v) && (new_v <= 255));
    if ( get_v_0_255( ) != new_v ) {
        QColor const new_qcolor = QColor::fromHsv( get_h_0_359( ), get_s_0_255( ), new_v);
        d_assert( is_qcolor_valid( new_qcolor));
        set_qcolor( new_qcolor);
        d_assert( get_v_0_255( ) == new_v);
    }
}

// _______________________________________________________________________________________________

  int_range_steps_holder *
  color_holder::
get_holder__r( )
{
    if ( ! p_holder_r_ ) {
        p_holder_r_ = new int_range_steps_holder( this, get_r_0_255( ), 0, 255);
        d_verify( connect( this, SIGNAL( has_changed__r_0_255( int)), p_holder_r_, SLOT( set_value( int)) ));
        d_verify( connect( p_holder_r_, SIGNAL( has_changed( int)), this, SLOT( set_r_0_255( int)) ));
    }
    d_assert( p_holder_r_);
    return p_holder_r_ ;
}

  int_range_steps_holder *
  color_holder::
get_holder__g( )
{
    if ( ! p_holder_g_ ) {
        p_holder_g_ = new int_range_steps_holder( this, get_g_0_255( ), 0, 255);
        d_verify( connect( this, SIGNAL( has_changed__g_0_255( int)), p_holder_g_, SLOT( set_value( int)) ));
        d_verify( connect( p_holder_g_, SIGNAL( has_changed( int)), this, SLOT( set_g_0_255( int)) ));
    }
    d_assert( p_holder_g_);
    return p_holder_g_ ;
}

  int_range_steps_holder *
  color_holder::
get_holder__b( )
{
    if ( ! p_holder_b_ ) {
        p_holder_b_ = new int_range_steps_holder( this, get_b_0_255( ), 0, 255);
        d_verify( connect( this, SIGNAL( has_changed__b_0_255( int)), p_holder_b_, SLOT( set_value( int)) ));
        d_verify( connect( p_holder_b_, SIGNAL( has_changed( int)), this, SLOT( set_b_0_255( int)) ));
    }
    d_assert( p_holder_b_);
    return p_holder_b_ ;
}

// _______________________________________________________________________________________________

  int_range_steps_holder *
  color_holder::
get_holder__h( )
{
    if ( ! p_holder_h_ ) {
        p_holder_h_ = new int_range_steps_holder( this, get_h_0_359( ), 0, 359);
        d_verify( connect( this, SIGNAL( has_changed__h_0_359( int)), p_holder_h_, SLOT( set_value( int)) ));
        d_verify( connect( p_holder_h_, SIGNAL( has_changed( int)), this, SLOT( set_h_0_359( int)) ));
    }
    d_assert( p_holder_h_);
    return p_holder_h_ ;
}

  int_range_steps_holder *
  color_holder::
get_holder__s( )
{
    if ( ! p_holder_s_ ) {
        p_holder_s_ = new int_range_steps_holder( this, get_s_0_255( ), 0, 255);
        d_verify( connect( this, SIGNAL( has_changed__s_0_255( int)), p_holder_s_, SLOT( set_value( int)) ));
        d_verify( connect( p_holder_s_, SIGNAL( has_changed( int)), this, SLOT( set_s_0_255( int)) ));
    }
    d_assert( p_holder_s_);
    return p_holder_s_ ;
}

  int_range_steps_holder *
  color_holder::
get_holder__v( )
{
    if ( ! p_holder_v_ ) {
        p_holder_v_ = new int_range_steps_holder( this, get_v_0_255( ), 0, 255);
        d_verify( connect( this, SIGNAL( has_changed__v_0_255( int)), p_holder_v_, SLOT( set_value( int)) ));
        d_verify( connect( p_holder_v_, SIGNAL( has_changed( int)), this, SLOT( set_v_0_255( int)) ));
    }
    d_assert( p_holder_v_);
    return p_holder_v_ ;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* could be a slot if needed */
  void
  color_holder::
set_qcolor( QColor new_qcolor)
{
    // Make sure the color is HSV and alpha is 255.
    ensure_qcolor_is_valid( new_qcolor);

    // Only do something if we have to.
    // This should work even if hue is -1 (==> saturation==0) in the qcolor.
    // Although I suppose this is wrong if they are both EQ except one hue is -1 and not the other.
    if ( get_qcolor( ) == new_qcolor ) {
        /* no change, do nothing */
    } else {
        // One of the HSV components should be different.
        d_assert(
            (get_qcolor( ).hue(        ) != new_qcolor.hue(        )) ||
            (get_qcolor( ).saturation( ) != new_qcolor.saturation( )) ||
            (get_qcolor( ).value(      ) != new_qcolor.value(      )) );
        // If the SV parts are the same, then hue must be different.
        // It should not be possible that one is -1 and the other not.
      # if 0
        // This fails. get_qcolor( ) was {-1, 0, 0} while new_color was {200, 0, 0}.
        d_assert(
            implies(
                ( (get_qcolor( ).saturation( ) == new_qcolor.saturation( )) &&
                  (get_qcolor( ).value(      ) == new_qcolor.value(      )) ),
                ( (get_qcolor( ).hue( ) >= 0) &&
                  (new_qcolor   .hue( ) >= 0) )
         ));
       # endif

        // If the only thing different is the new hue, and it is -1, then the color will not
        // actually change. This is the same test as the above (commented out) assert(..).
        if ( (get_qcolor( ).saturation( ) == new_qcolor.saturation( )) &&
             (get_qcolor( ).value(      ) == new_qcolor.value(      )) &&
             ( (new_qcolor.hue( ) < 0) ||
               ( (get_qcolor( ).hue( ) < 0) &&
                 (new_qcolor.hue( ) == last_good_hue_)
           ) ) )
        {
            // We want to discard this color. It will not change the state of this color holder.
            d_assert( (new_qcolor.hue( ) < 0) || (new_qcolor.hue( ) == get_h_0_359( )));
        } else
        if ( is_setting( ) ) {
            // This should not happen.
            d_assert( false);
        } else
        {
            // This wrapper dtor emits the has_changed( ) signal.
            while_setting_value_wrapper_type wrapper( this);

            // Remember all the old values.
            int const old_r = get_r_0_255( );
            int const old_g = get_g_0_255( );
            int const old_b = get_b_0_255( );
            int const old_h = get_h_0_359( );
            int const old_s = get_s_0_255( );
            int const old_v = get_v_0_255( );

            // Set the new values.
            qcolor_ = new_qcolor;
            ensure_qcolor_is_valid( );
            move_qcolor_to_color( );

            // Get all the new values.
            int const new_r = get_r_0_255( );
            int const new_g = get_g_0_255( );
            int const new_b = get_b_0_255( );
            int const new_h = get_h_0_359( );
            int const new_s = get_s_0_255( );
            int const new_v = get_v_0_255( );

            // Which values have changed.
            bool const is_changed_r = (old_r != new_r);
            bool const is_changed_g = (old_g != new_g);
            bool const is_changed_b = (old_b != new_b);
            bool const is_changed_h = (old_h != new_h);
            bool const is_changed_s = (old_s != new_s);
            bool const is_changed_v = (old_v != new_v);

            // Something should be changing.
            d_assert(
                is_changed_r || is_changed_g || is_changed_b ||
                is_changed_h || is_changed_s || is_changed_v );

            // These changes will bounce back.
            if ( is_changed_r ) { emit has_changed__r_0_255( new_r); }
            if ( is_changed_g ) { emit has_changed__g_0_255( new_g); }
            if ( is_changed_b ) { emit has_changed__b_0_255( new_b); }
            if ( is_changed_h ) { emit has_changed__h_0_359( new_h); }
            if ( is_changed_s ) { emit has_changed__s_0_255( new_s); }
            if ( is_changed_v ) { emit has_changed__v_0_255( new_v); }

            // Prepare wrapper for dtor.
            wrapper.done_with_no_throws( );
        }
    }
}

// _______________________________________________________________________________________________

  /* not a slot because color_type is not registered */
  void
  color_holder::
set_color( color_type new_color)
{
    // This emits the has_changed( ) signal, but not the (non-existent) has_changed( color) signal.
    // This code is based on: set_value_base__no_value_signal( this, color_, new_color).

    // Make sure alpha is 255.
    ensure_color_is_valid( new_color);

    // Only do something if we have to.
    if ( color_ == new_color ) {
        /* no change, do nothing */
    } else
    if ( is_setting( ) ) {
        // This should not happen.
        d_assert( false);
    } else
    {
        // This wrapper dtor emits the has_changed( ) signal.
        while_setting_value_wrapper_type wrapper( this);

        // Remember all the old values.
        int const old_r = get_r_0_255( );
        int const old_g = get_g_0_255( );
        int const old_b = get_b_0_255( );
        int const old_h = get_h_0_359( );
        int const old_s = get_s_0_255( );
        int const old_v = get_v_0_255( );

        // Set the new values.
        color_ = new_color;
        ensure_color_is_valid( );
        move_color_to_qcolor( );

        // Get all the new values.
        int const new_r = get_r_0_255( );
        int const new_g = get_g_0_255( );
        int const new_b = get_b_0_255( );
        int const new_h = get_h_0_359( );
        int const new_s = get_s_0_255( );
        int const new_v = get_v_0_255( );

        // Which values have changed.
        bool const is_changed_r = (old_r != new_r);
        bool const is_changed_g = (old_g != new_g);
        bool const is_changed_b = (old_b != new_b);
        bool const is_changed_h = (old_h != new_h);
        bool const is_changed_s = (old_s != new_s);
        bool const is_changed_v = (old_v != new_v);

        // Something should be changing.
        d_assert(
            is_changed_r || is_changed_g || is_changed_b ||
            is_changed_h || is_changed_s || is_changed_v );

        // These changes will bounce back.
        if ( is_changed_r ) { emit has_changed__r_0_255( new_r); }
        if ( is_changed_g ) { emit has_changed__g_0_255( new_g); }
        if ( is_changed_b ) { emit has_changed__b_0_255( new_b); }
        if ( is_changed_h ) { emit has_changed__h_0_359( new_h); }
        if ( is_changed_s ) { emit has_changed__s_0_255( new_s); }
        if ( is_changed_v ) { emit has_changed__v_0_255( new_v); }

        // Prepare wrapper for dtor.
        wrapper.done_with_no_throws( );
    }
}

// Some of the above should probably be slots, and we could have more has_changed(..) signals,
// but that would mean registering the types with Qt, and not with the typedef names.
// And probably not with template names either, although I'm not sure of this.
//
//   qRegisterMetaType( "gl_env::rgba_type< gl_env::float_type >"); // probably won't work
//   qRegisterMetaType( "global_color_type"); // probably need to do this
//   // Don't need Q_DECLARE_METATYPE(..) unless we're using variants and scripting.

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* public member */
  void
  color_holder::
attach( QAbstractButton * p_button)
{
    d_assert( p_button);

    // Keep a pointer to the button object.
    // This not a very good design. This class should be independent of UI objects like buttons
    // and should instead communicate through signals. The best solution is probably to have a
    // color-button class that changes its background color when it gets a has_changed signal
    // from this class. But for now we'll just keep pointers to standard buttons.
    d_assert( ! buttons_.contains( p_button));
    buttons_.push_front( p_button);
    d_assert( buttons_.contains( p_button));

    // Let us know when the button is dtor'ed so we can remove it from the list.
    d_verify( connect( p_button, SIGNAL( destroyed( QObject *)), this, SLOT( before_dtor( QObject *)) ));

    // Setup the button.
    // This sets the color for ALL the attached buttons, which is overkill, but not wrong.
    // But if we start attaching a bunch of buttons to each color_holder we may want to
    // fix this to only set the button color for p_button.
    set_button_color( );

    // Attach the button so we know when it's clicked.
    d_verify( connect( p_button, SIGNAL( clicked( )), this, SLOT( choose_color_dlg( )) ));

    // If someone changes this color from the outside, make sure to update the button.
    // We could (should?) replace this with a loop in set_value(..).
    d_verify( connect( this, SIGNAL( has_changed( )), this, SLOT( set_button_color( )) ));
}

// _______________________________________________________________________________________________

  /* public member */
  void
  color_holder::
detach( QAbstractButton * p_button)
{
    d_assert( p_button);
    d_assert( buttons_.contains( p_button));
    d_verify( disconnect( p_button, SIGNAL( destroyed( QObject *)), this, SLOT( before_dtor( QObject *)) ));
    d_verify( 1 == buttons_.removeAll( p_button));
    d_assert( ! buttons_.contains( p_button));
}

// _______________________________________________________________________________________________

  /* protected slot */
  void
  color_holder::
before_dtor( QObject * p_attached_object_about_to_be_destroyed)
{
    // This cast doesn't work here if you ask for QAbstractButton. The cast works elsewhere,
    // just not here. This is during shutdown, so maybe some class info is already destroyed?
    // I should look at how QPointer<..> is implemented.
    // Bug in Qt, Qt bug.
    QWidget * const
        p_attached_button_about_to_be_destroyed =
            qobject_cast< QWidget * >( p_attached_object_about_to_be_destroyed);
    d_assert( p_attached_button_about_to_be_destroyed);
    d_verify( 1 == buttons_.removeAll( p_attached_button_about_to_be_destroyed));
}

// _______________________________________________________________________________________________

  /* public slot */
  void
  color_holder::
choose_color_dlg( )
  //
  // Returns true if color is changed.
{
    // We could try to find a widget to make the owner of the modal color dialog.
    // But it's not necessary. The owner window gets some messages from the dialog
    // but nothing that we're relying on.
    // QWidget * const p_widget = qobject_cast< QWidget * >( parent( ));

    QColor const  init_color   = get_qcolor( );
    QColor const  final_color  = QColorDialog::getColor( init_color, 0 /* p_widget */, dlg_title_);
    if ( final_color.isValid( ) /* && (final_color != init_color) */ ) {
        set_qcolor( final_color);
    }
}

// _______________________________________________________________________________________________

  /* protected slot */
  void
  color_holder::
set_button_color( )
  //
  // There's a color-picking button at Qt Solutions:
  //   http://www.qtsoftware.com/products/appdev/add-on-products/catalog/4/Widgets/qtcolorpicker/
  //
  // Notes about setting the background-color for a button.
  //
  // This works, as long as none of the ancestor windows have any colors set:
  //   p_button->setPalette( QPalette( color));
  // This is equivalent:
  //   QPalette palette;
  //   palette.setColor( p_button->backgroundRole( ), color);
  //   p_button->setPalette( palette);
  //
  // These do not work (or maybe they do if no ancestor sets style):
  //   p_button->setBackgroundRole( QPalette::Window);
  //   p_button->autoFillBackground( );
  //
  //   QBrush brush( Qt::red);
  //   QPalette palette;
  //   palette.setBrush( p_button->backgroundRole( ), brush);
  //   p_button->setPalette( palette);
  //
  // These are from Qt3 and do not compile:
  //   p_button->setPaletteBackgroundColor ( color);
  //   p_button->setBackgroundColor( color);
  //
  // Haven't explored:
  //   p_button->setStyle( QStyle*)
  // But it doesn't look like a good match.
  //
  // From the Qt code:
  //   setBackgroundRole(QPalette::ColorRole)
  //
  //   virtual void setBackgroundColor(const QColor &color)
  //      { setData(Qt::BackgroundColorRole, color); }
  //
  //   Qt::BackgroundColorRole is enum ItemDataRole
  //     Maybe it's only for list items?
{
    QColor const back_color = get_qcolor( );
    QColor const text_color =
        ((255 - (
            (back_color.red(   ) * 0.299) +
            (back_color.green( ) * 0.587) +
            (back_color.blue(  ) * 0.114) )) < 105) ? Qt::black : Qt::white;
    QString const style_template( QString::fromUtf8( "QPushButton { background-color : %1; color : %2; }"));
    QString const style_string( style_template.arg( back_color.name( )).arg( text_color.name( )));

    // The calculation for text_color above comes from:
    //   http://www.codeproject.com/cs/media/IdealTextColor.asp
    // Also see:
    //   http://stackoverflow.com/questions/282938/qt-show-result-of-color-selection

    // Loop thru all the attached buttons.
    list_type::iterator        iter        = buttons_.begin( );
    list_type::iterator const  iter_limit  = buttons_.end( );
    for ( ; iter != iter_limit ; ++ iter ) {
        QWidget * p_button = *iter;
        d_assert( p_button);
        p_button->setStyleSheet( style_string);
    }
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// color_holder.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
