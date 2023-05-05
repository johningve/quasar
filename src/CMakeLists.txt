add_library(quasar library.cpp)

target_include_directories(quasar PRIVATE ${BOTAN_INCLUDE_DIR})
target_link_libraries(quasar PRIVATE
        ${BOTAN_LIBRARY}
        cppzmq-static
        fmt::fmt
        protobuf::libprotobuf-lite
        spdlog::spdlog
        )
