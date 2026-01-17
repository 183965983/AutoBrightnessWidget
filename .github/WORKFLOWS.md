# GitHub Actions CI/CD Pipeline

This repository uses GitHub Actions to automate building, testing, and releasing the AutoBrightnessWidget application on Windows 11.

## Workflows

### 1. Build Workflow (`.github/workflows/build.yml`)

**Triggers:**
- Push to `main`, `master`, or `develop` branches
- Pull requests to `main`, `master`, or `develop` branches
- Manual dispatch

**What it does:**

This unified workflow consolidates build, test, and packaging into a single pipeline with three jobs:

#### Job 1: Build
- Builds the application on Windows 2022 (Windows 11)
- Installs Qt 6.5.0
- Installs OpenCV 4.10.0
- Configures and builds with CMake and Visual Studio 2022 (with BUILD_TESTS=ON)
- Uploads build artifacts (executable and DLLs)

#### Job 2: Test
- Depends on the build job
- Runs comprehensive tests:
  - Camera functionality tests (using mocked camera with grayscale test images)
  - Windows brightness adjustment API tests (actual API calls with verification)
  - Qt UI functionality tests (with mocked camera)

**Test Coverage:**
- Brightness calculation algorithm with dark, medium, and bright test images
- Windows WMI brightness control interface (sets and verifies brightness levels)
- Qt widget creation and interaction
- UI button functionality

#### Job 3: Package
- Depends on the test job
- **Only runs on push to `main`, `master`, `develop`, or `dev` branches** (not on pull requests)
- Packages the application with all dependencies using `windeployqt`
- Creates a ZIP archive with the executable and all required DLLs
- Uploads the package as an artifact

**Purpose:**
- Serves as the primary CI/CD pipeline
- **Required for pull requests**: PRs must pass build and test jobs before merging
- Automatically packages releases when code is pushed to main branches
- Prevents broken code from entering main branches

**Artifacts:**
- Build artifacts: `build-artifacts` (from build job)
- Release package: `release-package` (from package job, only on push to main branches)

### 2. Release Workflow (`.github/workflows/release.yml`)

**Triggers:**
- Push of tags matching `v*.*.*` (e.g., `v0.1.0`, `v1.2.3`)
- Manual dispatch

**What it does:**
- Builds the application on Windows 2022 (Windows 11)
- Installs Qt 6.5.0
- Installs OpenCV 4.10.0
- Configures and builds with CMake and Visual Studio 2022
- Packages the application with all dependencies using `windeployqt`
- Creates a ZIP archive with the executable and all required DLLs
- Creates a GitHub Release with the packaged application

**Creating a Release:**

To create a new release, push a tag:

```bash
git tag v0.1.0
git push origin v0.1.0
```

The workflow will automatically:
1. Build the application
2. Package it with all dependencies
3. Create a GitHub Release with the ZIP file attached
4. Generate release notes automatically

## Environment Details

- **Operating System:** Windows Server 2022 (Windows 11 compatible)
- **Qt Version:** 6.5.0 (MSVC 2019 64-bit)
- **OpenCV Version:** 4.10.0
- **Compiler:** Visual Studio 2022 (MSVC)
- **Architecture:** x64

## Dependencies

The workflows automatically install all required dependencies:
- Qt 6.5.0 with qtbase and qttools modules
- OpenCV 4.8.1 for Windows
- Visual Studio 2022 build tools

## Customization

### Changing Qt Version

Edit the `version` field in the "Install Qt" step:

```yaml
- name: Install Qt
  uses: jurplel/install-qt-action@v3
  with:
    version: '6.6.0'  # Change this
    arch: 'win64_msvc2019_64'
    modules: 'qtbase qttools'
    cache: true
```

### Changing OpenCV Version

Edit the `opencvVersion` variable in the "Install OpenCV" step:

```yaml
- name: Install OpenCV
  run: |
    $opencvVersion = "4.9.0"  # Change this
    ...
```

### Adding Tests

Tests are now fully implemented using Qt Test framework. To add more tests:

1. Create new test files in the `tests/` directory
2. Include them in `tests/test_main.cpp`
3. Update `tests/CMakeLists.txt` if needed
4. Test locally:
   ```cmd
   mkdir build
   cd build
   cmake .. -G "Visual Studio 17 2022" -A x64 -DOpenCV_DIR="C:\path\to\opencv\build" -DBUILD_TESTS=ON
   cmake --build . --config Release
   Release\AutoBrightnessWidgetTests.exe
   ```

**Current Test Structure:**
- `test_brightness.cpp`: Camera mocking and brightness API tests
- `test_ui.cpp`: Qt UI widget tests
- `MockCamera`: Stub class for camera input using grayscale test images
- Test images: Located in `tests/test_images/` (dark, medium, bright)

## Troubleshooting

### Build Fails

- Check the workflow logs in the GitHub Actions tab
- Verify that all dependencies are correctly specified in `CMakeLists.txt`
- Ensure the OpenCV path in `CMakeLists.txt` is correctly configured

### Release Workflow Doesn't Trigger

- Ensure you're pushing tags, not just creating them locally
- Verify the tag matches the pattern `v*.*.*`
- Check repository permissions for creating releases

### Missing DLLs in Release Package

- Update the packaging step in `release.yml` to include additional DLLs
- Verify `windeployqt` is finding all Qt dependencies
- Add explicit `Copy-Item` commands for any missing dependencies

## Manual Workflow Dispatch

All workflows can be triggered manually:

1. Go to the "Actions" tab in GitHub
2. Select the workflow (Build, Test, or Release)
3. Click "Run workflow"
4. Select the branch and click "Run workflow"

## Status Badges

Add status badges to your README:

```markdown
[![Build](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/build.yml/badge.svg)](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/build.yml)
[![Release](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/release.yml/badge.svg)](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/release.yml)
```

## Pipeline Configuration Summary

The repository now uses a streamlined CI/CD setup:

1. **For Pull Requests**: The `build.yml` workflow runs build and test jobs. PRs must pass both jobs before merging.
2. **For Pushes to `main`, `master`, `develop`, or `dev` branches**: The `build.yml` workflow runs build, test, and package jobs.
3. **For Tag Pushes**: The `release.yml` workflow creates official GitHub releases with packaged binaries.

This configuration ensures:
- All PRs are validated before merging
- Continuous integration on main branches
- Automated release process
- Efficient use of CI/CD resources
