protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS quasar.proto)

add_library(quasar
        ${PROTO_SRCS} ${PROTO_HDRS}
        network.cpp network.h
        types.cpp types.h
        crypto.cpp crypto.h
        consensus.cpp consensus.h
        blockchain.cpp blockchain.h
        util/array_hasher.h)

target_include_directories(quasar PRIVATE
        ${BOTAN_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}) # needed in order to find generated proto header

target_link_libraries(quasar PRIVATE
        ${BOTAN_LIBRARY}
        cppzmq-static
        fmt::fmt
        protobuf::libprotobuf-lite
        spdlog::spdlog)

add_executable(tests
        blockchain_test.cpp)

include(Catch)
catch_discover_tests(tests)

target_include_directories(tests PRIVATE
        ${BOTAN_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(tests PRIVATE
        quasar
        Catch2::Catch2WithMain
        ${BOTAN_LIBRARY})