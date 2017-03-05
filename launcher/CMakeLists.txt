# Build and link the launcher.
set(LAUNCHER_SRC launcher.cpp resource.rc)
add_executable(launchthyme ${LAUNCHER_SRC})
target_link_libraries(launchthyme)
