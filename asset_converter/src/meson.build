project(
  'asset converter',
  'cpp',
default_options : ['c_std=c11', 'cpp_std=c++20']
)

incdir = include_directories('./','/usr/local/include/')

cc = meson.get_compiler('cpp')
# opengl = dependency('gl')
# glfw = cc.find_library('glfw')
# enet = cc.find_library('enet')
# threads = dependency('threads')
assimp = cc.find_library('assimp', dirs: '/usr/local/lib64/')

sources = files(
'main.cpp'
)

executable('exec',sources, include_directories : incdir, dependencies : [assimp], override_options : ['std=c++20'])
