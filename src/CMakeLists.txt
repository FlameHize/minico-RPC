add_library(minico-rpc SHARED
    logger.cc
    context.cc
    coroutine.cc
    epoller.cc
    timer.cc
    mstime.cc
    mutex.cc
    processor.cc
    processor_selector.cc
    socket.cc
    scheduler.cc
    minico_api.cc
    tcp_server.cc)
