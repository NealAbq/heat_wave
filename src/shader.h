// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// shader.h
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
# ifndef SHADER_H
# define SHADER_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

# include "gl_shader.h"

// _______________________________________________________________________________________________

  struct
with_blinn_phong_shading__wrapper_type
{
    /* ctor */
  with_blinn_phong_shading__wrapper_type
   (  bool    is_on
    , bool    is_bitmap
    , bool    is_isotherm_showing
    , float   isotherm_z_factor
    , float   isotherm_offset
    , gl_env::rgb_type< float > const &
              isotherm_color
   );

    /* dtor */
  ~with_blinn_phong_shading__wrapper_type( )
    ;

  private:
    gl_env::shader::program_holder_variant_type  previous_program_ ;
};

  extern
  void
init_shader_program__blinn_phong( )
  ;

  extern
  void
maybe_post_warning_dialog__blinn_phong__for_bitmap_context( )
  ;

  extern
  void
release_shader_program__blinn_phong( )
  ;

  extern
  void
release_shader_program__blinn_phong__for_bitmap_context( )
  ;

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif /* ifndef SHADER_H */
//
// shader.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
