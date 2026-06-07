[![Build all examples](https://github.com/azhel12/Zhele/actions/workflows/build_examples.yml/badge.svg)](https://github.com/azhel12/Zhele/actions/workflows/build_examples.yml)

# Zhele
Framework for MCU on C++ templates. Project based on "mcucpp" by Konstantin Chizhov.

Originally an STM32-only framework, Zhele is now multi-platform. The platform-specific code lives
under `include/zhele/platform/`, and the active backend is detected automatically from the linked
CMSIS device headers (or can be forced with `-DZHELE_PLATFORM_XX`). Supported platforms:
- **STM32** (F0 / F1 / F4 / G0 / L4) — the most complete backend
- **WCH CH32** — initial support (V0 line only for now)
- **NIIET K1921VG015** — initial GPIO support

The getting-started guide below uses STM32; CH32 and NIIET need their own CMSIS/SDK and toolchain
setup (see the `wch_cmake` / `niiet_cmake` helpers and the examples under `example/ch32` and
`example/niiet`).

Support: I created public group in [telegram](https://t.me/stm32_zhele), where I'll try help everyone with using the framework. Sorry, but I can answer only on russian:)

Also I write small [lessons](https://github.com/azhel12/ZheleLessons) about applying c++ templates (and my framework) for stm32.
# Getting started
I'm using following stack:
- **VSCode IDE**
- **CMake**
- **GNU Arm Embedded Toolchain Version** (latest is 15.2.rel1)
- **stm32-cmake** project for CMSIS/HAL access
- **openocd** and cmsis-dap/stlink for flash and debug

The framework targets **C++23** — the `zhele::zhele` CMake target requires `cxx_std_23`, and the
example below sets `CMAKE_CXX_STANDARD 23`. The practical floor is C++20: concepts are now used in
the core GPIO headers (`iopins.h`, `ioports.h`), not just in USB. C++17 is no longer enough. New
features from modern C++ (concepts and maybe coroutines, plus some C++23 features) keep being
adopted, so it's strongly recommended to use the latest toolchain.
1. Download Visual Studio Code from [official site](https://code.visualstudio.com/download) and install it.

2. Download new Arm Embedded Toolchain from [official site](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads). Choose latest (e.g. arm-gnu-toolchain-15.2.rel1-mingw-w64-i686-arm-none-eabi).
Download *exe* and install, or you can download zip and add **bin** folder location to system PATH.\
If you use *nix system download appropriate linux hosted cross toolchains.
There is no latest version in official repo (I tried on ubuntu), so unpack toolchain manually
and add to path. [Tutorial post](https://askubuntu.com/questions/1243252/how-to-install-arm-none-eabi-gdb-on-ubuntu-20-04-lts-focal-fossa).\
Check that compiler available from terminal.
![gcc](https://github.com/user-attachments/assets/8d162f35-19af-4798-93eb-7710a02f7956)

3. Download and install [Cmake](https://cmake.org/download/).
Check that cmake is available from terminal.
![cmake](https://github.com/azhel12/Zhele/assets/8615986/d6f8e1b3-3d85-45da-84ab-c773fc8a3b8e)

4. Install C++ extension pack and Cmake Tools in VSCode (it's not requires but useful). Or use any another extension for C++ development (such as clangd).
![Extensions](https://github.com/azhel12/Zhele/assets/8615986/29a2506d-275c-4cce-bdec-a377adad46cc)

5. Create project folder or [download](https://github.com/azhel12/Zhele-template) template.\
You can download [stm32-cmake](https://github.com/ObKo/stm32-cmake) manually. Read README.

6. Add **CMakeLists** to project root
```cmake
cmake_minimum_required(VERSION 3.16)
# Uncomment if stm32-cmake used manually (not as submodule)
#set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/stm32_gcc.cmake)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/stm32-cmake/cmake/stm32_gcc.cmake)

set (CMAKE_CXX_STANDARD 23)

project(zhele_template CXX C ASM)

# Populate CMSIS using stm32-cmake project
stm32_fetch_cmsis(F1)
find_package(CMSIS COMPONENTS STM32F1 REQUIRED)

# Add zhele
# You can download zhele and use add_subdirectory instead:
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/Zhele)
include(FetchContent)
FetchContent_Declare(Zhele
    GIT_REPOSITORY https://github.com/azhel12/Zhele.git
    GIT_TAG        master
)
FetchContent_MakeAvailable(Zhele)

# Project sources. Add other cpp if needed
set(PROJECT_SOURCES
    src/main.cpp
)

# Add executable (firmware) target
add_executable(zhele_template ${PROJECT_SOURCES})

# Link CMSIS and Zhele. Add other dependences here
target_link_libraries(zhele_template zhele::zhele CMSIS::STM32::F103C8 STM32::NoSys STM32::Nano)
# Recommended options for stm32 dev
target_compile_options(zhele_template PRIVATE -fno-exceptions $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti> -ffunction-sections -fdata-sections)
# Preprocessor options
target_compile_definitions(zhele_template PRIVATE FCPU=80000000)

# Print size
stm32_print_size_of_target(zhele_template)
# Generate bin file
stm32_generate_binary_file(zhele_template)
```

6. Write code in source file.
```c++
#include <zhele/iopins.h>

using namespace Zhele::IO;

// Connect LED on A4 for this example (or edit example code, it's simple).
int main()
{
    Pa4::Port::Enable();
    Pa4::SetConfiguration(Pa4::Configuration::Out);
    Pa4::SetDriverType(Pa4::DriverType::PushPull);
    Pa4::Clear();

    for (;;)
    {
    }
}
```

7. Press **Build** button in CMake extension.
First time you need choose toolchain.
![build](https://github.com/user-attachments/assets/ce1ea738-a10c-441d-99fa-035d540a05b9)

8. Profit! You can flash files i
![output](https://github.com/user-attachments/assets/5ef52eb8-79bb-4419-9280-7257b33dba3a)

![files](https://github.com/azhel12/Zhele/assets/8615986/ad4364ac-d2b8-4d44-983d-2ca029144e17)
