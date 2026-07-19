# CH32V examples

ch32_fetch_cmsis(V0 V2)
find_package(CMSIS COMPONENTS CH32V0 CH32V2 REQUIRED)

add_subdirectory(Clock)
add_subdirectory(Drivers)
add_subdirectory(GPIO)
add_subdirectory(Timer)
