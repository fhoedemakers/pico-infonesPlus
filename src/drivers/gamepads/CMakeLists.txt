if (NOT TARGET gamepads)
  add_library(gamepads INTERFACE)

  target_sources(gamepads INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/gamepads.cpp
    ${CMAKE_CURRENT_LIST_DIR}/gamepads.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dualshock4.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dualshock5.cpp
    ${CMAKE_CURRENT_LIST_DIR}/joystick.cpp
    ${CMAKE_CURRENT_LIST_DIR}/keyboard.cpp
    ${CMAKE_CURRENT_LIST_DIR}/nintendo.cpp
    ${CMAKE_CURRENT_LIST_DIR}/psclassic.cpp
    ${CMAKE_CURRENT_LIST_DIR}/nespad.cpp
    ${CMAKE_CURRENT_LIST_DIR}/wiipad.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bluepad.cpp
  )

  target_link_libraries(gamepads INTERFACE pico_stdlib)
  target_include_directories(gamepads INTERFACE ${CMAKE_CURRENT_LIST_DIR})
endif()
