# Central test entrypoint. Individual test targets are grouped here by area.

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

target_include_directories(project_service_test
    PRIVATE
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
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

target_link_libraries(app_context_test
    PRIVATE
        Qt6::Core
        Qt6::Sql
        Qt6::Test
        travis_data_database
        travis_services
)

add_test(NAME project_service_test COMMAND project_service_test)
add_test(NAME session_service_test COMMAND session_service_test)
add_test(NAME structure_service_test COMMAND structure_service_test)
add_test(NAME execution_service_test COMMAND execution_service_test)
add_test(NAME result_service_test COMMAND result_service_test)
add_test(NAME video_service_test COMMAND video_service_test)
add_test(NAME inspection_clip_service_test COMMAND inspection_clip_service_test)
add_test(NAME app_context_test COMMAND app_context_test)
