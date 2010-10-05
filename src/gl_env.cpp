// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_env.cpp
//   Neal Binnendyk, nealabq -at- gmail, nealabq.com
//   Copyright (c) 2010. All rights reserved.
// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// Design improvement:
//   This should be part of the class that creates and manipulates the GL context.
//   It should be like the QGLContext class.
//   The Qt classes QGLContext and QGLFormat are a good start in that direction.
//   Remember that GL-context objects are tied to an HDC (in Windows), so you have to create
//   the target (HWND, HBITMAP) before creating the context. You cannot shift contexts around.
//
//   EGL provides some abstraction from the native system, including types for:
//     Native window (HWND)
//     Native pixmap (HBITMAP)
//     Native device (HDC)
//     Display
//     Surface
//     Context
//     Config
//     ConfigAttrib
//     Buffers (raw memory?):
//       Color (different formats), depth, stencil
//       VBOs? Render buffers? Probably part of GL/GLES but not EGL.
//
//   Objects like frame-buffers are closely tied to their context.
//   Objects like shader-programs, textures, the lighting-rig, etc, have a definition independent
//   of context, and can be bound to zero, one, or several contexts.
//
//   So really there are two separate things here:
//     The context object, like QGLContext, except it should know about the environment object.
//     The environment object that holds the shaders, textures, etc.
//
//   The env (environment) class defined in this file is a first attempt to provide a root for
//   environment-like objects like shader-programs, lighting-rigs, camera-rigs, etc.
//   You could call it a scene-object instead of an environment, although that might be confusing
//   because it is NOT the beginnings of a scene-graph.
//
// Conflicts between GLee and Qt:
//
//   Conflicts:
//     "GLee.h" insists that it be the one to include gl.h.
//     <QtOpenGL/QGLWidget> also insists that it be the first to touch gl.h.
//     So you cannot include both in the same compilation unit.
//
//     Qt is probably mostly used with GLew and not GLee because of this.
//
//     It'd be easy if we could just include gl.h in this header, but we cannot if we
//     want to use GLee.h or <QtOpenGL/QGLWidget>.
//     It'd also be easy if we could just include GLee.h in this file and isolate QGLWidget.
//     But to do that we'd have to isolate ALL of QT away from GLee because under Linux (Ubuntu,
//     Mesa GL headers) gl.h includes a lot of stuff and leaves X11 #define cruft behind.
//     And Qt CONFLICTS with those names, specifically with Status and CursorShape which are
//     both X11 #defines and Qt enums.
//
//     Another complication is that GLee.h typedefs GLhandleARB to be "int", while the
//     Linux/Mesa glext.h typedefs it as "unsigned int". So if you're getting it thru
//     QGLWidget I'm not sure which one you end up with, but you can easily end up with
//     linking problems (functions declared but never defined).
//
//   Solutions:
//     Use GLew instead of GLee. I haven't tried this yet.
//
//     Isolate GL to a single CPP file that doesn't use Qt.
//     You'd have to completely cover the GL interface, without exposing types like GLuint
//     and consts (#defines) like GL_QUAD_STRIP. You'd have very few inline functions.
//
//     Isolate just the GL extensions in one or more CPP files. Those CPP files will
//     include GLee.h before the more generally available header that provides gl.h.
//     This is what we're doing now, in gl_env.cpp and gl_shader.cpp.
//
//     Isolate QGLWidget and try to get GLee.h to work with the rest of Qt. Either
//     don't compile for Linux/Mesa, or try to correct mistakes by #undef Status
//     and #undef CursorShape. This might not be enough.
//
// Suggestions:
//   Wrapper to build a display list.
//   Wrapper to build a set of indexed objects and indexes.
//   Provide a set of primitive geometric objects like pt< count, type >, etc.
//
// We should have a way to push server attributes by name. Something like this:
//   with pushed server attributes
//     push shade_model, model-view matrix (automatically restore them when we're done)
//       let us change them, or
//       set them here to a value and then freeze them
//     assert line_width will be restored by the end
//     freeze everything else
//
// During debug we should have a dictionary of all the ::glEnable*(..) and ::glGet*(..) flags
// an all their properties, especially data type (and cardinality).
//   Expected data types:
//     bool
//     GL symbol (name)
//     signed int
//     unsigned int
//     float or double
//     vertex< order, type >  - also serves as a vector and texture coord
//     color< order, type >  - RGB or RBGA
//     matrix< float or double >
//     range< type >  - min/max
//     GL_NUM_COMPRESSED_TEXTURE_FORMATS GL symbols
//     xy and xyxy endpoints (bounding box?)
//     x,y,width,height box of window coords
// The table would also record the GL version required for support. Many of the values listed
// below are not covered by OpenGL1.1 (the default Windows version).
//
// These are all the ::glEnable(..) flags:
//   GL_ALPHA_TEST                 GL_AUTO_NORMAL
//   GL_BLEND                      GL_CLIP_PLANEi (0-5)
//   GL_COLOR_LOGIC_OP             GL_COLOR_MATERIAL           GL_COLOR_SUM              GL_COLOR_TABLE
//   GL_CONVOLUTION_1D             GL_CONVOLUTION_2D
//   GL_CULL_FACE                  GL_DEPTH_TEST
//   GL_DITHER                     GL_FOG
//   GL_HISTOGRAM                  GL_INDEX_LOGIC_OP
//   GL_LIGHTi (0-7)               GL_LIGHTING
//   GL_LINE_SMOOTH                GL_LINE_STIPPLE
//   GL_MAP1_COLOR_4               GL_MAP1_INDEX               GL_MAP1_NORMAL
//   GL_MAP1_TEXTURE_COORD_1       GL_MAP1_TEXTURE_COORD_2     GL_MAP1_TEXTURE_COORD_3   GL_MAP1_TEXTURE_COORD_4
//   GL_MAP1_VERTEX_3 (see note)   GL_MAP1_VERTEX_4 (see note)
//   GL_MAP2_COLOR_4               GL_MAP2_INDEX               GL_MAP2_NORMAL
//   GL_MAP2_TEXTURE_COORD_1       GL_MAP2_TEXTURE_COORD_2     GL_MAP2_TEXTURE_COORD_3   GL_MAP2_TEXTURE_COORD_4
//   GL_MAP2_VERTEX_3              GL_MAP2_VERTEX_4
//   GL_MINMAX                     GL_MULTISAMPLE
//   GL_NORMALIZE
//   GL_POINT_SMOOTH               GL_POINT_SPRITE
//   GL_POLYGON_OFFSET_FILL        GL_POLYGON_OFFSET_LINE      GL_POLYGON_OFFSET_POINT
//   GL_POLYGON_SMOOTH             GL_POLYGON_STIPPLE
//   GL_POST_COLOR_MATRIX_COLOR_TABLE
//   GL_POST_CONVOLUTION_COLOR_TABLE
//   GL_RESCALE_NORMAL
//   GL_SAMPLE_ALPHA_TO_COVERAGE   GL_SAMPLE_ALPHA_TO_ONE      GL_SAMPLE_COVERAGE
//   GL_SEPARABLE_2D
//   GL_SCISSOR_TEST               GL_STENCIL_TEST
//   GL_TEXTURE_1D                 GL_TEXTURE_2D               GL_TEXTURE_3D
//   GL_TEXTURE_CUBE_MAP
//   GL_TEXTURE_GEN_Q              GL_TEXTURE_GEN_R            GL_TEXTURE_GEN_S          GL_TEXTURE_GEN_T
//   GL_VERTEX_PROGRAM_POINT_SIZE  GL_VERTEX_PROGRAM_TWO_SIDE
//
// These (ARRAY) flags can also be used with is_enabled [but not with enable(..) and disable(..)].
//   GL_COLOR_ARRAY
//   GL_EDGE_FLAG_ARRAY
//   GL_FOG_COORD_ARRAY
//   GL_INDEX_ARRAY
//   GL_NORMAL_ARRAY
//   GL_SECONDARY_COLOR_ARRAY
//   GL_TEXTURE_COORD_ARRAY
//   GL_VERTEX_ARRAY
//
// Note: Although the opengl.org does not mention GL_MAP1_VERTEX_3 and GL_MAP1_VERTEX_4 as supported in the doc for
// ::glIsEnabled( flag), it does say that they'll work with ::glIsEnabled(..) in the doc for ::glMap1(..).
//
// Here are all the ::glGet..(..) flags:
//   GL_ACCUM_ALPHA_BITS                  1  count
//   GL_ACCUM_BLUE_BITS                   1  count
//   GL_ACCUM_GREEN_BITS                  1  count
//   GL_ACCUM_RED_BITS                    1  count
//   GL_ACCUM_CLEAR_VALUE                 4  RGBA (ints -1..+1), in this case are negatives allowed?
//   GL_ACTIVE_TEXTURE                    1  index like GL_TEXTURE0
//   GL_ALIASED_POINT_SIZE_RANGE          2  min/max
//   GL_ALIASED_LINE_WIDTH_RANGE          2  min/max
//   GL_ALPHA_BIAS                        1  bias
//   GL_ALPHA_BITS                        1  count
//   GL_ALPHA_SCALE                       1  scale
//   GL_ALPHA_TEST                        1  bool
//   GL_ALPHA_TEST_FUNC                   1  name like GL_ALWAYS
//   GL_ALPHA_TEST_REF                    1  float -1..+1 ref value for alpha test
//   GL_ARRAY_BUFFER_BINDING              1  name like GL_ARRAY_BUFFER or 0
//   GL_ATTRIB_STACK_DEPTH                1  depth of stack, 0 if stack is empty (initial stack)
//   GL_AUTO_NORMAL                       1  bool
//   GL_AUX_BUFFERS                       1  count
//   GL_BLEND                             1  bool  is_blending_available
//   GL_BLEND_COLOR                       4  RGBA
//   GL_BLEND_DST_ALPHA                   1  name like GL_ZERO
//   GL_BLEND_DST_RGB                     1  name like GL_ZERO (not rgb)
//   GL_BLEND_EQUATION_RGB                1  name like GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN or GL_MAX.
//   GL_BLEND_EQUATION_ALPHA              1  name like GL_FUNC_ADD, GL_FUNC_SUBTRACT, GL_FUNC_REVERSE_SUBTRACT, GL_MIN or GL_MAX.
//   GL_BLEND_SRC_ALPHA                   1  name like GL_ONE
//   GL_BLEND_SRC_RGB                     1  name like GL_ONE
//   GL_BLUE_BIAS                         1  bias, see glPixelTransfer.
//   GL_BLUE_BITS                         1  count
//   GL_BLUE_SCALE                        1  scale factor, see glPixelTransfer.
//   GL_CLIENT_ACTIVE_TEXTURE             1  integer like GL_TEXTURE0. See glClientActiveTexture.
//   GL_CLIENT_ATTRIB_STACK_DEPTH         1  count (depth of attrib stack). Initial value is 0 (empty stack). See glPushClientAttrib.
//   GL_CLIP_PLANEi (0-5)                 1  bool, see glClipPlane.
//   GL_COLOR_ARRAY                       1  bool, is the color array is enabled. See glColorPointer.
//   GL_COLOR_ARRAY_BUFFER_BINDING        1  the name of the buffer object associated with the color array. See GL_ARRAY_BUFFER, glColorPointer, glBindBuffer.
//   GL_COLOR_ARRAY_SIZE                  1  count, the number of components per color in the color array (4). See glColorPointer.
//   GL_COLOR_ARRAY_STRIDE                1  byte offset between consecutive colors in the color array. See glColorPointer.
//   GL_COLOR_ARRAY_TYPE                  1  data type in the color array, like GL_FLOAT. See glColorPointer.
//   GL_COLOR_CLEAR_VALUE                 4  RGBA, see glClearColor.
//   GL_COLOR_LOGIC_OP                    1  bool, are fragment RBGA values merged into framebuffer using logic op. See glLogicOp.
//   GL_COLOR_MATERIAL                    1  bool, are materials tracking current color. See glColorMaterial.
//   GL_COLOR_MATERIAL_FACE               1  name like GL_FRONT_AND_BACK. See glColorMaterial.
//   GL_COLOR_MATERIAL_PARAMETER          1  name indicating which material parameters are tracking the current color (GL_AMBIENT_AND_DIFFUSE). See glColorMaterial.
//   GL_COLOR_MATRIX                     16  color matrix (ie identity matrix). See glPushMatrix.
//   GL_COLOR_MATRIX_STACK_DEPTH          1  count, (online doc under glGet is wrong)
//                                             depth of the color matrix stack. See glPushMatrix.
//   GL_COLOR_SUM                         1  bool, see glSecondaryColor.
//   GL_COLOR_TABLE                       1  bool, is lookup enabled. See glColorTable.
//   GL_COLOR_WRITEMASK                   4  bools, RGBA enables. See glColorMask.
//   GL_COMPRESSED_TEXTURE_FORMATS        N (GL_NUM_COMPRESSED_TEXTURE_FORMATS) names. See glCompressedTexImage2D.
//   GL_CONVOLUTION_1D                    1  bool, is 1D convo enabled. See glConvolutionFilter1D.
//   GL_CONVOLUTION_2D                    1  bool, is 2D convo enabled. See glConvolutionFilter2D.
//   GL_CULL_FACE                         1  bool, is poly culling enabled. See glCullFace.
//   GL_CULL_FACE_MODE                    1  name, which polygon faces to cull. See glCullFace.
//   GL_CURRENT_COLOR                     4  RGBA, see glColor.
//   GL_CURRENT_FOG_COORD                 1  Fog coord. See glFogCoord.
//   GL_CURRENT_INDEX                     1  Int index, the current color index. See glIndex.
//   GL_CURRENT_NORMAL                    3  XYZ (floats) of current normal. Ints -1..+1 mapped. See glNormal.
//   GL_CURRENT_PROGRAM                   1  name (int), the program object that is currently active. See glUseProgram.
//   GL_CURRENT_RASTER_COLOR              4  RGBA, see glRasterPos.
//   GL_CURRENT_RASTER_DISTANCE           1  distance from eye to current raster pos. See glRasterPos.
//   GL_CURRENT_RASTER_INDEX              1  index, color index of the current raster position. See glRasterPos.
//   GL_CURRENT_RASTER_POSITION           4  XYZW of current raster position. x, y, and z are in window coords, w is in clip coords. See glRasterPos.
//   GL_CURRENT_RASTER_POSITION_VALID     1  bool, is current raster pos valid. See glRasterPos.
//   GL_CURRENT_RASTER_SECONDARY_COLOR    4  RGBA, secondary color values of the current raster position. See glRasterPos.
//   GL_CURRENT_RASTER_TEXTURE_COORDS     4  STRQ texture coords of current raster pos. See glRasterPos and glMultiTexCoord.
//   GL_CURRENT_SECONDARY_COLOR           4  RGBA, current secondary color. See glSecondaryColor.
//   GL_CURRENT_TEXTURE_COORDS            4  STRQ, current texture coords. See glMultiTexCoord.
//   GL_DEPTH_BIAS                        1  depth factor. See glPixelTransfer.
//   GL_DEPTH_BITS                        1  count of bitplanes in depth buffer.
//   GL_DEPTH_CLEAR_VALUE                 1  -1..+1, value used to clear depth buffer. See glClearDepth.
//   GL_DEPTH_FUNC                        1  name of depth compare, like GL_LESS. See glDepthFunc.
//   GL_DEPTH_RANGE                       2  near/far mapping limits in depth buffer. See glDepthRange.
//   GL_DEPTH_SCALE                       1  depth scale factor. See glPixelTransfer.
//   GL_DEPTH_TEST                        1  bool, is depth-testing of fragments enabled. See glDepthFunc and glDepthRange.
//   GL_DEPTH_WRITEMASK                   1  bool, is depth buffer write enabled. See glDepthMask.
//   GL_DITHER                            1  bool, is dithering of fragment colors and indexes on.
//   GL_DOUBLEBUFFER                      1  bool, is double-buf supported.
//   GL_DRAW_BUFFER                       1  name, which buffers are being drawn to: GL_BACK for back bufs, or GL_FRONT. See glDrawBuffer.
//   GL_DRAW_BUFFERi (not supported?)     1  name, which buffers are being drawn to by the corresponding output color. GL_BACK, GL_FRONT, GL_NONE, see glDrawBuffers.
//   GL_EDGE_FLAG                         1  bool, see glEdgeFlag.
//   GL_EDGE_FLAG_ARRAY                   1  bool, see glEdgeFlagPointer.
//   GL_EDGE_FLAG_ARRAY_BUFFER_BINDING    1  name of the buffer object associated with the edge flag array. See GL_ARRAY_BUFFER, glEdgeFlagPointer, glBindBuffer.
//   GL_EDGE_FLAG_ARRAY_STRIDE            1  offset, see glEdgeFlagPointer.
//   GL_ELEMENT_ARRAY_BUFFER_BINDING      1  name of the buffer object currently bound to the target GL_ELEMENT_ARRAY_BUFFER. See glBindBuffer.
//   GL_FEEDBACK_BUFFER_SIZE              1  size of the feedback buffer. See glFeedbackBuffer.
//   GL_FEEDBACK_BUFFER_TYPE              1  type, see glFeedbackBuffer.
//   GL_FOG                               1  bool, is fogging enabled. See glFog.
//   GL_FOG_COORD_ARRAY                   1  bool, is array enabled. See glFogCoordPointer.
//   GL_FOG_COORD_ARRAY_BUFFER_BINDING    1  name of buffer. See GL_ARRAY_BUFFER, glFogCoordPointer, glBindBuffer.
//   GL_FOG_COORD_ARRAY_STRIDE            1  count, byte offset, see glFogCoordPointer.
//   GL_FOG_COORD_ARRAY_TYPE              1  type like GL_FLOAT. See glFogCoordPointer.
//   GL_FOG_COORD_SRC                     1  name, source of fog coord (GL_FRAGMENT_DEPTH). See glFog.
//   GL_FOG_COLOR                         4  RGBA, see glFog.
//   GL_FOG_DENSITY                       1  density, see glFog.
//   GL_FOG_END                           1  factor, see glFog.
//   GL_FOG_HINT                          1  name like GL_DONT_CARE. See glHint.
//   GL_FOG_INDEX                         1  color index, see glFog.
//   GL_FOG_MODE                          1  name, which is selected, like GL_EXP. See glFog.
//   GL_FOG_START                         1  factor, see glFog.
//   GL_FRAGMENT_SHADER_DERIVATIVE_HINT   1  name like GL_DONT_CARE. See glHint.
//   GL_FRONT_FACE                        1  name (could be bool) like GL_CCW. See glFrontFace.
//   GL_GENERATE_MIPMAP_HINT              1  name like GL_DONT_CARE. See glHint.
//   GL_GREEN_BIAS                        1  factor for pixel transfers.
//   GL_GREEN_BITS                        1  count, number of green bitplanes in each color buf.
//   GL_GREEN_SCALE                       1  scale during pixel transfers. See glPixelTransfer.
//   GL_HISTOGRAM                         1  bool, is histo enabled. See glHistogram.
//   GL_INDEX_ARRAY                       1  bool, is color index array enabled. See glIndexPointer.
//   GL_INDEX_ARRAY_BUFFER_BINDING        1  name of buf, see GL_ARRAY_BUFFER, glIndexPointer, glBindBuffer.
//   GL_INDEX_ARRAY_STRIDE                1  int offset, see glIndexPointer.
//   GL_INDEX_ARRAY_TYPE                  1  type like GL_FLOAT. See glIndexPointer.
//   GL_INDEX_BITS                        1  count of bitplanes in each color index buf.
//   GL_INDEX_CLEAR_VALUE                 1  index of color used to clear color index bufs. See glClearIndex.
//   GL_INDEX_LOGIC_OP                    1  bool, is fragment index-value merged into framebuffer using a logical op. See glLogicOp.
//   GL_INDEX_MODE                        1  bool, is GL in color index mode or RGBA mode.
//   GL_INDEX_OFFSET                      1  int offset, for pixel transfers. See glPixelTransfer.
//   GL_INDEX_SHIFT                       1  shift, see glPixelTransfer.
//   GL_INDEX_WRITEMASK                   1  mask, which bitplanes of color index bufs can be written. See glIndexMask.
//   GL_LIGHTi (0-7)                      1  bool, which lights are on. See glLight and glLightModel.
//   GL_LIGHTING                          1  bool, is lighting enabled. See glLightModel.
//   GL_LIGHT_MODEL_AMBIENT               4  RGBA, init (0.2, 0.2, 0.2, 1.0). See glLightModel.
//   GL_LIGHT_MODEL_COLOR_CONTROL         1  name like GL_SINGLE_COLOR. See glLightModel.
//   GL_LIGHT_MODEL_LOCAL_VIEWER          1  bool, ortho or projection calcs for specular reflections. See glLightModel.
//   GL_LIGHT_MODEL_TWO_SIDE              1  bool, are front and back separate materials. See glLightModel.
//   GL_LINE_SMOOTH                       1  bool, enable antialias lines. See glLineWidth.
//   GL_LINE_SMOOTH_HINT                  1  name (line antialias mode), like GL_DONT_CARE. See glHint.
//   GL_LINE_STIPPLE                      1  bool, enable line stipple. See glLineStipple.
//   GL_LINE_STIPPLE_PATTERN              1  16-bit int. The bits are stipple pattern. See glLineStipple.
//   GL_LINE_STIPPLE_REPEAT               1  repeat factor (1). See glLineStipple.
//   GL_LINE_WIDTH                        1  width (1), see glLineWidth.
//   GL_LINE_WIDTH_GRANULARITY            1  width between adjacent widths for antialiased lines. See glLineWidth.
//   GL_LINE_WIDTH_RANGE                  2  min/max width, antialias lines. See glLineWidth.
//   GL_LIST_BASE                         1  offset, see glCallLists, glListBase.
//   GL_LIST_INDEX                        1  name of display list being constructed, 0 if none. See glNewList.
//   GL_LIST_MODE                         1  name list-construction mode. See glNewList.
//   GL_LOGIC_OP_MODE                     1  name selected logic op mode like GL_COPY. See glLogicOp.
//   GL_MAP1_COLOR_4                      1  bool, does 1D eval gen colors. See glMap1.
//   GL_MAP1_GRID_DOMAIN                  2  endpoints of 1D grid domain (0, 1). See glMapGrid.
//   GL_MAP1_GRID_SEGMENTS                1  count partitions in 1D map's grid domain (1). See glMapGrid.
//   GL_MAP1_INDEX                        1  bool, does 1D eval gen color indexes. See glMap1.
//   GL_MAP1_NORMAL                       1  bool, does 1D eval gen normals. See glMap1.
//   GL_MAP1_TEXTURE_COORD_1              1  bool, does 1D eval gen 1D texture coords. See glMap1.
//   GL_MAP1_TEXTURE_COORD_2              1  bool, does 1D eval gen 2D texture coords. See glMap1.
//   GL_MAP1_TEXTURE_COORD_3              1  bool, does 1D eval gen 3D texture coords. See glMap1.
//   GL_MAP1_TEXTURE_COORD_4              1  bool, does 1D eval gen 4D texture coords. See glMap1.
//   GL_MAP1_VERTEX_3                     1  bool, does 1D eval gen 3D vertex coords. See glMap1.
//   GL_MAP1_VERTEX_4                     1  bool, does 1D eval gen 4D vertex coords. See glMap1.
//   GL_MAP2_COLOR_4                      1  bool, does 2D eval gen colors. See glMap2.
//   GL_MAP2_GRID_DOMAIN                  4  endpoints in 2D map domains ((0,1)(0,1)). See glMapGrid.
//   GL_MAP2_GRID_SEGMENTS                2  count partitions in 2D map grid domains (1,1). See glMapGrid.
//   GL_MAP2_INDEX                        1  bool, does 2D eval gen color indexes. See glMap2.
//   GL_MAP2_NORMAL                       1  bool, does 2D eval gen normals. See glMap2.
//   GL_MAP2_TEXTURE_COORD_1              1  bool, does 2D eval gen 1D texture coords. See glMap2.
//   GL_MAP2_TEXTURE_COORD_2              1  bool, does 2D eval gen 2D texture coords. See glMap2.
//   GL_MAP2_TEXTURE_COORD_3              1  bool, does 2D eval gen 3D texture coords. See glMap2.
//   GL_MAP2_TEXTURE_COORD_4              1  bool, does 2D eval gen 4D texture coords. See glMap2.
//   GL_MAP2_VERTEX_3                     1  bool, does 2D eval gen 3D vertex coords. See glMap2.
//   GL_MAP2_VERTEX_4                     1  bool, does 2D eval gen 4D vertex coords. See glMap2.
//   GL_MAP_COLOR                         1  bool, are color/color-indexes replaced by table during pixel trans. See glPixelTransfer.
//   GL_MAP_STENCIL                       1  bool, are stencil indexes replaced by table during pixel trans. See glPixelTransfer.
//   GL_MATRIX_MODE                       1  name, which matrix stack is current target (GL_MODELVIEW). See glMatrixMode.
//   GL_MAX_3D_TEXTURE_SIZE               1  estimate largest 3D texture GL can take, >= 16. See GL_PROXY_TEXTURE_3D glTexImage3D.
//   GL_MAX_CLIENT_ATTRIB_STACK_DEPTH     1  int, max depth of client attrib stack. See glPushClientAttrib.
//   GL_MAX_ATTRIB_STACK_DEPTH            1  int, max depth of attrib stack >= 16. See glPushAttrib.
//   GL_MAX_CLIP_PLANES                   1  int, max # clip planes >= 6. See glClipPlane.
//   GL_MAX_COLOR_MATRIX_STACK_DEPTH      1  int, max depth of color matrix stack >= 2. See glPushMatrix.
//   GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS  1  count max # texture image units from vertex shader and fragment processor >= 2. See glActiveTexture.
//   GL_MAX_CUBE_MAP_TEXTURE_SIZE         1  estimate largest cube-map texture GL can take >= 16. See GL_PROXY_TEXTURE_CUBE_MAP, glTexImage2D.
//   GL_MAX_DRAW_BUFFERS                  1  max # simultan output colors from fragment shader using gl_FragData built-in array >= 1. See glDrawBuffers.
//   GL_MAX_ELEMENTS_INDICES              1  max # vertex array indexes. See glDrawRangeElements.
//   GL_MAX_ELEMENTS_VERTICES             1  max # vertex array vertexes. See glDrawRangeElements.
//   GL_MAX_EVAL_ORDER                    1  max equation order supported by 1D and 2D evaluators >= 8. See glMap1 and glMap2.
//   GL_MAX_FRAGMENT_UNIFORM_COMPONENTS   1  max # values held in uniform variable storage for fragment shader >= 64. See glUniform.
//   GL_MAX_LIGHTS                        1  max # lights >= 6. See glLight.
//   GL_MAX_LIST_NESTING                  1  max recursion depth allowed in display lists >= 64. See glCallList.
//   GL_MAX_MODELVIEW_STACK_DEPTH         1  max stack depth modelview matrix >= 32. See glPushMatrix.
//   GL_MAX_NAME_STACK_DEPTH              1  max stack depth selection name stack >= 64. See glPushName.
//   GL_MAX_PIXEL_MAP_TABLE               1  max size glPixelMap lookup table >= 32. See glPixelMap.
//   GL_MAX_PROJECTION_STACK_DEPTH        1  max stack depth projection matrix >= 2. See glPushMatrix.
//   GL_MAX_TEXTURE_COORDS                1  max # texture coord sets available to vertex and fragment shaders >= 2. See glActiveTexture, glClientActiveTexture.
//   GL_MAX_TEXTURE_IMAGE_UNITS           1  max supported texture image units that can access texture maps from the fragment shader >= 2. See glActiveTexture.
//   GL_MAX_TEXTURE_LOD_BIAS              1  max absolute texture level-of-detail bias >= 4.
//   GL_MAX_TEXTURE_SIZE                  1  estimate largest texture GL can take >= 64. See GL_PROXY_TEXTURE_1D, GL_PROXY_TEXTURE_2D, glTexImage1D, glTexImage2D.
//   GL_MAX_TEXTURE_STACK_DEPTH           1  max depth of texture matrix stack >= 2. See glPushMatrix.
//   GL_MAX_TEXTURE_UNITS                 1  # conventional (fixed-fn, non-shader) texture units supported >= 2. See glActiveTexture, glClientActiveTexture.
//   GL_MAX_VARYING_FLOATS                1  max # interpolators for varying variables used by vertex/fragment shaders >= 32.
//   GL_MAX_VERTEX_ATTRIBS                1  max # 4-component generic vertex attributes accessible to a vertex shader >= 16. See glVertexAttrib.
//   GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS    1  max texture image units that can access texture maps from the vertex shader. Maybe 0. See glActiveTexture.
//   GL_MAX_VERTEX_UNIFORM_COMPONENTS     1  max # individual values allowed in uniform variable storage for a vertex shader >= 512. See glUniform.
//   GL_MAX_VIEWPORT_DIMS                 2  max width/height of viewport. At least as big as display. See glViewport.
//   GL_MINMAX                            1  bool, are pixel min/max being computed. See glMinmax.
//   GL_MODELVIEW_MATRIX                 16  floats, modelview matrix top of stack. See glPushMatrix.
//   GL_MODELVIEW_STACK_DEPTH             1  count, modelview stack depth, initially 1 (not zero). See glPushMatrix.
//   GL_NAME_STACK_DEPTH                  1  count, number of names on selection name stack. Initially 0. See glPushName.
//   GL_NORMAL_ARRAY                      1  bool, is normal array enabled. See glNormalPointer.
//   GL_NORMAL_ARRAY_BUFFER_BINDING       1  name of normal array buffer. See GL_ARRAY_BUFFER, glNormalPointer, glBindBuffer.
//   GL_NORMAL_ARRAY_STRIDE               1  int offset, see glNormalPointer.
//   GL_NORMAL_ARRAY_TYPE                 1  type like GL_FLOAT. See glNormalPointer.
//   GL_NORMALIZE                         1  bool, are normals auto-scaled to unit length. See glNormal.
//   GL_NUM_COMPRESSED_TEXTURE_FORMATS    1  count of available compressed texture formats. >= 0. See glCompressedTexImage2D.
//   GL_PACK_ALIGNMENT                    1  byte alignment for writing pixel data to memory. Initially 4. See glPixelStore.
//   GL_PACK_IMAGE_HEIGHT                 1  image height for writing pixel data to memory.  Initially 0. See glPixelStore.
//   GL_PACK_LSB_FIRST                    1  bool, how single-bit pixels written to memory. See glPixelStore.
//   GL_PACK_ROW_LENGTH                   1  row length for writing pixel data to memory. Initially 0. See glPixelStore.
//   GL_PACK_SKIP_IMAGES                  1  number of pixel images skipped before first pixel is written to memory. Initially 0. See glPixelStore.
//   GL_PACK_SKIP_PIXELS                  1  number of pixel locations skipped before first pixel is written. See glPixelStore.
//   GL_PACK_SKIP_ROWS                    1  number of rows of pixel locations skipped before the first pixel is written. See glPixelStore.
//   GL_PACK_SWAP_BYTES                   1  bool, are 2- and 4-byte pixel indexes byte-swapped? See glPixelStore.
//   GL_PERSPECTIVE_CORRECTION_HINT       1  name like GL_DONT_CARE. See glHint.
//   GL_PIXEL_MAP_A_TO_A_SIZE             1  size of alpha-to-alpha pixel translation table (1). See glPixelMap.
//   GL_PIXEL_MAP_B_TO_B_SIZE             1  size of blue-to-blue pixel translation table (1). See glPixelMap.
//   GL_PIXEL_MAP_G_TO_G_SIZE             1  size of green-to-green pixel translation table (1). See glPixelMap.
//   GL_PIXEL_MAP_I_TO_A_SIZE             1  size of index-to-alpha pixel translation table (1). See glPixelMap.
//   GL_PIXEL_MAP_I_TO_B_SIZE             1  size of index-to-blue pixel translation table (1). See glPixelMap.
//   GL_PIXEL_MAP_I_TO_G_SIZE             1  size of index-to-green pixel translation table (1). See glPixelMap.
//   GL_PIXEL_MAP_I_TO_I_SIZE             1  size of index-to-index pixel translation table (1). See glPixelMap.
//   GL_PIXEL_MAP_I_TO_R_SIZE             1  size of index-to-red pixel translation table (1). See glPixelMap.
//   GL_PIXEL_MAP_R_TO_R_SIZE             1  size of red-to-red pixel translation table (1). See glPixelMap.
//   GL_PIXEL_MAP_S_TO_S_SIZE             1  size of stencil-to-stencil pixel translation table (1). See glPixelMap.
//   GL_PIXEL_PACK_BUFFER_BINDING         1  name of buffer object bound to target GL_PIXEL_PACK_BUFFER (0). See glBindBuffer.
//   GL_PIXEL_UNPACK_BUFFER_BINDING       1  name of buffer object bound to target GL_PIXEL_UNPACK_BUFFER (0). See glBindBuffer.
//   GL_POINT_DISTANCE_ATTENUATION        3  coefficients for computing the attenuation value for points. See glPointParameter.
//   GL_POINT_FADE_THRESHOLD_SIZE         1  point size threshold for determining the point size. See glPointParameter.
//   GL_POINT_SIZE                        1  point size from glPointSize (1).
//   GL_POINT_SIZE_GRANULARITY            1  size diff tween adjacent supported sizes for antialiased points. See glPointSize.
//   GL_POINT_SIZE_MAX                    1  upper bound for attenuated point sizes. Initially 0.0. See glPointParameter.
//   GL_POINT_SIZE_MIN                    1  lower bound for attenuated point sizes. Initially 1.0. See glPointParameter.
//   GL_POINT_SIZE_RANGE                  2  smallest/largest supported sizes for antialiased points. <=1 and >=1. See glPointSize.
//   GL_POINT_SMOOTH                      1  bool, is antialiasing of points enabled. See glPointSize.
//   GL_POINT_SMOOTH_HINT                 1  name, mode of point antialiasing hint (GL_DONT_CARE). See glHint.
//   GL_POINT_SPRITE                      1  bool, is point sprite enabled.
//   GL_POLYGON_MODE                      2  names, are front/back polys drawn as points, lines, or filled polygons (GL_FILL). See glPolygonMode.
//   GL_POLYGON_OFFSET_FACTOR             1  scale used for fragments when polygons are drawn (0). See glPolygonOffset.
//   GL_POLYGON_OFFSET_UNITS              1  another scale for fragments when polys are drawn (0). See glPolygonOffset.
//   GL_POLYGON_OFFSET_FILL               1  bool, is polygon offset on for polys in fill mode. See glPolygonOffset.
//   GL_POLYGON_OFFSET_LINE               1  bool, is poly offset on for polys in line mode. See glPolygonOffset.
//   GL_POLYGON_OFFSET_POINT              1  bool, is poly offset on for polys in point mode. See glPolygonOffset.
//   GL_POLYGON_SMOOTH                    1  bool, is antialias of polygons on. See glPolygonMode.
//   GL_POLYGON_SMOOTH_HINT               1  name, mode of poly antialias (GL_DONT_CARE). See glHint.
//   GL_POLYGON_STIPPLE                   1  bool, is poly stipple on. See glPolygonStipple.
//   GL_POST_COLOR_MATRIX_COLOR_TABLE     1  bool, is post color matrix transformation lookup on. See glColorTable.
//   GL_POST_COLOR_MATRIX_RED_BIAS        1  red bias applied to RGBA fragments after color matrix transformations. See glPixelTransfer.
//   GL_POST_COLOR_MATRIX_GREEN_BIAS      1  green bias applied to RGBA frags after color matrix transformations. See glPixelTransfer.
//   GL_POST_COLOR_MATRIX_BLUE_BIAS       1  blue bias applied to RGBA fragments after color matrix transformations. See glPixelTransfer.
//   GL_POST_COLOR_MATRIX_ALPHA_BIAS      1  alpha bias applied to RGBA fragments after color matrix transformations. See glPixelTransfer.
//   GL_POST_COLOR_MATRIX_RED_SCALE       1  red scale applied to RGBA fragments after color matrix transformations. See glPixelTransfer.
//   GL_POST_COLOR_MATRIX_GREEN_SCALE     1  green scale applied to RGBA fragments after color matrix transformations. See glPixelTransfer.
//   GL_POST_COLOR_MATRIX_BLUE_SCALE      1  blue scale applied to RGBA fragments after color matrix transformations. See glPixelTransfer.
//   GL_POST_COLOR_MATRIX_ALPHA_SCALE     1  alpha scale applied to RGBA fragments after color matrix transformations. See glPixelTransfer.
//   GL_POST_CONVOLUTION_COLOR_TABLE      1  bool, is post convolution lookup on. See glColorTable.
//   GL_POST_CONVOLUTION_RED_BIAS         1  red bias applied to RGBA fragments after convolution. See glPixelTransfer.
//   GL_POST_CONVOLUTION_GREEN_BIAS       1  green bias applied to RGBA fragments after convolution. See glPixelTransfer.
//   GL_POST_CONVOLUTION_BLUE_BIAS        1  blue bias applied to RGBA fragments after convolution. See glPixelTransfer.
//   GL_POST_CONVOLUTION_ALPHA_BIAS       1  alpha bias applied to RGBA fragments after convolution. See glPixelTransfer.
//   GL_POST_CONVOLUTION_RED_SCALE        1  red scale applied to RGBA fragments after convolution. See glPixelTransfer.
//   GL_POST_CONVOLUTION_GREEN_SCALE      1  green scale applied to RGBA fragments after convolution. See glPixelTransfer.
//   GL_POST_CONVOLUTION_BLUE_SCALE       1  blue scale applied to RGBA fragments after convolution. See glPixelTransfer.
//   GL_POST_CONVOLUTION_ALPHA_SCALE      1  alpha scale applied to RGBA fragments after convolution. See glPixelTransfer.
//   GL_PROJECTION_MATRIX                16  projection matrix on top of stack. See glPushMatrix.
//   GL_PROJECTION_STACK_DEPTH            1  count of matrices on the stack. Initially 1. See glPushMatrix.
//   GL_READ_BUFFER                       1  name, color buffer selected for reading (GL_BACK or GL_FRONT). See glReadPixels and glAccum.
//   GL_RED_BIAS                          1  red bias during pixel transfers. See glPixelTransfer.
//   GL_RED_BITS                          1  count red bitplanes in each color buffer.
//   GL_RED_SCALE                         1  red scale during pixel transfers. See glPixelTransfer.
//   GL_RENDER_MODE                       1  name, says if GL is in render, select, or feedback mode (GL_RENDER). See glRenderMode.
//   GL_RESCALE_NORMAL                    1  bool, is normal rescaling on.
//   GL_RGBA_MODE                         1  bool, is GL in RGBA (true) or color-index mode (false). See glColor.
//   GL_SAMPLE_BUFFERS                    1  count of sample buffers associated with the framebuffer. See glSampleCoverage.
//   GL_SAMPLE_COVERAGE_VALUE             1  positive floating-point value, the sample coverage value. See glSampleCoverage.
//   GL_SAMPLE_COVERAGE_INVERT            1  bool, should temporary coverage value should be inverted. See glSampleCoverage.
//   GL_SAMPLES                           1  int size of coverage mask. See glSampleCoverage.
//   GL_SCISSOR_BOX                       4  x/y/width/height window coords of scissor box. See glScissor.
//   GL_SCISSOR_TEST                      1  bool, is scissoring on. See glScissor.
//   GL_SECONDARY_COLOR_ARRAY             1  bool, is secondary color array on. See glSecondaryColorPointer.
//   GL_SECONDARY_COLOR_ARRAY_BUFFER_BINDING
//                                        1  name of buffer associated with 2ndary color array. See GL_ARRAY_BUFFER, glSecondaryColorPointer, glBindBuffer.
//   GL_SECONDARY_COLOR_ARRAY_SIZE        1  count of components/color in 2ndary color array (3). See glSecondaryColorPointer.
//   GL_SECONDARY_COLOR_ARRAY_STRIDE      1  byte offset between consecutive colors in 2ndary color array. See glSecondaryColorPointer.
//   GL_SECONDARY_COLOR_ARRAY_TYPE        1  type like GL_FLOAT. See glSecondaryColorPointer.
//   GL_SELECTION_BUFFER_SIZE             1  count, size of buffer. See glSelectBuffer.
//   GL_SEPARABLE_2D                      1  bool, is 2D separable convolution on. See glSeparableFilter2D.
//   GL_SHADE_MODEL                       1  name, is shading model GL_FLAT or GL_SMOOTH. See glShadeModel.
//   GL_SMOOTH_LINE_WIDTH_RANGE           2  smallest/largest widths for antialiased lines. See glLineWidth.
//   GL_SMOOTH_LINE_WIDTH_GRANULARITY     1  granularity of widths for antialiased lines. See glLineWidth.
//   GL_SMOOTH_POINT_SIZE_RANGE           2  smallest/largest supported widths for antialiased points. See glPointSize.
//   GL_SMOOTH_POINT_SIZE_GRANULARITY     1  granularity of sizes for antialiased points. See glPointSize.
//   GL_STENCIL_BACK_FAIL                 1  name, action taken for back-facing polygons when the stencil test fails (GL_KEEP). See glStencilOpSeparate.
//   GL_STENCIL_BACK_FUNC                 1  name, function used for back-facing polygons to compare stencil ref value
//                                             with stencil buffer value (GL_ALWAYS). See glStencilFuncSeparate.
//   GL_STENCIL_BACK_PASS_DEPTH_FAIL      1  name, action taken for back-facing polygons when stencil test passes,
//                                             but depth test fails (GL_KEEP). See glStencilOpSeparate.
//   GL_STENCIL_BACK_PASS_DEPTH_PASS      1  name, action taken for back-facing polygons when stencil test passes
//                                             and depth test passes (GL_KEEP). See glStencilOpSeparate.
//   GL_STENCIL_BACK_REF                  1  name, ref value that is compared with the contents of the stencil
//                                             buffer for back-facing polygons (0). See glStencilFuncSeparate.
//   GL_STENCIL_BACK_VALUE_MASK           1  mask used for back-facing polygons to mask both stencil ref value and
//                                             stencil buf value before they are compared. Initially all 1's. See glStencilFuncSeparate.
//   GL_STENCIL_BACK_WRITEMASK            1  mask for writing stencil bitplanes for back-facing polygons. Initially all 1's.
//                                             See glStencilMaskSeparate.
//   GL_STENCIL_BITS                      1  count of bitplanes in the stencil buffer.
//   GL_STENCIL_CLEAR_VALUE               1  index of stencil bitplanes to clear (0). See glClearStencil.
//   GL_STENCIL_FAIL                      1  name, action taken when stencil test fails (GL_KEEP). See glStencilOp.
//                                             If GL >= 2.0, this only affects non-polygons and front-facing polygons.
//                                             Back-facing polys use separate stencil state. See glStencilOpSeparate.
//   GL_STENCIL_FUNC                      1  name, function used to compare stencil ref value with the stencil buffer value (GL_ALWAYS).
//                                             See glStencilFunc.
//                                             If GL >= 2.0, only affects non-polys and front-facing polys.
//                                             Back-facing polys use separate stencil state. See glStencilFuncSeparate.
//   GL_STENCIL_PASS_DEPTH_FAIL           1  name, action taken when stencil test passes & depth test fails (GL_KEEP).
//                                             See glStencilOp. If GL >= 2.0, only affects non-polys and front-facing polys.
//                                             Back-facing polys use separate stencil state. See glStencilOpSeparate.
//   GL_STENCIL_PASS_DEPTH_PASS           1  name, action when stencil & depth tests both pass (GL_KEEP). See glStencilOp.
//                                             If GL >= 2.0, only affects non-polys and front-facing polys. Back-facing polys
//                                             use separate stencil state. See glStencilOpSeparate.
//   GL_STENCIL_REF                       1  ref value that is compared with contents of the stencil buf (0). See glStencilFunc.
//                                             If GL >= 2.0, only affects non-polys and front-facing polys. Back-facing polys use
//                                             separate stencil state. See glStencilFuncSeparate.
//   GL_STENCIL_TEST                      1  bool, is stencil testing of fragments on. See glStencilFunc and glStencilOp.
//   GL_STENCIL_VALUE_MASK                1  mask used to mask both stencil ref value and stencil buffer value before they are compared.
//                                             Initial all 1's. See glStencilFunc.
//                                             If GL >= 2.0, only affects non-polys and front-facing polys. Back-facing polys use
//                                             separate stencil state. See glStencilFuncSeparate.
//   GL_STENCIL_WRITEMASK                 1  mask for writing stencil bitplanes (all 1s). See glStencilMask.
//                                             If GL >= 2.0, only affects non-polys and front-facing polys. Back-facing polys use
//                                             separate stencil state. See glStencilMaskSeparate.
//   GL_STEREO                            1  bool, are stereo buffers (left and right) supported.
//   GL_SUBPIXEL_BITS                     1  # of bits (est) of subpixel resolution used to position rasterized geometry in window
//                                             coords. >= 4.
//   GL_TEXTURE_1D                        1  bool, is 1D texture mapping on. See glTexImage1D.
//   GL_TEXTURE_BINDING_1D                1  name of texture currently bound to target GL_TEXTURE_1D. Initially 0. See glBindTexture.
//   GL_TEXTURE_2D                        1  bool, is 2D texture mapping on. See glTexImage2D.
//   GL_TEXTURE_BINDING_2D                1  name of texture currently bound to target GL_TEXTURE_2D. Initially 0. See glBindTexture.
//   GL_TEXTURE_3D                        1  bool, is 3D texture mapping on. See glTexImage3D.
//   GL_TEXTURE_BINDING_3D                1  name of texture currently bound to target GL_TEXTURE_3D. Initially 0. See glBindTexture.
//   GL_TEXTURE_BINDING_CUBE_MAP          1  name of texture currently bound to target GL_TEXTURE_CUBE_MAP (0). See glBindTexture.
//   GL_TEXTURE_COMPRESSION_HINT          1  mode of texture compression hint (GL_DONT_CARE).
//   GL_TEXTURE_COORD_ARRAY               1  bool, is texture coord array on. See glTexCoordPointer.
//   GL_TEXTURE_COORD_ARRAY_BUFFER_BINDING
//                                        1  name of buffer associated with texture coord array.
//                                             See GL_ARRAY_BUFFER, glTexCoordPointer, glBindBuffer.
//   GL_TEXTURE_COORD_ARRAY_SIZE          1  # of coords / element in texture coord array. Initial 4. See glTexCoordPointer.
//   GL_TEXTURE_COORD_ARRAY_STRIDE        1  byte offset tween consecutive elements texture coord array. See glTexCoordPointer.
//   GL_TEXTURE_COORD_ARRAY_TYPE          1  type of coords in texture coord array (GL_FLOAT). See glTexCoordPointer.
//   GL_TEXTURE_CUBE_MAP                  1  bool, is cube-mapped texture mapping on. See glTexImage2D.
//   GL_TEXTURE_GEN_Q                     1  bool, is auto generation of Q texture coord on. See glTexGen.
//   GL_TEXTURE_GEN_R                     1  bool, is auto generation of R texture coord on. See glTexGen.
//   GL_TEXTURE_GEN_S                     1  bool, is auto generation of S texture coord on. See glTexGen.
//   GL_TEXTURE_GEN_T                     1  bool, is auto generation of T texture coord on. See glTexGen.
//   GL_TEXTURE_MATRIX                   16  matrix on top of texture stack. See glPushMatrix.
//   GL_TEXTURE_STACK_DEPTH               1  count of matrixes on texture stack. Initially 1. See glPushMatrix.
//   GL_TRANSPOSE_COLOR_MATRIX           16  matrix, color matrix in row-major order. See glLoadTransposeMatrix.
//   GL_TRANSPOSE_MODELVIEW_MATRIX       16  matrix, modelview matrix in row-major order. See glLoadTransposeMatrix.
//   GL_TRANSPOSE_PROJECTION_MATRIX      16  matrix, projection matrix in row-major order. See glLoadTransposeMatrix.
//   GL_TRANSPOSE_TEXTURE_MATRIX         16  matrix, texture matrix in row-major order. See glLoadTransposeMatrix.
//   GL_UNPACK_ALIGNMENT                  1  byte alignment for reading pixel data from memory (4). See glPixelStore.
//   GL_UNPACK_IMAGE_HEIGHT               1  image height for reading pixel data from memory (0). See glPixelStore.
//   GL_UNPACK_LSB_FIRST                  1  bool, how single-bit pixels read from memory are unpacked from ubytes. See glPixelStore.
//   GL_UNPACK_ROW_LENGTH                 1  row length when reading pixel data from memory (0). See glPixelStore.
//   GL_UNPACK_SKIP_IMAGES                1  # of pixel images skipped before first pixel is read (0). See glPixelStore.
//   GL_UNPACK_SKIP_PIXELS                1  # pixel locations skipped before first pixel is read (0). See glPixelStore.
//   GL_UNPACK_SKIP_ROWS                  1  # rows pixel locations skipped before the first pixel is read (0). See glPixelStore.
//   GL_UNPACK_SWAP_BYTES                 1  bool, are bytes of 2-/4-byte pix indexes swapped? See glPixelStore.
//   GL_VERTEX_ARRAY                      1  bool, is vertex array on. See glVertexPointer.
//   GL_VERTEX_ARRAY_BUFFER_BINDING       1  name of buf object associated with vertex array. Buffer object was bound to target
//                                             GL_ARRAY_BUFFER at last glVertexPointer. Or 0. See glBindBuffer.
//   GL_VERTEX_ARRAY_SIZE                 1  # of coords/vertex in vertex array (4). See glVertexPointer.
//   GL_VERTEX_ARRAY_STRIDE               1  offset between consecutive vertices in vertex array (0). See glVertexPointer.
//   GL_VERTEX_ARRAY_TYPE                 1  type in array (GL_FLOAT). See glVertexPointer.
//   GL_VERTEX_PROGRAM_POINT_SIZE         1  bool, is vertex program point size mode on. If so, and a vertex shader is active,
//                                             then the point size is taken from the shader built-in gl_PointSize.
//                                             If no, and a vertex shader is active, then the point size is taken from the point
//                                             state as specified by glPointSize.
//   GL_VERTEX_PROGRAM_TWO_SIDE           1  bool, is program two-sided color mode on. If yes, and a vertex shader is active,
//                                             then GL chooses the back color output for back-facing polygons, and the front
//                                             color output for non-polygons and front-facing polygons.
//                                             If no and a vertex shader is active, then front color output is always selected.
//   GL_VIEWPORT                          4  X,Y,Width,Height window coords of viewport. See glViewport.
//   GL_ZOOM_X                            1  x pixel zoom factor. Initially 1. See glPixelZoom.
//   GL_ZOOM_Y                            1  y pixel zoom factor. Initially 1. See glPixelZoom.
// _______________________________________________________________________________________________

# include "all.h"
# include "gl_env.h"

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

namespace gl_env {

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Debug
// _______________________________________________________________________________________________

# ifndef NDEBUG

  /* debug only */
  void
assert_not_vertex_entry_active( env_type & env, bool_type is_allowed_during_init /* = false */)
  //
  // is_allowed_during_init
  //   We need this because we call this during init, when is_init( ) is false and is_during_init_or_uninit( ) is true.
  //   The init_* (and uninit_*) methods call ::glGetIntegerv (and maybe ::glIsEnabled).
  //   We don't want to allow most GL calls during init_*, just a few query functions.
{
    if ( env.is_during_init_or_uninit( ) ) {
        d_assert( as_bool( is_allowed_during_init));
        d_assert( ! env.is_init( ));
        env.assert_not_vertex_entry_active__during_init_or_uninit( );
    } else {
        d_assert( as_bool( env.is_init( )));
        d_assert( ! env.is_vertex_entry_active( ));
    }
}

  /* debug only */
  void
assert_no_error( )
{
    // GL_NO_ERROR is guaranteed to be zero (according to the doc).
    enum_type const error = ::glGetError( );
    if ( GL_NO_ERROR == error ) return;

    // From the doc at opengl.org:
    //   If glGetError itself generates an error, it returns 0.
    //
    //   GL_INVALID_OPERATION is generated if glGetError is executed between the execution of glBegin
    //   and the corresponding execution of glEnd. In this case, glGetError returns 0.
    //
    // So if glGetError( ) finds NO error it returns GL_NO_ERROR, which is always zero.
    // But if glGetError( ) CAUSES an error it also returns zero. Yikes.
    // And you're not allowed to call glGetError( ) between glBegin and glEnd. Yikes again.

    // The documentation at opengl.org contains these seemingly contradictor statements:
    //
    //   1 When an error occurs, the error flag is set to the appropriate error code value.
    //     NO OTHER ERRORS are recorded until glGetError is called, the error code is returned,
    //     and the flag is reset to GL_NO_ERROR.
    //
    //   2 To allow for distributed implementations, there may be several error flags.
    //     If any single error flag has recorded an error, the value of that flag is returned
    //     and that flag is reset to GL_NO_ERROR when glGetError is called.
    //     If more than one flag has recorded an error, glGetError returns and clears an arbitrary error flag value.
    //     Thus, glGetError should always be called in a loop, until it returns GL_NO_ERROR, if all error flags are to be reset.
    //
    // The following loops until ::glGetError( ) returns GL_NO_ERROR.
    int count_extra_errors = 0;
    while ( count_extra_errors < 100 ) {
        enum_type const error2 = ::glGetError( );
        if ( error2 == GL_NO_ERROR ) break;
        d_assert( error == error2);
        count_extra_errors += 1;
    }

    // Switch on the error values so you can set break points for each one individually.
    switch ( error ) {
      case GL_INVALID_ENUM:
        d_assert( false);
        break;

      case GL_INVALID_VALUE:
        d_assert( false);
        break;

      case GL_INVALID_OPERATION:
        // GL_INVALID_OPERATION is 1282 or 0x502
        d_assert( false);
        break;

      case GL_STACK_OVERFLOW:
        d_assert( false);
        break;

      case GL_STACK_UNDERFLOW:
        d_assert( false);
        break;

      case GL_OUT_OF_MEMORY:
        d_assert( false);
        break;

      // Not part of 1.1:
      // case GL_TABLE_TOO_LARGE:
      //    d_assert( false);
      //    break;

      default:
        d_assert( false);
        break;
    }
}

// _______________________________________________________________________________________________

  /* ctor, debug only */
  assert_no_errors::
assert_no_errors( env_type & e, bool_type allow_during_init /* = false */)
  : env_val_base< bool_type >( e, allow_during_init)
{
    assert_not_vertex_entry_active( e, allow_during_init);
}

  /* dtor, debug only */
  assert_no_errors::
~assert_no_errors( )
{
    assert_not_vertex_entry_active( env, val);
    assert_no_error( );
}

// _______________________________________________________________________________________________

  /* ctor, debug only */
  assert_during_vertex_entry::
assert_during_vertex_entry( env_type & e)
  : env_base( e)
{
    d_assert( as_bool( e.is_vertex_entry_active( )));
}

  /* dtor, debug only */
  assert_during_vertex_entry::
~assert_during_vertex_entry( )
{
    d_assert( as_bool( env.is_vertex_entry_active( )));
}

# endif

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Init
// _______________________________________________________________________________________________

  void
  env_type::
init( bool_type is_pristine, bool_type check_against_minimum_max_values /* = true */)
{
    d_assert( ! is_during_init_or_uninit( ));
    if ( ! is_init( ) ) {
        is_during_init_or_uninit_ = true;

        is_assuming_pristine_ = is_pristine;
        is_checking_against_min_max_values_ = check_against_minimum_max_values;

        // These are called while is_init( ) is false.
        init_active_vertex( );
        init_matrix_values( );
        init_saved_server_attributes_stack_values( );
        init_clip_planes( );
        init_lighting( );

        // is_init( ) is false until all the sub-init functions are done.
        // This means you have to be careful calling functions that require is_init( )
        // in the above init..() sub-functions.
        is_init_ = true;
        is_during_init_or_uninit_ = false;
    }
    d_assert( as_bool( is_init( )));
}

  void
  env_type::
uninit( )
{
    d_assert( ! is_during_init_or_uninit( ));
    if ( is_init( ) ) {
        // is_init( ) is false during all the sub uninit..() functions.
        is_during_init_or_uninit_ = true;
        is_init_ = false;

        uninit_lighting( );
        uninit_clip_planes( );
        uninit_saved_server_attributes_stack_values( );
        uninit_matrix_values( );
        uninit_active_vertex( );

        // Should we change:
        //   is_assuming_pristine_
        //   is_checking_against_min_max_values_
        is_during_init_or_uninit_ = false;
    }
    d_assert( ! is_init( ));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// glGet*(..) wrappers
//
//   We don't wrap glGetBooleanv(..). Use is_enabled(..) instead.
//
//   This only provides functions that return 1, 2, 3 and 4 values.
//   The vast majority of the time glGet* returns just one value, and the 2/4 cases cover most
//   of the other cases. But there are some flags that expect other-length arrays:
//      16 (always a matrix, floats or doubles)
//      GL_NUM_COMPRESSED_TEXTURE_FORMATS (not in version 1.1)
//      3 (GL_CURRENT_NORMAL, GL_POINT_DISTANCE_ATTENUATION)
//
//   Many of the double values returned are min/max.
//   Many of the quad values are RGBA, XYZW, STQR (texture coord), or (x,y,width,height).
//
//   We need templated versions of these methods:
//      get_p3< float >( GL_CURRENT_XX, pArgs)
//      get_p< 3, float >( GL_CURRENT_XX, pArgs)
//      get_p< float >( name, count, pArgs)
//      get< float >( name, val0, val1)
//
//   We need methods that return boost::array< type, count >.
//     The user can choose between:
//       Returning a single value
//       Returning an array of values (an array of 1 is the single value)
//       Passing in N refs to be set
//       Passing in an array to be filled
//
//   We need pointer versions for 1 arg?
//
//   See boost::array< type, count >.
//
//   Since the only integer type supported is int_type, needs conversions to/from:
//     byte_type, ubyte_type
//     short_type, ushort_type
//     uint_type
//
//   We assume int_type converts easily to/from enum_type and size_type.
//   We need conversions between fixed-point ints (both signed and unsigned) and the other values.
// _______________________________________________________________________________________________

  bool_type
  env_type::
get_bool( enum_type flag)
  //
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    bool_type vals[ 2 ];
    ::memset( vals, -1, sizeof( vals));
  # ifndef NDEBUG
    bool_type const debug_val = vals[ 1 ];
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetBooleanv( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 1 ]);
    // If GL_FALSE and GL_TRUE are enum_type (are they? or are they bool_type?), the following
    // assumes bool_type can convert to enum_type.
    d_assert( (GL_FALSE == vals[ 0 ]) || (GL_TRUE == vals[ 0 ]));
    return vals[ 0 ];
}

  void
  env_type::
get_2_bools( enum_type flag, bool_type & val0, bool_type & val1)
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    bool_type vals[ 3 ];
    ::memset( vals, -1, sizeof( vals));
  # ifndef NDEBUG
    bool_type const debug_val = vals[ 2 ];
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetBooleanv( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 2 ]);
    // If GL_FALSE and GL_TRUE are enum_type (are they? or are they bool_type?), the following
    // assumes bool_type can convert to enum_type. In MSWindows they're #define'd 0 and 1.
    d_assert( (GL_FALSE == vals[ 0 ]) || (GL_TRUE == vals[ 0 ]));
    d_assert( (GL_FALSE == vals[ 1 ]) || (GL_TRUE == vals[ 1 ]));
    val0 = vals[ 0 ];
    val1 = vals[ 1 ];
}

// _______________________________________________________________________________________________

// The following implementation assumes you can safely convert between these types.
//   int_type *
//   enum_type *
// In other words, that their underlying representations are the same size.
d_static_assert( sizeof( enum_type) == sizeof( int_type));

// Casting refs between enum_type and int_type is really only safe if we know it's
// safe because both are primitive integral types of the same size.
d_static_assert( (boost::mpl::and_
     <  boost::is_fundamental< enum_type >
      , boost::is_integral<    enum_type >
     >::value));
d_static_assert( (boost::mpl::and_
     <  boost::is_fundamental< int_type >
      , boost::is_integral<    int_type >
     >::value));
d_static_assert( (boost::is_convertible< int_type, enum_type >::value));
d_static_assert( (boost::is_convertible< enum_type, int_type >::value));


  enum_type
  env_type::
get_enum( enum_type flag)
{
    // We're not assuming enum_type and int_type are the same size here, only that int_type
    // is convertable into enum_type.
    return static_cast< enum_type >( get_int( flag));
}

  void
  env_type::
get_2_enums( enum_type flag, enum_type & a, enum_type & b)
{
    // Even though enum_type and int_type are the same size, the compiler does not accept the following:
    //   int_type & ra = static_cast< int_type & >( a); /* does not compile */
    //   int_type & ra = * static_cast< int_type * >( & a); /* does not compile */
    get_2_ints( flag, * reinterpret_cast< int_type * >( & a), * reinterpret_cast< int_type * >( & b));
}

// Here is the implementation that does not assume enum_type and int_type are the same size:
# if 0
  enum_type
  env_type::
get_enum( enum_type flag)
{
    // Assume that int_type and enum_type are freely convertable back and forth.
    // This is an assumption implied by GL [specifically ::glGetIntegerv(..)].
    int_type  const ia = get_int( flag);
    enum_type const a  = static_cast< enum_type >( ia);
    d_assert( static_cast< int_type const >( a) == ia);
    return a;
}

  void
  env_type::
get_2_enums( enum_type flag, enum_type & a, enum_type & b)
{
    // Assume that int_type and enum_type are freely convertable back and forth.
    // This is an assumption implied by GL [specifically ::glGetIntegerv(..)].
    int_type ia = -1;
    int_type ib = -1;
    get_2_ints( flag, ia, ib);
    a = static_cast< enum_type >( ia);
    b = static_cast< enum_type >( ib);
    d_assert( static_cast< int_type const >( a) == ia);
    d_assert( static_cast< int_type const >( b) == ib);
}
# endif

// _______________________________________________________________________________________________

// The following implementation assumes you can safely convert between these types.
//   int_type *
//   uint_type *
// In other words, that their underlying representations are the same size.
d_static_assert( sizeof( uint_type) == sizeof( int_type));

// Casting refs between uint_type and int_type is really only safe if we know it's
// safe because both are primitive integral types of the same size.
// We've already performed some of these tests above in this file.
d_static_assert( (boost::mpl::and_
     <  boost::is_fundamental< uint_type >
      , boost::is_integral<    uint_type >
     >::value));
d_static_assert( (boost::is_convertible< int_type, uint_type >::value));
d_static_assert( (boost::is_convertible< uint_type, int_type >::value));


  uint_type
  env_type::
get_uint( enum_type flag)
{
    // We're not assuming uint_type and int_type are the same size here, only that int_type
    // is convertable into uint_type.
    return static_cast< uint_type >( get_int( flag));
}

  void
  env_type::
get_2_uints( enum_type flag, uint_type & a, uint_type & b)
{
    // These static_casts will not compile if uint_type is smaller than int_type.
    get_2_ints( flag, * reinterpret_cast< int_type * >( & a), * reinterpret_cast< int_type * >( & b));
}

// _______________________________________________________________________________________________

  int_type
  env_type::
get_int( enum_type flag)
  //
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    int_type vals[ 2 ];
    vals[ 0 ] = -1;
  # ifndef NDEBUG
    int_type const debug_val = -555;
    vals[ 1 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    // This has to work during init_*, when is_init( ) is still false.
    {   assert_no_errors ne( *this, true);
        ::glGetIntegerv( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 1 ]);
    return vals[ 0 ];
}

  void
  env_type::
get_2_ints( enum_type flag, int_type & val0, int_type & val1)
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    int_type vals[ 3 ];
    ::memset( vals, -1, sizeof( vals) - sizeof( vals[ 0 ]));
  # ifndef NDEBUG
    int_type const debug_val = -555;
    vals[ 2 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetIntegerv( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 2 ]);
    val0 = vals[ 0 ];
    val1 = vals[ 1 ];
}

  void
  env_type::
get_3_ints( enum_type flag, int_type & val0, int_type & val1, int_type & val2)
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    int_type vals[ 4 ];
    ::memset( vals, -1, sizeof( vals) - sizeof( vals[ 0 ]));
  # ifndef NDEBUG
    int_type const debug_val = -555;
    vals[ 3 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetIntegerv( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 3 ]);
    val0 = vals[ 0 ];
    val1 = vals[ 1 ];
    val2 = vals[ 2 ];
}

  void
  env_type::
get_4_ints( enum_type flag, int_type & val0, int_type & val1, int_type & val2, int_type & val3)
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    int_type vals[ 5 ];
    ::memset( vals, -1, sizeof( vals) - sizeof( vals[ 0 ]));
  # ifndef NDEBUG
    int_type const debug_val = -555;
    vals[ 4 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetIntegerv( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 4 ]);
    val0 = vals[ 0 ];
    val1 = vals[ 1 ];
    val2 = vals[ 2 ];
    val3 = vals[ 3 ];
}

// _______________________________________________________________________________________________

  float_type
  env_type::
get_float( enum_type flag)
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    float_type vals[ 2 ];
    vals[ 0 ] = -1;
  # ifndef NDEBUG
    float_type const debug_val = -555.5555f;
    vals[ 1 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetFloatv( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 1 ]);
    return vals[ 0 ];
}

  void
  env_type::
get_2_floats( enum_type flag, float_type & val0, float_type & val1)
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    float_type vals[ 3 ];
    ::memset( vals, -1, sizeof( vals) - sizeof( vals[ 0 ]));
  # ifndef NDEBUG
    float_type const debug_val = -555.5555f;
    vals[ 2 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetFloatv( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 2 ]);
    val0 = vals[ 0 ];
    val1 = vals[ 1 ];
}

  void
  env_type::
get_3_floats( enum_type flag, float_type & val0, float_type & val1, float_type & val2)
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    float_type vals[ 4 ];
    ::memset( vals, -1, sizeof( vals) - sizeof( vals[ 0 ]));
  # ifndef NDEBUG
    float_type const debug_val = -555.5555f;
    vals[ 3 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetFloatv( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 3 ]);
    val0 = vals[ 0 ];
    val1 = vals[ 1 ];
    val2 = vals[ 2 ];
}

  void
  env_type::
get_4_floats( enum_type flag, float_type & val0, float_type & val1, float_type & val2, float_type & val3)
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    float_type vals[ 5 ];
    ::memset( vals, -1, sizeof( vals) - sizeof( vals[ 0 ]));
  # ifndef NDEBUG
    float_type const debug_val = -555.5555f;
    vals[ 4 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetFloatv( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 4 ]);
    val0 = vals[ 0 ];
    val1 = vals[ 1 ];
    val2 = vals[ 2 ];
    val3 = vals[ 3 ];
}

// _______________________________________________________________________________________________

  double_type
  env_type::
get_double( enum_type flag)
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    double_type vals[ 2 ];
    vals[ 0 ] = -1;
  # ifndef NDEBUG
    double_type const debug_val = -555.5555;
    vals[ 1 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetDoublev( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 1 ]);
    return vals[ 0 ];
}

  void
  env_type::
get_2_doubles( enum_type flag, double_type & val0, double_type & val1)
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    double_type vals[ 3 ];
    ::memset( vals, -1, sizeof( vals) - sizeof( vals[ 0 ]));
  # ifndef NDEBUG
    double_type const debug_val = -555.5555;
    vals[ 2 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetDoublev( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 2 ]);
    val0 = vals[ 0 ];
    val1 = vals[ 1 ];
}

  void
  env_type::
get_3_doubles( enum_type flag, double_type & val0, double_type & val1, double_type & val2)
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    double_type vals[ 4 ];
    ::memset( vals, -1, sizeof( vals) - sizeof( vals[ 0 ]));
  # ifndef NDEBUG
    double_type const debug_val = -555.5555;
    vals[ 3 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetDoublev( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 3 ]);
    val0 = vals[ 0 ];
    val1 = vals[ 1 ];
    val2 = vals[ 2 ];
}

  void
  env_type::
get_4_doubles( enum_type flag, double_type & val0, double_type & val1, double_type & val2, double_type & val3)
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    double_type vals[ 5 ];
    ::memset( vals, -1, sizeof( vals) - sizeof( vals[ 0 ]));
  # ifndef NDEBUG
    double_type const debug_val = -555.5555;
    vals[ 4 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetDoublev( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 4 ]);
    val0 = vals[ 0 ];
    val1 = vals[ 1 ];
    val2 = vals[ 2 ];
    val3 = vals[ 3 ];
}

  void
  env_type::
get_16_doubles
 (  enum_type     flag
  , double_type & val0 , double_type & val1 , double_type & val2 , double_type & val3
  , double_type & val4 , double_type & val5 , double_type & val6 , double_type & val7
  , double_type & val8 , double_type & val9 , double_type & val10, double_type & val11
  , double_type & val12, double_type & val13, double_type & val14, double_type & val15
 )
  // flag should be one of:
  //   GL_MODELVIEW_MATRIX
  //   GL_PROJECTION_MATRIX
  //   GL_TEXTURE_MATRIX
  //   GL_COLOR_MATRIX
  //
  //   GL_TRANSPOSE_MODELVIEW_MATRIX
  //   GL_TRANSPOSE_PROJECTION_MATRIX
  //   GL_TRANSPOSE_TEXTURE_MATRIX
  //   GL_TRANSPOSE_COLOR_MATRIX
  //
  // The color matrices are not part of GL 1.1, and the # defines are missing from the windows <GL.h> include file.
{
    // Setup call to GL. Provide extra buffer so we can watch for overflows.
    double_type vals[ 17 ];
    ::memset( vals, -1, sizeof( vals) - sizeof( vals[ 0 ]));
  # ifndef NDEBUG
    double_type const debug_val = -555.5555;
    vals[ 16 ] = debug_val;
  # endif

    // Call GL and check for errors and state.
    {   assert_no_errors ne( *this);
        ::glGetDoublev( flag, vals);
    }

    // Check overflow and return value(s).
    d_assert( debug_val == vals[ 16 ]);
    val0  = vals[  0 ];
    val1  = vals[  1 ];
    val2  = vals[  2 ];
    val3  = vals[  3 ];

    val4  = vals[  4 ];
    val5  = vals[  5 ];
    val6  = vals[  6 ];
    val7  = vals[  7 ];

    val8  = vals[  8 ];
    val9  = vals[  9 ];
    val10 = vals[ 10 ];
    val11 = vals[ 11 ];

    val12 = vals[ 12 ];
    val13 = vals[ 13 ];
    val14 = vals[ 14 ];
    val15 = vals[ 15 ];
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
// Matrix stacks
//
//   OpenGL comes with these matrix stacks:
//     ModelView
//     Projection
//     Texture
//     Color
//
//   The GL.h header included with MSVC9 is for version 1.1 and does not include the color
//   matrix stack or other ARB_imaging extensions (although most machines probably support it).
//
//   OpenGL has other stacks too. We're not dealing with them here.
//     Name
//     Attrib
//     ClientAttrib
//
//   Matrix stacks come with these names:
//     GL_MATRIX_MODE         - identifies which stack is active
//     GL_*                   - id for ::glMatrixMode(id)
//     GL_*_MATRIX            - get 16 values of the matrix in column-major order
//     GL_*_STACK_DEPTH       - get the current depth, 1..MAX
//     GL_MAX_*_STACK_DEPTH   - get the constant max depth
//     GL_TRANSPOSE_*_MATRIX  - get 16 values in row-major order
//   Where * is MODELVIEW, PROJECTION, TEXTURE, or COLOR.
//
//   This API all deals with the vertex matrices:
//     glFrustum
//     glLoadIdentity
//     glLoadMatrix
//     glLoadTransposeMatrix  - not 1.1
//     glMatrixMode
//     glMultMatrix
//     glMultTransposeMatrix  - not 1.1
//     glOrtho
//     glPushMatrix
//     glPopMatrix
//     glRotate
//     glScale
//     glTranslate
//     glViewport
//
//     gluLookAt
//     gluOrtho2D
//     gluPerspective
//
//     gluPickMatrix
//     gluProject
//     gluUnProject
//     gluUnProject4
// _______________________________________________________________________________________________

  void
  env_type::
init_matrix_values( )
{
  # ifndef NDEBUG
    // Check the matrix-mode value.
    {   enum_type const mat_mode = get_current_matrix_name_from_gl( );
        // Strong assumption, always true (unless this is GL_COLOR).
        d_assert(
            (mat_mode == GL_MODELVIEW ) ||
            (mat_mode == GL_PROJECTION) ||
            (mat_mode == GL_TEXTURE   ) );
    }

    // Check stack depths.
    {   int_type const  mv_sd         = get_stack_depth_modelview_matrix_from_gl(  );
        int_type const  pr_sd         = get_stack_depth_projection_matrix_from_gl( );
        int_type const  tx_sd         = get_stack_depth_texture_matrix_from_gl(    );

        int_type const  mv_max_sd     = get_max_stack_depth_modelview_matrix_from_gl(  );
        int_type const  pr_max_sd     = get_max_stack_depth_projection_matrix_from_gl( );
        int_type const  tx_max_sd     = get_max_stack_depth_texture_matrix_from_gl(    );

        int_type const  mv_minmax_sd  = get_min_max_stack_depth_modelview_matrix(  );
        int_type const  pr_minmax_sd  = get_min_max_stack_depth_projection_matrix( );
        int_type const  tx_minmax_sd  = get_min_max_stack_depth_texture_matrix(    );

        // Check that stack depths do not exceed their maximums.
        d_assert( (0 < mv_sd) && (mv_sd <= mv_max_sd));
        d_assert( (0 < pr_sd) && (pr_sd <= pr_max_sd));
        d_assert( (0 < tx_sd) && (tx_sd <= tx_max_sd));

        // Check that the actual maximums are at least as big as what they're supposed to be
        // according to the standard, as reported on opengl.org.
        // Check this even if is_checking_against_min_max_values( ) is false.
        d_assert( mv_minmax_sd <= mv_max_sd);
        d_assert( pr_minmax_sd <= pr_max_sd);
        d_assert( tx_minmax_sd <= tx_max_sd);
    }
  # endif

    // If this environment is starting on a pristine GL setup.
    if ( is_assuming_pristine( ) ) {
        // The model-view matrix starts out as the current one.
        d_assert( get_current_matrix_name_from_gl( ) == GL_MODELVIEW);

        // Correct initialization, but ONLY IF the weak assumptions (that we are starting out pristine) hold.
        current_matrix_name_ = GL_MODELVIEW;

        // The matrix stacks are all supposed to start out with a single matrix in them.
        // This is different from the attribute stack, which saves attributes that are not on a stack and starts out empty.
        d_assert( get_stack_depth_modelview_matrix_from_gl(  ) == 1);
        d_assert( get_stack_depth_projection_matrix_from_gl( ) == 1);
        d_assert( get_stack_depth_texture_matrix_from_gl(    ) == 1);

        stack_depth_modelview_matrix_  = 1;
        stack_depth_projection_matrix_ = 1;
        stack_depth_texture_matrix_    = 1;
    } else {
        // Initialize from GL.
        current_matrix_name_ = get_current_matrix_name_from_gl( );

        // Initialize from GL.
        stack_depth_modelview_matrix_  = get_stack_depth_modelview_matrix_from_gl(  );
        stack_depth_projection_matrix_ = get_stack_depth_projection_matrix_from_gl( );
        stack_depth_texture_matrix_    = get_stack_depth_texture_matrix_from_gl(    );
    }

    assert_current_matrix_name( );
    assert_current_stack_depths( );
}

  void
  env_type::
uninit_matrix_values( )
{
    // We cannot make any of these assumptions because this object gets dtor'ed after GL is gone and
    // there is no active context.
  # if 0

    // get_current_matrix_name( ) can be any value when we get here. But is should agree with GL.
    assert_current_matrix_name( );

    // The stacks should agree with our stack-depth values.
    assert_current_stack_depths( );

    // All the stacks should be popped and set to values set in init_matrix_values( ).
    // We don't save these init values, but we know they're 1 when is_assuming_pristine( ) is true.
  # ifndef NDEBUG
    if ( is_assuming_pristine( ) ) {
        d_assert( get_stack_depth_modelview_matrix(  ) == 1);
        d_assert( get_stack_depth_projection_matrix( ) == 1);
        d_assert( get_stack_depth_texture_matrix(    ) == 1);
    }
  # endif
  # endif
}

// _______________________________________________________________________________________________

# ifndef NDEBUG
  void
  env_type::
assert_is_identity_matrix( enum_type matrix_name)
  //
  // This works for regular (column-major) and transpose (row-major) matrices:
  //   GL_MODELVIEW_MATRIX
  //   GL_PROJECTION_MATRIX
  //   GL_TEXTURE_MATRIX
  //   GL_COLOR_MATRIX  - not in GL 1.1
  //
  //   GL_TRANSPOSE_MODELVIEW_MATRIX
  //   GL_TRANSPOSE_PROJECTION_MATRIX
  //   GL_TRANSPOSE_TEXTURE_MATRIX
  //   GL_TRANSPOSE_COLOR_MATRIX  - not in GL 1.1

{
    double_type  val0  = -1, val1  = -1, val2  = -1, val3  = -1;
    double_type  val4  = -1, val5  = -1, val6  = -1, val7  = -1;
    double_type  val8  = -1, val9  = -1, val10 = -1, val11 = -1;
    double_type  val12 = -1, val13 = -1, val14 = -1, val15 = -1;

    get_16_doubles
     (  matrix_name
      , val0 , val1 , val2 , val3
      , val4 , val5 , val6 , val7
      , val8 , val9 , val10, val11
      , val12, val13, val14, val15
     );

    // This test works for both regular and transpose matrices.
    d_assert( 1 == val0 );
    d_assert( 0 == val1 );
    d_assert( 0 == val2 );
    d_assert( 0 == val3 );
    d_assert( 0 == val4 );
    d_assert( 1 == val5 );
    d_assert( 0 == val6 );
    d_assert( 0 == val7 );
    d_assert( 0 == val8 );
    d_assert( 0 == val9 );
    d_assert( 1 == val10);
    d_assert( 0 == val11);
    d_assert( 0 == val12);
    d_assert( 0 == val13);
    d_assert( 0 == val14);
    d_assert( 1 == val15);
 }
# endif

// _______________________________________________________________________________________________

  int_type
  env_type::
get_stack_depth( enum_type matrix_name) const
{
    switch ( matrix_name ) {
      case GL_MODELVIEW :  return get_stack_depth_modelview_matrix(  );
      case GL_PROJECTION:  return get_stack_depth_projection_matrix( );
      case GL_TEXTURE   :  return get_stack_depth_texture_matrix(    );
      default:
        d_assert( false);
    }
    return 0;
}

// _______________________________________________________________________________________________

  void
  env_type::
push_copy_on_current_matrix_stack( )
{
    // Stack depths should agree at first.
    assert_current_stack_depths( );

    // Check that max-depth will not be exceeded.
    // Should we check min-max? Or just max?
  # ifndef NDEBUG
    if ( is_checking_against_min_max_values( ) ) {
        switch ( get_current_matrix_name( ) ) {
          case GL_MODELVIEW :  d_assert( get_stack_depth_modelview_matrix(  ) < get_min_max_stack_depth_modelview_matrix(  ));  break;
          case GL_PROJECTION:  d_assert( get_stack_depth_projection_matrix( ) < get_min_max_stack_depth_projection_matrix( ));  break;
          case GL_TEXTURE   :  d_assert( get_stack_depth_texture_matrix(    ) < get_min_max_stack_depth_texture_matrix(    ));  break;
          default:
            d_assert( false);
        }
    } else {
        switch ( get_current_matrix_name( ) ) {
          case GL_MODELVIEW :  d_assert( get_stack_depth_modelview_matrix(  ) < get_max_stack_depth_modelview_matrix_from_gl(  ));  break;
          case GL_PROJECTION:  d_assert( get_stack_depth_projection_matrix( ) < get_max_stack_depth_projection_matrix_from_gl( ));  break;
          case GL_TEXTURE   :  d_assert( get_stack_depth_texture_matrix(    ) < get_max_stack_depth_texture_matrix_from_gl(    ));  break;
          default:
            d_assert( false);
        }
    }
  # endif

    // Push a new copy of the current matrix to the stack.
    {   assert_no_errors ne( *this);
        ::glPushMatrix( );
    }

    // Increment our counter of the stack depth.
    switch ( get_current_matrix_name( ) ) {
      case GL_MODELVIEW :  stack_depth_modelview_matrix_  += 1;  break;
      case GL_PROJECTION:  stack_depth_projection_matrix_ += 1;  break;
      case GL_TEXTURE   :  stack_depth_texture_matrix_    += 1;  break;
      default:
        d_assert( false);
    }

    // Stack depths should agree again.
    assert_current_stack_depths( );
}

// _______________________________________________________________________________________________

  void
  env_type::
pop_current_matrix_stack( )
{
    // Stack depths should agree at first.
    assert_current_stack_depths( );

    // Check that stack will not underflow.
  # ifndef NDEBUG
    switch ( get_current_matrix_name( ) ) {
      case GL_MODELVIEW :  d_assert( get_stack_depth_modelview_matrix(  ) > 1);  break;
      case GL_PROJECTION:  d_assert( get_stack_depth_projection_matrix( ) > 1);  break;
      case GL_TEXTURE   :  d_assert( get_stack_depth_texture_matrix(    ) > 1);  break;
      default:
        d_assert( false);
    }
  # endif

    // Pop top copy of the current matrix on the stack.
    {   assert_no_errors ne( *this);
        ::glPopMatrix( );
    }

    // Decrement our counter of the stack depth.
    switch ( get_current_matrix_name( ) ) {
      case GL_MODELVIEW :  stack_depth_modelview_matrix_  -= 1;  break;
      case GL_PROJECTION:  stack_depth_projection_matrix_ -= 1;  break;
      case GL_TEXTURE   :  stack_depth_texture_matrix_    -= 1;  break;
      default:
        d_assert( false);
    }

    // Stack depths should agree again.
    assert_current_stack_depths( );
}

// _______________________________________________________________________________________________

  void
  env_type::
set_current_matrix_name( enum_type name)
{
    // Check param
    d_assert(
        (GL_MODELVIEW  == name) ||
        (GL_PROJECTION == name) ||
        (GL_TEXTURE    == name) );

    // Set mode, both here and in GL
    if ( get_current_matrix_name( ) != name ) {
        { assert_no_errors ne( *this);
          ::glMatrixMode( name);
        }
        current_matrix_name_ = name;
    }

    // Confirm that change happened.
    assert_current_matrix_name( name);
}

// _______________________________________________________________________________________________

  void
  env_type::
reset_current_matrix( )
{
    assert_current_stack_depths( );
    {   assert_no_errors ne( *this);
        ::glLoadIdentity( );
    }
    assert_current_stack_depths( );
}

  void
  env_type::
set_current_matrix( float_type const * p16_matrix)
{
    assert_current_stack_depths( );
    {   assert_no_errors ne( *this);
        ::glLoadMatrixf( p16_matrix);
    }
    assert_current_stack_depths( );
}

  void
  env_type::
set_current_matrix( double_type const * p16_matrix)
{
    assert_current_stack_depths( );
    {   assert_no_errors ne( *this);
        ::glLoadMatrixd( p16_matrix);
    }
    assert_current_stack_depths( );
}

// _______________________________________________________________________________________________

  void
  env_type::
multiply_current_matrix( float_type const * p16_matrix)
{
    assert_current_stack_depths( );
    {   assert_no_errors ne( *this);
        ::glMultMatrixf( p16_matrix);
    }
    assert_current_stack_depths( );
}

  void
  env_type::
multiply_current_matrix( double_type const * p16_matrix)
{
    assert_current_stack_depths( );
    {   assert_no_errors ne( *this);
        ::glMultMatrixd( p16_matrix);
    }
    assert_current_stack_depths( );
}

// _______________________________________________________________________________________________

  void
  env_type::
rotate_current_matrix( float_type angle, float_type x_vect, float_type y_vect, float_type z_vect)
  //
  // Rotates the scene counter-clockwise around the vector (assuming the vect is pointing away from you).
  // Angle is given in degrees, NOT radians. If angle is zero, this does nothing.
{
    // Only accept sane values.
  # if 0
    d_assert( ! util::is_non_number( angle));
    d_assert( ! util::is_non_number( x_vect));
    d_assert( ! util::is_non_number( y_vect));
    d_assert( ! util::is_non_number( z_vect));
  # endif

    // The vector cannot be zero-length.
    d_assert( (0 != x_vect) || (0 != y_vect) || (0 != z_vect));

    assert_no_errors ne( *this);
    ::glRotatef( angle, x_vect, y_vect, z_vect);
}

  void
  env_type::
rotate_current_matrix( double_type angle, double_type x_vect, double_type y_vect, double_type z_vect)
  //
  // Rotates the scene counter-clockwise around the vector (assuming the vect is pointing away from you).
  // Angle is given in degrees, NOT radians. If angle is zero, this does nothing.
{
    // Only accept sane values.
  # if 0
    d_assert( ! util::is_non_number( angle));
    d_assert( ! util::is_non_number( x_vect));
    d_assert( ! util::is_non_number( y_vect));
    d_assert( ! util::is_non_number( z_vect));
  # endif

    // The vector cannot be zero-length.
    d_assert( (0 != x_vect) || (0 != y_vect) || (0 != z_vect));

    assert_no_errors ne( *this);
    ::glRotated( angle, x_vect, y_vect, z_vect);
}

// _______________________________________________________________________________________________

  void
  env_type::
scale_current_matrix( float_type x_scale, float_type y_scale, float_type z_scale)
  //
  // This can mess up the normals, and thus the lighting. Should we keep track of that?
{
    // Only accept sane values.
  # if 0
    d_assert( ! util::is_non_number( x_scale));
    d_assert( ! util::is_non_number( y_scale));
    d_assert( ! util::is_non_number( z_scale));
  # endif

    // Zero values are probably legal, but probably accidental.
    // Negative values reflect around a plane, and so are wrong unless you are using them
    // for some special effect, probably involving mirrors.
    d_assert( x_scale > 0);
    d_assert( y_scale > 0);
    d_assert( z_scale > 0);

    assert_no_errors ne( *this);
    ::glScalef( x_scale, y_scale, z_scale);
}

  void
  env_type::
scale_current_matrix( double_type x_scale, double_type y_scale, double_type z_scale)
  //
  // This can mess up the normals, and thus the lighting. Should we keep track of that?
{
    // Only accept sane values.
  # if 0
    d_assert( ! util::is_non_number( x_scale));
    d_assert( ! util::is_non_number( y_scale));
    d_assert( ! util::is_non_number( z_scale));
  # endif

    // Zero values are probably legal, but probably accidental.
    // Negative values reflect around a plane, and so are wrong unless you are using them
    // for some special effect, probably involving mirrors.
    d_assert( x_scale > 0);
    d_assert( y_scale > 0);
    d_assert( z_scale > 0);

    assert_no_errors ne( *this);
    ::glScaled( x_scale, y_scale, z_scale);
}

// _______________________________________________________________________________________________

  void
  env_type::
translate_current_matrix( float_type x_offset, float_type y_offset, float_type z_offset)
{
    // Only accept sane values.
  # if 0
    d_assert( ! util::is_non_number( x_offset));
    d_assert( ! util::is_non_number( y_offset));
    d_assert( ! util::is_non_number( z_offset));
  # endif

    assert_no_errors ne( *this);
    ::glTranslatef( x_offset, y_offset, z_offset);
}

  void
  env_type::
translate_current_matrix( double_type x_offset, double_type y_offset, double_type z_offset)
{
    // Only accept sane values.
  # if 0
    d_assert( ! util::is_non_number( x_offset));
    d_assert( ! util::is_non_number( y_offset));
    d_assert( ! util::is_non_number( z_offset));
  # endif

    assert_no_errors ne( *this);
    ::glTranslated( x_offset, y_offset, z_offset);
}

// _______________________________________________________________________________________________

  void
  env_type::
setup_ortho
 (  double_type  xlo
  , double_type  xhi
  , double_type  ylo
  , double_type  yhi
  , double_type  zlo
  , double_type  zhi
 )
{
    // Only accept sane values.
  # if 0
    d_assert( ! util::is_non_number( xlo));
    d_assert( ! util::is_non_number( xhi));
    d_assert( ! util::is_non_number( ylo));
    d_assert( ! util::is_non_number( yhi));
    d_assert( ! util::is_non_number( zlo));
    d_assert( ! util::is_non_number( zhi));
  # endif

    // These assumptions are necessary to avoid divide-by-zero.
    d_assert( xlo != xhi);
    d_assert( ylo != yhi);
    d_assert( zlo != zhi);

    // Although these are not necessary assumptions, an assert failure here may mean you are doing something wrong.
    // I believe breaking one of these assumptions either reflects everything around a plane or clips everything away.
    // If it flips then this could be useful for simulating reflections.
    d_assert( xlo < xhi);
    d_assert( ylo < yhi);
    d_assert( zlo < zhi);

    // This works on the current matrix. And this operation is meant for the projection matrix.
    d_assert( as_bool( is_current_matrix_projection( )));

    assert_no_errors ne( *this);
    ::glOrtho( xlo, xhi, ylo, yhi, zlo, zhi);
}

  void
  env_type::
setup_frustum
 (  double_type  xlo
  , double_type  xhi
  , double_type  ylo
  , double_type  yhi
  , double_type  zlo  // aka z_near. zlo > 0.
  , double_type  zhi  // aka z_far. zhi > zlo.
 )
{
    // Only accept sane values.
    // util::is_non_number(..) is not a complete test and has given me problems with
    // signaling NaN floats. Probably these assert(..)s should be commented out.
    // I've never seen a non-number here anyway.
  # if 0
    d_assert( ! util::is_non_number( xlo));
    d_assert( ! util::is_non_number( xhi));
    d_assert( ! util::is_non_number( ylo));
    d_assert( ! util::is_non_number( yhi));
    d_assert( ! util::is_non_number( zlo));
    d_assert( ! util::is_non_number( zhi));
  # endif

    // These assumptions are necessary to avoid divide-by-zero.
    d_assert( xlo != xhi);
    d_assert( ylo != yhi);
    d_assert( zlo != zhi);

    // GL also requires this. This is not necessary for ortho projections however, since the eye
    // is not really in the scene.
    d_assert( zlo > 0);
    d_assert( zhi > 0);

    // Although these are not necessary assumptions, violating them may mean you are doing something wrong.
    // I believe (xlo > xhi) and (ylo > yhi) reflect the drawing around a plane, and (zlo < zhi)
    // either clips away everything or maybe looks behind you?
    d_assert( xlo < xhi);
    d_assert( ylo < yhi);
    d_assert( zlo < zhi);

    // This works on the current matrix. And this operation is meant for the projection matrix.
    d_assert( as_bool( is_current_matrix_projection( )));

    assert_no_errors ne( *this);
    ::glFrustum( xlo, xhi, ylo, yhi, zlo, zhi);
}

// _______________________________________________________________________________________________

  void
  env_type::
get_viewport_max_sizes
 (  size_type &  x_delta
  , size_type &  y_delta
 )
{
    int_type x_delta_i = 0, y_delta_i = 0;
    get_2_ints( GL_MAX_VIEWPORT_DIMS, x_delta_i, y_delta_i);
    x_delta = x_delta_i;
    y_delta = y_delta_i;

    // These can be zero, but never negative.
    d_assert( x_delta >= 0);
    d_assert( y_delta >= 0);
}

  void
  env_type::
get_viewport
 (  int_type  &  xlo
  , int_type  &  ylo
  , size_type &  x_delta
  , size_type &  y_delta
 )
{
    // The viewport dimensions are two GLint and two GLsizei. Since we have to
    // get them all together with glGetIntegerv(..) they had better be the same size.
    d_static_assert( sizeof( int_type) == sizeof( size_type));

    int_type x_delta_i = 0, y_delta_i = 0;
    get_4_ints( GL_VIEWPORT, xlo, ylo, x_delta_i, y_delta_i);
    x_delta = x_delta_i;
    y_delta = y_delta_i;

    // These are window coords. Often the origin is (0,0), but it doesn't have to be.
    // And sometimes the width is zero, although it shouldn't be negative.
    d_assert( x_delta >= 0);
    d_assert( y_delta >= 0);

 # ifndef NDEBUG
    size_type debug_x_delta, debug_y_delta;
    get_viewport_max_sizes( debug_x_delta, debug_y_delta);
    d_assert( x_delta <= debug_x_delta);
    d_assert( y_delta <= debug_y_delta);
 # endif
}

  void
  env_type::
set_viewport
 (  int_type   xlo
  , int_type   ylo
  , size_type  x_delta
  , size_type  y_delta
 )
{
    // The sizes can be zero, but not negative.
    d_assert( x_delta >= 0);
    d_assert( y_delta >= 0);

    // The sizes should not exceed the max size.
 # ifndef NDEBUG
    {   size_type debug_x_delta, debug_y_delta;
        get_viewport_max_sizes( debug_x_delta, debug_y_delta);
        d_assert( x_delta <= debug_x_delta);
        d_assert( y_delta <= debug_y_delta);
    }
 # endif

    // There are also probably limits on x_lo and y_lo.
    // Maybe check that (xlo + x_delta) does not overflow int_type?

    assert_no_errors ne( *this);
    ::glViewport( xlo, ylo, x_delta, y_delta);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Attribute stack
// _______________________________________________________________________________________________

  void
  env_type::
init_saved_server_attributes_stack_values( )
{
    d_assert( ! is_init( ));

  # ifndef NDEBUG
    // Check stack depth.
    {   int_type const  sd         = get_stack_depth_saved_server_attributes_from_gl( );
        int_type const  max_sd     = get_max_stack_depth_saved_server_attributes_from_gl( );
        int_type const  minmax_sd  = get_min_max_stack_depth_saved_server_attributes( );

        // Check that stack depth does not exceed its maximum.
        d_assert( (0 <= sd) && (sd <= max_sd));

        // Check that the actual maximum is at least as big as what it's supposed to be
        // according to the standard, as reported on opengl.org.
        // Check this even if is_checking_against_min_max_values( ) is false.
        d_assert( minmax_sd <= max_sd);
    }
  # endif

    // If this environment is starting on a pristine GL setup.
    if ( is_assuming_pristine( ) ) {
        // The attribute-save stack starts out empty.
        d_assert( get_stack_depth_saved_server_attributes_from_gl( ) == 0);
        stack_depth_saved_server_attributes_ = 0;
    } else {
        // Initialize from GL.
        stack_depth_saved_server_attributes_ = get_stack_depth_saved_server_attributes_from_gl( );
    }

    assert_saved_server_attributes_stack_deph( );
}

  void
  env_type::
uninit_saved_server_attributes_stack_values( )
{
    d_assert( ! is_init( ));

    // We cannot make this assumption because this object gets dtor'ed after GL is gone and
    // there is no active context.
  # if 0
    assert_saved_server_attributes_stack_deph( );
  # endif

    // All pushes should be popped and stack depth should be back to where it started.
    // This is an inconvenient assumption. It assumes creation/destruction is wrapper-like.
  # if 0
  # ifndef NDEBUG
    if ( is_assuming_pristine( ) ) {
        d_assert( get_stack_depth_saved_server_attributes( ) == 0);
    }
  # endif
  # endif
}

// _______________________________________________________________________________________________

  void
  env_type::
push_server_attribute_stack( bitfield_type bits)
  //
  // The attibute bits are:
  //   GL_CURRENT_BIT
  //   GL_POINT_BIT
  //   GL_LINE_BIT
  //   GL_POLYGON_BIT
  //   GL_POLYGON_STIPPLE_BIT
  //   GL_PIXEL_MODE_BIT
  //   GL_LIGHTING_BIT
  //   GL_FOG_BIT
  //   GL_DEPTH_BUFFER_BIT
  //   GL_ACCUM_BUFFER_BIT
  //   GL_STENCIL_BUFFER_BIT
  //   GL_VIEWPORT_BIT
  //   GL_TRANSFORM_BIT
  //   GL_ENABLE_BIT
  //   GL_COLOR_BUFFER_BIT
  //   GL_HINT_BIT
  //   GL_EVAL_BIT
  //   GL_LIST_BIT
  //   GL_TEXTURE_BIT
  //   GL_SCISSOR_BIT
  //   GL_MULTISAMPLE_BIT   - not in 1.1
  //
  // In addition you can save them all with
  //   GL_ALL_ATTRIB_BITS
{
    d_assert( as_bool( is_init( )));

    // Stack depth should agree with GL.
    assert_saved_server_attributes_stack_deph( );

    // Check that max-depth will not be exceeded.
    if ( is_checking_against_min_max_values( ) ) {
        d_assert( get_stack_depth_saved_server_attributes( ) < get_min_max_stack_depth_saved_server_attributes( ));
    } else {
        d_assert( get_stack_depth_saved_server_attributes( ) < get_max_stack_depth_saved_server_attributes_from_gl( ));
    }

    // Save server attributes to the stack.
    {   assert_no_errors ne( *this);
        ::glPushAttrib( bits);
    }
    // Increment our counter of the stack depth.
    stack_depth_saved_server_attributes_ += 1;

    // Stack depth should agree with GL.
    assert_saved_server_attributes_stack_deph( );
}

  void
  env_type::
pop_server_attribute_stack( )
{
    d_assert( as_bool( is_init( )));

    // Stack depth should agree with GL.
    assert_saved_server_attributes_stack_deph( );

    // Check that stack will not underflow.
    d_assert( get_stack_depth_saved_server_attributes( ) > 0);

    // Pop save stack and write the values back to the current values.
    {   assert_no_errors ne( *this);
        ::glPopAttrib( );
    }
    // Decrement our counter of the stack depth.
    stack_depth_saved_server_attributes_ -= 1;

    // Stack depth should agree with GL.
    assert_saved_server_attributes_stack_deph( );
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Vertex-entry mode
// _______________________________________________________________________________________________

  void
  env_type::
activate_vertex_entry( enum_type mode)
{
    d_assert( as_bool( is_init( )));

    // These are all the legal values for mode:
    d_assert( is_vertex_entry_mode( mode));
    // The following modes require a certain multiple of vertices:
    //   GL_LINES (2)
    //   GL_TRIANGLES (3)
    //   GL_QUADS (4)
    //   GL_QUAD_STRIP (2)

    // We cannot check ::glGetError( ) while we are twixt begin/end. So check it now while we
    // have the chance.
    assert_no_error( );

    // These are the only legal OpenGL functions that you can call between begin/end:
    //   ::glVertex*(..)        ::glVertexAttrib*(..)   ::glNormal*(..)
    //   ::glMaterial*(..)      ::glColor*(..)          ::glSecondaryColor*(..)
    //   ::glIndex*(..)
    //   ::glFogCoord*(..)
    //   ::glTexCoord*(..)      ::glMultiTexCoord*(..)
    //   ::glEvalCoord*(..)     ::glEvalPoint*(..)
    //   ::glArrayElement*(..)  ::glEdgeFlag*(..)
    //   ::glCallList*(..)      ::glCallLists*(..)
    // The last two are only valid when the display lists only include the above commands.

    // Set the state in this (environment) object.
    d_assert( ! is_vertex_entry_active( ));
    is_vertex_entry_active_ = true;
    vertex_entry_mode_      = mode;
    dropped_vertex_count_   = 0;
    d_assert( as_bool( is_vertex_entry_active( )));

    // Call the OpenGL function.
    ::glBegin( mode);
}

  void
  env_type::
deactivate_vertex_entry( )
{
    d_assert( as_bool( is_init( )));

    // Set the state in this (environment) object.
    d_assert( as_bool( is_vertex_entry_active( )));
    is_vertex_entry_active_ = false;
    vertex_entry_mode_      = static_cast< enum_type >( -1); /* not necessary */
    dropped_vertex_count_   = 0; /* not necessary */
    d_assert( ! is_vertex_entry_active( ));

    // Call the OpenGL function.
    ::glEnd( );

    // Now that we're no longer between begin/end we can call ::glGetError( ) again.
    assert_no_error( );
}

// _______________________________________________________________________________________________

  bool_type
  env_type::
is_vertex_entry_line_type( ) const
{
    if ( is_vertex_entry_active( ) ) {
        enum_type const mode = get_vertex_entry_mode( );
        if ( (mode == GL_LINES     ) ||
             (mode == GL_LINE_STRIP) ||
             (mode == GL_LINE_LOOP ) )
        {
            return true;
        }
    }
    return false;
}

  bool_type
  env_type::
is_vertex_entry_triangle_type( ) const
{
    if ( is_vertex_entry_active( ) ) {
        enum_type const mode = get_vertex_entry_mode( );
        if ( (mode == GL_TRIANGLES     ) ||
             (mode == GL_TRIANGLE_STRIP) ||
             (mode == GL_TRIANGLE_FAN  ) )
        {
            return true;
        }
    }
    return false;
}

  bool_type
  env_type::
is_vertex_entry_quad_type( ) const
{
    if ( is_vertex_entry_active( ) ) {
        enum_type const mode = get_vertex_entry_mode( );
        if ( (mode == GL_QUADS     ) ||
             (mode == GL_QUAD_STRIP) )
        {
            return true;
        }
    }
    return false;
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// OpenGL versions
// _______________________________________________________________________________________________

  bool_type /* static */ env_type::
is_extension_explicitly_supported( char_type const *  p_extension)
  //
  // Instead of relying on GLee, this explicitly checks the extension string associated with
  // the current context. You need to do this if you create an alternative gl-context, either
  // one that asks for software-only rendering, or one that renders into a bitmap (as opposed
  // to a pbuffer). (Bitmap memory lives in system ram and not on the graphics card, so hardware
  // rendering may not be supported. Particularly on Windows.)
{
    // Make sure the string starts with a non-space char.
    d_assert( p_extension && (*p_extension) && (! isspace( *p_extension)));

    // ::glGetString(..) returns a string with this type (these are all equiv):
    //   GLubyte const *
    //   ubyte_type const *
    //   unsigned char const *
    // Too bad there's not signed_cast<..>(..) and unsigned_cast<..>(..) functions.

    // You can get just the version from ::glGetString( GL_VERSION).
    //   It returns something like "2.1 junk" or "2.1.44 possible junk".

    // The function GLeeGetExtStrGL( ) returns the same thing as ::glGetString( GL_EXTENSIONS),
    // except cast to "char_type const *".

    typedef unsigned char const *  ustr_type;
    typedef          char const *  str_type ;
    ustr_type const  pu_all_extensions  = ::glGetString( GL_EXTENSIONS);
    str_type  const  p_all_extensions   = reinterpret_cast< str_type >( pu_all_extensions);
    str_type  const  p_found            = strstr( p_all_extensions, p_extension);

    return p_found != 0;

    // Safety note:
    //   This assumes these are terminated strings. But p_all_extensions comes from somewhere
    //   else (OpenGL) and might be very long so it's difficult to watch out.
    //   We could check a few things though:
    //     It must have less than 64K chars.
    //     It should be all ascii. No weird chars.
    //     It is a space-separated list of strings with always start with prefix "GL_".
    //     Strings always have the form GL_AZ09_az09_etc.
    //       The 2 prefixes are upper-char (or the digit 3).
    //       All the remaining strings are lower-case (or digits). No hyphens anywhere.
    //     The longest string (so far) is "GL_NV_framebuffer_multisample_coverage", 38 chars
    //       or 39 if you count the null terminator.
    //     There are 322 extension strings in the GLee 5.4 (which includes GL_VERSION_3_0 but not 3_1).
}

  bool_type /* static */ env_type::
is_gl_version_1_2( )    { return GLEE_VERSION_1_2; }

  bool_type /* static */ env_type::
is_gl_version_1_3( )    { return GLEE_VERSION_1_3; }

  bool_type /* static */ env_type::
is_gl_version_1_4( )    { return GLEE_VERSION_1_4; }

  bool_type /* static */ env_type::
is_gl_version_1_5( )    { return GLEE_VERSION_1_5; }

  bool_type /* static */ env_type::
is_gl_version_2_0( )    { return GLEE_VERSION_2_0; }

  bool_type /* static */ env_type::
is_gl_version_2_1( )    { return GLEE_VERSION_2_1; }

  bool_type /* static */ env_type::
is_gl_version_3_0( )    { return GLEE_VERSION_3_0; }

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Simple shader methods
//   These would be inline except we need to isolate GLee.h from Qt headers.
// _______________________________________________________________________________________________

  /* static */
  bool_type
  env_type::
are_shaders_supported( )
{
    return is_gl_version_2_0( );
}

  /* static */
  bool_type
  env_type::
are_shaders_explicitly_supported( )
{
    return is_extension_explicitly_supported( "GL_VERSION_2_0");
}

  /* static */
  bool_type
  env_type::
are_shaders_supported__ARB( )
{
    return
        GLEE_ARB_shader_objects       &&
        GLEE_ARB_vertex_shader        &&
        GLEE_ARB_fragment_shader      &&
        GLEE_ARB_shading_language_100 ;
}

  /* static */
  bool_type
  env_type::
are_shaders_explicitly_supported__ARB( )
{
    return
        is_extension_explicitly_supported( "GL_ARB_shader_objects")       &&
        is_extension_explicitly_supported( "GL_ARB_vertex_shader")        &&
        is_extension_explicitly_supported( "GL_ARB_fragment_shader")      &&
        is_extension_explicitly_supported( "GL_ARB_shading_language_100") ;
}

  uint_type
  env_type::
get_active_shader_program( )
{
    d_assert( are_shaders_supported( ));
    return get_int( GL_CURRENT_PROGRAM);
}

  void
  env_type::
use_shader_program( uint_type program_id)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUseProgram( program_id);
}

  bool_type
  env_type::
is_shader_program( uint_type program_id)
{
    d_assert( are_shaders_supported( ));
    d_assert( program_id);
    assert_no_errors ne( *this);
    return ::glIsProgram( program_id);
}

  bool_type
  env_type::
is_shader( uint_type shader_id)
{
    d_assert( are_shaders_supported( ));
    d_assert( shader_id);
    assert_no_errors ne( *this);
    return ::glIsShader( shader_id);
}

  uint_type
  env_type::
create_shader_program( )
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    return ::glCreateProgram( );
}

  void
  env_type::
delete_shader_program( uint_type program_id)
{
    // allow program_id == 0
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glDeleteProgram( program_id);
}

  uint_type
  env_type::
create_shader( enum_type shader_type)
{
    d_assert( are_shaders_supported( ));
    d_assert( (GL_VERTEX_SHADER   == shader_type) ||
              (GL_FRAGMENT_SHADER == shader_type) );
    assert_no_errors ne( *this);
    return ::glCreateShader( shader_type);
}

  void
  env_type::
delete_shader( uint_type shader_id)
{
    // allow shader_id == 0
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glDeleteShader( shader_id);
}

  void
  env_type::
attach_shader( uint_type program_id, uint_type shader_id)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glAttachShader( program_id, shader_id);
}

  void
  env_type::
detach_shader( uint_type program_id, uint_type shader_id)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glDetachShader( program_id, shader_id);
}

  void
  env_type::
link_shader_program( uint_type program_id)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glLinkProgram( program_id);
}

  void
  env_type::
validate_shader_program( uint_type program_id)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glValidateProgram( program_id);
}

  void
  env_type::
set_shader_src( uint_type shader_id, char_type const * p_src_string)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glShaderSource( shader_id, 1, & p_src_string, 0);
}

  void
  env_type::
compile_shader( uint_type shader_id)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glCompileShader( shader_id);
}

  int_type
  env_type::
get_uniform_location( uint_type program_id, char_type const * p_uniform_name)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    return ::glGetUniformLocation( program_id, p_uniform_name);
}

  void
  env_type::
get_uniform_values( uint_type program_id, int_type uniform_id, float_type * p_values)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glGetUniformfv( program_id, uniform_id, p_values);
}

  void
  env_type::
get_uniform_values( uint_type program_id, int_type uniform_id, int_type * p_values)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glGetUniformiv( program_id, uniform_id, p_values);
}

  void
  env_type::
set_uniform( int_type uniform_id, float_type v0)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform1f( uniform_id, v0);
}

  void
  env_type::
set_uniform( int_type uniform_id, float_type v0, float_type v1)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform2f( uniform_id, v0, v1);
}

  void
  env_type::
set_uniform( int_type uniform_id, float_type v0, float_type v1, float_type v2)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform3f( uniform_id, v0, v1, v2);
}

  void
  env_type::
set_uniform( int_type uniform_id, float_type v0, float_type v1, float_type v2, float_type v3)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform4f( uniform_id, v0, v1, v2, v3);
}

  void
  env_type::
set_uniform( int_type uniform_id, int_type v0)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform1i( uniform_id, v0);
}

  void
  env_type::
set_uniform( int_type uniform_id, int_type v0, int_type v1)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform2i( uniform_id, v0, v1);
}

  void
  env_type::
set_uniform( int_type uniform_id, int_type v0, int_type v1, int_type v2)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform3i( uniform_id, v0, v1, v2);
}

  void
  env_type::
set_uniform( int_type uniform_id, int_type v0, int_type v1, int_type v2, int_type v3)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform4i( uniform_id, v0, v1, v2, v3);
}

// _______________________________________________________________________________________________

  GLhandleARB
  env_type::
get_active_shader_program__ARB( )
{
    d_assert( are_shaders_supported__ARB( ));
    assert_no_errors ne( *this);
    return ::glGetHandleARB( GL_PROGRAM_OBJECT_ARB);
}

  void
  env_type::
use_shader_program__ARB( GLhandleARB program_id)
{
    d_assert( are_shaders_supported__ARB( ));
    assert_no_errors ne( *this);
    ::glUseProgramObjectARB( program_id);
}

  bool_type
  env_type::
is_shader_program__ARB( GLhandleARB program_id)
{
    d_assert( are_shaders_supported__ARB( ));
    d_assert( program_id);
    assert_no_errors ne( *this);
    return ::glIsProgramARB( program_id);
    // There is no support for an is_shader__ARB(..) method.
}

  GLhandleARB
  env_type::
create_shader_program__ARB( )
{
    d_assert( are_shaders_supported__ARB( ));
    assert_no_errors ne( *this);
    return ::glCreateProgramObjectARB( );
}

  GLhandleARB
  env_type::
create_shader__ARB( enum_type shader_type)
{
    d_assert( are_shaders_supported__ARB( ));
    d_assert( (GL_VERTEX_SHADER_ARB   == shader_type) ||
              (GL_FRAGMENT_SHADER_ARB == shader_type) );
    assert_no_errors ne( *this);
    return ::glCreateShaderObjectARB( shader_type);
}

  void
  env_type::
delete_shader_object__ARB( GLhandleARB object_id)
{
    d_assert( are_shaders_supported__ARB( ));
    d_assert( object_id); /* we could allow zero */
    assert_no_errors ne( *this);
    ::glDeleteObjectARB( object_id);
}

  void
  env_type::
attach_shader__ARB( GLhandleARB program_id, GLhandleARB shader_id)
{
    d_assert( are_shaders_supported__ARB( ));
    assert_no_errors ne( *this);
    ::glAttachObjectARB( program_id, shader_id);
}

  void
  env_type::
detach_shader__ARB( GLhandleARB parent_id, GLhandleARB shader_id)
{
    d_assert( are_shaders_supported__ARB( ));
    d_assert( parent_id && shader_id && (parent_id != shader_id));
    ::glDetachObjectARB( parent_id, shader_id);
}

  void
  env_type::
link_shader_program__ARB( GLhandleARB program_id)
{
    d_assert( are_shaders_supported__ARB( ));
    assert_no_errors ne( *this);
    ::glLinkProgramARB( program_id);
}

  void
  env_type::
validate_shader_program__ARB( GLhandleARB program_id)
{
    d_assert( are_shaders_supported__ARB( ));
    assert_no_errors ne( *this);
    ::glValidateProgramARB( program_id);
}

  void
  env_type::
set_shader_src__ARB( GLhandleARB shader_id, GLcharARB const * p_src_string)
{
    d_assert( are_shaders_supported__ARB( ));
    assert_no_errors ne( *this);
    ::glShaderSourceARB( shader_id, 1, & p_src_string, 0);
}

  void
  env_type::
compile_shader__ARB( GLhandleARB shader_id)
{
    d_assert( are_shaders_supported__ARB( ));
    assert_no_errors ne( *this);
    ::glCompileShaderARB( shader_id);
}

  int_type
  env_type::
get_uniform_location__ARB( uint_type program_id, char_type const * p_uniform_name)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    return ::glGetUniformLocationARB( program_id, p_uniform_name);
}

  void
  env_type::
get_uniform_values__ARB( uint_type program_id, int_type uniform_id, float_type * p_values)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glGetUniformfvARB( program_id, uniform_id, p_values);
}

  void
  env_type::
get_uniform_values__ARB( uint_type program_id, int_type uniform_id, int_type * p_values)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glGetUniformivARB( program_id, uniform_id, p_values);
}

  void
  env_type::
set_uniform__ARB( int_type uniform_id, float_type v0)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform1fARB( uniform_id, v0);
}

  void
  env_type::
set_uniform__ARB( int_type uniform_id, float_type v0, float_type v1)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform2fARB( uniform_id, v0, v1);
}

  void
  env_type::
set_uniform__ARB( int_type uniform_id, float_type v0, float_type v1, float_type v2)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform3fARB( uniform_id, v0, v1, v2);
}

  void
  env_type::
set_uniform__ARB( int_type uniform_id, float_type v0, float_type v1, float_type v2, float_type v3)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform4fARB( uniform_id, v0, v1, v2, v3);
}

  void
  env_type::
set_uniform__ARB( int_type uniform_id, int_type v0)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform1iARB( uniform_id, v0);
}

  void
  env_type::
set_uniform__ARB( int_type uniform_id, int_type v0, int_type v1)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform2iARB( uniform_id, v0, v1);
}

  void
  env_type::
set_uniform__ARB( int_type uniform_id, int_type v0, int_type v1, int_type v2)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform3iARB( uniform_id, v0, v1, v2);
}

  void
  env_type::
set_uniform__ARB( int_type uniform_id, int_type v0, int_type v1, int_type v2, int_type v3)
{
    d_assert( are_shaders_supported( ));
    assert_no_errors ne( *this);
    ::glUniform4iARB( uniform_id, v0, v1, v2, v3);
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Shaders and shader programs (GLSL) (GL version 2.0)
// _______________________________________________________________________________________________

  int_type
  env_type::
get_shader_program_status_value( uint_type program_id, enum_type name, int_type init_value)
  //
  // This is a paranoid way of calling the OpenGL function ::glGetProgramiv(..).
{
    d_assert( are_shaders_supported( ));
    d_assert( program_id);
    d_assert( name);

    // Even though we are asking for a single value, we put a lot of padding on both sides
    // of the value we want.
    int_type value_with_padding[ 6 ] = { 1234, 5678, 0, -123, -456, -789 };
    value_with_padding[ 2 ] = init_value;

    // Ask for the value. Watch for buffer overruns on both sides.
    { assert_no_errors ne( *this);
      ::glGetProgramiv( program_id, name, & value_with_padding[ 2 ]);
    }
    d_assert( 1234 == value_with_padding[ 0 ]);
    d_assert( 5678 == value_with_padding[ 1 ]);
    d_assert( -123 == value_with_padding[ 3 ]);
    d_assert( -456 == value_with_padding[ 4 ]);
    d_assert( -789 == value_with_padding[ 5 ]);

    return value_with_padding[ 2 ];
}

  int_type
  env_type::
get_shader_status_value( uint_type shader_id, enum_type name, int_type init_value)
  //
  // This is a paranoid way of calling the OpenGL function ::glGetShaderiv(..).
{
    d_assert( are_shaders_supported( ));
    d_assert( shader_id);
    d_assert( name);

    // Even though we are asking for a single value, we put a lot of padding on both sides
    // of the value we want.
    int_type value_with_padding[ 6 ] = { 1234, 5678, 0, -123, -456, -789 };
    value_with_padding[ 2 ] = init_value;

    // Ask for the value. Watch for buffer overruns on both sides.
    { assert_no_errors ne( *this);
      ::glGetShaderiv( shader_id, name, & value_with_padding[ 2 ]);
    }
    d_assert( 1234 == value_with_padding[ 0 ]);
    d_assert( 5678 == value_with_padding[ 1 ]);
    d_assert( -123 == value_with_padding[ 3 ]);
    d_assert( -456 == value_with_padding[ 4 ]);
    d_assert( -789 == value_with_padding[ 5 ]);

    return value_with_padding[ 2 ];
}

  bool_type
  env_type::
did_shader_program_op_fail_with_errors( enum_type op_name, uint_type program_id)
{
    d_assert( are_shaders_supported( ));
    d_assert( program_id);
    d_assert( (GL_LINK_STATUS == op_name) || (GL_VALIDATE_STATUS == op_name));

    int_type const op_status = get_shader_program_status_value( program_id, op_name, GL_FALSE);
    d_assert( (GL_FALSE == op_status) || (GL_TRUE == op_status));

  # ifndef NDEBUG
    {   // Find out how long the log is supposed to be.
        // Does this include space for the null term? If so, does it ever return zero?
        int_type const info_log_length = get_shader_program_status_value( program_id, GL_INFO_LOG_LENGTH, -1);
        d_assert( info_log_length >= 0);

        // Set up a buffer. Make sure it has an extra char and is null terminated.
        size_type const info_log_buf_length = 255;
        char_type info_log_buf[ info_log_buf_length + 1 ]; /* make a buffer with an extra char at the end */
        info_log_buf[ info_log_buf_length ] = 0; /* set the null term */

        // Get the error messages about the link/validate.
        // This is a good place for a break-point when you're trying out a new set of shaders.
        size_type info_log_count_chars_written = 0;
        { assert_no_errors ne( *this);
          ::glGetProgramInfoLog( program_id, info_log_buf_length, & info_log_count_chars_written, info_log_buf);
        }
        d_assert( 0 == info_log_buf[ info_log_buf_length ]); /* the null term should not be disturbed */
        d_assert( info_log_count_chars_written >= 0);
        d_assert( info_log_count_chars_written <= info_log_buf_length);
        d_assert( info_log_count_chars_written <= info_log_length);
    }
  # endif

    // Return true if last operation (compile, link, validate) failed.
    return op_status != GL_TRUE;
}

  bool_type
  env_type::
did_shader_op_fail_with_errors( enum_type op_name, uint_type shader_id)
{
    d_assert( are_shaders_supported( ));
    d_assert( shader_id);
    d_assert( GL_COMPILE_STATUS == op_name);

    int_type const op_status = get_shader_status_value( shader_id, op_name, GL_FALSE);
    d_assert( (GL_FALSE == op_status) || (GL_TRUE == op_status));

  # ifndef NDEBUG
    {   // Find out how long the log is supposed to be.
        // Does this include space for the null term? If so, does it ever return zero?
        int_type const info_log_length = get_shader_status_value( shader_id, GL_INFO_LOG_LENGTH, -1);
        d_assert( info_log_length >= 0);

        // Set up a buffer. Make sure it has an extra char and is null terminated.
        size_type const info_log_buf_length = 255;
        char_type info_log_buf[ info_log_buf_length + 1 ]; /* make a buffer with an extra char at the end */
        info_log_buf[ info_log_buf_length ] = 0; /* set the null term */

        // Get the error messages about the compile.
        // This is a good place for a break-point when you're trying out a new shader.
        size_type info_log_count_chars_written = 0;
        { assert_no_errors ne( *this);
          ::glGetShaderInfoLog( shader_id, info_log_buf_length, & info_log_count_chars_written, info_log_buf);
        }
        d_assert( 0 == info_log_buf[ info_log_buf_length ]); /* the null term should not be disturbed */
        d_assert( info_log_count_chars_written >= 0);
        d_assert( info_log_count_chars_written <= info_log_buf_length);
        d_assert( info_log_count_chars_written <= info_log_length);
    }
  # endif

    // Return true if last operation (compile, link, validate) failed.
    return op_status != GL_TRUE;
}

  uint_type
  env_type::
create_shader( enum_type shader_type, char_type const * p_src_string)
{
    d_assert( are_shaders_supported( ));
    d_assert( (GL_VERTEX_SHADER == shader_type) || (GL_FRAGMENT_SHADER == shader_type));

    uint_type const shader_id = create_shader( shader_type);
    if ( shader_id == 0 ) {
        d_assert( false);
        return 0;
    }
    d_assert( is_shader( shader_id));

    set_shader_src( shader_id, p_src_string);
    compile_shader( shader_id);
    if ( did_shader_op_fail_with_errors( GL_COMPILE_STATUS, shader_id) ) {
        d_assert( false);
        delete_shader( shader_id);
        return 0;
    }
    return shader_id;
}

  uint_type
  env_type::
create_shader_program( uint_type vertex_shader_id, uint_type fragment_shader_id)
{
    d_assert( are_shaders_supported( ));
    d_assert( vertex_shader_id);
    d_assert( fragment_shader_id);

    uint_type const program_id = create_shader_program( );
    if ( program_id == 0 ) {
        d_assert( false);
        return 0;
    }
    d_assert( is_shader_program( program_id));

    attach_shader( program_id, vertex_shader_id);
    attach_shader( program_id, fragment_shader_id);

    // We could delete the shaders after attaching them to the program.
    // They would just be marked for delete and would not actually be deleted until
    // they were detached, which probably wouldn't happen until the program itself
    // was deleted.

    link_shader_program( program_id);
    if ( did_shader_program_op_fail_with_errors( GL_LINK_STATUS, program_id) ) {
        d_assert( false);
        //detach_all_shaders( program_id);
        delete_shader_program( program_id);
        return 0;
    }

    validate_shader_program( program_id);
    if ( did_shader_program_op_fail_with_errors( GL_VALIDATE_STATUS, program_id) ) {
        d_assert( false);
        //detach_all_shaders( program_id);
        delete_shader_program( program_id);
        return 0;
    }

    return program_id;
}

  void
  env_type::
detach_all_shaders( uint_type program_id)
  //
  // Detach all the shaders attached to a program.
{
    d_assert( are_shaders_supported( ));
    d_assert( program_id && is_shader_program( program_id));

  # ifndef NDEBUG
    uint_type debug_last_shader = 0;
  # endif

    // Make sure we don't get caught in an infinite loop.
    int countdown = 10;
    while ( countdown > 0 ) {
        -- countdown;

        // Get one attached shader at a time.
        // But build a fence of extra values around the one shader we're getting.
        uint_type shaders[ 4 ] = { 0, 0, 0, 0 };
        size_type returned_count = 0;
        { assert_no_errors ne( *this);
          ::glGetAttachedShaders( program_id, 1, & returned_count, & (shaders[ 1 ]));
        }
        d_assert( 0 == shaders[ 0 ]);
        d_assert( 0 == shaders[ 2 ]);
        d_assert( 0 == shaders[ 3 ]);
        if ( 1 != returned_count ) {
            d_assert( 0 == returned_count);
            d_assert( 0 == shaders[ 1 ]);
            break;
        }
        d_assert( 0 != shaders[ 1 ]);

        // Make sure we're not seeing the same shader over and over.
      # ifndef NDEBUG
        d_assert( debug_last_shader != shaders[ 1 ]);
        debug_last_shader = shaders[ 1 ];
      # endif

        // Detach the one shader we just found and look for another.
        detach_shader( program_id, shaders[ 1 ]);
    }
    // Assume we've detached 0, 1, 2 or 3 shaders.
    d_assert( (7 <= countdown) && (countdown <= 10));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________
//
// Shaders and shader programs (GLSL) (ARB extension)
// _______________________________________________________________________________________________

  int_type
  env_type::
get_object_status_value__ARB( GLhandleARB object_id, enum_type name, int_type init_value)
  //
  // This is a paranoid way of calling the OpenGL function ::glGetObjectParameterivARB(..).
  //
  // An "object" is either a shader or a program.
{
    d_assert( are_shaders_supported__ARB( ));
    d_assert( object_id);
    d_assert( name);

    // Even though we are asking for a single value, we put a lot of padding on both sides
    // of the value we want.
    int_type value_with_padding[ 6 ] = { 1234, 5678, 0, -123, -456, -789 };
    value_with_padding[ 2 ] = init_value;

    // Ask for the value. Watch for buffer overruns on both sides.
    { assert_no_errors ne( *this);
      ::glGetObjectParameterivARB( object_id, name, & value_with_padding[ 2 ]);
    }
    d_assert( 1234 == value_with_padding[ 0 ]);
    d_assert( 5678 == value_with_padding[ 1 ]);
    d_assert( -123 == value_with_padding[ 3 ]);
    d_assert( -456 == value_with_padding[ 4 ]);
    d_assert( -789 == value_with_padding[ 5 ]);

    return value_with_padding[ 2 ];
}

  bool_type
  env_type::
did_object_op_fail_with_errors__ARB( enum_type op_name, GLhandleARB object_id)
{
    d_assert( are_shaders_supported__ARB( ));
    d_assert( object_id);
    d_assert(
        (GL_OBJECT_COMPILE_STATUS_ARB  == op_name) ||
        (GL_OBJECT_LINK_STATUS_ARB     == op_name) ||
        (GL_OBJECT_VALIDATE_STATUS_ARB == op_name) );

    int_type const op_status = get_object_status_value__ARB( object_id, op_name, GL_FALSE);
    d_assert( (GL_FALSE == op_status) || (GL_TRUE == op_status));

    // Get the status description.
    // This is a good place to put a breakpoint when you're testing a new shader.
  # ifndef NDEBUG
    {   // Find out how long the log is supposed to be.
        // Does this include space for the null term? If so, does it ever return zero?
        int_type const info_log_length = get_object_status_value__ARB( object_id, GL_OBJECT_INFO_LOG_LENGTH_ARB, 0);
        d_assert( info_log_length >= 0);

        // Set up a buffer. Make sure it has an extra char and is null terminated.
        size_type const info_log_buf_length = 255;
        GLcharARB info_log_buf[ info_log_buf_length + 1 ]; /* make a buffer with an extra char at the end */
        info_log_buf[ info_log_buf_length ] = 0; /* set the null term */

        // Get the error messages about the compile or link.
        // This is a good place for a break-point when you're trying out a new shader/program or new hardware.
        size_type info_log_count_chars_written = 0;
        { assert_no_errors ne( *this);
          ::glGetInfoLogARB( object_id, info_log_buf_length, & info_log_count_chars_written, info_log_buf);
        }
        d_assert( 0 == info_log_buf[ info_log_buf_length ]); /* the null term should not be disturbed */
        d_assert( info_log_count_chars_written >= 0);
        d_assert( info_log_count_chars_written <= info_log_buf_length);
        d_assert( info_log_count_chars_written <= info_log_length);
    }
  # endif

    // Return true if last operation (compile, link, validate) failed.
    return op_status != GL_TRUE;
}

  GLhandleARB
  env_type::
create_shader__ARB( enum_type shader_type, GLcharARB const * p_src_string)
{
    d_assert( are_shaders_supported__ARB( ));
    d_assert( (GL_VERTEX_SHADER_ARB == shader_type) || (GL_FRAGMENT_SHADER_ARB == shader_type));

    GLhandleARB const shader_id = create_shader__ARB( shader_type);
    if ( shader_id == 0 ) {
        d_assert( false);
        return 0;
    }
    //d_assert( ::glIsShaderARB( shader_id));

    set_shader_src__ARB( shader_id, p_src_string);
    compile_shader__ARB( shader_id);
    if ( did_object_op_fail_with_errors__ARB( GL_OBJECT_COMPILE_STATUS_ARB, shader_id) ) {
        d_assert( false);
        delete_shader_object__ARB( shader_id);
        return 0;
    }
    return shader_id;
}

  GLhandleARB
  env_type::
create_shader_program__ARB( GLhandleARB vertex_shader_id, GLhandleARB fragment_shader_id)
{
    d_assert( are_shaders_supported__ARB( ));
    d_assert( vertex_shader_id);
    d_assert( fragment_shader_id);

    GLhandleARB const program_id = create_shader_program__ARB( );
    if ( program_id == 0 ) {
        d_assert( false);
        return 0;
    }
    d_assert( is_shader_program__ARB( program_id));

    attach_shader__ARB( program_id, vertex_shader_id);
    attach_shader__ARB( program_id, fragment_shader_id);

    link_shader_program__ARB( program_id);
    if ( did_object_op_fail_with_errors__ARB( GL_OBJECT_LINK_STATUS_ARB, program_id) ) {
        d_assert( false);
        delete_shader_object__ARB( program_id);
        return 0;
    }

    validate_shader_program__ARB( program_id);
    if ( did_object_op_fail_with_errors__ARB( GL_OBJECT_VALIDATE_STATUS_ARB, program_id) ) {
        d_assert( false);
        delete_shader_object__ARB( program_id);
        return 0;
    }

    return program_id;
}

  void
  env_type::
detach_all_shaders__ARB( GLhandleARB program_id)
  //
  // Detach all the shaders attached to a program.
{
    d_assert( program_id && is_shader_program__ARB( program_id));

  # ifndef NDEBUG
    GLhandleARB debug_last_shader = 0;
  # endif

    // Make sure we don't get caught in an infinite loop.
    int countdown = 10;
    while ( countdown > 0 ) {
        -- countdown;

        // Get one attached shader at a time.
        // But build a fence of extra values around the one shader we're getting.
        GLhandleARB shaders[ 4 ] = { 0, 0, 0, 0 };
        size_type returned_count = 0;
        { assert_no_errors ne( *this);
          ::glGetAttachedObjectsARB( program_id, 1, & returned_count, & (shaders[ 1 ]));
        }
        d_assert( 0 == shaders[ 0 ]);
        d_assert( 0 == shaders[ 2 ]);
        d_assert( 0 == shaders[ 3 ]);
        if ( 1 != returned_count ) {
            d_assert( 0 == returned_count);
            d_assert( 0 == shaders[ 1 ]);
            break;
        }
        d_assert( 0 != shaders[ 1 ]);

        // Make sure we're not seeing the same shader over and over.
      # ifndef NDEBUG
        d_assert( debug_last_shader != shaders[ 1 ]);
        debug_last_shader = shaders[ 1 ];
      # endif

        // Detach the one shader we just found and look for another.
        detach_shader__ARB( program_id, shaders[ 1 ]);
    }
    // Assume we've detached 0, 1 or 2 shaders.
    // ARB doesn't support geometry shaders so we'll never have 3.
    d_assert( (7 <= countdown) && (countdown <= 10));
}

// _______________________________________________________________________________________________
// _______________________________________________________________________________________________

} /* end namespace gl_env */

// _______________________________________________________________________________________________
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// gl_env.cpp - End of File
// _______________________________________________________________________________________________
// |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
