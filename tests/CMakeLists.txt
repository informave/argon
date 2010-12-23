
FILE (GLOB ARGON_TEST_FILES_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cc )

foreach(t ${ARGON_TEST_FILES_SRC})
	string(REGEX REPLACE "\\.cc$" "" TMP_TEST_NAME ${t})
	add_executable(${TMP_TEST_NAME}_test ${t})
	target_link_libraries (${TMP_TEST_NAME}_test argon)
	add_test(${TMP_TEST_NAME} ${TMP_TEST_NAME}_test)
	message("Building test: " ${TMP_TEST_NAME})
endforeach(t)


FILE (GLOB ARGON_TEST_SCRIPTS_SRC RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.dts )

foreach(t ${ARGON_TEST_SCRIPTS_SRC})
        string(REGEX REPLACE "\\.dts$" "" TMP_TEST_NAME ${t})
        #add_executable(${TMP_TEST_NAME}_test ${t})
        #target_link_libraries (${TMP_TEST_NAME}_test argon)
	add_test(${TMP_TEST_NAME} ../argoncli
		"${CMAKE_CURRENT_SOURCE_DIR}/${t}")
        message("Building test: " ${TMP_TEST_NAME})
endforeach(t)



#add_test(test1 ../argoncli ${CMAKE_CURRENT_SOURCE_DIR}/decl-connections.dts)

