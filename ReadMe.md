# Maszyna Uart Driver Desk Controller

## Compile

Run cmake with setting paths:

```console
cmake .. -Dpicotool_DIR=<path to pico tool> -DPICO_SDK_PATH-<path to pico sdk>
```

If path to picotool or pico sdk will be not defined it will be download by cmake from git repository.

Download picotool: [download](https://github.com/raspberrypi/pico-sdk-tools/releases)

```console
cmake ../ -DCMAKE_BUILD_TYPE=Release -G Ninja -DCMAKE_INSTALL_PREFIX=../installDir -DPICOTOOL_FLAT_INSTALL=1
cmake --build . -t install
```

## Communication

```plantuml
@startuml

@enduml
```