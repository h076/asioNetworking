add_library(networking INTERFACE)

# Where to find headers
target_include_directories(networking
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
)
