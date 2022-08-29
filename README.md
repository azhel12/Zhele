# Zhele
Framework for Stm32 MCU on C++ templates. Project based on "mcucpp" by Konstantin Chizhov.
Support: I created public group in [telegram](https://t.me/stm32_zhele), where I'll try help everyone with using the framework. Sorry, but I can answer only on russian:)
# Getting started
I'm using VSCode IDE + Platformio + GNU Arm Embedded Toolchain Version 10.3-2021.10 (it supports features from c++20).
Basically c++17 is required, c++20 needed only for USB. But I'm planning to use more new features from modern C++ (such as concepts and maybe coroutines), so it's recommended to use latest toolchain.
1. Download Visual Studio Code from [official site](https://code.visualstudio.com/download) and install it.
2. Run VS code, Press _Ctrl+Shift+X_ (or press _Extension_ in left sidebar), input _PlatformIO_ in search and push install.
You can get some errors about python virtual environments. There is many solutions in Internet.
![VSCode Extensions](https://user-images.githubusercontent.com/8615986/117636383-6a7aca00-b189-11eb-915a-6ada899ad39b.png)
3. Open _PIO Home_, push _Platforms_, check _Emedded_ and print _stm32_ and press _ST STM32_ header.
![PlatformIO platforms](https://user-images.githubusercontent.com/8615986/117636466-82524e00-b189-11eb-80cb-5127f36f4157.png)
4. Press "Install" and wait.
![Install plaform](https://user-images.githubusercontent.com/8615986/117636624-ac0b7500-b189-11eb-8d93-2fd990c3a6fd.png)
5. Open _PIO Home_ _Home_, press _New Project_, input project name, board (examples for stm32f103) and **CMSIS** in _Framework_.
![New project](https://user-images.githubusercontent.com/8615986/117640495-c2b3cb00-b18d-11eb-8f3a-791a0e9aa443.PNG)
7. Download new Arm Embedded Toolchain from [official site](https://developer.arm.com/downloads/-/gnu-rm). Choose gcc-arm-none-eabi-10.3-2021.10-win32.zip
![Arm Embedded Toolchain download](https://user-images.githubusercontent.com/8615986/173829139-9afdb9fe-0ac7-42ad-8ba0-a0308000940e.png)
7. Unpack archive and copy witj replace folders _bin_, _share_, _arm-none-eabi_ and _lib_ to _C:\\Users\\%Username%\\.platformio\\packages\\toolchain-gccarmnoneeabi\\_ (With last Platofrmio update this directory has new name: _toolchain-gccarmnoneeabi@VERSION_). Warning! Do not delete files _.piopm_ and _package.json_.
![Copy new toolchain](https://user-images.githubusercontent.com/8615986/117638529-972fe100-b18b-11eb-9aff-e4deec2e6707.png)
9. Open _PIO Home_ _Home_, press _New Project_, input project name, board (examples for stm32f103) and **CMSIS** in _Framework_. Press _Finish_.
![New project](https://user-images.githubusercontent.com/8615986/117637568-964a7f80-b18a-11eb-8022-35007a8d913f.PNG)
10. Copy _Zhele/Zhele_ to _lib_ folder.
![Copy Zhele](https://user-images.githubusercontent.com/8615986/117638795-e544e480-b18b-11eb-8fdb-39096f67f3fe.PNG)
12. Copy some example in src folder or create new *.cpp file
![Add source file](https://user-images.githubusercontent.com/8615986/117638994-1d4c2780-b18c-11eb-9dad-e68d75127d1b.PNG)
14. Edit platformio.ini for target device. In my case (stm32f103c8t6) platformio.ini like this:

```[env:bluepill_f103c8]
platform = ststm32
board = bluepill_f103c8
framework = cmsis
debug_tool = stlink
upload_port = stlink
build_flags = 
    -D STM32F1
    -std=c++20
build_unflags =
    -std=c++11
```
15. Build project, upload it on your device.
![Building](https://user-images.githubusercontent.com/8615986/117639903-196cd500-b18d-11eb-861d-abe8046e91d8.png)
If VSCode doesnt build and shows editbox in top, restart VSCode and try again.
