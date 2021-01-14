CMAKE_MINIMUM_REQUIRED(VERSION 3.7)
PROJECT(PRT)

SET(CMAKE_CXX_STANDARD 20)

IF (EXISTS ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	INCLUDE(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
	CONAN_BASIC_SETUP()
ELSE ()
	MESSAGE(WARNING "The file conanbuildinfo.cmake doesn't exist, you have to run conan install first")
ENDIF ()

INCLUDE_DIRECTORIES(include)

IF (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	SET(OpenCLLib "-framework OpenCL")
	SET(SDLLib "-framework AudioToolbox -framework CoreAudio -framework Carbon")
	SET(SDLLib ${SDLLib} "-framework ForceFeedback -framework IOKit -framework Cocoa -framework CoreVideo")
	SET(SDLLib ${SDLLib} iconv SDL2)
ELSE ()
	FIND_PACKAGE(Threads)
	SET(ExtraLib  ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
	SET(SDLLib SDL2)
	SET(OpenCLLib OpenCL)
ENDIF ()

ADD_EXECUTABLE(PRT main.cc icosahedron.cc)
ADD_EXECUTABLE(CLTest CLTest.cc)

TARGET_LINK_LIBRARIES(PRT ${OpenCLLib} ${SDLLib} ${ExtraLib})
TARGET_LINK_LIBRARIES(CLTest ${OpenCLLib} ${SDLLib})
