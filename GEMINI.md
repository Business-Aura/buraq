# Buraq Developer & AI Assistant Guide

Welcome to the **Buraq** project. This guide provides developers and AI assistants with an architectural overview, development environment setup, build workflows, coding conventions, and contribution standards.

---

## 1. Project Overview & Architecture

Buraq is a cross-language desktop development environment and editor running primarily on Windows. It combines a high-performance modern C++20/Qt 6 user interface with a self-contained .NET 9.0 C# background service for PowerShell language and execution support.

```
┌──────────────────────────────────────────────────────────┐
│                   Buraq Desktop App                      │
│                                                          │
│  ┌───────────────────────────┐ ┌──────────────────────┐  │
│  │ Frameless Window Frame    │ │  Theme Manager (QSS) │  │
│  │ (Custom Title Bar/Borders)│ │  (Dark & Light Mode) │  │
│  └───────────────────────────┘ └──────────────────────┘  │
│  ┌───────────────────────────┐ ┌──────────────────────┐  │
│  │ Code Editor & Margins     │ │  Integrated Terminal │  │
│  │ (Highlighters, Runners)   │ │  (ANSI Parser/Widget)│  │
│  └─────────────┬─────────────┘ └──────────────────────┘  │
│                │                                         │
│  ┌─────────────┴─────────────┐ ┌──────────────────────┐  │
│  │ Plugin Manager & APIs     │ │  Version / Updater   │  │
│  │ (PluginInterface.h)       │ │  (Dialog & Downloader│  │
│  └───────────────────────────┘ └──────────────────────┘  │
└────────────────────────┬─────────────────────────────────┘
                         │ Loopback TCP (127.0.0.1:12345)
                         ▼
┌──────────────────────────────────────────────────────────┐
│              Managed Bridge (CSharpManaged)              │
│                                                          │
│  ┌───────────────────────────┐ ┌──────────────────────┐  │
│  │ Buraq.Bridge (TCP Server) │ │  Buraq.PowerShell    │  │
│  │ (Async Request Handling)  │ │  (Runspace Engine)   │  │
│  └───────────────────────────┘ └──────────────────────┘  │
└──────────────────────────────────────────────────────────┘
```

### Key Subsystems:
1. **Frontend / UI (`app/`)**:
   - Built with **C++20** and **Qt 6** (`Widgets`, `Gui`, `Core`, `Network`, `Sql`, `Xml`, `Svg`, `Qt6SvgWidgets`).
   - Frameless custom window frame with border snapping and custom title bar controls (`ui/frameless_window/`).
   - Tabbed/split editor component supporting line numbering, margins, and custom syntax highlighters for PowerShell and C++ (`ui/editor/`).
   - Integrated terminal widget with ANSI escape code rendering (`ui/terminal/`).
   - Dynamic theme engine switching between `dark_theme.qss` and `light_theme.qss` (`ui/Filters/ThemeManager/`).
   - Plugin architecture supporting modular extensions (`PluginManager.cpp`, `include/PluginInterface.h`).
   - Auto-updater client checking release manifests and triggering `buraq.iss` updates.

2. **Managed Bridge (`CSharpManaged/`)**:
   - **.NET 9.0 (C#)** self-contained background server (`Buraq.Bridge`).
   - Listens on `127.0.0.1:12345` to receive script execution requests and stream results back over TCP.
   - Hosts the PowerShell execution runtime via `PowerShellManager` (`Buraq.PowerShell`).
   - Published as a self-contained runtime into the `PS.Bridge` folder alongside the binary.

3. **Packaging & Deployment**:
   - Inno Setup script (`buraq.iss`) generates Windows installers (`setup-buraq-windows-x64-<version>.exe`).
   - `Qt6::windeployqt` automates runtime DLL collection.

---

## 2. Prerequisites & Environment

Ensure the following tools and environment variables are configured on the machine:

| Component | Required Version | Notes / Recommended Path |
| :--- | :--- | :--- |
| **C++ Toolchain** | GCC 13+ (MinGW-w64) | MSYS2 `mingw-w64-x86_64-toolchain` (`C:\msys64\mingw64\bin`) |
| **Qt 6** | Qt 6.8+ (MinGW 64-bit) | Must export `QT_PATH` (e.g. `C:/Qt/6.8.3/mingw_64`) |
| **.NET SDK** | .NET 9.0 SDK (9.0.300+) | `dotnet` on `PATH` |
| **CMake** | 3.27+ | Usually fetched via vcpkg or system PATH |
| **Ninja** | 1.12+ | Fetched via vcpkg or system PATH |
| **vcpkg** | Latest master | Must set `VCPKG_ROOT` (e.g. `C:\vcpkg` or local repo) |
| **Inno Setup** | 6.x | `C:\Program Files (x86)\Inno Setup 6\ISCC.exe` |

### Key Environment Variables
```powershell
$env:VCPKG_ROOT = "C:\vcpkg"
$env:QT_PATH = "C:\Qt\6.8.3\mingw_64"
$env:VCPKG_TARGET_TRIPLET = "x64-mingw-dynamic"
$env:VCPKG_DEFAULT_HOST_TRIPLET = "x64-mingw-dynamic"
```

---

## 3. Build & Run Procedures

### A. Building the Managed Bridge (.NET)
Before building the C++ app, compile and publish the .NET bridge:
```powershell
cd CSharpManaged
dotnet publish Buraq.Bridge.csproj -c Release -r win-x64 --self-contained true
```
Alternatively, in bash:
```bash
cd CSharpManaged && ./build.sh
```

### B. Building the C++ Application
From the repository root:
```bash
# Using the provided build script:
./build.sh

# Or configuring manually with CMake:
cmake -B build -S . -G "Ninja" `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET="x64-mingw-dynamic" `
  -DCMAKE_BUILD_TYPE=Release `
  -DCMAKE_PREFIX_PATH="$env:QT_PATH"

# Compiling:
cmake --build build --config Release
```

### C. Running the Application
The compiled executable and dependencies reside in `build/build/`:
```powershell
.\build\build\buraq.exe
```

---

## 4. Gitflow, Contribution & MR/PR Creation Rules

The repository strictly enforces a Gitflow branching model:

1. **Branch Roles**:
   - `main`: Production release branch. Contains tagged, deployable releases. **Direct commits and non-develop PRs are blocked by CI (`validate-merge.yml`).**
   - `develop`: Primary integration branch. All active development converges here.
   - `feature/*`, `fix/*`, `chore/*`: Feature and fix branches. Always branched from `develop` and merged back into `develop`.
2. **Pull Requests / Merge Requests**:
   - **Always target `develop`** for features, fixes, and chores.
   - PRs targeting `main` will automatically fail CI unless the head branch is `develop`.
   - **Automated MR/PR Creation**: When asked to create an MR/PR or upon completing a feature branch, automate the creation directly using Git Credential Manager and the GitHub REST API or the bundled skill script:
     ```powershell
     & .agents/skills/create-mr/scripts/create-mr.ps1 `
       -Title "<Conventional Commit Title>" `
       -Base "develop" `
       -Body "<Markdown Description>"
     ```
     Never ask the user to manually open the browser to create PRs. Always return the generated GitHub PR URL.
3. **Commit Messages**:
   - Use Conventional Commits format: `<type>(<scope>): <short description>`
   - Types: `feat`, `fix`, `chore`, `refactor`, `docs`, `style`, `test`, `ci`.

---

## 5. Coding Standards & Conventions

### C++ (C++20) & Qt 6
- **Standard**: C++20 with `-std=c++20` (strict standard compliance, no compiler extensions).
- **Memory Safety**:
  - Use `std::unique_ptr` for non-QObject resources or objects whose lifetime is strictly controlled by a single owner.
  - Rely on Qt's parent-child ownership tree for `QWidget` hierarchies.
  - Avoid bare `new` and never use bare `delete` without encapsulation.
- **Modern Qt Syntax**:
  - Always use pointer-to-member-function (PMF) syntax for signals and slots: `connect(sender, &Sender::signal, receiver, &Receiver::slot);`.
  - Use `QStringLiteral` for non-localized static strings to avoid unnecessary runtime memory allocation.
- **Const Correctness**:
  - Mark member methods `const` where state is not modified.
  - Annotate return values with `[[nodiscard]]` for getters and factory functions.
  - Explicitly mark overriding methods with `override`.

### C# (.NET 9.0)
- **Language Level**: C# 13 with Nullable Reference Types enabled (`<Nullable>enable</Nullable>`).
- **Async Programming**:
  - Use `async`/`await` for all I/O, socket, and stream operations.
  - Always handle `SocketException` and `OperationCanceledException` gracefully.
- **Resource Cleanup**:
  - Use `await using` or `using` declarations for all `IDisposable` and `IAsyncDisposable` types.

---

## 6. Directory Structure Reference

```
buraq/
├── .agents/
│   ├── rules/                  # Antigravity agent workspace rules
│   └── skills/                 # Antigravity workspace skills (e.g. create-mr)
├── .github/
│   └── workflows/              # GitHub Actions CI/CD pipelines
├── app/                        # Main C++ Qt application
│   ├── extensions/             # ExtensionManager and manifest parser
│   ├── clients/                # Network and version clients
│   ├── database/               # Local SQLite database integration
│   ├── icons/                  # Application icons (.ico, .png, .svg)
│   ├── res/                    # Win32 resources (.rc)
│   ├── ui/                     # UI components (editor, terminal, window, settings)
│   │   ├── app_ui/             # AppUi controller and layout orchestrator
│   │   ├── editor/             # Editor widget, highlighters, line numbers
│   │   ├── frameless_window/   # Custom titlebar and border handling
│   │   ├── settings/           # Settings dialog (with Extensions management tab)
│   │   ├── terminal/           # Terminal session and ANSI renderer
│   │   └── Filters/            # Theme manager and event filters
│   ├── utils/                  # Common utilities and configuration helpers
│   ├── dark_theme.qss          # Dark theme stylesheet
│   ├── light_theme.qss         # Light theme stylesheet
│   ├── resources.qrc           # Qt embedded resources
│   └── main.cpp                # Application entry point
├── extensions/                 # Built-in declarative extensions (powershell, cpp)
├── exts/                       # External plugins / modular extensions
├── include/                    # Public headers (PluginInterface.h, version.h)
├── .clang-format               # C++ code formatting configuration
├── .clang-tidy                 # C++ static analysis and lint rules
├── .editorconfig               # Universal editor formatting rules
├── build.sh                    # Root build script
├── buraq.iss                   # Inno Setup installer script
├── CMakeLists.txt              # Root CMake configuration
└── vcpkg.json                  # C++ dependency declarations
```
