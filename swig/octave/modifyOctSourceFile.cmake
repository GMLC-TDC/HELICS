# modifyOctSourceFile.cmake

if (WIN32)
if(helicsOCTAVE_wrap.cxx IS_NEWER_THAN helicsOCTAVE_wrap.cpp)
file(READ helicsOCTAVE_wrap.cxx HELICS_OCT_SOURCE)

string(REPLACE "long swig_this\(\) const"
       "long long swig_this\(\)" HELICS_OCT_SOURCE
       "${HELICS_OCT_SOURCE}")
string(REPLACE "return \(long\) this"
       "return \(long long\) this" HELICS_OCT_SOURCE
       "${HELICS_OCT_SOURCE}")
string(REPLACE "\(long\) types[0].second.ptr"
       "\(long long\) types[0].second.ptr" HELICS_OCT_SOURCE
       "${HELICS_OCT_SOURCE}")
   
file(WRITE helicsOCTAVE_wrap.cpp "${HELICS_OCT_SOURCE}")
endif()
else(WIN32)

configure_file(helicsOCTAVE_wrap.cxx helicsOCTAVE_wrap.cpp COPYONLY)
endif(WIN32)