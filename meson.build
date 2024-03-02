project(
  'vidgrok',
  'cpp',
  version: '0.0.1',
  license : 'GPL-3.0-or-later',
  default_options: [
    'warning_level=3',
    'cpp_std=c++20',
  ],
)

source_files = [
  'src/main.cpp',
  'src/DataDispatcher.cpp',
  'src/DataVisualizer.cpp',
  'src/HardwareDataSource.cpp',
  'src/OptionProcessing.cpp',
  'src/SdlWrapper.cpp',
]

dependencies = [
  dependency('cxxopts'),
  dependency('libsigrokcxx'),
  dependency('sdl2'),
]

executable(
  'vidgrok',
  source_files,
  dependencies: dependencies,
  install: true,
)

install_man('doc/vidgrok.1')
