# This file is part of the MicroPython project, http://micropython.org/
#
# The MIT License (MIT)
#
# Copyright 2020 NXP
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

cmake_minimum_required(VERSION 3.20.0)

project(micropython)

set(MICROPY_PORT_DIR    ${CMAKE_CURRENT_SOURCE_DIR})
set(MICROPY_DIR         ${MICROPY_PORT_DIR}/../..)
set(MICROPY_TARGET      micropython)

set(CONFIG_MICROPY_CONFIGFILE mpconfigport.h)
set(CONFIG_MICROPY_HEAP_SIZE 16384)

include(${MICROPY_DIR}/py/py.cmake)
include(${MICROPY_DIR}/extmod/extmod.cmake)

set(MICROPY_SOURCE_PORT
    main.c
    mphalport.c
    system.c
)
list(TRANSFORM MICROPY_SOURCE_PORT PREPEND ${MICROPY_PORT_DIR}/)

set(MICROPY_SOURCE_SHARED
    libc/printf.c
    readline/readline.c
    runtime/interrupt_char.c
    runtime/mpirq.c
    runtime/pyexec.c
    runtime/stdout_helpers.c
    runtime/sys_stdio_mphal.c
    timeutils/timeutils.c
)
list(TRANSFORM MICROPY_SOURCE_SHARED PREPEND ${MICROPY_DIR}/shared/)

#set(MICROPY_SOURCE_LIB
#    oofatfs/ff.c
#    oofatfs/ffunicode.c
#    littlefs/lfs1.c
#    littlefs/lfs1_util.c
#    littlefs/lfs2.c
#    littlefs/lfs2_util.c
#)
#list(TRANSFORM MICROPY_SOURCE_LIB PREPEND ${MICROPY_DIR}/lib/)

set(MICROPY_SOURCE_QSTR
    ${MICROPY_SOURCE_PY}
    ${MICROPY_SOURCE_EXTMOD}
    ${MICROPY_SOURCE_SHARED}
    ${MICROPY_SOURCE_LIB}
    ${MICROPY_SOURCE_PORT}
)

add_executable(${MICROPY_TARGET})

target_include_directories(${MICROPY_TARGET} PRIVATE
    ${MICROPY_INC_CORE}
    ${MICROPY_PORT_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)

target_include_directories(${MICROPY_TARGET} PRIVATE ${WIN_SDK_INCLUDE_DIR})

#zephyr_library_compile_options(
#    -std=gnu99 -fomit-frame-pointer
#)

target_compile_definitions(
    ${MICROPY_TARGET} PRIVATE
    MP_CONFIGFILE=<${CONFIG_MICROPY_CONFIGFILE}>
    MICROPY_HEAP_SIZE=${CONFIG_MICROPY_HEAP_SIZE}
    #FFCONF_H=\"${MICROPY_OOFATFS_DIR}/ffconf.h\"
    #MICROPY_VFS_FAT=$<BOOL:${CONFIG_MICROPY_VFS_FAT}>
    #MICROPY_VFS_LFS1=$<BOOL:${CONFIG_MICROPY_VFS_LFS1}>
    #MICROPY_VFS_LFS2=$<BOOL:${CONFIG_MICROPY_VFS_LFS2}>
)

target_sources(${MICROPY_TARGET} PRIVATE ${MICROPY_SOURCE_QSTR})
target_link_libraries(${MICROPY_TARGET} PUBLIC c)
target_link_libraries(${MICROPY_TARGET} PRIVATE c.host)

include(${MICROPY_DIR}/py/mkrules.cmake)

#target_sources(app PRIVATE
#    src/zephyr_start.c
#    src/zephyr_getchar.c
#)
#
#target_link_libraries(app PRIVATE ${MICROPY_TARGET})
