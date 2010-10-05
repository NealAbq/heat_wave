# Qt profile file
# Used to generate makefiles

TARGET = heat_wave_1
VERSION = 0.8.0
TEMPLATE = app

CONFIG += qt thread exceptions stl resources warn_on debug_and_release
QT += opengl

DEFINES += QT_THREAD_SUPPORT
DEFINES += QT_NO_USING_NAMESPACE
DEFINES += QT_ASCII_CAST_WARNINGS
DEFINES += QT_MOC_COMPAT

win32:DEFINES += WIN32
win32:DEFINES += _SECURE_SCL=0

# You cannot qualify at top-level with debug and release.
#   debug:DEFINES += xyzzy_debug       - does not work, always eval'd
#   release:DEFINES += xyzzy_release   - does not work, always eval'd
CONFIG(debug, debug|release) {
  # Debug only:
  # (I don't think QT_DLL is necessary).
  DEFINES += QT_DLL
} else {
  # Release only:
  # DEFINES += QT_NO_DEBUG  - redundant
  # DEFINES -= QT_SHARED  - doesn't get rid of -DQT_SHARED and probably isn't the right thing to do
  DEFINES += NDEBUG
  DEFINES += QT_NODLL

  # The default linux-gcc flags appear to be
  #   QMAKE_CXXFLAGS_DEBUG   = -c -pipe -g -Wall -W
  #   QMAKE_CXXFLAGS_RELEASE = -c -pipe -O2 -Wall
  #
  #   QMAKE_LFLAGS_DEBUG     =
  #   QMAKE_LFLAGS_RELEASE   =
  #
  # -O3
  # -ffast-math
}

# The CONFIG and QT variables above take care of these libraries for us:
# win32:LIBS += opengl32.lib
# win32:LIBS += QtOpenGL4.lib
# win32:LIBS += QtOpenGLd4.lib

win32:INCLUDEPATH = c:/boost_1_39_0

FORMS         = heat_simd.ui
RESOURCES     = heat_simd.qrc
win32:RC_FILE = heat_simd.rc

HEADERS =                          \
  all.h                            \
  bezier.h                         \
  debug.h                          \
  finite_diff.h                    \
  finite_diff_solver.h             \
  gl_env_fractional_fixed_point.h  \
  moving_sum.h                     \
  pair_iter.h                      \
  pt3.h                            \
  stride_iter.h                    \
  tri_diag.h                       \
  angle_holder.h                   \
  animate.h                        \
  animate_ui.h                     \
  antialias_style.h                \
  bool_holder.h                    \
  bristle_position.h               \
  bristle_properties_style.h       \
  bristle_style.h                  \
  color_gradient_holder.h          \
  color_holder.h                   \
  date_time.h                      \
  draw_sheet_base.h                \
  draw_sheet_bristles.h            \
  draw_sheet_surface.h             \
  face_properties_style.h          \
  face_style.h                     \
  full_screen.h                    \
  gl_draw_back_grid.h              \
  gl_draw_lights.h                 \
  gl_env.h                         \
  gl_env_global.h                  \
  gl_env_type_colors.h             \
  gl_env_type_primitives.h         \
  gl_shader.h                      \
  GLee.h                           \
  heat_simd.h                      \
  heat_solver.h                    \
  heat_widget.h                    \
  holder.h                         \
  int_holder.h                     \
  isotherm_properties_style.h      \
  lighting_rig.h                   \
  line_walker.h                    \
  out_of_date.h                    \
  out_of_date_ui.h                 \
  pack_holder.h                    \
  shader.h                         \
  shading_style.h                  \
  sheet.h                          \
  solve_control.h                  \
  util.h

SOURCES =                          \
  main.cpp                         \
  angle_holder.cpp                 \
  animate.cpp                      \
  animate_ui.cpp                   \
  antialias_style.cpp              \
  bool_holder.cpp                  \
  bristle_position.cpp             \
  bristle_properties_style.cpp     \
  bristle_style.cpp                \
  color_gradient_holder.cpp        \
  color_holder.cpp                 \
  date_time.cpp                    \
  draw_sheet_base.cpp              \
  draw_sheet_bristles.cpp          \
  draw_sheet_surface.cpp           \
  face_properties_style.cpp        \
  face_style.cpp                   \
  full_screen.cpp                  \
  gl_draw_back_grid.cpp            \
  gl_draw_lights.cpp               \
  gl_env.cpp                       \
  gl_env_global.cpp                \
  gl_env_type_colors.cpp           \
  gl_env_type_primitives.cpp       \
  gl_shader.cpp                    \
  GLee.c                           \
  heat_simd.cpp                    \
  heat_solver.cpp                  \
  heat_widget.cpp                  \
  holder.cpp                       \
  int_holder.cpp                   \
  isotherm_properties_style.cpp    \
  lighting_rig.cpp                 \
  line_walker.cpp                  \
  out_of_date.cpp                  \
  out_of_date_ui.cpp               \
  pack_holder.cpp                  \
  shader.cpp                       \
  shading_style.cpp                \
  sheet.cpp                        \
  solve_control.cpp                \
  util.cpp
