// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// shader.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// The standard Blinn-Phong shading used by OpenGL is:
//   Vertex Color =
//     materialEmission +
//     (globalAmbient * materialAmbient) +
//     sum for every light {
//       attenuation * spotlight *
//       ( (lightAmbient  * materialAmbient ) +
//         (lightDiffuse  * materialDiffuse ) * max( 0, normal . surface_to_light) +
//         (lightSpecular * materialSpecular) * (max( 0, normal . half_vector) ^ shininess)
//       )
//     }
//
// References:
//   http://www.ozone3d.net/tutorials/glsl_lighting_phong.php
//   http://www.opengl.org/sdk/docs/tutorials/ClockworkCoders/lighting.php
// _______________________________________________________________________________________________

# include "all.h"
# include "shader.h"
# include <QtGui/QMessageBox>

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// GLSL (1.10) Vertex and Fragment shaders:
//   Global ambient
//   Two directional lights, gl_LightSource[ 0 ] and gl_LightSource[ 1 ].
//   Front and back sides showing
//   Front and back material have same ambient color, gl_Color.
//   Front and back material have same diffuse color, also gl_Color.
//   Color alpha is always 1.
//
// Shading model:
//   Blinn–Phong shading model (also called Blinn–Phong reflection model or modified Phong reflection model)
//   See http://en.wikipedia.org/wiki/Blinn%E2%80%93Phong_shading_model

// The vertex and fragment shaders are shown here, between "#if 0" and "# endif" because they
// are not C++ and not compiled here (they are compiled by OpenGL when the program is running).
// Exactly the same vertex shader, stripped of comments and made into strings, appears later.
# if 0
// _______________________________________________________________________________________________
// GLSL (1.20) Vertex shader:

# version 120

uniform sampler1D  u_isotherm_texture      ; // only use one component, as a luminance
uniform vec3       u_mix_color             ;
uniform float      u_z_factor              ; // this is zero if we should not show isotherms
uniform float      u_z_offset              ;

varying vec3       v_normal_vector_any_len ; // normal, for diffuse and specular light
varying vec3       v_ambient_color         ; // ambient color product, both sides
varying vec3       v_diffuse_color_0       ; // diffuse color product, both sides
varying vec3       v_diffuse_color_1       ; // diffuse color product, both sides
varying vec3       v_half_vector_0_any_len ; // for specular light 0, average to-eye and to-light vectors
varying vec3       v_half_vector_1_any_len ; // for specular light 1

varying float      v_isotherm_texture_s    ; // s-coord, for isotherm bars

  void
main( )
{
    // S-coord into isotherm texture.
    v_isotherm_texture_s = (gl_Vertex.z * u_z_factor) + u_z_offset;

    // We need the normal vector in the camera (mv) coord system.
    // We don't need to normalize it now because we normalize in the fragment shader.
    v_normal_vector_any_len = gl_NormalMatrix * gl_Normal;

    // The front and back of the materials have the same ambient color, passed in with glColor*(..)
    // or gl_env::global::drop_color(..).
    // We could use gl_FrontMaterial.ambient and gl_BackMaterial.ambient if we wanted to use material colors.
    // This is the same as (gl_FrontLightModelProduct.sceneColor.rgb + gl_FrontLightProduct[0].ambient.rgb)
    // (or Back) except those use material colors instead of gl_Color.
    // We don't clamp this even though these colors may be greater than 1. The fragment shader should take
    // care of clamping -- clamping here will ruin the interpolation.
    v_ambient_color = gl_Color.rgb;
    v_ambient_color *= (gl_LightModel.ambient.rgb +
                          gl_LightSource[ 0 ].ambient.rgb +
                          gl_LightSource[ 1 ].ambient.rgb );
    // Don't bother adding emission (gl_FrontMaterial.emission.rgb or ..Back..).

    // The front and back hae the same diffuse material color.
    // We need separate diffuse colors for each of the lights.
    // This component is multiplied by normal-dot-light in the fragment shader.
    // This is the same as gl_FrontLightProduct[0].diffuse.rgb (or ..Back..) except those use
    // material colors instead of gl_Color.
    v_diffuse_color_0 = (gl_Color.rgb * gl_LightSource[ 0 ].diffuse.rgb);
    v_diffuse_color_1 = (gl_Color.rgb * gl_LightSource[ 1 ].diffuse.rgb);

    // The half-vector is halfway between the surface-to-eye (camera) and surface-to-light vectors.
    // Each light has a different half vector.
    // This value is also available in the fragment shader, but we want it varying.
    // Calculating this is pretty involved, involving ortho-vs-perspective projections
    // and point-vs-directional lighting. See notes below.
    v_half_vector_0_any_len = gl_LightSource[ 0 ].halfVector.xyz;
    v_half_vector_1_any_len = gl_LightSource[ 1 ].halfVector.xyz;

    // We only need the clip-vertex if user-defined clip planes are enabled.
    // Sometimes they are, and sometimes they aren't.
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;

    // Finally compute the vertex position in mv space.
    // ftransform( ) is deprecated in 1.40.
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

// _______________________________________________________________________________________________
// GLSL (1.10) Fragment shader:

# version 120

uniform sampler1D  u_isotherm_texture      ;
uniform vec3       u_mix_color             ;
uniform float      u_z_factor              ;
uniform float      u_z_offset              ;

// Interpolated from the vertex shader.
varying vec3       v_normal_vector_any_len ;
varying vec3       v_ambient_color         ;
varying vec3       v_diffuse_color_0       ;
varying vec3       v_diffuse_color_1       ;
varying vec3       v_half_vector_0_any_len ;
varying vec3       v_half_vector_1_any_len ;

varying float      v_isotherm_texture_s    ; // s-coord, for isotherm bars

// Calculated in the functions below.
vec3  normal_vector_unit_len  = vec3( 0.0, 0.0, 1.0);
float normal_dot_light_0      = 0.0;
float normal_dot_light_1      = 0.0;
float normal_dot_half_0       = 0.0;
float normal_dot_half_1       = 0.0;

  bool
calc_normal_vector_unit_len( )
{
    // It is very rare, but not impossible for the interpolated normal vector to be zero.
    // If the original normals were unit, the interpolated length would usually be just a little less,
    // since we're tracing a chord inside a unit circle.
    // (We're tracing a chord inside an ellipse since we're interpolating non-unit normals.)
    // If we run into almost-zero it probably means we're edge-on. Treat the dot-products as zero.
    float length_normal_vector  = length( v_normal_vector_any_len);
    if ( length_normal_vector < 0.000001 ) return false;

    // If we're on the back side of the material we have to reverse the normal.
    normal_vector_unit_len = gl_FrontFacing ?
                                  v_normal_vector_any_len :
                                - v_normal_vector_any_len ;
    normal_vector_unit_len /= length_normal_vector;
    return true;
}

  float
calc_normal_dot_product_unit_length( vec3 other_vector_unit_len)
{
    // Calc the dot product. We're only interested if it is > 0. It is often negative.
    float normal_dot_product = dot( normal_vector_unit_len, other_vector_unit_len);
    return max( 0.0, normal_dot_product);
}

  float
calc_normal_dot_product_any_length( vec3 other_vector_any_len)
{
    float normal_dot_product = dot( normal_vector_unit_len, other_vector_any_len);
    if ( normal_dot_product <= 0.000001 ) return 0.0;

    // The dot product would be zero if other_vector_any_len was zero length.
    // So we can safely divide by it.
    // And we don't have to min( 1.0, dot_product) because it should already be <= 1
    // (unless rounding gives us a value slightly more than 1).
    return normal_dot_product / length( other_vector_any_len);
}

  void
calc_dot_products( )
  //
  // Calculates the dot products between the normal and the light- and half-vectors.
  // The calculated dot products are in the range [0..1]. Negative dot products
  // are set to zero.
{
    if ( calc_normal_vector_unit_len( ) ) {

        // In our application we know that the light is directional (the z component is zero)
        // and position.xyz is a unit vector.
        //
        // With a directional light, position.xyz is the surface-to-light vector.
        // With a point light, (position.xyz - surface_position) is the surface-to-light vector.
        // surface_position would be a varying set to vec3( gl_ModelViewMatrix * gl_Vertex) in the
        // vertex shader.
        //
        // If we couldn't assume light.position.xyz was unit-length we'd do this:
        // float length_light_vector = length( gl_LightSource[ 0 ].position.xyz);

        normal_dot_light_0 = calc_normal_dot_product_unit_length( gl_LightSource[ 0 ].position.xyz);
        normal_dot_light_1 = calc_normal_dot_product_unit_length( gl_LightSource[ 1 ].position.xyz);

        // The half-vector is half-way between the camera vector and the light vector.
        // I don't know if it's ever zero, although it's easy to imagine an arrangment of
        // eye+material+light all in a line that would give us a zero half-vector.
        // Or an arrangement where interpolation would give us a zero.

        normal_dot_half_0 = calc_normal_dot_product_any_length( v_half_vector_0_any_len);
        normal_dot_half_1 = calc_normal_dot_product_any_length( v_half_vector_1_any_len);
    }
}

  void
main( )
{
    // Calculate the globals:
    //   normal_dot_light_0
    //   normal_dot_light_1
    //   normal_dot_half_0
    //   normal_dot_half_1
    calc_dot_products( );

    // Calculate the shininess value for this fragment.
    // Remember, the front and back shininess values can be different, but the front/back specular
    // colors are the same - they are both white.
    float material_shininess =
            gl_FrontFacing ?
                gl_FrontMaterial.shininess :
                gl_BackMaterial .shininess ;

    // Calculate components and sum into final_color.
    // Start with the global ambient and ambient from the light.
    vec3 final_color = v_ambient_color;

    // Add in the diffuse colors.
    final_color += normal_dot_light_0 * v_diffuse_color_0;
    final_color += normal_dot_light_1 * v_diffuse_color_1;

    // Add specular light.
    if ( normal_dot_half_0 > 0.0 ) {
        // We don't need to clamp this. (normal_dot_half <= 1) and (shininess >= 0).
        float exponent_0 = pow( normal_dot_half_0, material_shininess);
        // Assume gl_FrontMaterial.specular.rgb (and ..Back..) is (1,1,1) or white.
        //   final_color += gl_FrontMaterial.specular.rgb * gl_LightSource[ 0 ].specular.rgb * exponent;
        // We could use gl_FrontLightProduct[ 0 ].specular.rgb or gl_BackLightProduct[ 0 ].specular.rgb instead,
        // but that's what the standard pipeline does and defeats the purpose of this shader.
        final_color += gl_LightSource[ 0 ].specular.rgb * exponent_0;
    }

    // Repeat the above calculation for light 1.
    if ( normal_dot_half_1 > 0.0 ) {
        float exponent_1 = pow( normal_dot_half_1, material_shininess);
        final_color += gl_LightSource[ 1 ].specular.rgb * exponent_1;
    }

    // Pre-texture color.
    final_color = min( final_color, 1.0);

    // Apply texture.
    if ( u_z_factor != 0 ) {
        float luminance = texture1D( u_isotherm_texture, v_isotherm_texture_s).r;
        final_color = mix( final_color, u_mix_color, luminance);
    }

    // Always assign an alpha of 1.
    gl_FragColor = vec4( final_color, 1.0);
}

# endif

// _______________________________________________________________________________________________
// How to calculate the half-vector for Blinn-Phong shading
//
//   The half-vector is the average of these two vectors (in the mv coord system):
//     surface-vertex --> light-position
//     surface-vertex --> eye-position
//   Eye-position is the origin (0,0,0) with a perspective projection.
//   Surface vertex (position) is (gl_ModelViewMatrix * gl_Vertex).xyz;
//
//   We use the pre-calculated gl_LightSource[N].halfVector.xyz vector so we don't have to
//   calculate it ourselves.
//
//   The half-vector is constant (and we don't need it varying) if:
//        The light is directional (and not a point source), and
//        The projection is orthogonal.
//
//   The surface-to-light vector is constant if the light is directional.
//   The surface-to-eye vector is constant if the projection is ortho.
//
//   For an ortho projection,      (1 == gl_ProjectionMatrix[ 3 ][ 3 ])
//   For a perspective projection, (0 == gl_ProjectionMatrix[ 3 ][ 3 ])
//
//   For a point-source light, (1 == gl_LightSource[0].position.w)
//   For a directional light,  (0 == gl_LightSource[0].position.w)
//
//   How to calc the half vector, point light, perspective projection:
//     vec3 surface_to_eye_vector   = normalize( - surface_position_in_mv.xyz);
//     vec3 surface_to_light_vector = normalize( gl_LightSource[0].position.xyz - surface_position_in_mv.xyz);
//     vec3 half_vector_any_length  = surface_to_light_vector + surface_to_eye_vector;
//
//   How to calc the half vector, point light, ortho projection:
//     vec3 surface_to_eye_vector   = vec3( 0, 0, 1);
//     vec3 surface_to_light_vector = normalize( gl_LightSource[0].position.xyz - surface_position_in_mv.xyz);
//     vec3 half_vector_any_length  = surface_to_light_vector + surface_to_eye_vector;
//
//   How to calc the half vector, directional light, perspective projection:
//     vec3 surface_to_eye_vector   = normalize( - surface_position_in_mv.xyz);
//     vec3 surface_to_light_vector = normalize( gl_LightSource[0].position.xyz);
//     vec3 half_vector_any_length  = surface_to_light_vector + surface_to_eye_vector;
//
//   How to calc the half vector, directional light, ortho projection:
//     vec3 surface_to_eye_vector   = vec3( 0, 0, 1);
//     vec3 surface_to_light_vector = normalize( gl_LightSource[0].position.xyz);
//     vec3 half_vector_any_length  = surface_to_light_vector + surface_to_eye_vector;
//
//   In all of these you must watch out for zero vectors.

// _______________________________________________________________________________________________
// Notes on the Phong (not Blinn-Phong) shading model
//
//   Phong shading is similar to the standard OpenGL Blinn-Phong model.
//   They only differ in how specular lighting is calculated.
//   Blinn-Phong is implemented in the shader above.

# if 0
// This is what Phong shaders look like:
varying vec3 v_material_surface;

.. in the vertex shader ..
v_material_surface = vec3( gl_ModelViewMatrix * gl_Vertex).xyz;

.. in the fragment shader ..
// The first is for directional lights, the second for point lights.
// For directional lights surface_to_light is constant and should probable be calculated and
// normalized ahead of time, and passed in as a uniform.
vec3 surface_to_light = normalize( gl_LightSource[ 0 ].position.xyz); /* wasteful, not varying! */
vec3 surface_to_light = normalize( gl_LightSource[ 0 ].position.xyz - v_material_surface);

// This only works for perspective projections, NOT for orthogonal projections.
// For ortho projections surface_to_eye is const and should be passed in as a uniform.
// Or maybe it's just always (0,0,1)? I think it is.
// Remember the eye (or camera) is at the origin for perspective projections.
vec3 surface_to_eye = normalize( - v_material_surface);

// surface_to_reflect is surface_to_light reflected around the normal (like it would on
// a mirror surface).
//
// reflect( L, N) is (L - 2 * dot( L, N) * N).
// reflect(..) needs the 2nd param to be normalized, but not the first.
// reflect(..) treats the 2nd param as a surface TANGENT, not a NORMAL. Which is why
// we use (- reflect(..)), which is a reflection around the normal.
vec3 surface_to_reflect = normalize( -reflect( surface_to_light, surface_normal));

// The ambient and diffuse (and emissive) terms are the same for Phong and Blinn-Phong.
// Here are the front-face calculations. The back-face calculations are similar. See gl_FrontFacing.
gl_FrontLightModelProduct.sceneColor.rgb +
gl_FrontLightProduct[0].ambient.rgb +
gl_FrontLightProduct[0].diffuse.rgb * max( dot( surface_to_light, surface_normal), 0.0) +

// The specular term is different in the two shading models.
// The 0.3 is there to help match this (Phong) specular term with the term calculated in Blinn-Phong.
gl_FrontLightProduct[0].specular *
    pow( max( dot( surface_to_reflect, surface_to_eye), 0.0), (0.3 * gl_FrontMaterial.shininess))

// Off topic: To get a texture color:
uniform sampler2D texture;
gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0; /* vertex shader */
vec3 tx = texture2D( texture, gl_TexCoord[0].st).rgb; /* frag shader */

// Off topic: How to do attenuation in a frag shader (point light only):
vec3 lightVector = gl_LightSource[0].position.xyz - v_material_surface;
float dist = length( lightVector);
float attenuation = 1.0 / (
        gl_LightSource[0].constantAttenuation +
        gl_LightSource[0].linearAttenuation * dist +
        gl_LightSource[0].quadraticAttenuation * dist * dist);
// Multiply diffuse and specular light contributions (for this light) by attenuation.
// Probably multiply ambient contribution too.

# endif

// _______________________________________________________________________________________________

namespace /* anonymous */ {

  GLchar const * const /* or GLcharARB const * const */
c_src_string__vertex_shader__blinn_phong__light =

    "# version 120 \n"

    "uniform float u_z_factor; \n"
    "uniform float u_z_offset; \n"

    "varying vec3 v_normal_vector_any_len ; \n"
    "varying vec3 v_ambient_color         ; \n"
    "varying vec3 v_diffuse_color_0       ; \n"
    "varying vec3 v_diffuse_color_1       ; \n"
    "varying vec3 v_half_vector_0_any_len ; \n"
    "varying vec3 v_half_vector_1_any_len ; \n"

    "varying float v_isotherm_texture_s; \n"

      "void \n"
    "main( ) \n"
    "{ \n"
        "v_isotherm_texture_s = (gl_Vertex.z * u_z_factor) + u_z_offset; \n"

        "v_normal_vector_any_len = gl_NormalMatrix * gl_Normal; \n"
        "v_ambient_color = gl_Color.rgb; \n"
        "v_ambient_color *= (gl_LightModel.ambient.rgb + \n"
                              "gl_LightSource[ 0 ].ambient.rgb + \n"
                              "gl_LightSource[ 1 ].ambient.rgb ); \n"
        "v_diffuse_color_0 = (gl_Color.rgb * gl_LightSource[ 0 ].diffuse.rgb); \n"
        "v_diffuse_color_1 = (gl_Color.rgb * gl_LightSource[ 1 ].diffuse.rgb); \n"
        "v_half_vector_0_any_len = gl_LightSource[ 0 ].halfVector.xyz; \n"
        "v_half_vector_1_any_len = gl_LightSource[ 1 ].halfVector.xyz; \n"
        "gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex; \n"
        "gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; \n"
    "} \n";

// _______________________________________________________________________________________________

  GLchar const * const /* or GLcharARB const * const */
c_src_string__fragment_shader__blinn_phong__light =

    "# version 120 \n"

    "uniform sampler1D u_isotherm_texture; \n"
    "uniform vec3 u_mix_color; \n"
    "uniform float u_z_factor; \n"

    "varying vec3 v_normal_vector_any_len ; \n"
    "varying vec3 v_ambient_color         ; \n"
    "varying vec3 v_diffuse_color_0       ; \n"
    "varying vec3 v_diffuse_color_1       ; \n"
    "varying vec3 v_half_vector_0_any_len ; \n"
    "varying vec3 v_half_vector_1_any_len ; \n"

    "varying float v_isotherm_texture_s; \n"

    "vec3  normal_vector_unit_len = vec3( 0.0, 0.0, 1.0); \n"
    "float normal_dot_light_0     = 0.0; \n"
    "float normal_dot_light_1     = 0.0; \n"
    "float normal_dot_half_0      = 0.0; \n"
    "float normal_dot_half_1      = 0.0; \n"

      "bool \n"
    "calc_normal_vector_unit_len( ) \n"
    "{ \n"
        "float length_normal_vector  = length( v_normal_vector_any_len); \n"
        "if ( length_normal_vector < 0.000001 ) return false; \n"

        "normal_vector_unit_len = gl_FrontFacing ? \n"
                                    "  v_normal_vector_any_len : \n"
                                    "- v_normal_vector_any_len ; \n"
        "normal_vector_unit_len /= length_normal_vector; \n"
        "return true; \n"
    "} \n"

      "float \n"
    "calc_normal_dot_product_unit_length( vec3 other_vector_unit_len) \n"
    "{ \n"
        "float normal_dot_product = dot( normal_vector_unit_len, other_vector_unit_len); \n"
        "return max( 0.0, normal_dot_product); \n"
    "} \n"

      "float \n"
    "calc_normal_dot_product_any_length( vec3 other_vector_any_len) \n"
    "{ \n"
        "float normal_dot_product = dot( normal_vector_unit_len, other_vector_any_len); \n"
        "if ( normal_dot_product <= 0.000001 ) return 0.0; \n"

        "return normal_dot_product / length( other_vector_any_len); \n"
    "} \n"

      "void \n"
    "calc_dot_products( ) \n"
    "{ \n"
        "if ( calc_normal_vector_unit_len( ) ) { \n"
            "normal_dot_light_0 = calc_normal_dot_product_unit_length( gl_LightSource[ 0 ].position.xyz); \n"
            "normal_dot_light_1 = calc_normal_dot_product_unit_length( gl_LightSource[ 1 ].position.xyz); \n"
            "normal_dot_half_0 = calc_normal_dot_product_any_length( v_half_vector_0_any_len); \n"
            "normal_dot_half_1 = calc_normal_dot_product_any_length( v_half_vector_1_any_len); \n"
        "} \n"
    "} \n"

      "void \n"
    "main( ) \n"
    "{ \n"
        "calc_dot_products( ); \n"
        "float material_shininess = \n"
                "gl_FrontFacing ? \n"
                    "gl_FrontMaterial.shininess : \n"
                    "gl_BackMaterial .shininess ; \n"

        "vec3 final_color = v_ambient_color; \n"
        "final_color += normal_dot_light_0 * v_diffuse_color_0; \n"
        "final_color += normal_dot_light_1 * v_diffuse_color_1; \n"

        "if ( normal_dot_half_0 > 0.0 ) { \n"
            "float exponent_0 = pow( normal_dot_half_0, material_shininess); \n"
            "final_color += gl_LightSource[ 0 ].specular.rgb * exponent_0; \n"
        "} \n"
        "if ( normal_dot_half_1 > 0.0 ) { \n"
            "float exponent_1 = pow( normal_dot_half_1, material_shininess); \n"
            "final_color += gl_LightSource[ 1 ].specular.rgb * exponent_1; \n"
        "} \n"

        "final_color = min( final_color, 1.0); \n"
        "if ( u_z_factor != 0 ) { \n"
            "float luminance = texture1D( u_isotherm_texture, v_isotherm_texture_s).r; \n"
            "final_color = mix( final_color, u_mix_color, luminance); \n"
        "} \n"

        "gl_FragColor = vec4( final_color, 1.0); \n"
    "} \n";

// _______________________________________________________________________________________________

# if 0
  program_owner_any_type
shader_program__reset( 0, 0);
  // This is the default pipeline shader program.
  // You can activate this like any shader.
# endif

} /* end anonymous namespace */

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

typedef gl_env::int_type                             int_type                    ;
typedef gl_env::float_type                           float_type                  ;
typedef gl_env::rgb_type< float_type >               color_type                  ;

typedef gl_env::shader::program_owner_any_type       program_owner_any_type      ;
typedef gl_env::shader::program_holder_variant_type  program_holder_variant_type ;

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
blinn_phong_program_owner_type
{
  public:
    /* ctor */    blinn_phong_program_owner_type( )   ;
    virtual       ~blinn_phong_program_owner_type( )  { }

  public:
    program_holder_variant_type
                  get_program_holder( )               ;

    void          set_uniform_values
                   (  bool                is_showing
                    , float_type          z_factor
                    , float_type          z_offset
                    , color_type const &  isotherm_color
                   )                                  ;

    void          release( )                          ;

  protected:
    virtual bool  is_supported( )                     = 0;
    virtual void  warn_compile_link_error( )          = 0;

  protected:
    void          reset_uniform_ids( )                ;
    void          setup_uniform_ids( )                ;

  protected:
    static bool             has_message_been_posted_      ;

    program_owner_any_type  program_owner_                ;

    int_type                uniform_id__z_factor_         ;
    int_type                uniform_id__z_offset_         ;
    int_type                uniform_id__mix_color_        ;
    int_type                uniform_id__isotherm_texture_ ;
};

// _______________________________________________________________________________________________

  /* static */
  bool
  blinn_phong_program_owner_type::
has_message_been_posted_ = false;

  /* ctor */
  blinn_phong_program_owner_type::
blinn_phong_program_owner_type( )
  : program_owner_
     (  c_src_string__vertex_shader__blinn_phong__light
      , c_src_string__fragment_shader__blinn_phong__light
     )
{
    reset_uniform_ids( );
}

// _______________________________________________________________________________________________

  void
  blinn_phong_program_owner_type::
release( )
{
    if ( not_deep_reset( program_owner_.get_program__no_init( )) ) {
        d_assert( not_reset( program_owner_.get_program__no_init( )));
        d_assert( 0 <= uniform_id__z_factor_);

        program_owner_.uninit( );
        reset_uniform_ids( );
    }
    d_assert( is_deep_reset( program_owner_.get_program__no_init( )));
    d_assert( -1 == uniform_id__z_factor_);
}

// _______________________________________________________________________________________________

  void
  blinn_phong_program_owner_type::
set_uniform_values
 (  bool                is_showing
  , float_type          z_factor
  , float_type          z_offset
  , color_type const &  isotherm_color
 )
{
    d_assert( z_factor >= 1.0f);

    if ( uniform_id__z_factor_ >= 0 ) {
        program_owner_.set_uniform( uniform_id__z_factor_ , is_showing ? z_factor : 0.0f);
        program_owner_.set_uniform( uniform_id__z_offset_ , z_offset);
        program_owner_.set_uniform( uniform_id__mix_color_, isotherm_color);

        // The texture is bound and setup when GL_TEXTURE0 is active, so the texture unit is 0.
        // We pass in the texture unit, not the texture id number (which is not zero, probably 1).
        program_owner_.set_uniform( uniform_id__isotherm_texture_, 0);
    }
}

// _______________________________________________________________________________________________

  program_holder_variant_type
  blinn_phong_program_owner_type::
get_program_holder( )
  //
  // This will return a deep-reset object if shaders are not supported (for the current gl-context),
  // of if we run into a compile/link error.
{
    // There's a subtle potential bug to watch out for here.
    // This will post a dialog the first time it sees an error.
    // That dialog will run a modal loop, processing messages.
    // One of the messages might be telling the main widget to paint.
    //
    // If we get here when we're in the middle of paint, and we post a dialog, the main widget
    // may process the paint message again, recursively. If we're in the middle of wrappers
    // during the first paint, the 2nd paint can unwrap those wrappers while the first paint
    // is still using them.
    //
    // It seems like a good idea to never allow a popup dialog while a thread is painting.
    // Although we could also handle this with lock-out flags, as long as we watch out for
    // thrashing.

    // is_supported( ) is a virtual that might post a warning dialog.
    if ( ! is_supported( ) ) {
        // If the variant type is deep-reset then we do not attempt to set the shader.
        return tag::reset_type( ); /* deep reset */
    }

    if ( ! program_owner_.is_init_attempted( ) ) {
        program_owner_.init( );
        if ( program_owner_.is_init_successful( ) ) {
            d_assert( not_reset( program_owner_.get_program__no_init( )));
            setup_uniform_ids( );
        } else {
            d_assert( is_deep_reset( program_owner_.get_program__no_init( )));
            reset_uniform_ids( );
            warn_compile_link_error( );
        }
    }

    // If init( ) failed above this will be deep-reset, not just shallow-reset.
    // Although we could return shader_program__reset instead, which will make sure the
    // default pipeline shader is used.
    return program_owner_.get_program__no_init( );
}

// _______________________________________________________________________________________________

  void
  blinn_phong_program_owner_type::
reset_uniform_ids( )
{
    uniform_id__z_factor_         = -1;
    uniform_id__z_offset_         = -1;
    uniform_id__mix_color_        = -1;
    uniform_id__isotherm_texture_ = -1;
}

  void
  blinn_phong_program_owner_type::
setup_uniform_ids( )
{
    d_assert( -1 == uniform_id__z_factor_ );

    uniform_id__z_factor_         = program_owner_.get_uniform_location( "u_z_factor"         );
    uniform_id__z_offset_         = program_owner_.get_uniform_location( "u_z_offset"         );
    uniform_id__mix_color_        = program_owner_.get_uniform_location( "u_mix_color"        );
    uniform_id__isotherm_texture_ = program_owner_.get_uniform_location( "u_isotherm_texture" );

    d_assert( 0 <= uniform_id__z_factor_         );
    d_assert( 0 <= uniform_id__z_offset_         );
    d_assert( 0 <= uniform_id__mix_color_        );
    d_assert( 0 <= uniform_id__isotherm_texture_ );

    d_assert( uniform_id__z_factor_ != uniform_id__z_offset_         );
    d_assert( uniform_id__z_factor_ != uniform_id__mix_color_        );
    d_assert( uniform_id__z_factor_ != uniform_id__isotherm_texture_ );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
blinn_phong_program_owner__normal__type
  : public blinn_phong_program_owner_type
{
  protected:
    virtual bool  is_supported( )             ;
    virtual void  warn_compile_link_error( )  ;
};

  bool
  blinn_phong_program_owner__normal__type::
is_supported( )
{
    if ( ! gl_env::shader::are_shaders_supported( ) ) {
        if ( ! has_message_been_posted_ ) {
            has_message_been_posted_ = true;

            QMessageBox::warning( 0,
              QObject::tr( "Missing OpenGL Shader Support"),
              QObject::tr(
                "Cannot use Blinn-Phong shading because OpenGL on this system "
                "does not support it. "
                "\n\n"
                "Press OK to close."
            ));
        }
        return false;
    }
    return true;
}

  void
  blinn_phong_program_owner__normal__type::
warn_compile_link_error( )
{
    if ( ! has_message_been_posted_ ) {
        has_message_been_posted_ = true;

        QMessageBox::warning( 0,
          QObject::tr( "Shader Compile/Link Error"),
          QObject::tr(
            "Cannot use Blinn-Phong shaders due to shader error. "
            "\n\n"
            "Press OK to close."
        ));
    }
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  class
blinn_phong_program_owner__bitmap__type
  : public blinn_phong_program_owner_type
{
  public:
    /* ctor */    blinn_phong_program_owner__bitmap__type( )  ;

  public:
    void          maybe_post_warning_dialog( )                ;
  protected:
    void          maybe_post_is_supported_warning_dialog( )   ;
    void          maybe_post_is_error_warning_dialog( )       ;

  protected:
    virtual bool  is_supported( )                             ;
    virtual void  warn_compile_link_error( )                  ;

  private:
    bool  is_supported_message_requested_ ;
    bool  is_supported_message_posted_    ;
    bool  is_error_message_requested_     ;
    bool  is_error_message_posted_        ;
};

  /* ctor */
  blinn_phong_program_owner__bitmap__type::
blinn_phong_program_owner__bitmap__type( )
  : is_supported_message_requested_( false)
  , is_supported_message_posted_   ( false)
  , is_error_message_requested_    ( false)
  , is_error_message_posted_       ( false)
  { }

  bool
  blinn_phong_program_owner__bitmap__type::
is_supported( )
{
    d_assert( implies( ! gl_env::shader::are_shaders_supported( ), has_message_been_posted_));
    if ( has_message_been_posted_ ) {
        return false;
    }
    if ( ! gl_env::shader::are_shaders_explicitly_supported( ) ) {
        // We get here when the normal (hardware accelerated) gl-context supports shaders
        // but the current one, that writes to a bitmap (not hardware accelerated), does not.

        // We mark this, but the message hasn't really been posted.
        has_message_been_posted_ = true;

        // We'd like to post the is-supported warning-message here, but we cannot. When we get
        // here we are inside the paintGL( ) function, and we don't want to a popup generating
        // re-draw requests. So instead we take note and post the message later.
        d_assert( ! is_supported_message_posted_);
        d_assert( ! is_supported_message_requested_);
        is_supported_message_requested_ = true;

        return false;
    }
    return true;
}

  void
  blinn_phong_program_owner__bitmap__type::
maybe_post_is_supported_warning_dialog( )
{
    if ( is_supported_message_requested_ && ! is_supported_message_posted_ ) {
        is_supported_message_posted_ = true;

        QMessageBox::warning( 0,
          QObject::tr( "Missing OpenGL Shader Support (for bitmaps)"),
          QObject::tr(
            "Cannot use Blinn-Phong shading for hardcopy because OpenGL on this "
            "system requires that we use a software-only OpenGL driver to write "
            "to a bitmap, and that driver does not support GLSL shader programs. "
            "\n\n"
            "Press OK to close."
        ));
    }
}

  void
  blinn_phong_program_owner__bitmap__type::
warn_compile_link_error( )
{
    if ( ! has_message_been_posted_ ) {
        has_message_been_posted_ = true;

        // Delay the message.
        d_assert( ! is_error_message_posted_);
        d_assert( ! is_error_message_requested_);
        is_error_message_requested_ = true;
    }
}

  void
  blinn_phong_program_owner__bitmap__type::
maybe_post_is_error_warning_dialog( )
{
    if ( is_error_message_requested_ && ! is_error_message_posted_ ) {
        is_error_message_posted_ = true;

        QMessageBox::warning( 0,
          QObject::tr( "Shader Compile/Link Error (for bitmaps)"),
          QObject::tr(
            "Cannot use Blinn-Phong shaders due to shader error. "
            "\n\n"
            "Press OK to close."
        ));
    }
}

  void
  blinn_phong_program_owner__bitmap__type::
maybe_post_warning_dialog( )
{
    // We never request both messages.
    d_assert( (! is_supported_message_requested_) || (! is_error_message_requested_));

    maybe_post_is_supported_warning_dialog( );
    maybe_post_is_error_warning_dialog( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace /* anonymous */ {

blinn_phong_program_owner__normal__type  blinn_phong_program_owner__normal ;
blinn_phong_program_owner__bitmap__type  blinn_phong_program_owner__bitmap ;

} /* end namespace anonymous */

  /* extern */
  void
init_shader_program__blinn_phong( )
{
    // Return value is ignored in the following.
    // This ensures that the shader is compiled any warning dialog is posted.
    // We don't want to wait and post a dialog later during paintGL( ).
    blinn_phong_program_owner__normal.get_program_holder( );
}

  /* extern */
  void
maybe_post_warning_dialog__blinn_phong__for_bitmap_context( )
{
    blinn_phong_program_owner__bitmap.maybe_post_warning_dialog( );
}

  /* extern */
  void
release_shader_program__blinn_phong( )
{
    blinn_phong_program_owner__normal.release( );
}

  /* extern */
  void
release_shader_program__blinn_phong__for_bitmap_context( )
{
    blinn_phong_program_owner__bitmap.release( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

  /* ctor */
  with_blinn_phong_shading__wrapper_type::
with_blinn_phong_shading__wrapper_type
 (  bool                is_on
  , bool                is_bitmap
  , bool                is_isotherm_showing
  , float               isotherm_z_factor
  , float               isotherm_offset
  , color_type const &  isotherm_color
 )
{
    d_assert( is_deep_reset( previous_program_));

    // Do nothing if we don't want to use shaders.
    if ( is_on ) {

        // Get the shader program.
        blinn_phong_program_owner_type &  program_normal  = blinn_phong_program_owner__normal;
        blinn_phong_program_owner_type &  program_bitmap  = blinn_phong_program_owner__bitmap;
        blinn_phong_program_owner_type &  program_owner   = is_bitmap ? program_bitmap : program_normal;

        // Get the program holder.
        program_holder_variant_type       program_holder  = program_owner.get_program_holder( );

        if ( not_deep_reset( program_holder) ) {

            // Activate the shader.
            d_assert( not_reset( program_holder));
            previous_program_ = gl_env::shader::activate( program_holder);
            d_assert( not_deep_reset( previous_program_));

            // Setup the shader parameters.
            program_owner.set_uniform_values
             (  is_isotherm_showing
              , isotherm_z_factor
              , isotherm_offset
              , isotherm_color
             );
        }
    }
}

  /* dtor */
  with_blinn_phong_shading__wrapper_type::
~with_blinn_phong_shading__wrapper_type( )
{
    // If we activated a shader, we'll have saved the previously active shader.
    if ( not_deep_reset( previous_program_) ) {

        // Restore by activating the previously active shader.
        gl_env::shader::activate( previous_program_); /* discard return */
        deep_reset( previous_program_);
    }
}

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// shader.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
