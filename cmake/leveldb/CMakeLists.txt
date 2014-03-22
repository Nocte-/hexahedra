cmake_minimum_required(VERSION 2.8)
PROJECT(leveldb)

set(Boost_USE_STATIC_LIBS    ON)
set(Boost_USE_STATIC_RUNTIME ON)
set(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG}  /MTd /wd4244 /wd4996 /wd4018")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /DNDEBUG /MT /GS- /Ox /Ob2 /Oi /Oy /arch:SSE /fp:fast /Zi  /wd4244 /wd4996 /wd4018")
add_definitions(-DLEVELDB_PLATFORM_WINDOWS -DOS_WIN -DWIN32_LEAN_AND_MEAN -DCOMPILER_MSVC -D_CRT_SECURE_NO_WARNINGS)
include_directories(. include)

set(SRCS 
    db/builder.cc
    db/db_impl.cc
    db/db_iter.cc
    db/filename.cc
    db/dbformat.cc
    db/log_reader.cc
    db/log_writer.cc
    db/memtable.cc
    db/repair.cc
    db/table_cache.cc
    db/version_edit.cc
    db/version_set.cc
    db/write_batch.cc
    table/block.cc
    table/block_builder.cc
    table/format.cc
    table/iterator.cc
    table/merger.cc
    table/table.cc
    table/table_builder.cc
    table/two_level_iterator.cc
    util/env.cc
    util/arena.cc
    util/cache.cc
    util/coding.cc
    util/comparator.cc
    util/crc32c.cc
    util/hash.cc
    util/histogram.cc
    util/logging.cc
    util/options.cc
    util/status.cc
    port/port_win.cc
    )

add_library(leveldb ${SRCS})
    