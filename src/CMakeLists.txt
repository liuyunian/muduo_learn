# ======================== source =========================
SET(SRCS 
    main.cpp
    Server.cpp
    HttpConnection.cpp
    reactor/Channel.cpp
    reactor/EventLoop.cpp
    reactor/Poller.cpp
    reactor/poller/PollPoller.cpp
    reactor/poller/EPollPoller.cpp)

# ========================= build ==========================
ADD_EXECUTABLE(webserver ${SRCS})
TARGET_LINK_LIBRARIES(webserver tools_log tools_base)

# ========================= install ========================
# webserver -- install path = /usr/local/bin
INSTALL(TARGETS webserver RUNTIME DESTINATION bin)