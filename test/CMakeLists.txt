# test cmake lists

message(STATUS "testing")

# add the test files
set(test_list 
    test_template.cxx
    test_board.cxx
    test_perft.cxx
    test_alphabeta.cxx
    test_uci.cxx
)
if(ENABLE_GUI)
    set(test_list ${test_list} 
        test_board_gui.cxx)

endif()

# walk over source files
foreach(srcfile ${test_list})
  string(REPLACE ".cxx" "" name ${srcfile})
  add_executable(${name} ${srcfile})
  target_link_libraries(${name} 
      collider_lib
      )

  add_test(${name} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${name})
endforeach()

message(STATUS "end of test creation")
