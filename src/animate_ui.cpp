// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// animate_ui.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Improvement:
//   Add a label to the UI reporting how many animators are active.
//   Add a button to turn them all off? Although you can always just stop the animation.
//   Change the background color of the tab is animation is active?
//     And change the color in the buttons?
//
//   Allow blank list items? This is the beginning of tree structure.
//   Allow full tree structure.
//     Some tree nodes are attached to bool holders and toggle on/off.
//     Group boxes in the UI can also toggle on/off.
//     The tree structure would match the tree of the UI, so it should probably be
//       mixed with the UI. We could provide a UI filter so the user could ask
//       "when navigating this menu hierarchy, only show animation UI".
//
//     The label that shows the name of the bound animator would need to show all the
//       ancestors to the bound animator. So it would have to be several lines long,
//       and vary in length.
//
//     We probably should not allow the bound animator to be hidden by closed tree nodes.
//
//     There are really two trees:
//       The UI controls
//       The DOM (data model navigation)
//
//   Consider more animations:
//     Auto-solve - on/off
//     Auto-solve - single step
//
//     Full screen - on/off
//
//     Change solve rates.
//     Change sheet sizes.
//     Turn on/off:
//       Rotating heat spikes.
//       Center lo spike.
//       Frozen edges.
//
//     Enum animation.
//       We don't have animation that runs thru a set of enum value.
//
//     Meta animation?
//       bool_holder - animation on/off?
//       Turn individual animators on/off.
//       Change animation rates of selected animators?
// _______________________________________________________________________________________________

# include "all.h"
# include "animate_ui.h"
# include "bool_holder.h"
# include "int_holder.h"
# include "pack_holder.h"
# include "angle_holder.h"
# include "color_holder.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// typedefs
// _______________________________________________________________________________________________

typedef animate_ui_type:: animator_type   animator_type  ;

typedef animate_ui_type:: list_type       list_type      ;
typedef animate_ui_type:: item_base_type  item_base_type ;

typedef animate_ui_type:: label_type      label_type     ;
typedef animate_ui_type:: button_type     button_type    ;
typedef animate_ui_type:: slider_type     slider_type    ;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// item_type - local class
// _______________________________________________________________________________________________

  class
item_type
  : public item_base_type
{
  public:
      explicit
    item_type
     (  QString const &  name
      , list_type     *  p_list_widget
      , animator_type *  p_animator
     )                          : item_base_type( name, p_list_widget)
                                , p_animator_( p_animator)
                                { }

      animator_type *
    get_animator( )       const { return p_animator_; }

  private:
      animator_type * const
    p_animator_ ;
};

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// local functions
// _______________________________________________________________________________________________

  namespace /* anonymous */ {
  animator_type *
get_animator( item_base_type * p_item_base)
{
    if ( 0 == p_item_base ) return 0;

    item_type     * const  p_item      = static_cast< item_type * >( p_item_base);
    d_assert( p_item);

    animator_type * const  p_animator  = p_item->get_animator( );
    d_assert( p_animator);

    return p_animator;
} } /* end namespace anonymous */

// _______________________________________________________________________________________________

  namespace /* anonymous */ {
  template< typename HOLDER_TYPE >
  bool
attach
 (  button_type *  p_button
  , HOLDER_TYPE *  p_holder
  , void (HOLDER_TYPE :: *
                   p_attach_method)( button_type *)  = & HOLDER_TYPE :: attach
 )
{
    d_assert( p_attach_method);

    if ( p_button && p_holder ) {
        d_assert( ! p_button->isEnabled( ));
        p_button->setEnabled( true);
        (p_holder->*p_attach_method)( p_button);
        return true;
    }
    return false;
} } /* end namespace anonymous */

  namespace /* anonymous */ {
  template< typename HOLDER_TYPE >
  bool
attach
 (  slider_type *  p_slider
  , HOLDER_TYPE *  p_holder
  , void (HOLDER_TYPE :: *
                   p_attach_method)( slider_type *)  = & HOLDER_TYPE :: attach
 )
{
    d_assert( p_attach_method);

    if ( p_slider && p_holder ) {
        d_assert( ! p_slider->isEnabled( ));
        p_slider->setEnabled( true);
        (p_holder->*p_attach_method)( p_slider);
        return true;
    }
    return false;
} } /* end namespace anonymous */

  namespace /* anonymous */ {
  template< typename HOLDER_TYPE >
  bool
detach
 (  button_type *  p_button
  , HOLDER_TYPE *  p_holder
  , void (HOLDER_TYPE :: *
                   p_detach_method)( button_type *)  = & HOLDER_TYPE :: detach
 )
{
    d_assert( p_detach_method);

    if ( p_button && p_holder ) {
        (p_holder->*p_detach_method)( p_button);
        d_assert( p_button->isEnabled( ));
        p_button->setChecked( false);
        p_button->setEnabled( false);
        return true;
    }
    return false;
} } /* end namespace anonymous */

  namespace /* anonymous */ {
  template< typename HOLDER_TYPE >
  bool
detach
 (  slider_type *  p_slider
  , HOLDER_TYPE *  p_holder
  , void (HOLDER_TYPE :: *
                   p_detach_method)( slider_type *)  = & HOLDER_TYPE :: detach
 )
{
    d_assert( p_detach_method);

    if ( p_slider && p_holder ) {
        (p_holder->*p_detach_method)( p_slider);
        d_assert( p_slider->isEnabled( ));
        p_slider->setValue( p_slider->minimum( ));
        p_slider->setEnabled( false);
        return true;
    }
    return false;
} } /* end namespace anonymous */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// animate_ui_type - class
// _______________________________________________________________________________________________

  /* ctor, public */
  animate_ui_type::
animate_ui_type
 (  QObject     *  p_parent
  , list_type   *  p_list
  , label_type  *  p_label
  , button_type *  p_button__is_on
  , button_type *  p_button__is_wrapping
  , button_type *  p_button__is_reversed
  , slider_type *  p_slider__rate
  , slider_type *  p_slider__value
  , slider_type *  p_slider__trigger
 )
  : holder_base_type       ( p_parent              )
  , p_list_                ( d_verify_not_zero(
                             p_list)               )
  , p_label_               ( p_label               )
  , p_button__is_on_       ( p_button__is_on       )
  , p_button__is_wrapping_ ( p_button__is_wrapping )
  , p_button__is_reversed_ ( p_button__is_reversed )
  , p_slider__rate_        ( p_slider__rate        )
  , p_slider__value_       ( p_slider__value       )
  , p_slider__trigger_     ( p_slider__trigger     )
  , p_animate_             ( 0)
{
    attach_list( );
}

  /* ctor, public */
  animate_ui_type::
animate_ui_type
 (  QObject          *  p_parent
  , scafold_type     *  p_ui
  , animate_type     *  p_animate
  , heat_widget_type *  p_hw
 )
  : holder_base_type       ( p_parent                             )
  , p_list_                ( d_verify_not_zero(
                               d_verify_not_zero( p_ui)
                                 ->p_list_animators_)             )
  , p_label_               ( p_ui->p_label_animator_name_         )
  , p_button__is_on_       ( p_ui->p_checkb_animator_is_on_       )
  , p_button__is_wrapping_ ( p_ui->p_checkb_animator_is_wrapping_ )
  , p_button__is_reversed_ ( p_ui->p_checkb_animator_is_reversed_ )
  , p_slider__rate_        ( p_ui->p_slider_animator_rate_        )
  , p_slider__value_       ( p_ui->p_slider_animator_value_       )
  , p_slider__trigger_     ( p_ui->p_slider_animator_trigger_     )
  , p_animate_             ( 0)
{
    attach_list( );

    if ( p_animate && p_hw ) {
        build_list_of_animators( p_animate, p_hw, p_ui);
    }
}

// _______________________________________________________________________________________________

  /* protected */
  void
  animate_ui_type::
attach_list( )
{
    // Any of the ctrls can be zero, except p_list_.
    d_assert( p_list_);

    d_verify( connect(
        p_list_, SIGNAL( currentItemChanged( QListWidgetItem *, QListWidgetItem *)),
        this, SLOT( on_list__current_item_changed( QListWidgetItem *, QListWidgetItem *))
    ));
    d_verify( connect(
        p_list_, SIGNAL( itemActivated( QListWidgetItem *)),
        this, SLOT( on_item__activated( QListWidgetItem *))
    ));
}

// _______________________________________________________________________________________________

  /* public */
  void
  animate_ui_type::
build_list_of_animators( animate_type * p_animate, heat_widget_type * p_hw, scafold_type * p_ui)
  //
  // I'd prefer to keep scafold_type classes out of this file, but need code to
  // dig out all the buttons for click animators.
  //
  // I'd prefer to keep the heat_widget_type out of this file, but we need code to dig out all the
  // holders we're going to animate, and heat_widget_type is the source of those holders.
{
    d_assert( p_hw);
    d_assert( p_ui);

    d_assert( p_animate && ! p_animate_);
    p_animate_ = p_animate;

    create_holder_animators( p_hw);
    create_click_animators( p_ui);

    // We don't need p_animate_ any more. Should we set it to zero?
    p_animate_ = 0;

    // Fixup the colors after creating all the list items.
    set_background_colors__list( );

    // Select the animator. This will bind it to the ctrls.
    p_list_->setCurrentRow( 0);
}

// _______________________________________________________________________________________________

  animator_type *
  animate_ui_type::
create_click_animator( QString const & name, button_type * p_button)
  //
  // The click animator is the only one with no holder.
  // It always uses the default min/max/wrapping values.
{
    d_assert( p_button);
    d_assert( p_animate_);

    click_animator_type * const p_animator_new = new click_animator_type( p_animate_);
    d_assert( p_animator_new);
    //d_assert( p_animator_new == p_animate_->find_animator( p_holder));

    p_animator_new->attach__click( p_button);
    add_list_item( name, p_animator_new);
    return p_animator_new;
}

  template< typename HOLDER_T >
  animator_type *
  animate_ui_type::
create_animator( QString const & name, HOLDER_T * p_holder, double init_value)
  //
  // Attaches the holder to a new animator and puts it in the list.
  // It always uses the default min/max/wrapping values.
{
    d_assert( p_holder);
    d_assert( p_animate_);

    typedef typename HOLDER_T::default_animator_type local_animator_type;

    d_assert( p_animate_->find_animator( p_holder) == 0);
    local_animator_type * const
        p_animator_new = new local_animator_type( p_animate_, p_holder, init_value);
    d_assert( p_animator_new);
    d_assert( p_animate_->find_animator( p_holder) == p_animator_new);

    add_list_item( name, p_animator_new);
    return p_animator_new;
}

// _______________________________________________________________________________________________

  void
  animate_ui_type::
add_list_item
 (  QString const &  name
  , animator_type *  p_new_animator
 )
{
    d_assert( p_new_animator);
    d_assert( ! name.isEmpty( ));
    d_assert( p_list_);

    // Create a new list item. This will appear in the list, at the bottom (so far), because
    // the parent is p_list_.
    item_type * const p_new_item = new item_type( name, p_list_, p_new_animator);
    maybe_unused( p_new_item); /* variable not used, except when you're in the debugger */

    // When the animator flips on/off we change its background color.
    d_verify( connect(
        p_new_animator, SIGNAL( has_changed__is_on( bool)),
        this, SLOT( on_animator__on_off( bool))
    ));
}

// _______________________________________________________________________________________________

  void
  animate_ui_type::
create_color_animators
 (  QString const &  prefix          /* not empty, last char should be space */
  , color_holder  *  p_color_holder  /* never zero */
  , bool             include_hue     /* usually true */
  , bool             include_sv
  , bool             include_rgb     /* usually false */
  , bool_holder   *  p_auto_on_0     /* usually zero */
  , bool_holder   *  p_auto_on_1     /* usually zero */
 )
{
    d_assert( ! prefix.isEmpty( ));
    d_assert( p_color_holder);

    QString const h_suffix = tr( "Hue");
    QString const s_suffix = tr( "Saturation");
    QString const v_suffix = tr( "Value");
    QString const r_suffix = tr( "Red");
    QString const g_suffix = tr( "Green");
    QString const b_suffix = tr( "Blue");

    if ( include_hue ) {
      create_animator( prefix + h_suffix, p_color_holder->get_holder__h( ))->set_auto_on( p_auto_on_0, p_auto_on_1)
                                                                           ->set_is_wrapping( );
    }

    if ( include_sv ) {
      create_animator( prefix + s_suffix, p_color_holder->get_holder__s( ))->set_auto_on( p_auto_on_0, p_auto_on_1);
      create_animator( prefix + v_suffix, p_color_holder->get_holder__v( ))->set_auto_on( p_auto_on_0, p_auto_on_1);
    }

    if ( include_rgb ) {
      create_animator( prefix + r_suffix, p_color_holder->get_holder__r( ))->set_auto_on( p_auto_on_0, p_auto_on_1);
      create_animator( prefix + g_suffix, p_color_holder->get_holder__g( ))->set_auto_on( p_auto_on_0, p_auto_on_1);
      create_animator( prefix + b_suffix, p_color_holder->get_holder__b( ))->set_auto_on( p_auto_on_0, p_auto_on_1);
    }
}

  void
  animate_ui_type::
create_hue_animator
 (  QString const &  prefix          /* not empty, last char should be space */
  , color_holder  *  p_color_holder  /* never zero */
  , bool_holder   *  p_auto_on_0     /* usually zero */
  , bool_holder   *  p_auto_on_1     /* usually zero */
 )
{
    create_color_animators( prefix, p_color_holder, true, false, false, p_auto_on_0, p_auto_on_1);
}

  void
  animate_ui_type::
create_hsv_animators
 (  QString const &  prefix          /* not empty, last char should be space */
  , color_holder  *  p_color_holder  /* never zero */
  , bool_holder   *  p_auto_on_0     /* usually zero */
  , bool_holder   *  p_auto_on_1     /* usually zero */
 )
{
    create_color_animators( prefix, p_color_holder, true, true, false, p_auto_on_0, p_auto_on_1);
}

  void
  animate_ui_type::
create_rgb_animators
 (  QString const &  prefix          /* not empty, last char should be space */
  , color_holder  *  p_color_holder  /* never zero */
  , bool_holder   *  p_auto_on_0     /* usually zero */
  , bool_holder   *  p_auto_on_1     /* usually zero */
 )
{
    create_color_animators( prefix, p_color_holder, false, false, true, p_auto_on_0, p_auto_on_1);
}

// _______________________________________________________________________________________________

  void
  animate_ui_type::
create_holder_animators( heat_widget_type * p_hw)
{
    d_assert( p_hw);

    // Spin means yaw. Rise means pitch. We don't do roll.
    create_animator( tr( "Model spin"), p_hw->get_z_axis_rotation_angle_holder( ), 35)->set_on( );
    create_animator( tr( "Model rise"), p_hw->get_rise_angle_holder(            ), 10)->set_on( );

    create_animator( tr( "Model left/right"), p_hw->get_eye_right_translation_holder( ));
    create_animator( tr( "Model up/down"   ), p_hw->get_eye_up_translation_holder(    ));

    create_animator( tr( "XY scale"), p_hw->get_model_scale_holder( ));
    create_animator( tr( "Z scale" ), p_hw->get_z_scale_holder(     ));
    create_animator( tr( "Zoom"    ), p_hw->get_zoom_scale_holder(  ));

    create_animator( tr( "Perspective"), p_hw->get_perspective_angle_holder( ), 15)->set_on( );

    create_animator( tr( "Grid showing"), p_hw->get_is_showing_grid( ));

    // Here's where a tree structure would be handy.
    // We'd have a parent (not checkable, although some tree parents are checkable) and all the color children.
    { color_gradient_holder * const p_grad = p_hw->get_color_gradient_holder( );
      create_rgb_animators( tr( "Back color " ), p_hw->get_color_holder_background( ));
      create_hue_animator(  tr( "Hot color "  ), p_grad->get_color_holder_hot(   ));
      create_hsv_animators( tr( "Warm color " ), p_grad->get_color_holder_warm(  ));
      create_hsv_animators( tr( "Tepid color "), p_grad->get_color_holder_tepid( ));
      create_rgb_animators( tr( "Tepid color "), p_grad->get_color_holder_tepid( )); /* all 6 tepid ctrls */
      create_hsv_animators( tr( "Cool color " ), p_grad->get_color_holder_cool(  ));
      create_hue_animator(  tr( "Cold color " ), p_grad->get_color_holder_cold(  ));
    }

    // In this tree structure the parent would be checkable.
    { bool_holder * const p_is_clipping = p_hw->get_is_clipping( );
      create_animator( tr( "Is clipping"), p_is_clipping);
      create_animator( tr( "..Center"), p_hw->get_clipping_center_holder( ))->set_auto_on( p_is_clipping);
      create_animator( tr( "..Spread"), p_hw->get_clipping_spread_holder( ))->set_auto_on( p_is_clipping);
    }

    { lighting_rig_type * const p_rig        = p_hw->get_lighting_rig( );
      { bool_holder * const p_is_ambient = p_rig->get_is_ambient_on( );
        create_animator( tr( "Light Ambient"), p_is_ambient);
        create_hsv_animators( tr( ".. "), p_rig->get_ambient_color( ), p_is_ambient);
      }
      { light_type  * const p_light = p_rig->get_light_0( );
        bool_holder * const p_is_on = p_light->get_is_on( );
        create_animator( tr( "Light 0"), p_is_on);

        { bool_holder * const p_is_color = p_light->get_is_ambient_on( );
          create_animator( tr( "..Ambient" ), p_is_color)->set_auto_on( p_is_on);
          create_hue_animator( tr( ".... "), p_light->get_ambient_color( ), p_is_on, p_is_color);
        }
        { bool_holder * const p_is_color = p_light->get_is_diffuse_on( );
          create_animator( tr( "..Diffuse" ), p_is_color)->set_auto_on( p_is_on);
          create_hsv_animators( tr( ".... "), p_light->get_diffuse_color( ), p_is_on, p_is_color);
        }
        { bool_holder * const p_is_color = p_light->get_is_specular_on( );
          create_animator( tr( "..Specular" ), p_is_color)->set_auto_on( p_is_on);
          create_hue_animator( tr( ".... "), p_light->get_specular_color( ), p_is_on, p_is_color);
        }
        create_animator( tr( "..Spin"      ), p_light->get_spin_angle(             ), 15)->set_auto_on( p_is_on);
        create_animator( tr( "....with eye"), p_light->get_is_move_with_eye__spin( )    )->set_auto_on( p_is_on);
        create_animator( tr( "..Rise"      ), p_light->get_rise_angle(             ), 10)->set_auto_on( p_is_on);
        create_animator( tr( "....with eye"), p_light->get_is_move_with_eye__rise( )    )->set_auto_on( p_is_on);

        create_animator( tr( "..Arrow"     ), p_light->get_is_displaying_arrow(    )    );
      }

      // Many of these items for the 2nd light are commented out because the list in the UI is too long.
      // Any of these commented-out lines can be commented back in if you need the animator.
      { light_type  * const p_light = p_rig->get_light_1( );
        bool_holder * const p_is_on = p_light->get_is_on( );
        create_animator( tr( "Light 1"), p_is_on);

      //{ bool_holder * const p_is_color = p_light->get_is_ambient_on( );
      //  create_animator( tr( "..Ambient" ), p_is_color)->set_auto_on( p_is_on);
      //  create_hue_animator( tr( ".... "), p_light->get_ambient_color( ), p_is_on, p_is_color);
      //}
        { bool_holder * const p_is_color = p_light->get_is_diffuse_on( );
          create_animator( tr( "..Diffuse" ), p_is_color)->set_auto_on( p_is_on);
          create_hue_animator( tr( ".... "), p_light->get_diffuse_color( ), p_is_on, p_is_color);
        }
      //{ bool_holder * const p_is_color = p_light->get_is_specular_on( );
      //  create_animator( tr( "..Specular" ), p_is_color)->set_auto_on( p_is_on);
      //  create_hue_animator( tr( ".... "), p_light->get_specular_color( ), p_is_on, p_is_color);
      //}
        create_animator( tr( "..Spin"      ), p_light->get_spin_angle(             ), 45)->set_auto_on( p_is_on);
      //create_animator( tr( "....with eye"), p_light->get_is_move_with_eye__spin( )    )->set_auto_on( p_is_on);
        create_animator( tr( "..Rise"      ), p_light->get_rise_angle(             ), 30)->set_auto_on( p_is_on);
      //create_animator( tr( "....with eye"), p_light->get_is_move_with_eye__rise( )    )->set_auto_on( p_is_on);

        create_animator( tr( "..Arrow"     ), p_light->get_is_displaying_arrow(    )    );
      }
    }

    { isotherm_properties_style_type * const p_iso     = p_hw->get_isotherm_properties( );
      bool_holder                    * const p_is_show = p_iso->get_is_showing( );

      create_animator( tr( "Iso showing"), p_is_show);
      create_animator( tr( "Iso width"  ), p_iso->get_width(   ), 10)->set_auto_on( p_is_show);
      create_animator( tr( "Iso spacing"), p_iso->get_spacing( ), 20)->set_auto_on( p_is_show);
      create_animator( tr( "Iso offset" ), p_iso->get_offset(  ), 15)->set_auto_on( p_is_show)->set_is_wrapping( );
      create_animator( tr( "Iso opacity"), p_iso->get_opacity( ),  5)->set_auto_on( p_is_show);

      create_hsv_animators( tr( "Iso color "), p_iso->get_color( ), p_is_show);
    }

    { bristle_properties_style_type * const p_bristle = p_hw->get_bristle_properties( );
      create_animator( tr( "Bristle top length"), p_bristle->get_top_length( ));
      create_hue_animator( tr( "Bristle top "), p_bristle->get_top_color( ));
      create_animator( tr( "Bristle under length"), p_bristle->get_under_length( ));
      create_hue_animator( tr( "Bristle under "), p_bristle->get_under_color( ));
      create_animator( tr( "Bristle width"), p_bristle->get_pixel_width( ));
    }

    { face_properties_style_type * const p_face = p_hw->get_face_properties( );
      create_animator( tr( "Face top shine"), p_face->get_top_face_shininess( ));
      create_animator( tr( "Face under shine"), p_face->get_under_face_shininess( ));
      create_animator( tr( "Triangle mesh"), p_face->get_is_triangle_mesh( ));
      create_animator( tr( "Interpolate normals"), p_face->get_is_interpolate( ));
      create_animator( tr( "Face width"), p_face->get_pixel_width( ));
    }
 }

// _______________________________________________________________________________________________

  void
  animate_ui_type::
create_click_animators( scafold_type * p_ui)
{
    create_click_animator( tr( "Ck normalize"   ), p_ui->p_button_heat_normalize_      );
    create_click_animator( tr( "Ck flatten"     ), p_ui->p_button_heat_flatten_        );
    create_click_animator( tr( "Ck delta"       ), p_ui->p_button_heat_delta_          );
    create_click_animator( tr( "Ck square top"  ), p_ui->p_button_heat_init_           );
    create_click_animator( tr( "Ck stair steps" ), p_ui->p_button_heat_stair_steps_    );
    create_click_animator( tr( "Ck random"      ), p_ui->p_button_heat_random_         );
    create_click_animator( tr( "Ck bel 1a"      ), p_ui->p_button_heat_bell_1a_        );
    create_click_animator( tr( "Ck bel 2a"      ), p_ui->p_button_heat_bell_2a_        );
    create_click_animator( tr( "Ck bel 3a"      ), p_ui->p_button_heat_bell_3a_        );
    create_click_animator( tr( "Ck bel 1b"      ), p_ui->p_button_heat_bell_1b_        );
    create_click_animator( tr( "Ck bel 2b"      ), p_ui->p_button_heat_bell_2b_        );
    create_click_animator( tr( "Ck bel 3b"      ), p_ui->p_button_heat_bell_3b_        );
    create_click_animator( tr( "Ck bel 1c"      ), p_ui->p_button_heat_bell_1c_        );
    create_click_animator( tr( "Ck bel 2c"      ), p_ui->p_button_heat_bell_2c_        );
    create_click_animator( tr( "Ck bel 3c"      ), p_ui->p_button_heat_bell_3c_        );
    create_click_animator( tr( "Ck sin 1"       ), p_ui->p_button_heat_sin_over_dist_1_);
    create_click_animator( tr( "Ck sin 2"       ), p_ui->p_button_heat_sin_over_dist_2_);
    create_click_animator( tr( "Ck sin 3"       ), p_ui->p_button_heat_sin_over_dist_3_);
    create_click_animator( tr( "Ck tilt 1"      ), p_ui->p_button_heat_ramp_1_         );
    create_click_animator( tr( "Ck tilt 2"      ), p_ui->p_button_heat_ramp_2_         );
    create_click_animator( tr( "Ck tilt 3"      ), p_ui->p_button_heat_ramp_3_         );
    create_click_animator( tr( "Ck reverse wave"), p_ui->p_button_heat_reverse_wave_   );
}

// _______________________________________________________________________________________________

  /* slot */
  void
  animate_ui_type::
on_list__current_item_changed( item_base_type * p_item_now, item_base_type * p_item_before)
  //
  // The current item in the list-control has changed.
  // We need to bind the current-item animator to the detail controls.
{
    d_assert( p_item_before != p_item_now);
    d_assert( p_list_ && (p_list_->currentItem( ) == p_item_now));

    if ( p_item_before ) {
        set_background_color__list_item( p_item_before);
    }

    detach_animator( get_animator( p_item_before));
    attach_animator( get_animator( p_item_now   ));

    set_text_and_color__label( );

    if ( p_item_now ) {
        set_background_color__list_item( p_item_now);
    }
}

// _______________________________________________________________________________________________

  void
  animate_ui_type::
attach_animator( animator_type * p_animator)
{
    // We could use p_list_->currentItem( ) since that is the animator we are attaching.
    d_assert( get_animator( p_list_->currentItem( )) == p_animator);

    // Attach the animator that is now "current".
    if ( p_animator ) {
        // Attach the animator from the main 3 UI ctrls.
        attach( p_button__is_on_      , p_animator,
            implicit_cast< void (animator_type:: *)( button_type *) >(
                                                     & animator_type::attach__is_on          ));
        attach( p_button__is_wrapping_, p_animator, & animator_type::attach__is_wrapping     );
        attach( p_slider__rate_       , p_animator, & animator_type::attach__delta_per_second);

        // Bool and click holders attach differently from the others.

        // Try to attach the click animator.
        // This will also catch bool animators because bool_animator is a subtype of click_animator.
        click_animator_type * p_click_animator = qobject_cast< click_animator_type * >( p_animator);
        if ( p_click_animator ) {

            // The is_wrapping button is disabled when we are attached to a click or checkbox.
            // It should also be checked.
            if ( p_button__is_wrapping_ ) {
                d_assert( p_button__is_wrapping_->isEnabled( ));
                p_button__is_wrapping_->setEnabled( false);
            }

            // Use the moving-value from the animator as the value. In all the remaining cases we
            // attach the value slider to the holder value, not the animator.
            attach( p_slider__value_, p_click_animator, & click_animator_type::attach__moving_value);

            // Change the name of the "is_reversed" button and attach it to go on/off with
            // the bool holder. There are two ways to do this:
            //
            //   Cast (as above) p_animator to p_bool_animator and call:
            //     p_bool_animator->attach__bool( p_button__is_reversed_);
            //
            //   Cast p_holder_base to a bool_holder and attach that. This is what we do:
            if ( attach( p_button__is_reversed_, qobject_cast< bool_holder * >( p_animator->get_animated( ))) ) {
                set_text__is_reversed_button__is_target_on( );
            }

            // Attach the trigger slider.
            // bool_animator_type is a subtype of click_animator_type.
            attach
             (  p_slider__trigger_
              , qobject_cast< bool_animator_type * >( p_animator)
              , & bool_animator_type::attach__transition
             );
        } else {
            // Attach the is_reversed button in the normal way.
            // The text should already be correctly set up.
            attach( p_button__is_reversed_, p_animator, & animator_type::attach__is_reversed);

            // We don't use slider__trigger in this case. Don't attach it.

            if ( p_slider__value_ ) {
                // Since this is not a click animator, look for a known holder type.
                // Click animators are the only ones without attached holders.
                holder_base_type * const p_holder = p_animator->get_animated( );
                d_assert( p_holder);
                d_verify(
                  attach( p_slider__value_, qobject_cast< angle_holder            * >( p_holder)) ||
                  attach( p_slider__value_, qobject_cast< int_range_steps_holder  * >( p_holder)) ||
                  attach( p_slider__value_, qobject_cast< double_slide_holder     * >( p_holder)) ||
                  attach( p_slider__value_, qobject_cast< pack_range_steps_holder * >( p_holder)) );
            }
        }
    } else {
        // The ctrls should already be disabled.
        d_assert( implies( p_button__is_on_      , ! p_button__is_on_      ->isEnabled( )));
        d_assert( implies( p_button__is_wrapping_, ! p_button__is_wrapping_->isEnabled( )));
        d_assert( implies( p_button__is_reversed_, ! p_button__is_reversed_->isEnabled( )));
        d_assert( implies( p_slider__rate_       , ! p_slider__rate_       ->isEnabled( )));
        d_assert( implies( p_slider__value_      , ! p_slider__value_      ->isEnabled( )));
        d_assert( implies( p_slider__trigger_    , ! p_slider__trigger_    ->isEnabled( )));
    }
}

// _______________________________________________________________________________________________

  void
  animate_ui_type::
detach_animator( animator_type * p_animator)
{
    // We should not be detaching the current item.
    d_assert( get_animator( p_list_->currentItem( )) != p_animator);

    // Detach the animator that was previously "current".
    if ( p_animator ) {
        // Bool and click holders detach differently from the others.

        // Try to detach the click animator.
        // This will also catch bool animators because bool_animator is a subtype of click_animator.
        click_animator_type * p_click_animator = qobject_cast< click_animator_type * >( p_animator);
        if ( p_click_animator ) {

            // Enable the is_wrapping button again, even though we're just going to disable it below.
            if ( p_button__is_wrapping_ ) {
                d_assert( ! p_button__is_wrapping_->isEnabled( ));
                p_button__is_wrapping_->setEnabled( true);
            }

            // Detach moving-value slider.
            detach( p_slider__value_, p_click_animator, & click_animator_type::detach__moving_value);

            // Detach is-reversed checkbox.
            if ( detach( p_button__is_reversed_, qobject_cast< bool_holder * >( p_animator->get_animated( ))) ) {
                set_text__is_reversed_button__is_reversed( );
            }

            // Detach trigger slider.
            detach
             (  p_slider__trigger_
              , qobject_cast< bool_animator_type * >( p_animator)
              , & bool_animator_type::detach__transition
             );
        } else {
            // Detach the is_reversed button in the normal way.
            // The text should already be correctly set up.
            detach( p_button__is_reversed_, p_animator, & animator_type::detach__is_reversed);

            // We never attached the slider__trigger to anything so we don't have to detach it.

            if ( p_slider__value_ ) {
                // Since this is not a click animator, look for a known holder type.
                // Click animators are the only ones without attached holders.
                holder_base_type * const p_holder = p_animator->get_animated( );
                d_assert( p_holder);
                d_verify(
                  detach( p_slider__value_, qobject_cast< angle_holder            * >( p_holder)) ||
                  detach( p_slider__value_, qobject_cast< int_range_steps_holder  * >( p_holder)) ||
                  detach( p_slider__value_, qobject_cast< double_slide_holder     * >( p_holder)) ||
                  detach( p_slider__value_, qobject_cast< pack_range_steps_holder * >( p_holder)) );
            }
        }

        // Detach the animator from the main 3 UI ctrls.
        detach( p_button__is_on_      , p_animator,
            implicit_cast< void (animator_type:: *)( button_type *) >(
                                                     & animator_type::detach__is_on          ));
        detach( p_button__is_wrapping_, p_animator, & animator_type::detach__is_wrapping     );
        detach( p_slider__rate_       , p_animator, & animator_type::detach__delta_per_second);
    } else {
        // The ctrls should already be disabled.
        d_assert( implies( p_button__is_on_      , ! p_button__is_on_      ->isEnabled( )));
        d_assert( implies( p_button__is_wrapping_, ! p_button__is_wrapping_->isEnabled( )));
        d_assert( implies( p_button__is_reversed_, ! p_button__is_reversed_->isEnabled( )));
        d_assert( implies( p_slider__rate_       , ! p_slider__rate_       ->isEnabled( )));
        d_assert( implies( p_slider__value_      , ! p_slider__value_      ->isEnabled( )));
        d_assert( implies( p_slider__trigger_    , ! p_slider__trigger_    ->isEnabled( )));
    }
}

// _______________________________________________________________________________________________

  /* slot */
  void
  animate_ui_type::
on_item__activated( item_base_type * p_item_poked)
{
    if ( p_item_poked ) {
        animator_type * const p_animator_poked = get_animator( p_item_poked);

        // Toggle on/off.
        p_animator_poked->set_on( p_animator_poked->is_off( ));
    }
}

// _______________________________________________________________________________________________

  namespace /* anonymous */ {
QColor const  background_on_brite(  0xff, 0x80, 0x80);
QColor const  background_on_dull(   0xff, 0xc0, 0xc0);
QColor const  background_off_brite( 0x70, 0xff, 0x70);
QColor const  background_off_dull(  0xd0, 0xff, 0xd0);
  } /* end namespace anonymous */

  /* slot */
  void
  animate_ui_type::
on_animator__on_off( bool)
  //
  // Called when an animator flips on/off.
  // Unfortunately we don't know which list-item is attached to the animator, or even which
  // animator is flipping.
{
    // This sets the background color for ALL the animators, not just the one that's changed.
    // Inefficient, but acceptable.
    set_background_colors__list( );

    // Make sure the color in the label is right for the current animator.
    // We only have to do this if the current item changed.
    set_text_and_color__label( );
}

  void
  animate_ui_type::
set_background_colors__list( )
{
    for ( int
            index = p_list_->count( ) - 1
          ; index >= 0
          ; index -= 1 )
    {
        set_background_color__list_item( p_list_->item( index));
    }
}

  void
  animate_ui_type::
set_background_color__list_item( item_base_type * p_item)
{
    d_assert( p_item);
    d_assert( p_list_);

    bool   const    is_on        = get_animator( p_item)->is_on( );
    bool   const    is_current   = p_list_->currentItem( ) == p_item;

    p_item->setBackgroundColor(
        is_current ?
            (is_on ? background_on_brite : background_off_brite) :
            (is_on ? background_on_dull  : background_off_dull ) );
}

// _______________________________________________________________________________________________

  void
  animate_ui_type::
set_text_and_color__label( )
{
    if ( p_label_ ) {
        d_assert( p_list_);
        item_base_type * const p_item = p_list_->currentItem( );
        if ( p_item ) {
            p_label_->setText( p_item->text( ));

            QString const style_template( tr( "QLabel { background-color : %1 }"));
            // You can also set the color of the group-box, but it doesn't look good.
            // The ctrls in the box are not normally transparent, nor do they inherit
            // the parent (group-box) background.
            //   "QGroupBox { background-color : %1 }"

            if ( get_animator( p_item)->is_on( ) ) {
                static QString style_string;
                if ( style_string.isEmpty( ) ) {
                    style_string = style_template.arg( background_on_dull.name( ));
                }
                p_label_->setStyleSheet( style_string);
            } else {
                static QString style_string;
                if ( style_string.isEmpty( ) ) {
                    style_string = style_template.arg( background_off_dull.name( ));
                }
                p_label_->setStyleSheet( style_string);
            }
        } else {
            QString const empty_string;
            if ( p_label_ ) {
                p_label_->setText( empty_string);
                p_label_->setStyleSheet( empty_string);
            }
        }
    }
}

// _______________________________________________________________________________________________

  void
  animate_ui_type::
set_text__is_reversed_button__is_reversed( )
{
    QString const button_text( tr( "Is reversed"));
    if ( p_button__is_reversed_ ) {
        p_button__is_reversed_->setText( button_text);
    }
}

  void
  animate_ui_type::
set_text__is_reversed_button__is_target_on( )
{
    QString const button_text( tr( "Is target on"));
    if ( p_button__is_reversed_ ) {
        p_button__is_reversed_->setText( button_text);
    }
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// animate_ui.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
