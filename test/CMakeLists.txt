add_executable(platformchannel_test
    platformchannel_test.c
)

target_link_libraries(
    platformchannel_test
    flutterpi_module
    flutter_linux_gtk_shim
    Unity
)

add_test(platformchannel_test platformchannel_test)

add_executable(flutterpi_test
    flutterpi_test.c
)

target_link_libraries(
    flutterpi_test
    flutterpi_module
    flutter_linux_gtk_shim
    Unity
)

add_test(flutterpi_test flutterpi_test)