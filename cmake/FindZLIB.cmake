

message( "finding zlib!"  )

    set(ZLIB_PATH $ENV{ZLIB_PATH})
    if( ZLIB_PATH )

        message("Find ZLIB_PATH env!")
        message(${ZLIB_PATH})

        find_path( ZLIB_INCLUDE_DIR zlib.h "${ZLIB_PATH}" )
        find_library( ZLIB_LIBRARY zlib.lib "${ZLIB_PATH}/out/build/x64-Release" "${ZLIB_PATH}/build/x64-Release" )

        if( ZLIB_INCLUDE_DIR AND ZLIB_LIBRARY)

            set( ZLIB_FOUND TRUE )

        else()

            set( ZLIB_FOUND FALSE )

        endif()

    else()

        set( ZLIB_FOUND FALSE )
        message("Not Find ZLIB_PATH env!")

    endif()
message("................................................................")