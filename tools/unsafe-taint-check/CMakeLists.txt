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
