idf_component_register(
    SRCS "arm_control.cpp" "app_main.cpp" "joint_driver.cpp" "motion_task.cpp"
    INCLUDE_DIRS "."
    REQUIRES esp_driver_uart
)
