cmake_minimum_required(VERSION 3.22)
project(simple_http_server C)

set(CMAKE_C_STANDARD 99)

add_executable(simple_http_server webserv_linux.c webserv_win.c)
