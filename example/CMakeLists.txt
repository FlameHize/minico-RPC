#client
add_executable(client_test test_client.cc)
target_link_libraries(client_test minico-rpc)

#server
add_executable(server_test test_server.cc)
target_link_libraries(server_test minico-rpc)
