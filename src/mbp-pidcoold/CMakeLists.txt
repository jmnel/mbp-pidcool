set( CMAKE_CXX_STANDARD 14 )
add_executable( mbp-pidcoold Main.cpp )

set( DEBUG_FLAGS "-Wall" "-Wextra" "-fno-exceptions" "-O3" )
target_compile_options( mbp-pidcoold PUBLIC ${DEBUG_FLAGS} )
target_compile_options( mbp-pidcoold PUBLIC ${DEBUG_FLAGS} )
set(CMAKE_INSTALL_PREFIX "/usr/local/bin" )
install( TARGETS mbp-pidcoold DESTINATION bin)

install( FILES mbp-pidcoold.service 
    DESTINATION /etc/systemd/system COMPONENT config )


#add_custom_command( TARGET mbp-pidcoold POST_BUILD
#COMMAND ${CMAKE_COMMAND} -E copy_if_different
#${CMAKE_CURRENT_SOURCE_DIR}/mbp-pidcoold.service
#/etc/systemd/system/mpb-pidcoold.service )

#include_directories( ${X11_INCLUDE_DIR} )

#include_directories( ${CAIRO_INCLUDE_DIRS} )

#include_directories( "../arclib" )
#target_link_libraries( i3-overlay-backend ${X11_LIBRARIES} )
#target_link_libraries( i3-overlay-backend ${X11_Xfixes_LIB} )
#target_link_libraries( i3-overlay-backend ${X11_Xcomposite_LIB} )
#target_link_libraries( i3-overlay-backend ${CAIRO_LIBRARIES} )
