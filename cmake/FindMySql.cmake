set(MOD_NAME "MySql")

message( "finding ${MOD_NAME}!"  )

    set(MYSQL_PATH $ENV{MYSQL_PATH})
    if( MYSQL_PATH )

        message("Find MYSQL_PATH env!")
        message(${MYSQL_PATH})

        find_path( MYSQL_INCLUDE_DIR mysql.h "${MYSQL_PATH}/include" )
		find_library( MYSQL_LIBRARY1 "libmysql.lib" "${MYSQL_PATH}/lib" )

		set(MYSQL_LIBRARYS ${MYSQL_LIBRARY1})

        if( MYSQL_INCLUDE_DIR AND MYSQL_LIBRARYS)

            set( MYSQL_FOUND TRUE )

        else()

            set( MYSQL_FOUND FALSE )

        endif()

    else()

        set( MYSQL_FOUND FALSE )
        message("Not Find MINI_TEST_PATH env!")

    endif()
message("................................................................")