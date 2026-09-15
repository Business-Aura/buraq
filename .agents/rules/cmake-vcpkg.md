# CMake & Vcpkg Dependency Guidelines

These rules apply to `CMakeLists.txt`, `vcpkg.json`, and build scripting across the repository.

## 1. Modern Target-Based CMake
- Always use target-based CMake commands scoped to the target name:
  - Use `target_include_directories(${PROJECT_NAME} PRIVATE ...)` instead of `include_directories()`.
  - Use `target_link_libraries(${PROJECT_NAME} PRIVATE ...)` instead of `link_libraries()`.
  - Use `target_compile_definitions(${PROJECT_NAME} PRIVATE ...)` instead of `add_definitions()`.
- Specify explicit library namespaces where available (e.g., `Qt6::Core`, `Qt6::Widgets`, `CURL::libcurl`, `unofficial::sqlite3::sqlite3`, `Boost::property_tree`).
- Keep target source lists organized and grouped logically (`ITOOLS_MAIN_SOURCES`, `ITOOLS_UI_SOURCES`, etc.).

## 2. Dependency Management via Vcpkg
- Dependencies must be declared declaratively in `vcpkg.json` (manifest mode).
- Do not check installed libraries or binaries into git.
- The standard target triplet is `x64-mingw-dynamic` with `--host-triplet=x64-mingw-dynamic`.
- When adding a new dependency:
  1. Add it to `dependencies` array in `vcpkg.json`.
  2. Use `find_package(PackageName REQUIRED)` in `app/CMakeLists.txt`.
  3. Link the target using `target_link_libraries`.
  4. If the dependency contains runtime DLLs needed on Windows, configure `POST_BUILD` copy commands as done with `libsqlite3.dll`.

## 3. Qt-Specific CMake Rules
- Ensure `CMAKE_AUTOMOC`, `CMAKE_AUTORCC`, and `CMAKE_AUTOUIC` remain `ON`.
- Call `qt_standard_project_setup()` early in the root or app CMake configuration.
- Use `qt_add_resources()` for QRC files rather than treating `.qrc` as standard source files.
- On Windows deployment, configure `windeployqt` as a `POST_BUILD` step to deploy Qt runtime libraries.

## 4. Managed (.NET) Interop Deployment
- The output directory for the C# bridge is `${CMAKE_SOURCE_DIR}/CSharpManaged/bin/Release/net9.0/win-x64/publish`.
- A CMake `POST_BUILD` custom command copies this folder into `$<TARGET_FILE_DIR:${PROJECT_NAME}>/PS.Bridge`.
- If modifying C# outputs or target frameworks, update the corresponding paths in `app/CMakeLists.txt` and `build.sh`.
