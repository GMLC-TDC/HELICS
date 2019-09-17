#GADGETRON SOFTWARE LICENSE V1.0, NOVEMBER 2011

#PERMISSION IS HEREBY GRANTED, FREE OF CHARGE, TO ANY PERSON OBTAINING
#A COPY OF THIS SOFTWARE AND ASSOCIATED DOCUMENTATION FILES (THE
#"SOFTWARE"), TO DEAL IN THE SOFTWARE WITHOUT RESTRICTION, INCLUDING
#WITHOUT LIMITATION THE RIGHTS TO USE, COPY, MODIFY, MERGE, PUBLISH,
#DISTRIBUTE, SUBLICENSE, AND/OR SELL COPIES OF THE SOFTWARE, AND TO
#PERMIT PERSONS TO WHOM THE SOFTWARE IS FURNISHED TO DO SO, SUBJECT TO
#THE FOLLOWING CONDITIONS:
#
#THE ABOVE COPYRIGHT NOTICE, THIS PERMISSION NOTICE, AND THE LIMITATION
#OF LIABILITY BELOW SHALL BE INCLUDED IN ALL COPIES OR REDISTRIBUTIONS
#OF SUBSTANTIAL PORTIONS OF THE SOFTWARE.
#
#SOFTWARE IS BEING DEVELOPED IN PART AT THE NATIONAL HEART, LUNG, AND BLOOD
#INSTITUTE, NATIONAL INSTITUTES OF HEALTH BY AN EMPLOYEE OF THE FEDERAL
#GOVERNMENT IN THE COURSE OF HIS OFFICIAL DUTIES. PURSUANT TO TITLE 17, 
#SECTION 105 OF THE UNITED STATES CODE, THIS SOFTWARE IS NOT SUBJECT TO 
#COPYRIGHT PROTECTION AND IS IN THE PUBLIC DOMAIN. EXCEPT AS CONTAINED IN
#THIS NOTICE, THE NAME OF THE AUTHORS, THE NATIONAL HEART, LUNG, AND BLOOD
#INSTITUTE (NHLBI), OR THE NATIONAL INSTITUTES OF HEALTH (NIH) MAY NOT 
#BE USED TO ENDORSE OR PROMOTE PRODUCTS DERIVED FROM THIS SOFTWARE WITHOUT 
#SPECIFIC PRIOR WRITTEN PERMISSION FROM THE NHLBI OR THE NIH.THE SOFTWARE IS 
#PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
#INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
#FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
#AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
#IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
#IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#modified Philip Top 2018 to add windows search paths and find octave executable itself

# Try to find the build flags to compile octave shared objects (oct and mex files)
# Once done this will define
#
# OCTAVE_FOUND - if OCTAVE is found
# OCTAVE_CXXFLAGS - extra flags
# OCTAVE_INCLUDE_DIRS - include directories
# OCTAVE_LINK_DIRS - link directories
# OCTAVE_LIBRARY_RELEASE - the relase version
# OCTAVE_LIBRARY_DEBUG - the debug version
# OCTAVE_LIBRARY - a default library, with priority debug.

if (WIN32)

#message(STATUS "win32 octave search")
set (octave_versions
Octave-5.1.0.0
Octave-4.4.0
Octave-4.2.1
Octave-4.2.0
Octave-4.0.3
Octave-4.0.2
Octave-4.0.1
Octave-4.0.0
)

set(poss_prefixes
C:
C:/Octave
"C:/Program Files"
C:/local
C:/local/Octave
D:
D:/Octave
"D:/Program Files"
D:/local
D:/local/Octave
)

# create an empty list
list(APPEND octave_paths "")

foreach( dir ${poss_prefixes})
	foreach( octver ${octave_versions})
		if (IS_DIRECTORY ${dir}/${octver})
			list(APPEND oct_paths ${dir}/${octver})
			message(STATUS "found oct path ${dir}/${octver}")
		endif()
	endforeach()
endforeach()


endif(WIN32)

# use mkoctfile
if (NOT MKOCTFILE_EXECUTABLE)
set(MKOCTFILE_EXECUTABLE MKOCTFILE_EXECUTABLE-NOTFOUND)
endif()
find_program(MKOCTFILE_EXECUTABLE NAME mkoctfile 
	HINTS 
		${OCTAVE_INSTALL_LOCATION} 
	PATHS 
		${oct_paths} 
	PATH_SUFFIXES 
		bin
	)

mark_as_advanced(MKOCTFILE_EXECUTABLE)

if (NOT OCTAVE_EXECUTABLE)
set(OCTAVE_EXECUTABLE OCTAVE_EXECUTABLE-NOTFOUND)
endif()
find_program(OCTAVE_EXECUTABLE NAME octave octave.vbs
	HINTS 
		${OCTAVE_INSTALL_LOCATION} 
	PATHS 
		${oct_paths} 
	PATH_SUFFIXES 
		bin
	)

mark_as_advanced(OCTAVE_EXECUTABLE)

get_filename_component(OCTAVE_EXECUTABLE_DIR ${OCTAVE_EXECUTABLE} DIRECTORY)


if(MKOCTFILE_EXECUTABLE)
  set(OCTAVE_FOUND 1)

  execute_process(
    COMMAND ${MKOCTFILE_EXECUTABLE} -p ALL_CXXFLAGS
    OUTPUT_VARIABLE _mkoctfile_cppflags
    RESULT_VARIABLE _mkoctfile_failed)
  string(REGEX REPLACE "[\r\n]" " " _mkoctfile_cppflags "${_mkoctfile_cppflags}")
  execute_process(
    COMMAND ${MKOCTFILE_EXECUTABLE} -p INCFLAGS
    OUTPUT_VARIABLE _mkoctfile_includedir
    RESULT_VARIABLE _mkoctfile_failed)
  string(REGEX REPLACE "[\r\n]" " " _mkoctfile_includedir "${_mkoctfile_includedir}")
  execute_process(
    COMMAND ${MKOCTFILE_EXECUTABLE} -p ALL_LDFLAGS
    OUTPUT_VARIABLE _mkoctfile_ldflags
    RESULT_VARIABLE _mkoctfile_failed)
  string(REGEX REPLACE "[\r\n]" " " _mkoctfile_ldflags "${_mkoctfile_ldflags}")
  execute_process(
    COMMAND ${MKOCTFILE_EXECUTABLE} -p LFLAGS
    OUTPUT_VARIABLE _mkoctfile_lflags
    RESULT_VARIABLE _mkoctfile_failed)
  string(REGEX REPLACE "[\r\n]" " " _mkoctfile_lflags "${_mkoctfile_lflags}")
  execute_process(
    COMMAND ${MKOCTFILE_EXECUTABLE} -p LIBS
    OUTPUT_VARIABLE _mkoctfile_libs
    RESULT_VARIABLE _mkoctfile_failed)
  string(REGEX REPLACE "[\r\n]" " " _mkoctfile_libs "${_mkoctfile_libs}")
  execute_process(
    COMMAND ${MKOCTFILE_EXECUTABLE} -p OCTAVE_LIBS
    OUTPUT_VARIABLE _mkoctfile_octlibs
    RESULT_VARIABLE _mkoctfile_failed)
  string(REGEX REPLACE "[\r\n]" " " _mkoctfile_octlibs "${_mkoctfile_octlibs}")
  set(_mkoctfile_libs "${_mkoctfile_libs} ${_mkoctfile_octlibs}")

  string(REGEX MATCHALL "(^| )-l([./+-_\\a-zA-Z]*)" _mkoctfile_libs "${_mkoctfile_libs}")
  string(REGEX REPLACE "(^| )-l" "" _mkoctfile_libs "${_mkoctfile_libs}")

  string(REGEX MATCHALL "(^| )-L([./+-_\\a-zA-Z]*)" _mkoctfile_ldirs "${_mkoctfile_lflags}")
  string(REGEX REPLACE "(^| )-L" "" _mkoctfile_ldirs "${_mkoctfile_ldirs}")

  string(REGEX MATCHALL "(^| )-I([./+-_\\a-zA-Z]*)" _mkoctfile_includedir "${_mkoctfile_includedir}")
  string(REGEX REPLACE "(^| )-I" "" _mkoctfile_includedir "${_mkoctfile_includedir}")

  string(REGEX REPLACE "(^| )-l([./+-_\\a-zA-Z]*)" " " _mkoctfile_ldflags "${_mkoctfile_ldflags}")
  string(REGEX REPLACE "(^| )-L([./+-_\\a-zA-Z]*)" " " _mkoctfile_ldflags "${_mkoctfile_ldflags}")

  separate_arguments(_mkoctfile_includedir)

  set( OCTAVE_CXXFLAGS "${_mkoctfile_cppflags}" )
  set( OCTAVE_LINK_FLAGS "${_mkoctfile_ldflags}" )
  set( OCTAVE_INCLUDE_DIRS ${_mkoctfile_includedir})
  set( OCTAVE_LINK_DIRS ${_mkoctfile_ldirs})
  set( OCTAVE_LIBRARY ${_mkoctfile_libs})
  set( OCTAVE_LIBRARY_RELEASE ${OCTAVE_LIBRARY})
  set( OCTAVE_LIBRARY_DEBUG ${OCTAVE_LIBRARY})
  
endif()

mark_as_advanced(
    OCTAVE_LIBRARY_FOUND
    OCTAVE_CXXFLAGS
    OCTAVE_LINK_FLAGS
    OCTAVE_INCLUDE_DIRS
    OCTAVE_LINK_DIRS
    OCTAVE_LIBRARY
    OCTAVE_LIBRARY_RELEASE
    OCTAVE_LIBRARY_DEBUG
	OCTAVE_EXECUTABLE_DIR
)