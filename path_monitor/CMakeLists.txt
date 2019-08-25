# Include directories.
include_directories(${CMAKE_SOURCE_DIR}/lib)

# Library.
add_library(${PROJECT_NAME} INTERFACE)

target_include_directories(${PROJECT_NAME}
	INTERFACE ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})

# Install.
install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME})

install(FILES path_monitor.hpp basic_path_monitor.hpp
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/path_monitor)

install(FILES inotify/path_monitor_impl.hpp inotify/path_monitor_service.hpp
	DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/path_monitor/inotify)

install(EXPORT ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
	NAMESPACE path_monitor:: FILE ${PROJECT_NAME}-config.cmake
	EXPORT_LINK_INTERFACE_LIBRARIES
)
