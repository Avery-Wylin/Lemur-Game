project(
  'meson test',
  'cpp',
default_options : ['c_std=c11', 'cpp_std=c++20']
)

currentdir = meson.current_source_dir()
incdir = include_directories('./graphics','./library','./entity','./physics','./plants','./gui', './scene', './client', './server', './audio')


cc = meson.get_compiler('cpp')
opengl = dependency('gl')
threads = dependency('threads')
if(host_machine.system() == 'windows')
  glfw = cc.find_library('glfw3', static : true)
  enet = cc.find_library('enet64', static : true)
  openal = cc.find_library('OpenAL32')
  wsock32 = cc.find_library('ws2_32')
  winmm = cc.find_library('winmm')
else
  glfw = cc.find_library('glfw')
  enet = cc.find_library('enet')
  openal = cc.find_library('openal')
endif

sources = files(
'main.cpp',

'client/Client.cpp',
'client/ClientConnection.cpp',
'client/ClientMenus.cpp',
'client/ClientConfig.cpp',

'server/Server.cpp',
'server/ServerConnection.cpp',
'server/Packet.cpp',
'server/ServerConfig.cpp',

'graphics/Shader.cpp',
'graphics/VAO.cpp',
'graphics/View.cpp',
'graphics/Texture.cpp',
'graphics/FBO.cpp',
'graphics/Armature.cpp',
'graphics/Mesh.cpp',
'graphics/DebugDraw.cpp',

'audio/Audio.cpp',

'library/glad.cpp',
'library/stb_image.cpp',
'library/stb_vorbis.cpp',

'entity/EntityType.cpp',
'entity/EntitySystem.cpp',
'entity/BeachBall.cpp',

'plants/Plant.cpp',
'plants/PlantSpecies.cpp',
'plants/PlantMeshGen.cpp',

'scene/Scene.cpp',
'scene/Player.cpp',
'scene/Terrain.cpp',
'scene/Sky.cpp',
'scene/WaterPlane.cpp',

'physics/DBVH.cpp',
'physics/CollisionShape.cpp',
'physics/PhysicsSystem.cpp',

'gui/FontInfo.cpp',
'gui/Text.cpp',
'gui/GUI.cpp'
)

if(host_machine.system() == 'windows')
  executable('exec',sources, include_directories : incdir, dependencies : [glfw, opengl, openal, enet, threads, wsock32, winmm], override_options : ['std=c++20'])
else
  executable('exec',sources, include_directories : incdir, dependencies : [glfw, opengl, openal, enet, threads], override_options : ['std=c++20'])

endif

