protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS quasar.proto)

add_library(quasar
        ${PROTO_SRCS} ${PROTO_HDRS}
        network.cpp network.h
        types.cpp types.h
        crypto.cpp crypto.h
        blockchain.cpp blockchain.h
        testing/test_network.cpp testing/test_network.h
        quasar.cpp quasar.h
        exception.cpp exception.h keystore.cpp keystore.h consensus.cpp consensus.h synchronizer.cpp synchronizer.h leader_rotation.cpp leader_rotation.h quorum.h round_duration.cpp mempool.cpp mempool.h)

target_include_directories(quasar PUBLIC
        ${BOTAN_INCLUDE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}) # needed in order to find generated proto header

target_link_libraries(quasar PUBLIC
        ${BOTAN_LIBRARY}
        cpptime
        cppzmq-static
        eventpp::eventpp
        spdlog::spdlog)

target_link_libraries(quasar PRIVATE
        fmt::fmt
        protobuf::libprotobuf-lite)

add_executable(tests
        blockchain_test.cpp
        testing/test_network_test.cpp network_test.cpp quasar_test.cpp)

include(Catch)
catch_discover_tests(tests)

target_link_libraries(tests PRIVATE
        quasar
        Catch2::Catch2WithMain)