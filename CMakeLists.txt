cmake_minimum_required(VERSION 3.10)
project(OpenSentry_Node)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Find FFmpeg
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBAV REQUIRED IMPORTED_TARGET
        libavcodec
        libavformat
        libavutil
        libswscale
)

# Find Avahi for mDNS (avahi-common headers are included in avahi-client)
pkg_check_modules(AVAHI REQUIRED avahi-client)

# Find Paho MQTT
find_library(PAHO_MQTT_C paho-mqtt3a)
find_library(PAHO_MQTT_CPP paho-mqttpp3)

# Find OpenSSL for credential derivation
find_package(OpenSSL REQUIRED)

add_executable(OpenSentry_Node src/main.cpp)

# Include directories
target_include_directories(OpenSentry_Node PRIVATE
        ${AVAHI_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(OpenSentry_Node
        ${OpenCV_LIBS}
        PkgConfig::LIBAV
        ${PAHO_MQTT_C}
        ${PAHO_MQTT_CPP}
        ${AVAHI_LIBRARIES}
        OpenSSL::Crypto
        pthread
)

# Print found libraries for debugging
message(STATUS "OpenCV version: ${OpenCV_VERSION}")
message(STATUS "Avahi libraries: ${AVAHI_LIBRARIES}")