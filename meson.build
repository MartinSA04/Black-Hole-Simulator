project('GUI Project', 'cpp',
  version : '0.1',
  default_options : ['warning_level=2',
                     'cpp_std=c++23',
                     'c_std=c17',
                     'default_library=static'])

animationwindow_dep = dependency('animationwindow', fallback: ['animationwindow', 'animationwindow_dep'])
std_lib_facilities_dep = dependency('std_lib_facilities', fallback: ['std_lib_facilities', 'std_lib_facilities_dep'])

compiler_flags = ['-Wconversion', '-fdiagnostics-color=always', '-Werror=return-type', '-static']

inc = include_directories('src/math', 'src/simulation', 'include')

src = [
  'src/simulation/BlackHole.cpp',
  'src/simulation/Camera.cpp',
  'src/simulation/RK45.cpp',
  'src/simulation/renderer.cpp',
  'src/simulation/scene.cpp',
  'src/simulation/SimulationWindow.cpp',
]

if host_machine.system() == 'windows'
  ld_flags = ['-static', '-lstdc++exp']
else
  ld_flags = []
endif

exe = executable(
  'program',
  src,
'main.cpp',
  include_directories : inc,
  dependencies : [std_lib_facilities_dep, animationwindow_dep],
  cpp_args : compiler_flags,
  link_args : ld_flags
)
