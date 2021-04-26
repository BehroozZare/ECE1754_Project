cmake_minimum_required(VERSION 3.10)
project(Project)

set(CMAKE_CXX_STANDARD 11)

find_library(ROSE_LIBRARIES
        NAMES rose
        PATHS "/usr/rose/lib"
        )

find_library(PTHREAD2
        NAMES pthread
        PATHS "/usr/rose/lib"
        )

find_library(BOOST_DATE_TIME
        NAMES boost_thread
        PATHS "/usr/lib/x86_64-linux-gnu"
        )
find_library(BOOST_FILESYSTEM
        NAMES boost_filesystem
        PATHS "/usr/lib/x86_64-linux-gnu"
        )
find_library(BOOST_PROGRAM_OPTIONS
        NAMES boost_program_options
        PATHS "/usr/lib/x86_64-linux-gnu"
        )
find_library(BOOST_REGEX
        NAMES boost_regex
        PATHS "/usr/lib/x86_64-linux-gnu"
        )
find_library(BOOST_SYSTEM
        NAMES boost_system
        PATHS "/usr/lib/x86_64-linux-gnu"
        )
find_library(BOOST_SERIALIZATION
        NAMES boost_serialization
        PATHS "/usr/lib/x86_64-linux-gnu"
        )
find_library(BOOST_WAVE
        NAMES boost_wave
        PATHS "/usr/lib/x86_64-linux-gnu"
        )
find_library(BOOST_IOSTREAMS
        NAMES boost_iostreams
        PATHS "/usr/lib/x86_64-linux-gnu"
        )

find_library(BOOST_CHRONO
        NAMES boost_chrono
        PATHS "/usr/lib/x86_64-linux-gnu"
        )
find_library(DL
        NAMES dl
        PATHS "/usr/lib/x86_64-linux-gnu"
        )

find_library(Z
        NAMES z
        PATHS "/usr/lib" "/usr/lib/x86_64-linux-gnu"
        )
find_library(PTHREAD
        NAMES pthread
        PATHS "/usr/lib" "/usr/lib/x86_64-linux-gnu"
        )
find_library(DL2
        NAMES dl
        PATHS "/usr/lib"
        )
find_library(UTIL
        NAMES util
        PATHS "/usr/lib"
        )
find_library(M
        NAMES m
        PATHS "/usr/lib"
        )

find_library(QUADMATH
        NAMES quadmath
        HINTS "/usr/lib/x86_64-linux-gnu"
        )
set(ROSE_LIBS "/usr/rose/lib" "/usr/lib/x86_64-linux-gnu" "/usr/lib")



add_executable(Project PolyCheckDemo.cpp polycheck_demo_utils.cpp polycheck_demo_utils.h)
target_link_libraries(Project
        ${ROSE_LIBRARIES}
        ${BOOST_DATE_TIME}
        ${BOOST_FILESYSTEM}
        ${BOOST_PROGRAM_OPTIONS}
        ${BOOST_REGEX}
        ${BOOST_SYSTEM}
        ${BOOST_SERIALIZATION}
        ${BOOST_WAVE}
        ${BOOST_IOSTREAMS}
        ${BOOST_CHRONO}
        ${DL}
        ${Z}
        ${PTHREAD}
        ${PTHREAD2}
        ${DL2}
        ${UTIL}
        ${M}
#        ${QUADMATH}
        )
target_include_directories(Project
        PUBLIC "/usr/rose/include/rose"
        PUBLIC "/usr/lib/x86_64-linux-gnu"
        PUBLIC "/usr/lib")
add_compile_options(Project PUBLIC ${CXXFLAGS}
                            PUBLIC ${CPPFLAGS})





#add_executable(sidel input_code.cpp)
