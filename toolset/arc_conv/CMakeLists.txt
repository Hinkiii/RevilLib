project(ARCConvert)

build_target(
  NAME
  extract_arc
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  extract_arc.cpp
  LINKS
  revil-interface
  INCLUDES
  ${CMAKE_SOURCE_DIR}/src/
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF Archive Extractor"
  START_YEAR
  2020)

build_target(
  NAME
  make_arc
  TYPE
  ESMODULE
  VERSION
  1
  SOURCES
  make_arc.cpp
  LINKS
  revil-interface
  INCLUDES
  ${CMAKE_SOURCE_DIR}/src/
  AUTHOR
  "Lukas Cone"
  DESCR
  "MTF Archive Maker"
  START_YEAR
  2020)
