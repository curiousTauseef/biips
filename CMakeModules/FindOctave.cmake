set(MEX_EXT mex)
if (UNIX)
    find_program(OCTAVE_CONFIG octave-config) 
    if (OCTAVE_CONFIG) 
        message (STATUS "(octave-config) found : ${OCTAVE_CONFIG}")
    	set(MATLAB_COMPILE_FLAGS -DOCTAVE)
        # retrieve INCLUDE
        execute_process(COMMAND ${OCTAVE_CONFIG} -p OCTINCLUDEDIR
                OUTPUT_VARIABLE MATLAB_INCLUDE_DIR)
    	# retrieve LIBDIR
        execute_process(COMMAND ${OCTAVE_CONFIG} -p OCTLIBDIR
                OUTPUT_VARIABLE MATLAB_LIB_DIR)
        if (MATLAB_LIBRARY_DIR)
            # find MATLAB libaries
            find_library(OCTINTERP_LIBRARY octinterp  
                PATHS ${MATLAB_LIBRARY_DIR}
                NO_DEFAULT_PATH
            )
            find_library(OCTAVE_LIBRARY octave 
                PATHS ${MATLAB_LIBRARY_DIR}
                NO_DEFAULT_PATH
            )
            find_library(CRUFT_LIBRARY cruft
                PATHS ${MATLAB_LIBRARY_DIR}
                NO_DEFAULT_PATH
            )
            set(MATLAB_LIBRARIES
                "${OCTINTERP_LIBRARY}"
                "${OCTAVE_LIBRARY}"
                "${CRUFT_LIBRARY}"
            )
        endif(MATLAB_LIBRARY_DIR)
   endif(OCTAVE_CONFIG)
   find_program(OCTAVE "octave")
   if (OCTAVE)
       message(STATUS "octave found : ${OCTAVE}")
       set(MATLAB_COMMAND "${OCTAVE}")
       set(MATLAB_FLAGS --traditional)
   endif(OCTAVE)	    
else(UNIX)
    message(FATAL_ERROR "Octave is support only on Unix systems")
endif(UNIX)
mark_as_advanced(
    MATLAB_LIBRARIES
    MATLAB_COMMAND
    MATLAB_FLAGS
    MATLAB_ARCH
    MATLAB_LIBRARY_DIR
    MATLAB_INCLUDE_DIR
    MATLAB_COMPILE_FLAGS
)
