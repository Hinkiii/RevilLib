file(GLOB_RECURSE CORE_SOURCE_FILES "*.cpp")

add_library(revil-interface INTERFACE)
target_include_directories(revil-interface INTERFACE ../include)
target_link_libraries(revil-interface INTERFACE spike-interface)

file(GLOB ZLIB_SOURCES "${TPD_PATH}/zlib/*.c")

if(NOT NO_OBJECTS)
  build_target(
    NAME
    revil-objects
    TYPE
    OBJECT
    SOURCES
    ${CORE_SOURCE_FILES}
    ${TPD_PATH}/mspack/lzxd.c
    ${ZLIB_SOURCES}
    PROPERTIES
    POSITION_INDEPENDENT_CODE ${OBJECTS_PID}
    INCLUDES
    ${TPD_PATH}/mspack
    ${TPD_PATH}/zlib
    LINKS
    pugixml-interface
    spike-interface
    revil-interface
    generated_redb
    NO_VERINFO
    NO_PROJECT_H)

    target_compile_options(revil-objects PRIVATE -fvisibility=hidden)
    target_expose_defs(revil RE_EXPORT RE_IMPORT)
endif()

if(BUILD_SHARED_LIBS)
  build_target(
    NAME
    revil
    TYPE
    SHARED
    SOURCES
    ${CORE_SOURCE_FILES}
    ${TPD_PATH}/mspack/lzxd.c
    ${ZLIB_SOURCES}
    INCLUDES
    ${TPD_PATH}/mspack
    ${TPD_PATH}/zlib
    LINKS
    spike
    pugixml
    revil-interface
    START_YEAR
    2017
    AUTHOR
    "Lukas Cone"
    NO_PROJECT_H)

  target_compile_definitions(revil PRIVATE RE_EXPORT)

  if(WIN32 OR MINGW)
    set_target_properties(revil PROPERTIES SUFFIX ${PROJECT_VERSION_MAJOR}.dll
                                           PREFIX "")
  else()
    set_target_properties(revil PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})
  endif()

  target_compile_definitions(revil INTERFACE RE_IMPORT)

  install(
    TARGETS revil
    LIBRARY NAMELINK_SKIP DESTINATION $<IF:$<BOOL:${MINGW}>,bin,lib>
    RUNTIME DESTINATION bin)
endif()
