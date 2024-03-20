add_library(w3d_lib STATIC
  buff.cpp
  bufffile.cpp
  chunkio.cpp
  cpudetect.cpp
  crcengine.cpp
  critsection.cpp
  ffactory.cpp
  filestraw.cpp
  gcd_lcm.cpp
  iniclass.cpp
  mpu.cpp
  multilist.cpp
  mutex.cpp
  nstrdup.cpp
  random.cpp
  rawfile.cpp
  rcfile.cpp
  readline.cpp
  realcrc.cpp
  refcount.cpp
  straw.cpp
  systimer.cpp
  targa.cpp
  thread.cpp
  threadtrack.cpp
  wwfile.cpp
  wwstring.cpp
  hash.cpp
  xstraw.cpp
)

target_include_directories(w3d_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(w3d_lib PUBLIC captnlog base)
if(BUILD_DLL)
  target_compile_definitions(w3d_lib PUBLIC -D_USE_32BIT_TIME_T)
endif()