add_executable(unsafe-taint-check unsafe-taint-check.cpp)

target_link_libraries(unsafe-taint-check
    PUBLIC
    find_unsafe_rs
    phasar
    ${PHASAR_STD_FILESYSTEM}
)

install(TARGETS unsafe-taint-check
  RUNTIME DESTINATION bin
)

add_executable(unsafe-taint-check-v2 unsafe-taint-check-v2.cpp)

target_link_libraries(unsafe-taint-check-v2
    PUBLIC
    find_unsafe_rs
    phasar
    ${PHASAR_STD_FILESYSTEM}
)

install(TARGETS unsafe-taint-check-v2
  RUNTIME DESTINATION bin
)
