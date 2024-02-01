# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/frameworks/esp-idf-v5.1.1/components/bootloader/subproject"
  "C:/Users/PC/workspace/Manually_set_date_Time/build/bootloader"
  "C:/Users/PC/workspace/Manually_set_date_Time/build/bootloader-prefix"
  "C:/Users/PC/workspace/Manually_set_date_Time/build/bootloader-prefix/tmp"
  "C:/Users/PC/workspace/Manually_set_date_Time/build/bootloader-prefix/src/bootloader-stamp"
  "C:/Users/PC/workspace/Manually_set_date_Time/build/bootloader-prefix/src"
  "C:/Users/PC/workspace/Manually_set_date_Time/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "C:/Users/PC/workspace/Manually_set_date_Time/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "C:/Users/PC/workspace/Manually_set_date_Time/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
