idf_component_register(
    SRCS "app_main.cpp" "joint_driver.cpp" "motion_task.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_uart
)
