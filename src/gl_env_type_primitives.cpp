// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_env_type_primitives.cpp
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

# include "all.h"
# include "gl_env_type_primitives.h"

// _______________________________________________________________________________________________
//
namespace gl_env {
// _______________________________________________________________________________________________

  bool
is_data_type( enum_type t)
{
    return
        (GL_BYTE           == t) ||
        (GL_UNSIGNED_BYTE  == t) ||
        (GL_SHORT          == t) ||
        (GL_UNSIGNED_SHORT == t) ||
        (GL_INT            == t) ||
        (GL_UNSIGNED_INT   == t) ||
        (GL_FLOAT          == t) ||
        (GL_DOUBLE         == t) ;
}

  bool
is_hint_trg( enum_type t)
{
    // The standard Windows GL.h file lists GL_PHONG_HINT as a hint target, but doesn't
    // define it and it's non-standard. Maybe it's part of GL_WIN_phong_shading.

    // GL_GENERATE_MIPMAP_HINT
    // GL_TEXTURE_COMPRESSION_HINT
    // GL_FRAGMENT_SHADER_DERIVATIVE_HINT
    return
        (GL_FOG_HINT                    == t) ||
        (GL_POINT_SMOOTH_HINT           == t) ||
        (GL_LINE_SMOOTH_HINT            == t) ||
        (GL_POLYGON_SMOOTH_HINT         == t) ||
        (GL_PERSPECTIVE_CORRECTION_HINT == t) ;
}

  bool
is_hint_mode( enum_type m)
{
    return
        (GL_FASTEST   == m) ||
        (GL_NICEST    == m) ||
        (GL_DONT_CARE == m) ;
}

  bool
is_vertex_entry_mode( enum_type mode)
{
    return
        (GL_POINTS         == mode) ||
        (GL_LINES          == mode) ||
        (GL_LINE_STRIP     == mode) ||
        (GL_LINE_LOOP      == mode) ||
        (GL_TRIANGLES      == mode) ||
        (GL_TRIANGLE_STRIP == mode) ||
        (GL_TRIANGLE_FAN   == mode) ||
        (GL_QUADS          == mode) ||
        (GL_QUAD_STRIP     == mode) ||
        (GL_POLYGON        == mode) ;
}

  bool
is_material_face__get( enum_type face)
{
    return
        (GL_FRONT == face) ||
        (GL_BACK  == face) ;
}

  bool
is_material_face__set( enum_type face)
{
    return is_material_face__get( face) || (GL_FRONT_AND_BACK == face);
}

  bool
is_color_facet__get( enum_type facet)
{
    return
        (GL_AMBIENT  == facet) ||
        (GL_DIFFUSE  == facet) ||
        (GL_SPECULAR == facet) ||
        (GL_EMISSION == facet) ;
}

  bool
is_color_facet__set( enum_type facet)
{
    return is_color_facet__get( facet) || (GL_AMBIENT_AND_DIFFUSE == facet);
}

  bool
is_1D_pixel_format( enum_type f)
{
    // GL_BGR
    // GL_BGRA
    return
        (GL_COLOR_INDEX     == f) ||
        (GL_RED             == f) ||
        (GL_GREEN           == f) ||
        (GL_BLUE            == f) ||
        (GL_ALPHA           == f) ||
        (GL_LUMINANCE       == f) || // internal format 1
        (GL_LUMINANCE_ALPHA == f) || // internal format 2
        (GL_RGB             == f) || // internal format 3
        (GL_RGBA            == f) ;  // internal format 4
}

  bool
is_pixel_format( enum_type f)
{
    return is_1D_pixel_format( f) || (GL_DEPTH_COMPONENT == f);
}

  bool
is_read_write_pixel_format( enum_type f)
{
    return is_pixel_format( f) || (GL_STENCIL_INDEX == f);
}

  bool
is_pixel_data_type( enum_type t)
{
    // The unsigned types each have several packed forms (none supported in 1.1 except GL_BITMAP).
    //   GL_UNSIGNED_BYTE_3_3_2
    //   GL_UNSIGNED_BYTE_2_3_3_REV
    //   GL_UNSIGNED_SHORT_5_6_5
    //   GL_UNSIGNED_SHORT_5_6_5_REV
    //   GL_UNSIGNED_SHORT_4_4_4_4
    //   GL_UNSIGNED_SHORT_4_4_4_4_REV
    //   GL_UNSIGNED_SHORT_5_5_5_1
    //   GL_UNSIGNED_SHORT_1_5_5_5_REV
    //   GL_UNSIGNED_INT_8_8_8_8
    //   GL_UNSIGNED_INT_8_8_8_8_REV
    //   GL_UNSIGNED_INT_10_10_10_2
    //   GL_UNSIGNED_INT_2_10_10_10_REV

    // You cannot use doubles to specify pixel type.
    // GL_BITMAP is added. It is a form of GL_UNSIGNED_BYTE.
    return (is_data_type( t) && (GL_DOUBLE != t)) || (GL_BITMAP == t);
}

  bool
is_pixel_map_type( enum_type t)
{
    return
        (GL_PIXEL_MAP_I_TO_I == t) ||
        (GL_PIXEL_MAP_S_TO_S == t) ||
        (GL_PIXEL_MAP_I_TO_R == t) ||
        (GL_PIXEL_MAP_I_TO_G == t) ||
        (GL_PIXEL_MAP_I_TO_B == t) ||
        (GL_PIXEL_MAP_I_TO_A == t) ||
        (GL_PIXEL_MAP_R_TO_R == t) ||
        (GL_PIXEL_MAP_G_TO_G == t) ||
        (GL_PIXEL_MAP_B_TO_B == t) ||
        (GL_PIXEL_MAP_A_TO_A == t) ;
}

  bool
is_pixel_transfer_param( enum_type a)
{
    // With ARB_imaging extension you have more options:
    //   GL_POST_COLOR_MATRIX_RED_SCALE
    //   GL_POST_COLOR_MATRIX_GREEN_SCALE
    //   GL_POST_COLOR_MATRIX_BLUE_SCALE
    //   GL_POST_COLOR_MATRIX_ALPHA_SCALE
    //   GL_POST_COLOR_MATRIX_RED_BIAS
    //   GL_POST_COLOR_MATRIX_GREEN_BIAS
    //   GL_POST_COLOR_MATRIX_BLUE_BIAS
    //   GL_POST_COLOR_MATRIX_ALPHA_BIAS
    //   GL_POST_CONVOLUTION_RED_SCALE
    //   GL_POST_CONVOLUTION_GREEN_SCALE
    //   GL_POST_CONVOLUTION_BLUE_SCALE
    //   GL_POST_CONVOLUTION_ALPHA_SCALE
    //   GL_POST_CONVOLUTION_RED_BIAS
    //   GL_POST_CONVOLUTION_GREEN_BIAS
    //   GL_POST_CONVOLUTION_BLUE_BIAS
    //   GL_POST_CONVOLUTION_ALPHA_BIAS
    return
        (GL_INDEX_SHIFT  == a) ||
        (GL_INDEX_OFFSET == a) ||
        (GL_RED_SCALE    == a) ||
        (GL_RED_BIAS     == a) ||
        (GL_GREEN_SCALE  == a) ||
        (GL_GREEN_BIAS   == a) ||
        (GL_BLUE_SCALE   == a) ||
        (GL_BLUE_BIAS    == a) ||
        (GL_ALPHA_SCALE  == a) ||
        (GL_ALPHA_BIAS   == a) ||
        (GL_DEPTH_SCALE  == a) ||
        (GL_DEPTH_BIAS   == a) ;
}

  bool
is_copy_pixel_src( enum_type s)
{
    return
        (GL_COLOR   == s) ||
        (GL_DEPTH   == s) ||
        (GL_STENCIL == s) ;
}

  bool
is_texture_param__set_1( enum_type p)
{
    // These params are not available in version 1.1:
    //   GL_TEXTURE_MIN_LOD
    //   GL_TEXTURE_MAX_LOD
    //   GL_TEXTURE_BASE_LEVEL
    //   GL_TEXTURE_MAX_LEVEL
    //   GL_TEXTURE_WRAP_R
    //   GL_TEXTURE_COMPARE_MODE
    //   GL_TEXTURE_COMPARE_FUNC
    //   GL_DEPTH_TEXTURE_MODE
    //   GL_GENERATE_MIPMAP
    return
        (GL_TEXTURE_MAG_FILTER == p) ||
        (GL_TEXTURE_MIN_FILTER == p) ||
        (GL_TEXTURE_WRAP_S     == p) ||
        (GL_TEXTURE_WRAP_T     == p) ||
        (GL_TEXTURE_PRIORITY   == p) ;
}

  bool
is_texture_param__get_1( enum_type p)
{
    return is_texture_param__set_1( p) || (GL_TEXTURE_RESIDENT == p);
}

  bool
is_texture_param__N( enum_type p)
{
    return GL_TEXTURE_BORDER_COLOR == p;
}

  bool
is_texture_level_param( enum_type p)
{
    // The following level params are not available in version 1.1:
    //   GL_TEXTURE_DEPTH
    //   GL_TEXTURE_DEPTH_SIZE
    //   GL_TEXTURE_COMPRESSED
    //   GL_TEXTURE_COMPRESSED_IMAGE_SIZE
    return
        (GL_TEXTURE_WIDTH           == p) ||
        (GL_TEXTURE_HEIGHT          == p) ||
        (GL_TEXTURE_INTERNAL_FORMAT == p) ||
        (GL_TEXTURE_BORDER          == p) ||
        (GL_TEXTURE_RED_SIZE        == p) ||
        (GL_TEXTURE_GREEN_SIZE      == p) ||
        (GL_TEXTURE_BLUE_SIZE       == p) ||
        (GL_TEXTURE_ALPHA_SIZE      == p) ||
        (GL_TEXTURE_LUMINANCE_SIZE  == p) ||
        (GL_TEXTURE_INTENSITY_SIZE  == p) ;
}

  bool
is_texture_minifying_filter( enum_type f)
{
    return
        (GL_NEAREST                == f) ||
        (GL_LINEAR                 == f) ||
        (GL_NEAREST_MIPMAP_NEAREST == f) ||
        (GL_LINEAR_MIPMAP_NEAREST  == f) ||
        (GL_NEAREST_MIPMAP_LINEAR  == f) ||
        (GL_LINEAR_MIPMAP_LINEAR   == f) ;
}

  bool
is_texture_magnifying_filter( enum_type f)
{
    return
        (GL_NEAREST == f) ||
        (GL_LINEAR  == f) ;
}

  bool
is_texture_wrap( enum_type w)
{
    // GL_CLAMP_TO_BORDER
    // GL_CLAMP_TO_EDGE
    // GL_MIRRORED_REPEAT
    return
        (GL_CLAMP  == w) ||
        (GL_REPEAT == w) ;
}

  bool
is_auto_texture_gen_mode( enum_type m)
{
    // GL_REFLECTION_MAP
    // GL_NORMAL_MAP
    return
        (GL_OBJECT_LINEAR == m) ||
        (GL_EYE_LINEAR    == m) ||
        (GL_SPHERE_MAP    == m) ;
}

  bool
is_texture_env_mode( enum_type m)
{
    // A lot of these values are defined and work, but they're
    // not suppored in 1.1.
    //  GL_COMBINE
    //  GL_ADD_SIGNED
    //  GL_INTERPOLATE
    //  GL_SUBTRACT
    //  GL_TEXTURE
    //  GL_CONSTANT
    //  GL_PRIMARY_COLOR
    //  GL_PREVIOUS
    //  GL_SRC_COLOR
    //  GL_ONE_MINUS_SRC_COLOR
    //  GL_SRC_ALPHA
    //  GL_ONE_MINUS_SRC_ALPHA
    return
        (GL_ADD      == m) || // works, maybe not 1.1?
        (GL_MODULATE == m) ||
        (GL_DECAL    == m) ||
        (GL_BLEND    == m) ||
        (GL_REPLACE  == m) ;
}

  bool
is_texture_internal_format( enum_type f)
{
    // GL_COMPRESSED_ALPHA
    // GL_COMPRESSED_LUMINANCE
    // GL_COMPRESSED_LUMINANCE_ALPHA
    // GL_COMPRESSED_INTENSITY
    // GL_COMPRESSED_RGB
    // GL_COMPRESSED_RGBA
    // GL_DEPTH_COMPONENT16
    // GL_DEPTH_COMPONENT24
    // GL_DEPTH_COMPONENT32
    // GL_SLUMINANCE
    // GL_SLUMINANCE8
    // GL_SLUMINANCE_ALPHA
    // GL_SLUMINANCE8_ALPHA8
    // GL_SRGB
    // GL_SRGB8
    // GL_SRGB_ALPHA
    // GL_SRGB8_ALPHA8
    return
        (1                      == f) || // GL_LUMINANCE
        (2                      == f) || // GL_LUMINANCE_ALPHA
        (3                      == f) || // GL_RGB
        (4                      == f) || // GL_RGBA
        (GL_ALPHA               == f) ||
        (GL_ALPHA4              == f) ||
        (GL_ALPHA8              == f) ||
        (GL_ALPHA12             == f) ||
        (GL_ALPHA16             == f) ||
        (GL_DEPTH_COMPONENT     == f) ||
        (GL_LUMINANCE           == f) ||
        (GL_LUMINANCE4          == f) ||
        (GL_LUMINANCE8          == f) ||
        (GL_LUMINANCE12         == f) ||
        (GL_LUMINANCE16         == f) ||
        (GL_LUMINANCE_ALPHA     == f) ||
        (GL_LUMINANCE4_ALPHA4   == f) ||
        (GL_LUMINANCE6_ALPHA2   == f) ||
        (GL_LUMINANCE8_ALPHA8   == f) ||
        (GL_LUMINANCE12_ALPHA4  == f) ||
        (GL_LUMINANCE12_ALPHA12 == f) ||
        (GL_LUMINANCE16_ALPHA16 == f) ||
        (GL_INTENSITY           == f) ||
        (GL_INTENSITY4          == f) ||
        (GL_INTENSITY8          == f) ||
        (GL_INTENSITY12         == f) ||
        (GL_INTENSITY16         == f) ||
        (GL_R3_G3_B2            == f) ||
        (GL_RGB                 == f) ||
        (GL_RGB4                == f) ||
        (GL_RGB5                == f) ||
        (GL_RGB8                == f) ||
        (GL_RGB10               == f) ||
        (GL_RGB12               == f) ||
        (GL_RGB16               == f) ||
        (GL_RGBA                == f) ||
        (GL_RGBA2               == f) ||
        (GL_RGBA4               == f) ||
        (GL_RGB5_A1             == f) ||
        (GL_RGBA8               == f) ||
        (GL_RGB10_A2            == f) ||
        (GL_RGBA12              == f) ||
        (GL_RGBA16              == f) ;
}

  bool
is_blending_factor__common( enum_type f)
{
    return
        // GL_CONSTANT_COLOR            - ARB_imaging or 1.4
        // GL_ONE_MINUS_CONSTANT_COLOR  - ARB_imaging or 1.4
        // GL_CONSTANT_ALPHA            - ARB_imaging or 1.4
        // GL_ONE_MINUS_CONSTANT_ALPHA  - ARB_imaging or 1.4
        (GL_ZERO                == f) ||
        (GL_ONE                 == f) ||
        (GL_SRC_ALPHA           == f) ||
        (GL_ONE_MINUS_SRC_ALPHA == f) ||
        (GL_DST_ALPHA           == f) ||
        (GL_ONE_MINUS_DST_ALPHA == f) ;
}

  bool
is_blending_factor__src( enum_type f)
{
    return is_blending_factor__common( f) ||
        // GL_SRC_COLOR           - 1.4
        // GL_ONE_MINUS_SRC_COLOR - 1.4
        (GL_DST_COLOR           == f) ||
        (GL_ONE_MINUS_DST_COLOR == f) ||
        (GL_SRC_ALPHA_SATURATE  == f) ;
}

  bool
is_blending_factor__dst( enum_type f)
{
    return is_blending_factor__common( f) ||
        // GL_DST_COLOR           - 1.4
        // GL_ONE_MINUS_DST_COLOR - 1.4
        (GL_SRC_COLOR           == f) ||
        (GL_ONE_MINUS_SRC_COLOR == f) ;
}

// _______________________________________________________________________________________________
//
} /* end namespace gl_env */
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_env_type_primitives.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
