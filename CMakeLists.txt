cmake_minimum_required(VERSION 3.15)
project(TED5
	DESCRIPTION "id Software's Tile EDitor v5.0"
	HOMEPAGE_URL "https://github.com/erysdren/TED5"
	LANGUAGES C
	VERSION 5.0
)

add_executable(igrab
	${CMAKE_CURRENT_SOURCE_DIR}/source/igrab/dograb.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/igrab/finish.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/igrab/grabcga.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/igrab/grabega.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/igrab/grabvga.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/igrab/igrab.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/igrab/jhuff.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/igrab/lbmload.c
	${CMAKE_CURRENT_SOURCE_DIR}/source/igrab/xms.c
)

foreach(file nback1e.lbm nback2e.lbm nicon2e.lbm nicon2me.lbm nicon3e.lbm nicon3me.lbm nicone.lbm niconme.lbm rott.i)
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/assets/${file} ${CMAKE_CURRENT_BINARY_DIR}/${file} COPYONLY)
endforeach()
