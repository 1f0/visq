add_executable(visq.out main.cpp)
target_link_libraries(visq.out
  ${OPENGL_LIBRARIES}
  ${GLUT_LIBRARIES}
  )