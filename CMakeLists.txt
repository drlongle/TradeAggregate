cmake_minimum_required(VERSION 3.16)
project(solution)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS program_options system)
find_package(jsoncpp REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

# TODO Remove -fsanitize=address before submission
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Wno-unused-parameter -Werror -Wreturn-type -fno-strict-aliasing -std=c++20 -g -fsanitize=address -fsanitize=undefined -fsanitize=signed-integer-overflow")

add_executable(aggregate Aggregator.cpp FileWriter.cpp Main.cpp Scraper.cpp)

target_include_directories(aggregate PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(aggregate PRIVATE Boost::boost Boost::system ${Boost_LIBRARIES} jsoncpp_lib OpenSSL::SSL OpenSSL::Crypto pthread)
