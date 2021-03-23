# Find cpack command (chocolatey has a command with the same name)
cpack_dir="$(command -v cmake)"
cpack_dir="${cpack_dir%/cmake}"
export cpack_dir
