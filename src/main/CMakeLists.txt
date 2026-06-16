add_executable(main
  main.cpp
)

target_link_libraries(main PUBLIC lib_core sfml-graphics tmxlite TGUI::TGUI)

add_custom_target(copy-resources ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/resources)
add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/resources
                   DEPENDS ${CMAKE_SOURCE_DIR}/resources
                   COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
                           ${CMAKE_SOURCE_DIR}/resources
                           ${CMAKE_CURRENT_BINARY_DIR}/resources)
add_dependencies(main copy-resources)