#rpc-client
add_executable(rpc_client_test rpc_client_test.cc)
target_link_libraries(rpc_client_test minico-rpc)

#rpc-server
add_executable(rpc_server_test rpc_server_test.cc)
target_link_libraries(rpc_server_test minico-rpc)

#tcp-client
add_executable(tcp_client_test tcp_client_test.cc)
target_link_libraries(tcp_client_test minico-rpc)

#tcp-server
add_executable(tcp_server_test tcp_server_test.cc)
target_link_libraries(tcp_server_test minico-rpc)

#udp-client
add_executable(udp_client_test udp_client_test.cc)
target_link_libraries(udp_client_test minico-rpc)

#udp-server
add_executable(udp_server_test udp_server_test.cc)
target_link_libraries(udp_server_test minico-rpc)