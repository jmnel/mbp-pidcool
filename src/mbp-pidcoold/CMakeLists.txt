set( CMAKE_CXX_STANDARD 14 )
add_executable( mbp-pidcoold Main.cpp )

set( DEBUG_FLAGS "-Wall" "-Wextra" "-fno-exceptions" "-O3" )
target_compile_options( mbp-pidcoold PUBLIC ${DEBUG_FLAGS} )

#include_directories( ${X11_INCLUDE_DIR} )

#include_directories( ${CAIRO_INCLUDE_DIRS} )

#include_directories( "../arclib" )
#target_link_libraries( i3-overlay-backend ${X11_LIBRARIES} )
#target_link_libraries( i3-overlay-backend ${X11_Xfixes_LIB} )
#target_link_libraries( i3-overlay-backend ${X11_Xcomposite_LIB} )
#target_link_libraries( i3-overlay-backend ${CAIRO_LIBRARIES} )
