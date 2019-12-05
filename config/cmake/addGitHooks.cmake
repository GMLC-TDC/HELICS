cmake_minimum_required(VERSION 2.8.7)

function(set_git_hooks_enabled)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} config --local core.hooksPath .githooks
    )
    message(STATUS "Git hooks enabled")
endfunction()

function(set_git_hooks_disabled)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} config --local --unset core.hooksPath
    )
    message(STATUS "Git hooks disabled")
endfunction()


if (NOT DEFINED CLANG_FORMAT)
    find_program(CLANG_FORMAT "clang-format")
endif ()

cmake_dependent_advanced_option(HELICS_ENABLE_GIT_HOOKS "Activate git hooks to run clang-format on committed files." ON "CLANG_FORMAT" OFF)

find_package(Git)

if (GIT_FOUND AND (NOT GIT_VERSION_STRING VERSION_LESS "2.9"))
    if (HELICS_ENABLE_GIT_HOOKS)
        set_git_hooks_enabled()
    else ()
        set_git_hooks_disabled()
    endif ()
elseif (HELICS_ENABLE_GIT_HOOKS AND GIT_FOUND AND (GIT_VERSION_STRING VERSION_LESS "2.9"))
    message(WARNING "Git version earlier than 2.9 found, update Git to enable git hooks.")
elseif (HELICS_ENABLE_GIT_HOOKS AND (NOT GIT_FOUND))
    message(WARNING "Git not found, git hooks will not be enabled.")
endif ()
