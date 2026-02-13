add_executable(platformchannel_test
    platformchannel_test.c
)

target_link_libraries(
    platformchannel_test
    flutter_drm_embedder_module
    flutter_linux_gtk_shim
    Unity
)

add_test(platformchannel_test platformchannel_test)

add_executable(flutter_drm_embedder_test
    flutter_drm_embedder_test.c
)

target_link_libraries(
    flutter_drm_embedder_test
    flutter_drm_embedder_module
    flutter_linux_gtk_shim
    Unity
)

add_test(flutter_drm_embedder_test flutter_drm_embedder_test)