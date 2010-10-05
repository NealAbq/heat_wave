// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// face_style.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "all.h"
# include "face_style.h"
# include <QtGui/QRadioButton>

// _______________________________________________________________________________________________

  /* ctor */
  face_style_type::
face_style_type( QObject * p_parent, value_type init_value /* = e_filled */)
  : holder_base_type ( p_parent   )
  , value_           ( init_value )
{ }

  void
  face_style_type::
set_value( value_type new_value)
{
    if ( value_ != new_value ) {
        if ( is_setting( ) ) {
            // This should not recurse with a new value while setting the value.
            d_assert( false);
        } else {
            while_setting_value_wrapper_type wrapper( this);
            value_ = new_value;

            // Emit the signals that update any attached radio buttons.
            // We probably only have to emit one signal here, the true signal.
            // This is usually not necessary. Maybe we should skip it if we can.
            emit has_changed__filled(    is_filled(    ));
            emit has_changed__lined(     is_lined(     ));
            emit has_changed__dotted(    is_dotted(    ));
            emit has_changed__invisible( is_invisible( ));

            wrapper.done_with_no_throws( );
        }
    }
}

  void
  face_style_type::
attach
 (  QRadioButton *  p_radio_filled
  , QRadioButton *  p_radio_lined
  , QRadioButton *  p_radio_dotted
  , QRadioButton *  p_radio_invisible
  , bool            init_value_from_holder
 )
{
    d_assert( p_radio_filled   );
    d_assert( p_radio_lined    );
    d_assert( p_radio_dotted   );
    d_assert( p_radio_invisible);

    d_assert( p_radio_filled != p_radio_lined    );
    d_assert( p_radio_filled != p_radio_dotted   );
    d_assert( p_radio_filled != p_radio_invisible);
    d_assert( p_radio_lined  != p_radio_dotted   );
    d_assert( p_radio_lined  != p_radio_invisible);
    d_assert( p_radio_dotted != p_radio_invisible);

    if ( init_value_from_holder ) {
        // Set the init state of the radio buttons from this folder.
        p_radio_filled   ->setChecked( is_filled(    ));
        p_radio_lined    ->setChecked( is_lined(     ));
        p_radio_dotted   ->setChecked( is_dotted(    ));
        p_radio_invisible->setChecked( is_invisible( ));
    } else {
        // Set the init value of this holder from the radio buttons.
        if ( p_radio_filled->isChecked( ) ) {
            d_assert( ! p_radio_lined    ->isChecked( ));
            d_assert( ! p_radio_dotted   ->isChecked( ));
            d_assert( ! p_radio_invisible->isChecked( ));
            set_filled( );
        } else
        if ( p_radio_lined->isChecked( ) ) {
            d_assert( ! p_radio_filled   ->isChecked( ));
            d_assert( ! p_radio_dotted   ->isChecked( ));
            d_assert( ! p_radio_invisible->isChecked( ));
            set_lined( );
        } else
        if ( p_radio_dotted->isChecked( ) ) {
            d_assert( ! p_radio_filled   ->isChecked( ));
            d_assert( ! p_radio_lined    ->isChecked( ));
            d_assert( ! p_radio_invisible->isChecked( ));
            set_dotted( );
        } else
        if ( p_radio_invisible->isChecked( ) ) {
            d_assert( ! p_radio_filled->isChecked( ));
            d_assert( ! p_radio_lined ->isChecked( ));
            d_assert( ! p_radio_dotted->isChecked( ));
            set_invisible( );
        } else {
            // None of the radio buttons are on! Choose arbitrarily.
            d_assert( false);
            set_filled( ); /* set the holder value */
            p_radio_filled->setChecked( true); /* check the radio button */
        }
    }

    // Tell the radio buttons to update the values here.
    d_verify( connect( p_radio_filled   , SIGNAL( toggled( bool)), this, SLOT( set_filled(    bool)) ));
    d_verify( connect( p_radio_lined    , SIGNAL( toggled( bool)), this, SLOT( set_lined(     bool)) ));
    d_verify( connect( p_radio_dotted   , SIGNAL( toggled( bool)), this, SLOT( set_dotted(    bool)) ));
    d_verify( connect( p_radio_invisible, SIGNAL( toggled( bool)), this, SLOT( set_invisible( bool)) ));

    // Propagate changes here back to the radio buttons.
    d_verify( connect( this, SIGNAL( has_changed__filled(    bool)), p_radio_filled   , SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__lined(     bool)), p_radio_lined    , SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__dotted(    bool)), p_radio_dotted   , SLOT( setChecked( bool)) ));
    d_verify( connect( this, SIGNAL( has_changed__invisible( bool)), p_radio_invisible, SLOT( setChecked( bool)) ));
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// face_style.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
