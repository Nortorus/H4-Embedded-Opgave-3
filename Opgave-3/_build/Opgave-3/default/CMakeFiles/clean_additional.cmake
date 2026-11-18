# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  [[C:\Embedded\Opgave-3\out\Opgave-3\default.cmf]]
  [[C:\Embedded\Opgave-3\out\Opgave-3\default.hex]]
  [[C:\Embedded\Opgave-3\out\Opgave-3\default.hxl]]
  [[C:\Embedded\Opgave-3\out\Opgave-3\default.mum]]
  [[C:\Embedded\Opgave-3\out\Opgave-3\default.o]]
  [[C:\Embedded\Opgave-3\out\Opgave-3\default.sdb]]
  [[C:\Embedded\Opgave-3\out\Opgave-3\default.sym]]
  )
endif()
