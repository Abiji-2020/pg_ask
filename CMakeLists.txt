cmake_minimum_required(VERSION 3.15)
project(pg_ask LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)   # Required for PostgreSQL .so modules

# ----------------------------------------------------------------------
# Locate pg_config
# ----------------------------------------------------------------------

find_program(PG_CONFIG pg_config)
if(NOT PG_CONFIG)
    message(FATAL_ERROR "pg_config not found — install PostgreSQL server dev headers.")
endif()

# Retrieve PostgreSQL compile paths
execute_process(
    COMMAND ${PG_CONFIG} --includedir
    OUTPUT_VARIABLE PG_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${PG_CONFIG} --includedir-server
    OUTPUT_VARIABLE PG_INCLUDE_DIR_SERVER
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${PG_CONFIG} --libdir
    OUTPUT_VARIABLE PG_LIB_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
    COMMAND ${PG_CONFIG} --pkglibdir
    OUTPUT_VARIABLE PG_PKGLIBDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# ----------------------------------------------------------------------
# Add your AI SDK
# ----------------------------------------------------------------------

add_subdirectory(${CMAKE_SOURCE_DIR}/third_party/ai-sdk-cpp ai-sdk-build)

# ----------------------------------------------------------------------
# Gather source files
# ----------------------------------------------------------------------

file(GLOB CORE_SOURCES src/*.cpp)
file(GLOB CORE_HEADERS include/*.hpp)

# ----------------------------------------------------------------------
# Build PostgreSQL extension (.so module)
# PostgreSQL extensions must be SHARED MODULE libraries, never static.
# ----------------------------------------------------------------------

add_library(pg_ask MODULE ${CORE_SOURCES} ${CORE_HEADERS})

# PostgreSQL header paths
target_include_directories(pg_ask PRIVATE
    ${PG_INCLUDE_DIR}
    ${PG_INCLUDE_DIR_SERVER}
    ${CMAKE_SOURCE_DIR}/include
)

# Link against PostgreSQL internals + your AI SDK
target_link_directories(pg_ask PRIVATE ${PG_LIB_DIR})
target_link_libraries(pg_ask PRIVATE
    ai-sdk-cpp
)

# The Postgres extension .so must be output into the PostgreSQL pkglibdir
set_target_properties(pg_ask PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Remove "lib" prefix → PostgreSQL requires `pg_ask.so`, not `libpg_ask.so`
set_target_properties(pg_ask PROPERTIES PREFIX "")


# The Postgres extension .so must be output locally first
set_target_properties(pg_ask PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    PREFIX ""
)

# ------------------------------------------------------------
# Install the shared library (.so) into PostgreSQL's pkglibdir
# ------------------------------------------------------------
install(
    TARGETS pg_ask
    LIBRARY DESTINATION ${PG_PKGLIBDIR}
)

# ------------------------------------------------------------
# Install .control and .sql files into PostgreSQL extension dir
# ------------------------------------------------------------
execute_process(
    COMMAND ${PG_CONFIG} --sharedir
    OUTPUT_VARIABLE PG_SHARED_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
set(PG_EXTENSION_DIR "${PG_SHARED_DIR}/extension")

install(
    FILES
        ${CMAKE_SOURCE_DIR}/pg/pg_ask.control
        ${CMAKE_SOURCE_DIR}/pg/pg_ask--1.0.sql
    DESTINATION ${PG_EXTENSION_DIR}
)
