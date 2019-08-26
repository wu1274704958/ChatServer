set(MOD_NAME "Sqlpp")

message( "finding ${MOD_NAME}!"  )

    set(SQLPP_PATH $ENV{SQLPP_PATH})
    if( SQLPP_PATH )

        message("Find SQLPP_PATH env!")
        message(${SQLPP_PATH})

        find_path( SQLPP_INCLUDE_DIR sqlpp "${SQLPP_PATH}" )
		find_library( SQLPP_LIBRARY "sqlpp.lib" "${SQLPP_PATH}/build/x64-${CMAKE_BUILD_TYPE}" )
		message("${SQLPP_PATH}/build/x64-${CMAKE_BUILD_TYPE} ${SQLPP_LIBRARY}")

        if( SQLPP_INCLUDE_DIR AND SQLPP_LIBRARY)

            set( SQLPP_FOUND TRUE )

        else()

            set( SQLPP_FOUND FALSE )

        endif()

    else()

        set( SQLPP_FOUND FALSE )
        message("Not Find SQLPP_PATH env!")

    endif()
message("................................................................")