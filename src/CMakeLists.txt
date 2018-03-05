add_executable(visq.out main.cpp)
target_link_libraries(visq.out
  ${PROJECT_SOURCE_DIR}/antweakbar/lib/libAntTweakBar.so
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES}
  )