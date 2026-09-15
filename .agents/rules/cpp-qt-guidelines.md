# C++20 and Qt 6 Development Guidelines

These rules apply to all C++ files (`*.cpp`, `*.h`, `*.hpp`) in the `app/`, `include/`, and `exts/` directories.

## 1. Language Standards & Compilers
- The project targets **C++20** (`CMAKE_CXX_STANDARD 20`, extensions disabled).
- Use standard library headers (`<memory>`, `<filesystem>`, `<chrono>`, `<concepts>`, `<ranges>`) over custom wrappers or outdated idioms.
- The project is compiled with GCC/MinGW-w64 on Windows and must remain compatible with standard Clang/MSVC.

## 2. Memory Management & Object Ownership
- **QObject Hierarchy**: When instantiating `QObject` or `QWidget` subclasses that belong to a UI hierarchy, pass a parent pointer (`new QWidget(parent)` or `new LineNumberAreaWidget(this)`). Qt's parent-child ownership will manage deletion.
- **Standalone Objects**: For objects that do not participate in the `QObject` parent-child tree, or for top-level managers, use `std::unique_ptr` or `std::shared_ptr`.
- **Destructors**: If a class owns `std::unique_ptr<IncompleteType>`, declare the destructor in the header and define it in the `.cpp` file where the complete type definition is visible.
- **Zero Raw `delete`**: Never use bare `delete` statements in application code.

## 3. Qt Signals and Slots
- **Connection Syntax**: Always use the modern pointer-to-member-function (PMF) syntax:
  ```cpp
  connect(sender, &Sender::signalName, receiver, &Receiver::slotName);
  ```
  Do NOT use the old string-based macros `SIGNAL()` and `SLOT()`.
- **Lambda Slots**: When using lambdas as slots, always supply a context `QObject*` as the third parameter to ensure safe disconnection when the receiver is destroyed:
  ```cpp
  connect(button, &QPushButton::clicked, this, [this]() {
      handleButtonClick();
  });
  ```
- **Thread Affinity**: Never access UI widgets directly across different threads. Use signals and slots across `QThread` boundaries or use `QMetaObject::invokeMethod` with `Qt::QueuedConnection`.

## 4. Modern C++ Best Practices
- **Const Correctness**: Mark all member methods `const` unless they mutate internal object state.
- **`[[nodiscard]]`**: Apply `[[nodiscard]]` to non-mutating getter methods, calculations, and factory methods.
- **`override`**: Always explicitly add `override` to virtual functions. Do not combine with `virtual`.
- **Explicit Constructors**: Single-argument constructors must be marked `explicit` to prevent unintended implicit conversions.
- **String Handling**: Use `QStringLiteral("...")` for static strings and UI identifiers to bypass runtime UTF-16 conversions.

## 5. UI Architecture & Styling
- **Frameless Window**: Custom frame borders, dragging, and resizing must adhere to `ui/frameless_window/FramelessWindow.h`.
- **Theme Support**: UI elements must be styleable via Qt Style Sheets (`dark_theme.qss` and `light_theme.qss`). Use standard object names (`setObjectName(...)`) or dynamic properties (`setProperty(...)`) to support QSS selectors.
- **Resource Management**: Icons, images, and embedded assets belong in `resources.qrc` and should be referenced with `":/..."` paths.
