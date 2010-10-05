// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// shader.h
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
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
