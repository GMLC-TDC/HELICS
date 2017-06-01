
add_subdirectory (core)

if (Boost_FOUND)
add_subdirectory(application_api)
endif(Boost_FOUND)