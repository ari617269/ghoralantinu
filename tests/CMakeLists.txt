find_package(Qt6 6.5 REQUIRED COMPONENTS Test)

add_executable(tst_scaffold tst_scaffold.cpp)
target_link_libraries(tst_scaffold PRIVATE ttcore Qt6::Test)
add_test(NAME tst_scaffold COMMAND tst_scaffold)

add_executable(tst_clock tst_clock.cpp)
target_link_libraries(tst_clock PRIVATE ttcore Qt6::Test)
add_test(NAME tst_clock COMMAND tst_clock)

add_executable(tst_repositories tst_repositories.cpp)
target_link_libraries(tst_repositories PRIVATE ttcore Qt6::Test)
add_test(NAME tst_repositories COMMAND tst_repositories)

add_executable(tst_trackerservice tst_trackerservice.cpp)
target_link_libraries(tst_trackerservice PRIVATE ttcore Qt6::Test)
add_test(NAME tst_trackerservice COMMAND tst_trackerservice)

add_executable(tst_tracker_ui tst_tracker_ui.cpp)
target_link_libraries(tst_tracker_ui PRIVATE ttui Qt6::Test)
add_test(NAME tst_tracker_ui COMMAND tst_tracker_ui)

add_executable(tst_history_ui tst_history_ui.cpp)
target_link_libraries(tst_history_ui PRIVATE ttui Qt6::Test)
add_test(NAME tst_history_ui COMMAND tst_history_ui)

add_executable(tst_tray_ui tst_tray_ui.cpp)
target_link_libraries(tst_tray_ui PRIVATE ttui Qt6::Test)
add_test(NAME tst_tray_ui COMMAND tst_tray_ui)

add_executable(tst_robustness tst_robustness.cpp)
target_link_libraries(tst_robustness PRIVATE ttcore Qt6::Test)
add_test(NAME tst_robustness COMMAND tst_robustness)

add_executable(tst_robustness_ui tst_robustness_ui.cpp)
target_link_libraries(tst_robustness_ui PRIVATE ttui Qt6::Test)
add_test(NAME tst_robustness_ui COMMAND tst_robustness_ui)

add_executable(tst_timeutils tst_timeutils.cpp)
target_link_libraries(tst_timeutils PRIVATE ttcore Qt6::Test)
foreach(zone IN ITEMS UTC America/New_York Europe/Berlin)
    string(REPLACE "/" "_" suffix "${zone}")
    add_test(NAME tst_timeutils_${suffix} COMMAND tst_timeutils)
    set_tests_properties(tst_timeutils_${suffix} PROPERTIES ENVIRONMENT "TZ=${zone}")
endforeach()
