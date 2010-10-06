// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_env_type_primitives.h
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
# ifndef GL_ENV_TYPE_PRIMITIVES_H
# define GL_ENV_TYPE_PRIMITIVES_H
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// We need some decls from gl.h and GLee.h. GLee includes gl.h for us.
# include "GLee.h"

// GLee.h unfortunately includes <windows.h> under WIN32.
// <windows.h> is big and messy and I like to avoid including it whenever possible.
// We could do the following instead (it sure speeds up compiles):
# if 0
# ifdef WIN32
    // If we do this we have to include GLee.h in gl_env.cpp and probably gl_shader.cpp.
#   define DECLSPEC_IMPORT  __declspec(dllimport)
#   define WINGDIAPI        DECLSPEC_IMPORT
#   define CALLBACK         __stdcall
#   define WINAPI           __stdcall
#   define APIENTRY         WINAPI

#   include <GL/gl.h>
    // You can include gl.h thru GLee.h.
    // You can also include it through QT, with <qgl.h>.

    // The wgl* functions [like ::wglMakeCurrent(..)] are in <WinGDI.h>. But we don't need them here.
# else
#   include "GLee.h"
# endif

typedef char  GLchar      ;
typedef char  GLcharARB   ;
typedef int   GLhandleARB ;
// GLhandleARB is "unsigned int" in glext.h in the Mesa GL headers on my Linux Ubuntu.
// But GLee.h is careful not to include glext.h in that case and requires GLhandleARB
// to be "int" instead.
# endif

// Since GLee.h includes <windows.h>, the min and max macros can creep in here and we
// want to #undef them. They come in thru <WinDef.h> via <windows.h>.
// Some QT header files also include <windows.h>.
# ifdef min
#   undef min
# endif
# ifdef max
#   undef max
# endif

// Under X11 (Linux and Unix) and the Mesa GL headers, GLee.h brings in system files the
// #define Status and CursorShape. But these are enums in Qt. Most Qt includes break after
// GLee.h unless you do the following:
# ifdef Status
#   undef Status
# endif
# ifdef CursorShape
#   undef CursorShape
# endif

// Glu has utilities for setting up the prespective matrix, among other things.
//# include <GL/glu.h>

// Glut is useful for tests. But avoid using it in release code because you will then have to
// install a glut DLL.
//# include <GL/glut.h>

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
namespace gl_env {
// _______________________________________________________________________________________________

// ---------------------------------------------------------------------------------------------
// GL primitive types

typedef GLenum          enum_type       ;
typedef GLboolean       bool_type       ;
typedef GLbitfield      bitfield_type   ;
typedef GLbyte          byte_type       ;
typedef GLubyte         ubyte_type      ;
typedef GLchar          char_type       ;
typedef GLshort         short_type      ;
typedef GLushort        ushort_type     ;
typedef GLint           int_type        ;
typedef GLuint          uint_type       ;
typedef GLsizei         size_type       ;
typedef GLfloat         float_type      ;
typedef GLdouble        double_type     ;

//typedef GLvoid          void_type       ; // use void instead
//typedef GLclampf        clampf_type     ; // use float_type instead
//typedef GLclampd        clampd_type     ; // use double_type instead

// _______________________________________________________________________________________________

  template< typename T >
  struct
type_traits
  ;

  template< >
  struct
type_traits< byte_type >
  {
    static bool      const  is_integer    = true;
    static bool      const  is_signed     = true;
    static bool      const  has_wider     = true;
    static bool      const  has_narrower  = false;
    static size_t    const  alignment     = 1;
    static enum_type const  data_type     = GL_BYTE;

    typedef ubyte_type      u_type    ;
    typedef short_type      wider_type;
  };

  template< >
  struct
type_traits< ubyte_type >
  {
    static bool      const  is_integer    = true;
    static bool      const  is_signed     = false;
    static bool      const  has_wider     = true;
    static bool      const  has_narrower  = false;
    static size_t    const  alignment     = 1;
    static enum_type const  data_type     = GL_UNSIGNED_BYTE;

    typedef byte_type       s_type    ;
    typedef ushort_type     wider_type;
  };

// ------------------------------------------------------------------

  template< >
  struct
type_traits< short_type >
  {
    static bool      const  is_integer    = true;
    static bool      const  is_signed     = true;
    static bool      const  has_wider     = true;
    static bool      const  has_narrower  = true;
    static size_t    const  alignment     = 2;
    static enum_type const  data_type     = GL_SHORT;

    typedef ushort_type     u_type       ;
    typedef byte_type       narrower_type;
    typedef int_type        wider_type   ;
  };

  template< >
  struct
type_traits< ushort_type >
  {
    static bool      const  is_integer    = true;
    static bool      const  is_signed     = false;
    static bool      const  has_wider     = true;
    static bool      const  has_narrower  = true;
    static size_t    const  alignment     = 2;
    static enum_type const  data_type     = GL_UNSIGNED_SHORT;

    typedef short_type      s_type       ;
    typedef ubyte_type      narrower_type;
    typedef uint_type       wider_type   ;
  };

// ------------------------------------------------------------------

  template< >
  struct
type_traits< int_type >
  {
    static bool      const  is_integer    = true;
    static bool      const  is_signed     = true;
    static bool      const  has_wider     = false;
    static bool      const  has_narrower  = true;
    static size_t    const  alignment     = 4;
    static enum_type const  data_type     = GL_INT;

    typedef uint_type       u_type       ;
    typedef short_type      narrower_type;
  };

  template< >
  struct
type_traits< uint_type >
  {
    static bool      const  is_integer    = true;
    static bool      const  is_signed     = false;
    static bool      const  has_wider     = false;
    static bool      const  has_narrower  = true;
    static size_t    const  alignment     = 4;
    static enum_type const  data_type     = GL_UNSIGNED_INT;

    typedef int_type        s_type       ;
    typedef ushort_type     narrower_type;
  };

// ------------------------------------------------------------------

  template< >
  struct
type_traits< float_type >
  {
    static bool      const  is_integer    = false;
    static bool      const  is_signed     = true;
    static bool      const  has_wider     = true;
    static bool      const  has_narrower  = false;
    static size_t    const  alignment     = 4;
    static enum_type const  data_type     = GL_FLOAT;

    typedef double_type     wider_type;
  };

  template< >
  struct
type_traits< double_type >
  {
    static bool      const  is_integer    = false;
    static bool      const  is_signed     = true;
    static bool      const  has_wider     = false;
    static bool      const  has_narrower  = true;
    static size_t    const  alignment     = 8;
    static enum_type const  data_type     = GL_DOUBLE;

    typedef float_type      narrower_type;
  };

// _______________________________________________________________________________________________
// enum subsets:
//
//   Every function tha accepts an enum_type defines which enums are allowed.
//   There are lots of different types, with lots of overlap.
//   Ideally we would identify every group and wrap it in a type.
//
//   Here are the major enum groups (from GL.h, GL version 1.1 only).
//   Some of these overlap. Some of these can be divided further because some functions only accept
//   a subset of one of these groups.
//      AccumOp
//      AlphaFunction
//      AttribMask
//      BeginMode (aka vertex-entry)
//      BlendingFactorDest
//      BlendingFactorSrc
//      Boolean - low-level artifact, should always be abstracted away
//      ClearBufferMask
//      ClientArrayType
//      ClipPlaneName
//      ColorMaterialFace
//      ColorMaterialParameter
//      ColorPointerType
//      CullFaceMode
//      DataType
//      DepthFunction
//      DrawBufferMode
//      Enable - large set
//      ErrorCode
//      FeedBackMode
//      FeedBackToken
//      FogMode
//      FogParameter
//      FrontFaceDirection
//      GetMapTarget
//      GetPixelMap
//      GetPointerTarget
//      GetTarget - very large group, every enum accepted by glGet.
//      GetTextureParameter
//      HintMode
//      HintTarget
//      IndexPointerType
//      LightModelParameter
//      LightName - really an index, not a set of names
//      LightParameter
//      InterleavedArrays
//      ListMode
//      ListNameType
//      LogicOp
//      MapTarget
//      MaterialFace
//      MaterialParameter
//      MatrixMode
//      MeshMode1
//      MeshMode2
//      NormalPointerType
//      PixelCopyType
//      PixelFormat
//      PixelMap
//      PixelStore
//      PixelTransfer
//      PixelType
//      PolygonMode
//      ReadBufferMode
//      RenderingMode
//      ShadingModel
//      StencilFunction
//      StencilOp
//      StringName
//      TextureCoordName
//      TexCoordPointerType
//      TextureEnvMode
//      TextureEnvParameter
//      TextureEnvTarget
//      TextureGenMode
//      TextureGenParameter
//      TextureMagFilter
//      TextureMinFilter
//      TextureParameterName
//      TextureTarget
//      TextureWrapMode
//      VertexPointerType
//
//      ClientAttribMask - bitmask
//      polygon_offset
//      texture
//      texture_object
//      vertex_array

bool  is_data_type(                 enum_type) ;
bool  is_hint_trg(                  enum_type) ;
bool  is_hint_mode(                 enum_type) ;
bool  is_vertex_entry_mode(         enum_type) ;
bool  is_material_face__get(        enum_type) ;
bool  is_material_face__set(        enum_type) ;
bool  is_color_facet__get(          enum_type) ;
bool  is_color_facet__set(          enum_type) ;
bool  is_1D_pixel_format(           enum_type) ;
bool  is_pixel_format(              enum_type) ;
bool  is_read_write_pixel_format(   enum_type) ;
bool  is_pixel_data_type(           enum_type) ;
bool  is_pixel_map_type(            enum_type) ;
bool  is_pixel_transfer_param(      enum_type) ;
bool  is_copy_pixel_src(            enum_type) ;
bool  is_texture_param__set_1(      enum_type) ;
bool  is_texture_param__get_1(      enum_type) ;
bool  is_texture_param__N(          enum_type) ;
bool  is_texture_level_param(       enum_type) ;
bool  is_texture_minifying_filter(  enum_type) ;
bool  is_texture_magnifying_filter( enum_type) ;
bool  is_texture_wrap(              enum_type) ;
bool  is_auto_texture_gen_mode(     enum_type) ;
bool  is_texture_env_mode(          enum_type) ;
bool  is_texture_internal_format(   enum_type) ;
bool  is_blending_factor__common(   enum_type) ;
bool  is_blending_factor__src(      enum_type) ;
bool  is_blending_factor__dst(      enum_type) ;

// _______________________________________________________________________________________________
//
} /* end namespace gl_env */
// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# endif // ifndef GL_ENV_TYPE_PRIMITIVES_H
//
// gl_env_type_primitives.h - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
