cmake_minimum_required( VERSION 2.8.12 )
project( ES3_Book )

include_directories( External/Include )
include_directories( Common/Include )

find_library( OPENGLES3_LIBRARY GLESv2 "OpenGL ES v3.0 library")
find_library( EGL_LIBRARY EGL "EGL 1.4 library" )

SUBDIRS( Common
         ch_02/HelloTriangle
         ch_06/example3
         ch_06/example6
         Chapter_6/Example_6_3
         Chapter_6/Example_6_6
         Chapter_6/MapBuffers
         Chapter_6/VertexArrayObjects
         Chapter_6/VertexBufferObjects
         Chapter_7/Instancing
         Chapter_8/Simple_VertexShader
         Chapter_9/Simple_Texture2D
         Chapter_9/Simple_TextureCubemap
         Chapter_9/MipMap2D
         Chapter_9/TextureWrap
         Chapter_10/MultiTexture
         Chapter_11/MRTs
         Chapter_14/Noise3D
         Chapter_14/ParticleSystem
         Chapter_14/ParticleSystemTransformFeedback
         Chapter_14/Shadows
         Chapter_14/TerrainRendering )
