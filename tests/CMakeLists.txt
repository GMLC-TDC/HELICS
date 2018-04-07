if (BOOST_VERSION_LEVEL GREATER 0)
	add_subdirectory (helics)
else()
  message(WARNING "Boost version >=1.61 required for building HELICS tests (Found Boost version ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION})")
endif()

if (BUILD_JAVA_INTERFACE)
	add_subdirectory(java_helics)
endif()


