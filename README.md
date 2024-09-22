# WebServer

主要几个应该就是：时间堆，日志，http的处理，分别对应server文件中的timer,loger,http

```bash
.
├── CMakeLists.txt
├── README.md
├── build
│
│   ├── Makefile
│   ├── bin
│   │   └── server
│   └── log
│       └── webserver.log
├── main.cpp
├── output
│   └── demo
├── server
│   ├── buffer
│   │   ├── buffer.cpp
│   │   └── buffer.h
│   ├── epoller.cpp
│   ├── epoller.h
│   ├── http
│   │   ├── httprequest.cpp
│   │   └── httprequest.h
│   ├── loger
│   │   ├── blockqueue.hpp
│   │   ├── log.cpp
│   │   └── log.h
│   ├── thread_pool.hpp
│   ├── timer
│   │   └── time_heap.hpp
│   ├── webserver.cpp
│   └── webserver.h
└── temp
├── Eventloop.cpp
├── Eventloop.h
├── config.cpp
└── config.h
```

temp文件夹，config是准备实现的命令行解析，Eventloop是该删了的（
