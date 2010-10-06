// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_env_type_colors.cpp
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
// What we want:
//   To treat rgb_type like s supertype of rgba_type.
//   To treat rgb_type like an array of 3 components.
//   To treat rgba_type like an array of 4 components.
//
//   To have a class like this, freely convertable to/from rgb_type<COMPONENT_TYPE> when
//   (COMPONENT_COUNT==3) and rgba_type<COMPONENT_TYPE> when (COMPONENT_COUNT==4):
//     color_type< COMPONENT_COUNT, COMPONENT_TYPE >
//
//   To build this class on top of a generic array class?
//
//   The type system is not built to handle multiple interpretations as described above without
//   using reinterpret_cast<..>.
//
// Notes:
//   The OpenGL glMaterial*(..) methods are only defined for float_type and int_type, but
//   ubyte_type is often the most natural color-component type.
//   We cannot use built-in ubyte-to-float or ubyte-to-int conversions because the integer
//   types are interpreted as fractional fixed-point numbers. That is why we provide the
//   convert_components_to< type >( ) methods.
// _______________________________________________________________________________________________

# include "all.h"
# include "gl_env_type_colors.h"

// _______________________________________________________________________________________________
//
namespace gl_env {
// _______________________________________________________________________________________________

# define INTERPOLATE_COMPONENT( X )              \
      ((c0.get_ ## X( ) == c1.get_ ## X( )) ?    \
         c0.get_ ## X( ) :                       \
         (zfactor * c0.get_ ## X( )) +           \
         ( factor * c1.get_ ## X( ))             \
      )                                          /* end macro */

// _______________________________________________________________________________________________

  rgb_type< float_type >
interpolate
 (  float                           factor
  , rgb_type< float_type > const &  c0
  , rgb_type< float_type > const &  c1
 )
  // Returns a color on the line between c0 and c1.
  // How far along the line depends on factor.
  // If factor == 0, returns c0.
  // If factor == 1, returns c1.
  //
  // factor is usually in the range 0..1.
  // Does not clamp. May not be clamped if c0 or c1 are not clamped, or if factor is <0 or >1.
  //
  // This is a building block for Bézier curves.
{
    float const zfactor = 1.0f - factor;
    return rgb_type< float_type >(
        INTERPOLATE_COMPONENT( r),
        INTERPOLATE_COMPONENT( g),
        INTERPOLATE_COMPONENT( b) );
}

  rgba_type< float_type >
interpolate
 (  float                            factor
  , rgba_type< float_type > const &  c0
  , rgba_type< float_type > const &  c1
 )
{
    float const zfactor = 1.0f - factor;
    return rgba_type< float_type >(
        INTERPOLATE_COMPONENT( r),
        INTERPOLATE_COMPONENT( g),
        INTERPOLATE_COMPONENT( b),
        INTERPOLATE_COMPONENT( a) );
}

// _______________________________________________________________________________________________

  rgb_type< double_type >
interpolate
 (  double                           factor
  , rgb_type< double_type > const &  c0
  , rgb_type< double_type > const &  c1
 )
{
    double const zfactor = 1.0f - factor;
    return rgb_type< double_type >(
        INTERPOLATE_COMPONENT( r),
        INTERPOLATE_COMPONENT( g),
        INTERPOLATE_COMPONENT( b) );
}

  rgba_type< double_type >
interpolate
 (  double                            factor
  , rgba_type< double_type > const &  c0
  , rgba_type< double_type > const &  c1
 )
{
    double const zfactor = 1.0f - factor;
    return rgba_type< double_type >(
        INTERPOLATE_COMPONENT( r),
        INTERPOLATE_COMPONENT( g),
        INTERPOLATE_COMPONENT( b),
        INTERPOLATE_COMPONENT( a) );
}

// _______________________________________________________________________________________________

# undef INTERPOLATE_COMPONENT

// _______________________________________________________________________________________________
//
} /* end namespace gl_env */
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_env_type_colors.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
