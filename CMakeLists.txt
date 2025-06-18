zephyr_include_directories(include)

zephyr_library()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-threadsafe-statics")
zephyr_library_sources_ifdef(CONFIG_LOGIC_ANALYZER src/LogicAnalyzer.cpp)
