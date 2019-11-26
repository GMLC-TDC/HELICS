cmake_minimum_required(VERSION 2.8.7)

find_package(Git)
if(NOT GIT_FOUND)
    message(FATAL_ERROR "git not found!")
endif()

include(clang-cxx-dev-tools)

function(set_git_hooks_enabled)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} config --local core.hooksPath .githooks
    )
endfunction()

function(set_git_hooks_disabled)
    execute_process(
            COMMAND ${GIT_EXECUTABLE} config --local --unset core.hooksPath
    )
endfunction()

option(ENABLE_GIT_HOOKS "Activate git hooks to run clang-format on committed files." ON)
if (ENABLE_GIT_HOOKS)
    if (CLANG_FORMAT)
        set_git_hooks_enabled()
    else()
        message(FATAL_ERROR "Clang-format is required when ENABLE_GIT_HOOKS is ON. Install clang-format or set ENABLE_GIT_HOOKS to OFF")
    endif()
else ()
    set_git_hooks_disabled()
endif ()