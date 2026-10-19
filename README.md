# ESP32-C6 Matter Sensor

Student starter for DIOT Laboratory 3, **Phase 2: environmental sensors and multi-admin**. Continue the Phase 1 workflow from the Matter light project and follow the instructor's Phase 2 handout. Submit your own observations, not a copied answer sheet.

**Current hardware status: MOCK.** The BME680 backend generates simulated temperature, humidity, pressure, and gas-resistance samples. The OLED backend prints a local display to the serial monitor; it does not drive a physical SSD1306. Physical drivers and lab-board integration remain instructor preparation, not a student task. Do not describe mock values as physical measurements.

## What Is Provided and What You Complete

| Part | Starter status |
| --- | --- |
| Temperature | Complete C model entry and publication example |
| Pressure | Complete provided implementation |
| Gas resistance | Complete laboratory-specific example in ohms; not CO2 or IAQ |
| Humidity driver sample and local display | Provided; you do not implement the sensor driver |
| Humidity Matter model and publication | Removed for you to complete at `TODO(P2-1)` through `TODO(P2-5)` |
| Matter C++ backend and commissioning | Provided infrastructure; do not edit for this exercise |
| Second mobile administrator | An activity to perform and verify, not a missing C++ function |

The starter is intended to build before you complete the TODOs. Initially its C model contains temperature, pressure, and gas resistance, with no empty humidity slot. Humidity can still appear in the local mock display because the sample contains all fields; that does not mean a humidity endpoint exists.

The exercise keeps student edits in **C**. If an older handout refers to copying an endpoint constructor in `diot_matter_devices.c` or `.cpp`, use the **C model table in `diot_matter_bme680.c`** instead. The prepared C++ backend translates that table into the supported Matter endpoints. It already includes humidity support so you do not have to implement C++ cluster code.

## Requirements

- ESP32-C6 lab board, course ESP-IDF 6.0.1 installation, ESP-IDF VS Code extension, and the matching prepared ESP-Matter SDK.
- The course Ubuntu/WSL environment, with USB attached to WSL for flashing.
- SmartLife on your phone, the laboratory network, and another student's phone for the multi-admin activity. App behavior and supported measurements must be checked with the versions used in class.
- The instructor-prepared hardware/driver version if this session is run with physical BME/OLED devices. This repository currently supplies mocks only; do not change wiring or guess pins.

Cloning this project does not install the ESP-Matter SDK. The course `install-diot` installs it in `~/esp/esp-matter`, inside the Linux user's WSL home directory, not on the Windows Desktop.

## Open and Build in VS Code

Expected layout inside WSL:

```text
~/esp/
  esp-idf/
  esp-matter/
  esp32c6_matter_light/
  esp32c6_matter_sensor/
```

1. Open VS Code on Windows, press **F1**, and run **WSL: Connect to WSL**.
2. In the WSL-connected window, press **F1**, run **Git: Clone**, and enter `https://github.com/die-diot/esp32c6_matter_sensor.git`.
3. Select `~/esp` inside WSL as the parent folder, then click **Open** when cloning completes. Do not use the Windows Desktop or `/mnt/c` for this workflow.
4. Check the **bottom-left corner**: it must show **WSL: Ubuntu**, with `~/esp/esp32c6_matter_sensor` as the open project.
5. Select the course ESP-IDF installation and target **esp32c6**. Keep the instructor's board/partition settings.
6. Open **ESP-IDF: SDK Configuration Editor > Example Configuration > DIOT Matter Provisioning** and use your assigned station identity. Save. Do not edit generated `sdkconfig` manually or commit it.
7. Select **ESP-IDF: Build your project** to check the initial starter before editing. CMake uses the adjacent prepared SDK/Pigweed environment; do not copy a teacher's absolute paths or generated build.
8. Use the attached board's selected port to flash and monitor when the activity requires it. No machine-specific serial port is configured in this repository.

The repository includes minimal VS Code settings, a `compile_commands.json`-based IntelliSense configuration and the course debug attachment template. IntelliSense needs a build/configuration first; debugging also needs the relevant adapter and connection setup.

## Your Phase 2 Activities

| Slide | Activity | Source or evidence |
| --- | --- | --- |
| 15 | Trace sample -> callback -> C adapter -> Matter attribute, and the separate local display path. | `main/main.c`, `components/diot/`, `components/diot_matter/` |
| 16 | Identify node, endpoint, device type, cluster and attribute; find types/units in the references. | Course material, Matter Device Library and Application Cluster Specification |
| 17 | Build the baseline and locate the five humidity TODOs. | Files listed below |
| 18 | Adapt the temperature model and update API for humidity, using C only. | `TODO(P2-1)` through `TODO(P2-4)` |
| 19 | Publish humidity from the existing FSM callback; keep pressure/gas as provided. | `TODO(P2-5)` |
| 20 | Build, flash, commission and record actual values/app support. | Serial monitor, local display and SmartLife |
| 21 | Add another student's mobile administrator while retaining the first fabric. | Temporary commissioning code and both controllers |
| 22 | Explain the whole system, its local display and its limitations. | Your own observations and Moodle submission |

### Humidity TODO Map

| Marker | Location | What you complete |
| --- | --- | --- |
| `P2-1` | `components/diot_matter/diot_matter_bme680.c`, enum | Add the humidity index and keep the model count consistent |
| `P2-2` | Same file, `s_endpoints` | Add the standard humidity device-type/cluster/attribute entry, using temperature as the structural example |
| `P2-3` | `components/diot_matter/include/diot_matter_bme680.h` | Declare the C humidity publication API |
| `P2-4` | `components/diot_matter/diot_matter_bme680.c` | Implement validation, unit conversion, returned endpoint ID, correct typed writer and error propagation |
| `P2-5` | `main/main.c`, `publish_measurements()` | Pass the humidity field from the existing sample using the same error handling as the other measurements |

Look up the device type and required server cluster in **Matter Device Library**, then the attribute ID, signedness, units and range in **Application Cluster Specification** or the corresponding course references. The supplied C constants are interface definitions, not substitutes for explaining their meaning. Use the endpoint IDs returned by initialization, not assumed numbers from a diagram.

Do not modify the FSM schedule, create another polling task, implement a driver, change the SDK, or fill the exercise in `.cpp`. Keep the provided temperature, pressure and gas implementations. Complete the model and API before enabling the application call; keep every model entry valid.

## Architecture and Dependencies

```text
main/main.c                             Orchestrates the common FSM task in C
main/Kconfig.projbuild                  Station identity and button settings
components/fsm/                        Generic FSM engine
components/diot/                       Mock BME/OLED and managed button wrapper
  idf_component.yml                    External button package and version
components/diot_matter/
  diot_matter_bme680.c                  Student C model and unit adapters
  diot_matter_devices.cpp              Provided endpoint/server backend
  diot_matter.cpp                      Provided stack/commissioning/reset wrapper
  diot_commissioning.cpp               Laboratory/test identity providers
  include/                            Public headers and C interfaces
```

The BME FSM samples every 5 seconds; the local display refreshes every second. The application fires button, sensor and display FSMs from one FreeRTOS task every 20 ms. The driver sample includes humidity even before the Matter exercise is completed.

Component manifests declare external packages/versions; CMake registers sources and local/ESP-IDF requirements. Component Manager downloads packages to `managed_components/` and records resolved versions in `dependencies.lock`. Do not edit downloaded packages. The manifest of `main` is not the complete dependency list of the project.

## Verify Your Work

1. Rebuild after completing the humidity model/API/call. Check for missing declarations, mismatched model indices and errors returned by the wrapper.
2. In the boot log, match `Model: ep=... device=... cluster=... attr=...` lines to your model. Root and an optional network endpoint are infrastructure, not extra sensor readings.
3. Watch the local mock sample and successful attribute-update logs. An update attempt or a local display value alone is not proof of a successful Matter update or controller reception.
4. Commission in SmartLife using this device's current setup payload. Record which measurements are actually represented. Do not promise one app card per endpoint or invent an app reading for a hidden value.
5. The gas model uses laboratory test-vendor identifiers `0xFFF10001` and `0xFFF1FC00`, with a float attribute in ohms. It is not a standard CO2 sensor and SmartLife need not understand it. Pressure support also depends on the controller. If commissioning the provided model fails, report the error to the instructor rather than disguising a sensor as another type.

The instructor must validate the full supplied model with the laboratory app version before class. There is no command-line Matter-controller requirement in this activity, and no MQTT implementation to complete.

## Add a Second Administrator

1. Keep the first phone and its commissioned device unchanged.
2. In that controller, use its Matter sharing/add-another-ecosystem flow to open a commissioning window and obtain a temporary code.
3. Ask another student to choose **Add Matter device** on their own phone/account and use that temporary code before it expires. Do not reuse the original onboarding code for this step.
4. Verify that the original controller still works and that the second controller also sees supported measurements. Use redacted screenshots and the firmware's fabric/commissioning logs as instructed.
5. **Do not factory-reset between administrators.** Cloud/account sharing is not evidence of a second Matter fabric. If the available app only offers vendor-cloud sharing, use the instructor's validated alternate controller workflow.

## Reset and Switching from Phase 1

RST/EN reboots without normally clearing commissioning. A short BOOT press only logs an event in this sensor project. Holding BOOT for the configured duration and releasing it requests factory reset; it does not erase the firmware. Use the instructor's teardown procedure when switching from the light model to this sensor model. Do not erase flash automatically or reset merely because you rebuilt the same model.

## Optional Baseline Host Check

Before completing the exercise, the supplied host test checks only the three provided model entries and their adapters. It is not a hidden grader or a completed humidity solution. After adding a new endpoint, its baseline-only model expectations must be adapted if you continue using it.

```bash
gcc -std=c11 -Wall -Wextra -Werror \
    -I components/diot_matter/include -I "${IDF_PATH}/components/esp_common/include" \
    -I build/config tests/test_starter.c components/diot_matter/diot_matter_bme680.c \
    -lm -o /tmp/test_matter_sensor_starter
/tmp/test_matter_sensor_starter
```

This uses simulated writers. It does not test physical drivers, SDK servers in execution, radio delivery, SmartLife support or multi-admin. Perform the normal ESP-IDF build and the required on-board checks too.

## Before Leaving the Lab: Take Your Code Home

1. Select **File > Save All** in VS Code.
2. Press **Windows + E**, enter `\\wsl$\` in File Explorer, and open the course Ubuntu distribution, then **home > your Linux username > esp > esp32c6_matter_sensor**.
3. Show hidden items and copy the project contents to a Windows folder named `esp32c6_matter_sensor_backup`, excluding `build`, `managed_components` and `.git`. Keep source/config files and `.vscode`; your local `sdkconfig` may be kept in this private backup. Copy, do not move, and do not delete WSL files or copy the SDKs.
4. Right-click the Windows backup folder and select **Compress to ZIP file**, or **Send to > Compressed (zipped) folder** on Windows 10.
5. Open the ZIP, check your edited files, and keep it on your USB drive or personal university storage. Confirm the transfer completed. Keep your lab observations too, without publishing accounts or setup codes.

At home, install the course environment, restore the project to `~/esp/esp32c6_matter_sensor` inside WSL next to `esp-matter`, open it in WSL-connected VS Code, and build again. Do not reuse the laboratory PC's generated build directory.

## Troubleshooting

- **Humidity appears locally but not in Matter:** the mock sample/display already include it. Check the model, C function and application-call TODOs separately.
- **SDK/Pigweed missing:** finish the course SDK setup; do not alter cluster handlers to fix build tools.
- **No board in WSL:** attach the USB device and select its port using the lab procedure. A build does not require an attached board.
- **App does not display an attribute:** separate local model updates from app support. Do not treat a local log as a remote read.
- **Second phone only gets cloud sharing:** that is not the multi-admin objective. Do not factory-reset to make the second pairing replace the first.

This is laboratory firmware with test identity data, not a certified production sensor or a completed hardware-integration release. No teacher history, private answer key, generated build, station configuration or Wi-Fi credentials are included.