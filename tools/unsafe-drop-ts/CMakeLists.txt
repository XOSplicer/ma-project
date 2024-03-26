file(GLOB SOURCES
    *.h
    *.cpp
)

add_executable(unsafe-drop-ts ${SOURCES})

target_link_libraries(unsafe-drop-ts
    PUBLIC
    find_unsafe_rs
    phasar
    rustc_demangle
    ${PHASAR_STD_FILESYSTEM}
)

install(TARGETS unsafe-drop-ts
  RUNTIME DESTINATION bin
)
