# EFSW_FOUND               - efsw library was found
# EFSW_INCLUDE_DIR         - Path to efsw include dir
# EFSW_LIBRARY             - Path to efsw library


if (EFSW_INCLUDE_DIR AND EFSW_LIBRARY)
	# in cache already
	set (EFSW_FOUND TRUE)
else (EFSW_INCLUDE_DIR AND EFSW_LIBRARY)
	if (WIN32)
		FIND_PATH(EFSW_LIBRARY_DIR
			WIN32_DEBUG_POSTFIX d
			NAMES libefsw.dll libefsw.lib
			HINTS "C:/Program Files"
			PATH_SUFFIXES efsw/lib
		)
		FIND_LIBRARY(EFSW_LIBRARY NAMES libefsw.dll libefsw.lib HINTS ${EFSW_LIBRARY_DIR})
		FIND_PATH(EFSW_INCLUDE_DIR NAMES efsw.h efsw.hpp HINTS ${EFSW_LIBRARY_DIR}/../ PATH_SUFFIXES include/efsw)
	else (WIN32)
		FIND_LIBRARY(EFSW_LIBRARY
			WIN32_DEBUG_POSTFIX d
			NAMES efsw
			HINTS /usr/lib /usr/lib64 /usr/local/lib
		)
		FIND_PATH(EFSW_INCLUDE_DIR efsw.hpp efsw.h
			HINTS /usr/include /usr/local/include
			PATH_SUFFIXES efsw
		)
	endif (WIN32)
	include (FindPackageHandleStandardArgs)
	find_package_handle_standard_args(EFSW DEFAULT_MSG EFSW_LIBRARY EFSW_INCLUDE_DIR)
endif (EFSW_INCLUDE_DIR AND EFSW_LIBRARY)
