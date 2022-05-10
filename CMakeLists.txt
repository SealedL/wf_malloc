cmake_minimum_required(VERSION 3.22)
project(wf_malloc C)

set(CMAKE_C_STANDARD 11)

add_executable(wf_malloc main.c wf_malloc.c wf_malloc.h test.c)
