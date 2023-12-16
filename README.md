# Zhele
Framework for Stm32 MCU on C++ templates. Project based on "mcucpp" by Konstantin Chizhov.

Support: I created public group in [telegram](https://t.me/stm32_zhele), where I'll try help everyone with using the framework. Sorry, but I can answer only on russian:)

Also I write small [lessons](https://github.com/azhel12/ZheleLessons) about applying c++ templates (and my framework) for stm32.
# Getting started
I'm using following stack:
- **VSCode IDE**
- **CMake**
- **GNU Arm Embedded Toolchain Version** (latest is 13.2.1)
- **stm32-cmake** project for CMSIS/HAL access
- **openocd** and cmsis-dap/stlink for flash and debug

Basically c++17 is required, c++20 needed only for USB. But I'm planning to use more new features from modern C++ (such as concepts and maybe coroutines
and some features from c++23), so it's strongly recommended to use latest toolchain.
1. Download Visual Studio Code from [official site](https://code.visualstudio.com/download) and install it.

2. Download new Arm Embedded Toolchain from [official site](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads). Choose latest (arm-gnu-toolchain-13.2.rel1-mingw-w64-i686-arm-none-eabi).
Download *exe* and install, or you can download zip and add **bin** folder location to system PATH.\
If you use *nix system download appropriate linux hosted cross toolchains.
There is no latest version in official repo (I tried on ubuntu), so unpack toolchain manually
and add to path. [Tutorial post](https://askubuntu.com/questions/1243252/how-to-install-arm-none-eabi-gdb-on-ubuntu-20-04-lts-focal-fossa).\
Check that compiler available from terminal.
![gcc](https://github.com/azhel12/Zhele/assets/8615986/46a1962a-ba24-4c97-81ac-3d86d02c023f)

3. Download and install [Cmake](https://cmake.org/download/).
Check that cmake is available from terminal.
![cmake](https://github.com/azhel12/Zhele/assets/8615986/d6f8e1b3-3d85-45da-84ab-c773fc8a3b8e)

4. Install C++ extension pack and Cmake Tools in VSCode (it's not requires but useful).
![Extensions](https://github.com/azhel12/Zhele/assets/8615986/29a2506d-275c-4cce-bdec-a377adad46cc)

5. Create project folder or [download](https://github.com/azhel12/Zhele-template) template.\
You can download [stm32-cmake](https://github.com/ObKo/stm32-cmake) manually. Read README.

6. Add **CMakeLists** to project root
```cmake
cmake_minimum_required(VERSION 3.16)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/stm32_gcc.cmake)
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
    GIT_TAG        zhele_v0.1.1
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
target_compile_options(zhele_template PRIVATE -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections)

# Print size
stm32_print_size_of_target(zhele_template)
# Generate bin file
stm32_generate_binary_file(zhele_template)
```

6. Write code in source file.
```c++
// Define target cpu frequence.
#define F_CPU 8000000

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
![build](https://github.com/azhel12/Zhele/assets/8615986/a0859086-cf28-43a5-9f8a-3840fd425858)

8. Profit! You can flash files i
![output](https://github.com/azhel12/Zhele/assets/8615986/a75ba712-248b-4643-aa7a-5f8351a02b22)
![files](https://github.com/azhel12/Zhele/assets/8615986/ad4364ac-d2b8-4d44-983d-2ca029144e17)