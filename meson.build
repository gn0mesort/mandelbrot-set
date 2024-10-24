project(
  'mandelbrot-set',
  [ 'cpp', 'c' ],
  meson_version: '>=1.1',
  version: '1.0.2',
  license: 'AGPL-3.0-or-later',
  license_files: [ 'LICENSE' ],
  default_options: [
    'cpp_std=c++20',
    'c_std=c18',
    'warning_level=3',
    'b_ndebug=if-release',
    'b_pie=true',
    'cpp_args=-fstrict-aliasing -Wstrict-aliasing=3',
    # Glad generates a whole lot of compiler warnings. They're not my warnings so ignore them.
    'c_args=-Wno-pedantic'
  ]
)
description = 'An OpenGL 4.5 Mandelbrot Set Renderer'
includes = [ include_directories('include') ]
dependencies = [ dependency('gl'), dependency('sdl2'), dependency('dl') ]
config = configuration_data()
config.set('CONFIG_WINDOW_WIDTH', get_option('window_width'))
config.set('CONFIG_WINDOW_HEIGHT', get_option('window_height'))
if get_option('colors').allowed()
  config.set('CONFIG_ENABLE_COLORS', 1)
endif
config_header = configure_file(configuration: config, input: files('generated/config.hpp.in'), output: 'config.hpp')
sources = [ config_header, files('src/main.cpp', 'src/glad/glad.c') ]
executable('@0@.@1@.@2@'.format(meson.project_name(), host_machine.cpu_family(), meson.project_version()), sources,
           include_directories: includes, dependencies: dependencies, install: true)
