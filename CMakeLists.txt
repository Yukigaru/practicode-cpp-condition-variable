cmake_minimum_required(VERSION 3.13)
project(cpprussia-workshop-condition-variable)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_TSAN "Enable thread sanitizer" OFF)

# Exercises
set(TARGETS
      task-1
      task-2
      task-3
      task-4
      task-5
      task-6
      hw_call_once
      hw_future_promise
)

foreach(TARGET ${TARGETS})
  if(${TARGET} MATCHES task)
    add_executable(${TARGET} exercises/${TARGET}/main.cpp)
  else()
    add_executable(${TARGET} homework/${TARGET}/main.cpp)
  endif()

  target_include_directories(${TARGET} PRIVATE include/common)

  if(ENABLE_TSAN)
    target_compile_options(${TARGET} PRIVATE -fsanitize=thread)
    target_link_options(${TARGET} PRIVATE -fsanitize=thread)
  endif()
endforeach()

# Dependencies setup
if(UNIX)
  foreach(TARGET ${TARGETS})
    target_link_libraries(${TARGET} PUBLIC pthread)
  endforeach()
endif()

# Testing
enable_testing()
foreach(TARGET ${TARGETS})
  add_test(NAME ${TARGET} COMMAND ${TARGET})
  set_tests_properties(${TARGET} PROPERTIES TIMEOUT 5) # 5 sec limit
endforeach()
