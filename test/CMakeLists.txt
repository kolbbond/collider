# test cmake lists

message(STATUS "testing")

# add the test files
set(test_list 
    test_template.cpp
)


# walk over source files
foreach(srcfile ${test_list})
  string(REPLACE ".cpp" "" name ${srcfile})
  add_executable(${name} ${srcfile})
  target_link_libraries(${name} 
      collider
      )

  add_test(${name} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${name})
endforeach()

message(STATUS "end of test creation")
