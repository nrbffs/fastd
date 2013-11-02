find_package(BISON 2.5 REQUIRED)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads)
if(NOT CMAKE_USE_PTHREADS_INIT)
  MESSAGE(FATAL_ERROR "No pthread support found.")
endif(NOT CMAKE_USE_PTHREADS_INIT)

find_package(PkgConfig REQUIRED)
pkg_check_modules(UECC REQUIRED libuecc>=3)


set(NACL_INCLUDE_DIRS "")
set(NACL_CFLAGS_OTHER "")
set(NACL_LIBRARY_DIRS "")
set(NACL_LIBRARIES "")
set(NACL_LDFLAGS_OTHER "")

if(WITH_METHOD_XSALSA20_POLY1305 OR WITH_CIPHER_AES128_CTR_NACL)
  if(USE_LIBSODIUM)
    pkg_check_modules(SODIUM REQUIRED libsodium)

    set(NACL_INCLUDE_DIRS "${SODIUM_INCLUDE_DIRS}")
    foreach(dir "${SODIUM_INCLUDEDIR}" ${SODIUM_INCLUDE_DIRS})
      list(APPEND NACL_INCLUDE_DIRS "${dir}/sodium")
    endforeach(dir)

    set(NACL_CFLAGS_OTHER "${SODIUM_CFLAGS_OTHER}")
    set(NACL_LIBRARY_DIRS "${SODIUM_LIBRARY_DIRS}")
    set(NACL_LIBRARIES "${SODIUM_LIBRARIES}")
    set(NACL_LDFLAGS_OTHER "${SODIUM_LDFLAGS_OTHER}")
  else(USE_LIBSODIUM)
    find_package(NaCl REQUIRED)

    set(NACL_INCLUDE_DIRS "${NACL_INCLUDE_DIR}")
    set(NACL_LIBRARIES "${NACL_LIBRARY}")
  endif(USE_LIBSODIUM)
endif()


if(WITH_CAPABILITIES)
  find_package(CAP REQUIRED)
else(WITH_CAPABILITIES)
  set(CAP_INCLUDE_DIR "")
  set(CAP_LIBRARY "")
endif(WITH_CAPABILITIES)
