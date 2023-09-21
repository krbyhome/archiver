add_executable(${PROJECT_NAME} HamArc.cpp)

target_link_libraries(${PROJECT_NAME} PRIVATE get_options)
target_link_libraries(${PROJECT_NAME} PRIVATE haf)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})