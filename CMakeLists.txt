cmake_minimum_required(VERSION 3.16)
project(solution)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system)
find_package(jsoncpp REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -g")

add_executable(aggregate Main.cpp Scraper.cpp)

target_include_directories(aggregate PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(aggregate PRIVATE Boost::boost Boost::system jsoncpp_lib OpenSSL::SSL OpenSSL::Crypto pthread)
