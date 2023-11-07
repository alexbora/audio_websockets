# CMake generated Testfile for 
# Source directory: C:/Temp/libwebsockets/minimal-examples-lowlevel/http-client/minimal-http-client
# Build directory: C:/Temp/libwebsockets/minimal-examples-lowlevel/http-client/minimal-http-client
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(http-client-warmcat "C:/Temp/libwebsockets/bin/lws-minimal-http-client.exe")
set_tests_properties(http-client-warmcat PROPERTIES  TIMEOUT "20" WORKING_DIRECTORY "C:/Temp/libwebsockets/minimal-examples-lowlevel/http-client/minimal-http-client" _BACKTRACE_TRIPLES "C:/Temp/libwebsockets/minimal-examples-lowlevel/http-client/minimal-http-client/CMakeLists.txt;53;add_test;C:/Temp/libwebsockets/minimal-examples-lowlevel/http-client/minimal-http-client/CMakeLists.txt;0;")
add_test(http-client-warmcat-h1 "C:/Temp/libwebsockets/bin/lws-minimal-http-client.exe" "--h1")
set_tests_properties(http-client-warmcat-h1 PROPERTIES  TIMEOUT "20" WORKING_DIRECTORY "C:/Temp/libwebsockets/minimal-examples-lowlevel/http-client/minimal-http-client" _BACKTRACE_TRIPLES "C:/Temp/libwebsockets/minimal-examples-lowlevel/http-client/minimal-http-client/CMakeLists.txt;58;add_test;C:/Temp/libwebsockets/minimal-examples-lowlevel/http-client/minimal-http-client/CMakeLists.txt;0;")
