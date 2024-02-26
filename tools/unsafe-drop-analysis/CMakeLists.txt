add_executable(unsafe-drop-analysis unsafe-drop-analysis.cpp)

target_link_libraries(unsafe-drop-analysis
    PUBLIC
    find_unsafe_rs
    phasar
    ${PHASAR_STD_FILESYSTEM}
)

install(TARGETS unsafe-drop-analysis
  RUNTIME DESTINATION bin
)
