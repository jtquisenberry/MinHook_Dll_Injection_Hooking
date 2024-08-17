# Windows API Hooking and DLL Injection with MinHook

This solution provides examples of Windows API hooking and DLL injection using MinHook, "The Minimalistic x86/x64 API Hooking Library for Windows". MinHook is a simpler injection framework than Detours or EasyHook.

The injector application supports injection of a DLL into a new process when starting the process, and injection of a DLL into a running process.

The solution consists of these projects:

* Injector
* Payload1
* Target
* PrintingDll
* MonitorDll

# Usage

``` shell
> "Injector.exe --pid <PID> | --exe <exe file> --dll <dll file>"
```

* `--pid`: Indicates that the next argument is a PID.
* `<PID>`: The process identifier of a running process into which a DLL will be injected.
* `--exe`: Indicates that the next argument specifies an EXE file.
* `<exe file>` The fully-qualified filename of an executable file that will be started and into which a DLL will be injected.
* `--dll` Indicates that the next argument specifies a DLL file.
* `<dll file>` The fully-qualified filename of a DLL to inject into a target process.

If both a PID and an executable are specified, the PID is treated as the target, and the EXE is ignored.


## Examples

__Target1__

``` shell
> D:\Development\git\MinHook_Dll_Injection_Hooking\x64\Debug\Injector.exe --pid -1 --exe "D:\Development\git\Dll_Injector\x64\Debug\Target1.exe" --dll "D:\Development\git\Dll_Injector\x64\Debug\Payload1.dll"
```

Process Explorer shows `Payload1.dll` has been injected into `Target1.exe`.


![Payload DLL in Target](images/minhook_dll_in_target.png?raw=true "Payload DLL in Target")


__WordPad__

``` shell
> Injector.exe --pid 645708 --dll "D:\Development\git\MinHook_Dll_Injection_Hooking\x64\Debug\Payload1.dll"
```

Process Explorer shows `Payload1.dll` has been injected into WordPad.


![Payload DLL in WordPad](images/minhook_dll_in_wordpad.png?raw=true "Payload DLL in WordPad")



# Injector

The injector injects a 64-bit DLL into a 64-bit target application.

The injector supports two types of targets.
1. A new process, when starting the process.
1. A running process.


## New Process

The injector performs these operations:
1. Launches an executable and creates a process in `CREATE_SUSPENDED` mode using `CreateProcess`.
1. Allocates memory in the remote (target) process to store the filename of the DLL, using `VirtualAllocEx`.
1. Writes the filename of the DLL to the remote process using `WriteProcessMemory`.
1. Creates a new thread in the executable using `CreateRemoteThread`. The function specifies that the thread should start at the address of the `LoadLibraryW` function, which takes the DLL filename as an argument.
1. Frees the memory allocated to the DLL filename.
1. Resumes the remote process by resuming its main thread using `ResumeThread`.


## Running Process

The injector performs these operations:
1. Creates a handle to a running process specified by a PID using `OpenProcess`.
1. Allocates memory in the remote (target) process to store the filename of the DLL, using `VirtualAllocEx`.
1. Writes the filename of the DLL to the remote process using `WriteProcessMemory`.
1. Creates a new thread in the executable using `CreateRemoteThread`. The function specifies that the thread should start at the address of the `LoadLibraryW` function, which takes the DLL filename as an argument.
1. Frees the memory allocated to the DLL filename.


# Target

The `Target1` application is a sample executable meant to receive a DLL. Running the application does the following:
* Reports the username of the current user, using `GetUserNameW`. 
* Reports the printers installed on the local machine, using `EnumPrintersW`.
* Allows the user to perform either action repeatedly, on demand.

``` 
>Target1.exe
Username: myuser


BEGIN LIST PRINTERS
Microsoft XPS Document Writer
Microsoft Print to PDF
Bullzip PDF Printer
END LIST PRINTERS


Type 'U' for username, 'P' for printers, or 'Q' to quit.
```


# Payload

The `Payload1` DLL is a sample DLL meant to be injected into `Target1.exe` or another executable. It hooks functions `GetUserNameW` and `EnumPrintersW`. The proxy functions write messages to a log file. 

The log file is stored in `%USERPROFILE%\payload1.txt`


```
[DLL] DLL_PROCESS_ATTACH
[DLL] Installing hooks.
[DLL] Hook #:  0
[DLL] Installed hook in:  LoadLibraryW
[DLL] Hook #:  1
[DLL] Installed hook in:  GetUserNameW
[DLL] Hook #:  2
[DLL] Installed hook in:  EnumPrintersW
[DLL] Hooks installed, Resuming main thread.
[DLL] DLL_THREAD_ATTACH
[DLL] DLL_THREAD_DETACH
[HOOK] Intercepted call to GetUserNameW
[HOOK] GetUserNameW: myuser
[HOOK] Intercepted call to EnumPrinters
[HOOK] Name: (null)
[DLL] DLL_PROCESS_DETACH
```


# DLLs

The additional DLL projects provide minimally functional implementations of hooking use cases. 


## MonitorDll.dll

This DLL demonstrates hooking APIs related to starting a program and loading a DLL. 
* `CreateProcessInternalW`
* `LoadLibraryExW`
* `LoadLibraryW`
* `LoadLibraryA`
* `GetProcAddress`

Information about the DLL calls is logged to `<EXE path>\<EXE filename>_log.txt`.


## PrintingDll.dll

This DLL demonstrates hooking the Windows printing subsystem. It hooks `ExtTextOutW` and logs information about the printed text content and position. The log file is stored in `%USERPROFILE%\PrintingDll.txt`.

__Example of a log message.__

```
---------------------------------------- START
ProxyExtTextOutW
hdc: 000000000E016495
lpString: ABCD
x: 119
y: 0
lprect address: 0000006F40FDDCD0
lprect->left: 0
lprect->right: 816
lprect->top: 0
lprect->bottom: 34
---------------------------------------- END
```


# Credits

## Injector.exe

Logic for starting an EXE and injecting a DLL into it is based on the following:
* "Windows API Hooking and DLL Injection", https://dzone.com/articles/windows-api-hooking-and-dll-injection
* "win-api-monitor", https://github.com/jayo78/win-api-monitor 

## MinHook

DLL projects use _MinHook - The Minimalistic API Hooking Library for x64/x86_, https://github.com/TsudaKageyu/minhook 

## MonitorDll

"win-api-monitor", https://github.com/jayo78/win-api-monitor 


