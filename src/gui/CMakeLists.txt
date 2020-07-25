project(txs_explorer)


add_executable(
    ${PROJECT_NAME}
    mainframe.cpp
    txs_app.cpp
    txs_table.cpp
    blocks_table.cpp
    node_data_model.cpp
    )

target_link_libraries(
    ${PROJECT_NAME}
    ${wxWidgets_LIBRARIES}
    base core rpc
    )

message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_CURRENT_BINARY_DIR: ${CMAKE_CURRENT_BINARY_DIR}")

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/nodes.txt
    $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

