# C# and Managed Bridge Guidelines

These rules apply to all code within `CSharpManaged/` (`Buraq.Bridge`, `Buraq.PowerShell`, and related .NET projects).

## 1. Target Framework & Language Features
- Target **.NET 9.0** (`net9.0`) with C# 13.
- Nullable reference types must be enabled (`<Nullable>enable</Nullable>`). Every reference type should explicitly indicate whether it can be null (`string?`).
- Treat compiler warnings as errors where possible (`<TreatWarningsAsErrors>true</TreatWarningsAsErrors>`).

## 2. Asynchronous Networking & TCP Bridge
- The bridge communicates with the C++ host over loopback (`127.0.0.1:12345`).
- Use non-blocking async/await for network I/O:
  - `await listener.AcceptTcpClientAsync(cancellationToken)`
  - `await reader.ReadLineAsync(cancellationToken)`
  - `await writer.WriteLineAsync(result)` followed by `await writer.FlushAsync()`
- Never call `.Result` or `.Wait()` on asynchronous tasks, as this causes thread pool starvation or deadlocks.
- Always support graceful cancellation via `CancellationToken`.

## 3. PowerShell Execution & Runspaces
- Encapsulate PowerShell execution within `PowerShellManager` (`Buraq.PowerShell.cs`).
- Handle script errors, exceptions, and non-terminating error streams cleanly without crashing the host process.
- Ensure PowerShell runspaces and pipelines are properly disposed after use or recycled safely across client sessions.

## 4. Resource Management & Lifetime
- Use `await using` or `using` declarations on all `IDisposable` and `IAsyncDisposable` types (`TcpClient`, `NetworkStream`, `StreamReader`, `StreamWriter`, `PowerShell`).
- The bridge process is launched by the C++ application as a child process (`ManagedProcess`). Ensure the process terminates cleanly when the parent process exits or when the socket disconnects.

## 5. Deployment & Packaging
- When building for deployment, use self-contained publishing targeted for Windows x64:
  ```bash
  dotnet publish Buraq.Bridge.csproj -c Release -r win-x64 --self-contained true
  ```
- All output binaries and assemblies must be copied into the C++ executable's `PS.Bridge` subdirectory as orchestrated by CMake.
