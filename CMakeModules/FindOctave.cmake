# Variables defined:
# OCTAVE				path to OCTAVE exectuable or "OCTAVE-NOTFOUND"

# Variables defined if OCTAVE is not "OCTAVE-NOTFOUND":
# OCTAVE_CONFIG
# MATLAB_COMMAND
# MATLAB_FLAGS
# MKOCTFILE
# MEX_COMMAND
# MEX_EXT
# MEX_OBJ_EXT
# MATLAB_LIBRARY_DIR
# MATLAB_INCLUDE_DIR
# MATLAB_COMPILE_FLAGS
# MATLAB_LIBRARIES
# OCTAVE_OCTINTERPRET_LIBRARY
# OCTAVE_OCTAVE_LIBRARY
# OCTAVE_CRUFT_LIBRARY

# Variables that can be set by the user:
# OCTAVE_ROOT		path to the OCTAVE bindir. Helps finding OCTAVE.
# OCTAVE_LIBRARYDIR 	path to the OCTAVE libraries
# OCTAVE_INCLUDEDIR 	path to the OCTAVE headers



if (NOT $ENV{OCTAVE_ROOT} STREQUAL "")
    file (TO_CMAKE_PATH "$ENV{OCTAVE_ROOT}" OCTAVE_ROOT)
endif()

if (APPLE AND NOT OCTAVE_ROOT)
    file(GLOB _OCTAVE_PATHS "/usr/local/octave/*")
    list(GET _OCTAVE_PATHS 0 OCTAVE_ROOT)
endif()

if(OCTAVE_ROOT)
    file (TO_CMAKE_PATH "${OCTAVE_ROOT}/bin" OCTAVE_BINDIR)
endif()

if (OCTAVE_BINDIR)
    # try to find octave
    find_program(OCTAVE octave HINTS ${OCTAVE_BINDIR} NO_DEFAULT_PATH)
else()
    # try to find octave in default paths
    find_program(OCTAVE octave)
endif()

# Yes! found it
if (OCTAVE)
    message(STATUS "Found octave: ${OCTAVE}")
    set(MATLAB_COMMAND "${OCTAVE}")
    set(MATLAB_FLAGS --traditional --no-gui)
    # define mex file extension
    set(MEX_EXT mex)
    # define object file extension
    set(MEX_OBJ_EXT o)

    # if (NOT OCTAVE_BINDIR)
    #     # if octave is found in /usr/local/bin
    #     # mex and mexext programs will certainly not be there
    #     # hence we define MATLAB_BINDIR using the 'matlabroot' MATLAB command
    #     execute_process(COMMAND ${MATLAB_COMMAND} ${MATLAB_FLAGS}
    #         -r "disp(matlabroot);exit"
    #         OUTPUT_VARIABLE _out)
    #     string(REGEX MATCH "/.*$" MATLAB_ROOT ${_out})
    #     file (TO_CMAKE_PATH "${MATLAB_ROOT}/bin" MATLAB_BINDIR)
    # endif()


    # find mkoctfile program
    find_program(MKOCTFILE
        NAMES mkoctfile mkoctfile.exe
        PATHS ${OCTAVE_BINDIR} 
        NO_DEFAULT_PATH
    )
    set(MEX_COMMAND ${MKOCTFILE})
    set(MEX_FLAGS -DOCTAVE)
    set(MEX_OUTPUT_OPT --output)

    # find octave-config program
    find_program(OCTAVE_CONFIG octave-config ${OCTAVE_ROOT} ${OCTAVE_BINDIR})

    # We define compile flags and find OCTAVE libraries to link with
    # for manual compilations not using mkoctfile.

    # define compile flags
    set(MATLAB_COMPILE_FLAGS -DOCTAVE)

    # find OCTAVE libaries
    if (NOT $ENV{OCTAVE_LIBRARYDIR} STREQUAL "")
        file (TO_CMAKE_PATH $ENV{OCTAVE_LIBRARYDIR} OCTAVE_LIBRARYDIR)
    endif()

    if (OCTAVE_LIBRARYDIR)
        file (TO_CMAKE_PATH ${OCTAVE_LIBRARYDIR} MATLAB_LIBRARY_DIR)
    else(OCTAVE_LIBRARYDIR)
        execute_process(COMMAND ${OCTAVE_CONFIG} -p LIBDIR
                OUTPUT_VARIABLE OCTAVE_LIBDIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        execute_process(COMMAND ${OCTAVE_CONFIG} -p OCTLIBDIR
                OUTPUT_VARIABLE OCTAVE_OCTLIBDIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        find_path(MATLAB_LIBRARY_DIR liboctave.so NAMES liboctave.la
                  PATHS ${OCTAVE_LIBDIR} ${OCTAVE_OCTLIBDIR})
    endif(OCTAVE_LIBRARYDIR)

    find_library(OCTAVE_OCTINTERP_LIBRARY octinterp  
        PATHS ${MATLAB_LIBRARY_DIR}
        NO_DEFAULT_PATH
    )
    find_library(OCTAVE_OCTAVE_LIBRARY octave 
        PATHS ${MATLAB_LIBRARY_DIR}
        NO_DEFAULT_PATH
    )
    find_library(OCTAVE_CRUFT_LIBRARY cruft
        PATHS ${MATLAB_LIBRARY_DIR}
        NO_DEFAULT_PATH
    )
    set(MATLAB_LIBRARIES
        "${OCTAVE_OCTINTERP_LIBRARY}"
        "${OCTAVE_OCTAVE_LIBRARY}"
        "${OCTAVE_CRUFT_LIBRARY}"
    )

    # find OCTAVE includes
    if (NOT $ENV{OCTAVE_INCLUDEDIR} STREQUAL "")
        file (TO_CMAKE_PATH $ENV{OCTAVE_INCLUDEDIR} OCTAVE_INCLUDEDIR)
    endif()

    if (OCTAVE_INCLUDEDIR)
        find_path(MATLAB_INCLUDE_DIR mex.h
            PATHS ${OCTAVE_INCLUDEDIR}
        )
    else(OCTAVE_INCLUDEDIR)
        execute_process(COMMAND ${OCTAVE_CONFIG} -p OCTINCLUDEDIR
                OUTPUT_VARIABLE OCTAVE_INCLUDEDIR
                OUTPUT_STRIP_TRAILING_WHITESPACE)
        find_path(MATLAB_INCLUDE_DIR "mex.h"
                  PATHS ${OCTAVE_INCLUDEDIR} ${OCTAVE_INCLUDEDIR}/octave)
    endif(OCTAVE_INCLUDEDIR)


endif(OCTAVE)



mark_as_advanced(
    MATLAB_LIBRARIES
    OCTAVE_OCTINTERPRET_LIBRARY		
    OCTAVE_OCTAVE_LIBRARY	
    OCTAVE_CRUFT_LIBRARY	
    MATLAB_COMMAND
    MATLAB_FLAGS
    MATLAB_LIBRARY_DIR
    MATLAB_INCLUDE_DIR
    MATLAB_COMPILE_FLAGS
)
