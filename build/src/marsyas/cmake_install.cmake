# Install script for directory: /Users/nao/git/_public/marsyas/src/marsyas

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/marsyas" TYPE FILE FILES
    "/Users/nao/git/_public/marsyas/src/marsyas/basis.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/Collection.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/Color.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/CommandLineOptions.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/common_header.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/common_source.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/Communicator.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/Conversions.h"
    "/Users/nao/git/_public/marsyas/build/src/marsyas/export.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/dsound.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/ERBTools.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/fft.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/FileName.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/Heap.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/lu.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/marohtml.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/marojson.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/marostring.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/marosvg.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/maroxml.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/types.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/MATLABengine.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/mididevices.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/MrsLog.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/NumericLib.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/peakView.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/PeUtilities.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/QGMMModel.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/static_vq_codebook.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/statistics.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/Stk.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/StrobeList.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/Thread.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/TimeLine.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/TranscriberExtract.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/Transcriber.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/vmblock.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/WekaData.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/realvec.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/marsyas/system" TYPE FILE FILES
    "/Users/nao/git/_public/marsyas/src/marsyas/system/MarControlAccessor.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/system/MarControl.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/system/MarControlManager.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/system/MarControlValue.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/system/MarSystem.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/system/MarSystemManager.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/marsyas/sched" TYPE FILE FILES
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/EvEvent.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/EvExpr.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/EvGetUpd.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/EvValUpd.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/Repeat.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/Scheduler.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/TmControlValue.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/TmParam.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/TmRealTime.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/TmSampleCount.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/TmTime.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/TmTimer.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/TmTimerManager.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/sched/TmVirtualTime.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/marsyas/expr" TYPE FILE FILES
    "/Users/nao/git/_public/marsyas/src/marsyas/expr/ExCommon.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/expr/ExNode.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/expr/ExParser.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/expr/Expr.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/expr/ExScanner.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/expr/ExSymTbl.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/expr/ExVal.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/marsyas/realtime" TYPE FILE FILES
    "/Users/nao/git/_public/marsyas/src/marsyas/realtime/any.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/realtime/stage.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/realtime/queue.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/realtime/packet_queue.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/realtime/realvec_queue.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/realtime/atomic_control.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/realtime/controller.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/realtime/osc_receiver.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/realtime/osc_transmitter.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/realtime/udp_receiver.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/realtime/udp_transmitter.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/realtime/runner.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/marsyas/debug" TYPE FILE FILES
    "/Users/nao/git/_public/marsyas/src/marsyas/debug/debugger.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/debug/record.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/debug/recorder.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/debug/file_io.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/marsyas/script" TYPE FILE FILES
    "/Users/nao/git/_public/marsyas/src/marsyas/script/script.h"
    "/Users/nao/git/_public/marsyas/src/marsyas/script/manager.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/nao/git/_public/marsyas/build/lib/Debug/libmarsyas.a")
    if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a")
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a")
    endif()
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/nao/git/_public/marsyas/build/lib/Release/libmarsyas.a")
    if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a")
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a")
    endif()
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/nao/git/_public/marsyas/build/lib/MinSizeRel/libmarsyas.a")
    if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a")
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a")
    endif()
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/nao/git/_public/marsyas/build/lib/RelWithDebInfo/libmarsyas.a")
    if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a")
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a")
    endif()
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Pp][Rr][Oo][Ff][Ii][Ll][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY FILES "/Users/nao/git/_public/marsyas/build/lib/Profile/libmarsyas.a")
    if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a" AND
       NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a")
      execute_process(COMMAND "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/ranlib" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libmarsyas.a")
    endif()
  endif()
endif()

