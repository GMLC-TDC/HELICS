find_package(Git QUIET)
if(GIT_FOUND AND (GIT_VERSION_STRING VERSION_GREATER "1.5.2"))
    if(EXISTS "${PROJECT_SOURCE_DIR}/.git")
        option(UPDATE_GIT_SUBMODULE "Checkout and update git submodules" ON)
        message(STATUS "Git Submodule Update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_RESULT
                        OUTPUT_VARIABLE GIT_OUTPUT)
        if(GIT_RESULT)
            message(WARNING "Automatic submodule checkout with `git submodule --init` failed with error ${GIT_RESULT} and output ${GIT_OUTPUT}. Checkout the submodules before building.")
        endif()
    else()
        message(STATUS "${PROJECT_SOURCE_DIR} is not a Git repository. Clone ${PROJECT_NAME} with Git or ensure you get copies of all the Git submodules code.")
    endif()
endif()