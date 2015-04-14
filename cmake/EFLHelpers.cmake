#FIXME: Do not use CMAKE_CXX_FLAGS

include(FindPkgConfig)

SET(_efl_pkgs_list
    eina
    ecore
    evas
    ecore-evas
    eeze
    elementary
    ecore-input
    ecore-imf
    edje
    ecore-x
    )

pkg_check_modules(_efl_pkgs REQUIRED "${_efl_pkgs_list}")
include_directories(${_efl_pkgs_INCLUDE_DIRS})

FOREACH(flag ${_efl_pkgs_CFLAGS})
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
ENDFOREACH(flag)

set(EFL_LDFLAGS ${_efl_pkgs_LDFLAGS})
