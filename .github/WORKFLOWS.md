# GitHub Actions CI/CD Pipeline

This repository uses GitHub Actions to automate building, testing, and releasing the AutoBrightnessWidget application on Windows 11.

## Workflows

### 1. Build Workflow (`.github/workflows/build.yml`)

**Triggers:**
- Push to `main`, `master`, or `develop` branches
- Pull requests to `main`, `master`, or `develop` branches
- Manual dispatch

**What it does:**
- Builds the application on Windows 2022 (Windows 11)
- Installs Qt 6.5.0
- Installs OpenCV 4.8.1
- Configures and builds with CMake and Visual Studio 2022
- Uploads build artifacts (executable and DLLs)

**Artifacts:**
- Build artifacts are available for download after each successful build
- Artifact name: `AutoBrightnessWidget-Windows-x64`

### 2. Test Workflow (`.github/workflows/test.yml`)

**Triggers:**
- Push to `main`, `master`, or `develop` branches
- Pull requests to `main`, `master`, or `develop` branches
- Manual dispatch

**What it does:**
- Builds the application on Windows 2022 (Windows 11)
- Installs Qt 6.5.0
- Installs OpenCV 4.8.1
- Configures and builds with CMake and Visual Studio 2022
- Runs tests (currently placeholder - extend as needed)

**Note:** This workflow currently validates that the build completes successfully. Add unit tests to the project and update the "Run tests" step to execute them.

### 3. Release Workflow (`.github/workflows/release.yml`)

**Triggers:**
- Push of tags matching `v*.*.*` (e.g., `v0.1.0`, `v1.2.3`)
- Manual dispatch

**What it does:**
- Builds the application on Windows 2022 (Windows 11)
- Installs Qt 6.5.0
- Installs OpenCV 4.8.1
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
- **OpenCV Version:** 4.8.1
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

To add actual tests:

1. Create test files in your project (e.g., using Qt Test framework)
2. Update `CMakeLists.txt` to include test targets
3. Update the "Run tests" step in `test.yml`:

```yaml
- name: Run tests
  run: |
    cd build
    ctest -C Release --output-on-failure
  shell: pwsh
```

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
[![Test](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/test.yml/badge.svg)](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/test.yml)
[![Release](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/release.yml/badge.svg)](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/release.yml)
```
