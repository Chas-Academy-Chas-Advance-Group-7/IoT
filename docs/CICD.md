# CI/CD Pipeline Documentation

## Overview

This project uses GitHub Actions for continuous integration and deployment with three main workflows and two custom actions. The pipelines are designed to build and test IoT firmware for ESP32 broker and Arduino Uno R4 WiFi devices.

## Workflows

### 1. Build Hardware Firmware (`build-lint.yml`)

**File:** `.github/workflows/build-lint.yml`

**Purpose:** Builds, lints, and releases firmware for both hardware platforms.

**Triggers:**

- Push to branches: `main`, `develop`, `feature/*`, `fix/*`
- Tags matching pattern: `v*.*.*`
- Pull requests to: `main`, `develop`
- Manual dispatch

#### Jobs

##### build-push (Push Events)

- **Runner:** Ubuntu Latest
- **Permissions:** `contents: write`
- **Execution:** Only on push events

**Steps:**

1. Checkout repository (`actions/checkout@v5`)
2. Setup PlatformIO environment (custom action)
3. Build and lint firmware (custom action)
4. **For tagged releases only:**
   - Create firmware ZIP package
   - Upload ZIP to GitHub Release

**Release Process:**

- Detects version tags (`refs/tags/v*.*.*`)
- Creates ZIP containing:
  - `esp32-broker.bin` (from `src_broker_esp32/.pio/build/esp32_broker/firmware.bin`)
  - `uno-r4-wifi.bin` (from `src_package_arduino/.pio/build/uno_r4_wifi/firmware.bin`)
- Uploads to GitHub Release using `softprops/action-gh-release@v1`

##### build-pr (Pull Request Events)

- **Runner:** Ubuntu Latest
- **Permissions:** `contents: write`
- **Execution:** Only on pull request events

**Steps:**

1. Checkout repository
2. Setup PlatformIO environment
3. Build and lint firmware
4. Upload artifacts:
   - ESP32 broker firmware as `ESP32-broker`
   - Arduino firmware as `uno-r4-wifi`

### 2. Clang-Format Lint (`formatting.yml`)

**File:** `.github/workflows/formatting.yml`

**Purpose:** Enforces code formatting standards and automatically applies fixes.

**Triggers:**

- All push events
- All pull request events
- Manual dispatch

#### Job: build

- **Runner:** Ubuntu Latest
- **Permissions:** `contents: write`

**Steps:**

1. Checkout repository
2. Run clang-format lint using `DoozyX/clang-format-lint-action@v0.20`
   - **Source:** Entire repository (`.`)
   - **Excludes:** `./src_broker_esp32/lib`, `./src_package_arduino/lib`, `./.github`
   - **Extensions:** `cpp,h,hpp,c,ino`
   - **Version:** Clang-Format 20
   - **Mode:** In-place formatting (`inplace: True`)
3. Auto-commit changes using `EndBug/add-and-commit@v9`
   - **Author:** `chas-robot <root@chasacademy.se>`
   - **Message:** "Committing clang-format changes"

### 3. Unit Tests (`unit-tests.yml`)

**File:** `.github/workflows/unit-tests.yml`

**Purpose:** Runs native unit tests for the Arduino sensor package.

**Triggers:**

- Push to any branch (`"*"`)
- Pull requests to any branch (`"*"`)
- Manual dispatch

#### Job: runUnitTests

- **Runner:** Ubuntu Latest

**Steps:**

1. Checkout repository
2. Setup PlatformIO environment
3. Run native unit tests:
   ```bash
   cd src_package_arduino
   pio test -e native
   ```

## Custom Actions

### Setup PlatformIO Environment

**File:** `.github/workflows/setup-platformio/action.yml`

**Purpose:** Configures PlatformIO build environment with intelligent caching.

**Steps:**

1. **Cache pip dependencies**
   - Path: `~/.cache/pip`
   - Key: `${{ runner.os }}-pip-${{ hashFiles('**/requirements.txt') }}`
2. **Cache PlatformIO**
   - Path: `~/.platformio`
   - Key: `${{ runner.os }}-${{ hashFiles('**/platformio.ini') }}`
3. **Setup Python**
   - Version: `3.x` (latest)
   - Uses: `actions/setup-python@v5`
4. **Install PlatformIO**
   ```bash
   python -m pip install --upgrade pip
   pip install --upgrade platformio
   ```

### Build and Lint Firmware

**File:** `.github/workflows/build-lint/action.yml`

**Purpose:** Performs static analysis and builds firmware for both hardware platforms.

**Steps:**

1. **Lint ESP32 broker source**

   ```bash
   cd src_broker_esp32
   platformio check -e esp32_broker
   ```

2. **Lint Arduino package source**

   ```bash
   cd src_package_arduino
   platformio check -e uno_r4_wifi
   ```

3. **Build ESP32 broker firmware**

   ```bash
   cd src_broker_esp32
   platformio run -e esp32_broker
   ```

4. **Build Arduino package firmware**
   ```bash
   cd src_package_arduino
   platformio run -e uno_r4_wifi
   ```

## Hardware Platforms

### ESP32 Broker

- **Source Directory:** `src_broker_esp32/`
- **Environment:** `esp32_broker`
- **Output:** `src_broker_esp32/.pio/build/esp32_broker/firmware.bin`
- **Role:** Central broker for IoT communication

### Arduino Uno R4 WiFi Sensor Package

- **Source Directory:** `src_package_arduino/`
- **Environment:** `uno_r4_wifi`
- **Output:** `src_package_arduino/.pio/build/uno_r4_wifi/firmware.bin`
- **Role:** Sensor data collection and transmission
- **Testing:** Native unit tests available

## Code Quality Assurance

### Static Analysis

- **Tool:** PlatformIO Check
- **Scope:** Both ESP32 and Arduino codebases
- **Integration:** Automated in build pipeline

### Code Formatting

- **Tool:** Clang-Format v20
- **Enforcement:** Automatic formatting applied
- **Coverage:** All C/C++/Arduino source files
- **Exclusions:** Library directories

### Testing

- **Framework:** PlatformIO Test
- **Scope:** Arduino sensor package only
- **Environment:** Native (x86) for unit tests
- **Location:** `src_package_arduino/test/`

## Caching Strategy

The pipelines implement intelligent caching to reduce build times:

1. **pip Cache:** Python package dependencies
2. **PlatformIO Cache:** Core installation and library packages
3. **Cache Keys:** Based on configuration file hashes for invalidation

## Release Process

### Automatic Releases

1. **Trigger:** Git tag matching `v*.*.*` pattern
2. **Process:**
   - Build both firmware binaries
   - Package into ZIP file named `firmware-{tag}.zip`
   - Create GitHub Release with ZIP attachment
3. **Artifacts:** Production-ready firmware binaries

### Development Builds

1. **Trigger:** Pull requests
2. **Process:**
   - Build and validate firmware
   - Upload as GitHub Actions artifacts
3. **Purpose:** Testing and validation before merge

## Monitoring and Maintenance

### Workflow Status

- All workflows provide build status badges
- Failed builds block pull request merges
- Email notifications for failed builds (GitHub settings)

### Dependency Updates

- PlatformIO auto-updates during builds
- GitHub Actions versions should be updated periodically
- Python dependencies cached and updated as needed

