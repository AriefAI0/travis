# Central test entrypoint. Individual test targets are grouped here by area.

set(TRAVIS_GSTREAMER_TEST_RUNTIME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../runtime/gstreamer/1.0/msvc_x86_64")
set(TRAVIS_GSTREAMER_TEST_BIN_DIR "${TRAVIS_GSTREAMER_TEST_RUNTIME_ROOT}/bin")
set(TRAVIS_GSTREAMER_TEST_PLUGIN_DIR "${TRAVIS_GSTREAMER_TEST_RUNTIME_ROOT}/lib/gstreamer-1.0")
set(TRAVIS_GSTREAMER_TEST_RUNNER "${CMAKE_CURRENT_SOURCE_DIR}/support/run_with_gstreamer_runtime.cmake")

add_executable(project_service_test
    services/project_service_test.cpp
    support/sqlite_test_helper.cpp
)

add_executable(session_service_test
    services/session_service_test.cpp
    support/sqlite_test_helper.cpp
)

add_executable(structure_service_test
    services/structure_service_test.cpp
    support/sqlite_test_helper.cpp
)

add_executable(execution_service_test
    services/execution_service_test.cpp
    support/sqlite_test_helper.cpp
)

add_executable(result_service_test
    services/result_service_test.cpp
    support/sqlite_test_helper.cpp
)

add_executable(video_service_test
    services/video_service_test.cpp
    support/sqlite_test_helper.cpp
)

add_executable(inspection_clip_service_test
    services/inspection_clip_service_test.cpp
    support/sqlite_test_helper.cpp
)

add_executable(app_context_test
    app/app_context_test.cpp
)

add_executable(project_workspace_viewmodel_test
    ui/project_workspace_viewmodel_test.cpp
    ../include/ui/viewmodels/project_workspace_viewmodel.h
    ../src/ui/viewmodels/project_workspace_viewmodel.cpp
    support/sqlite_test_helper.cpp
)

add_executable(inspection_context_viewmodel_test
    ui/inspection_context_viewmodel_test.cpp
    ../include/ui/viewmodels/inspection_context_viewmodel.h
    ../src/ui/viewmodels/inspection_context_viewmodel.cpp
    support/sqlite_test_helper.cpp
)

add_executable(playback_surface_controller_test
    ui/playback_surface_controller_test.cpp
)

add_executable(media_runtime_test
    media/media_runtime_test.cpp
)

add_executable(source_discovery_test
    media/source_discovery_test.cpp
)

add_executable(audio_meter_engine_test
    media/audio_meter_engine_test.cpp
)

add_executable(playback_thumbnail_engine_test
    media/playback_thumbnail_engine_test.cpp
)

add_executable(playback_workflow_test
    media/playback_workflow_test.cpp
    support/sqlite_test_helper.cpp
)

add_executable(recording_recovery_service_test
    media/recording_recovery_service_test.cpp
    support/sqlite_test_helper.cpp
)

target_include_directories(project_service_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(project_service_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_data_database
        travis_services
)

target_include_directories(session_service_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(session_service_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_data_database
        travis_services
)

target_include_directories(structure_service_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(structure_service_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_data_database
        travis_services
)

target_include_directories(execution_service_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(execution_service_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_data_database
        travis_services
)

target_include_directories(result_service_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(result_service_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_data_database
        travis_services
)

target_include_directories(video_service_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(video_service_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_data_database
        travis_services
)

target_include_directories(inspection_clip_service_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(inspection_clip_service_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_data_database
        travis_services
)

target_include_directories(app_context_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(app_context_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_application
)

target_include_directories(project_workspace_viewmodel_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(project_workspace_viewmodel_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_data_database
        travis_services
)

target_include_directories(inspection_context_viewmodel_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(inspection_context_viewmodel_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_data_database
        travis_services
)

target_include_directories(playback_surface_controller_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(playback_surface_controller_test
    PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::Test
        travis_ui
)

target_include_directories(media_runtime_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(media_runtime_test
    PRIVATE
        Qt6::Core
        Qt6::Test
        travis_media_engine
)

target_include_directories(source_discovery_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(source_discovery_test
    PRIVATE
        Qt6::Core
        Qt6::Test
        travis_media_engine
)

target_include_directories(audio_meter_engine_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(audio_meter_engine_test
    PRIVATE
        Qt6::Core
        Qt6::Test
        travis_media_engine
)

target_include_directories(playback_thumbnail_engine_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(playback_thumbnail_engine_test
    PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::Test
        travis_media_engine
)

target_include_directories(playback_workflow_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(playback_workflow_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_data_database
        travis_services
        travis_application
)

target_include_directories(recording_recovery_service_test
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(recording_recovery_service_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_data_database
        travis_services
        travis_application
)

add_test(NAME project_service_test COMMAND project_service_test)
add_test(NAME session_service_test COMMAND session_service_test)
add_test(NAME structure_service_test COMMAND structure_service_test)
add_test(NAME execution_service_test COMMAND execution_service_test)
add_test(NAME result_service_test COMMAND result_service_test)
add_test(NAME video_service_test COMMAND video_service_test)
add_test(NAME inspection_clip_service_test COMMAND inspection_clip_service_test)
add_test(
    NAME media_runtime_test
    COMMAND
        ${CMAKE_COMMAND}
            -DTRAVIS_TEST_EXECUTABLE=$<TARGET_FILE:media_runtime_test>
            -DTRAVIS_GSTREAMER_RUNTIME_ROOT=${TRAVIS_GSTREAMER_TEST_RUNTIME_ROOT}
            -P ${TRAVIS_GSTREAMER_TEST_RUNNER}
)
add_test(
    NAME source_discovery_test
    COMMAND
        ${CMAKE_COMMAND}
            -DTRAVIS_TEST_EXECUTABLE=$<TARGET_FILE:source_discovery_test>
            -DTRAVIS_GSTREAMER_RUNTIME_ROOT=${TRAVIS_GSTREAMER_TEST_RUNTIME_ROOT}
            -P ${TRAVIS_GSTREAMER_TEST_RUNNER}
)
add_test(
    NAME audio_meter_engine_test
    COMMAND
        ${CMAKE_COMMAND}
            -DTRAVIS_TEST_EXECUTABLE=$<TARGET_FILE:audio_meter_engine_test>
            -DTRAVIS_GSTREAMER_RUNTIME_ROOT=${TRAVIS_GSTREAMER_TEST_RUNTIME_ROOT}
            -P ${TRAVIS_GSTREAMER_TEST_RUNNER}
)
add_test(
    NAME playback_thumbnail_engine_test
    COMMAND
        ${CMAKE_COMMAND}
            -DTRAVIS_TEST_EXECUTABLE=$<TARGET_FILE:playback_thumbnail_engine_test>
            -DTRAVIS_GSTREAMER_RUNTIME_ROOT=${TRAVIS_GSTREAMER_TEST_RUNTIME_ROOT}
            -P ${TRAVIS_GSTREAMER_TEST_RUNNER}
)
add_test(
    NAME app_context_test
    COMMAND
        ${CMAKE_COMMAND}
            -DTRAVIS_TEST_EXECUTABLE=$<TARGET_FILE:app_context_test>
            -DTRAVIS_GSTREAMER_RUNTIME_ROOT=${TRAVIS_GSTREAMER_TEST_RUNTIME_ROOT}
            -P ${TRAVIS_GSTREAMER_TEST_RUNNER}
)
add_test(NAME project_workspace_viewmodel_test COMMAND project_workspace_viewmodel_test)
add_test(NAME inspection_context_viewmodel_test COMMAND inspection_context_viewmodel_test)
add_test(
    NAME playback_surface_controller_test
    COMMAND
        ${CMAKE_COMMAND}
            -DTRAVIS_TEST_EXECUTABLE=$<TARGET_FILE:playback_surface_controller_test>
            -DTRAVIS_GSTREAMER_RUNTIME_ROOT=${TRAVIS_GSTREAMER_TEST_RUNTIME_ROOT}
            -P ${TRAVIS_GSTREAMER_TEST_RUNNER}
)
add_test(NAME playback_workflow_test COMMAND playback_workflow_test)
add_test(NAME recording_recovery_service_test COMMAND recording_recovery_service_test)
