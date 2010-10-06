// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// color_gradient_holder.h
//
//   Copyright (c) Neal Binnendyk 2009, 2010. <nealabq@gmail.com> nealabq.com
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
# ifndef COLOR_GRADIENT_HOLDER_H
# define COLOR_GRADIENT_HOLDER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "color_holder.h"

// _______________________________________________________________________________________________

  class
color_gradient_holder
  : public holder_base_type
{
  Q_OBJECT

  // ----------------------------------------------------------------------------------
  public:
    typedef color_holder                   color_holder_type ;
    typedef color_holder_type::color_type  color_type        ;

  // ----------------------------------------------------------------------------------
  public:
    /* ctor */           color_gradient_holder( QObject * p_parent)   ;
  private:
    /* copy */           color_gradient_holder( color_gradient_holder const *)
                                                                      ; // no implementation

  // ----------------------------------------------------------------------------------
  public:
    enum /* : int */
     {  e_cold
      , e_cool
      , e_tepid
      , e_warm
      , e_hot
      , e_color_count
     };
    int                  get_color_count( )                     const { return e_color_count; }
                         // Constant now, but maybe someday we can add/remove colors.

    color_holder_type *  get_color_holder_at( int index)        const ;
    color_type const  &  get_color_at(        int index)        const { return get_color_holder_at( index)->get_color( ); }

    float                get_normal_min_at( int i)              const ;
    float                get_normal_mid_at( int i)              const ;
    float                get_normal_max_at( int i)              const ;

    color_type           get_color_at_normal( float)            const ;

  // ----------------------------------------------------------------------------------
  public:
    color_holder_type *  get_color_holder_cold( )               const { return get_color_holder_at( e_cold ); }
    color_holder_type *  get_color_holder_cool( )               const { return get_color_holder_at( e_cool ); }
    color_holder_type *  get_color_holder_tepid( )              const { return get_color_holder_at( e_tepid); }
    color_holder_type *  get_color_holder_warm( )               const { return get_color_holder_at( e_warm ); }
    color_holder_type *  get_color_holder_hot( )                const { return get_color_holder_at( e_hot  ); }

    color_type const  &  get_color_cold(  )                     const { return get_color_holder_cold(  )->get_color( ); }
    color_type const  &  get_color_cool(  )                     const { return get_color_holder_cool(  )->get_color( ); }
    color_type const  &  get_color_tepid( )                     const { return get_color_holder_tepid( )->get_color( ); }
    color_type const  &  get_color_warm(  )                     const { return get_color_holder_warm(  )->get_color( ); }
    color_type const  &  get_color_hot(   )                     const { return get_color_holder_hot(   )->get_color( ); }

  // ----------------------------------------------------------------------------------
  // Member vars
  private:
    color_holder_type *  pp_color_holder_[ e_color_count ] ;
};

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef COLOR_GRADIENT_HOLDER_H */
//
// color_gradient_holder.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
