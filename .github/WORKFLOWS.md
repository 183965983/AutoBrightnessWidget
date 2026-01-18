# GitHub Actions CI/CD Pipeline

This repository uses a modern **Build → Package → Test in Clean Environment** CI/CD pipeline to ensure release package validity and quality.

## Workflow Architecture

### Philosophy

Traditional CI workflows often test in the build environment where all development tools are already installed. This can miss critical packaging issues that users will encounter. Our workflow validates the packaged application in a clean environment that simulates real user conditions.

### Key Principles

1. **Build once, use everywhere**: Artifacts are shared between jobs to avoid rebuilds
2. **Test the package, not the build**: Validate the actual deliverable users will download
3. **Clean environment testing**: Test without development tools to catch missing dependencies
4. **Parallel execution**: Independent jobs run concurrently for faster feedback

## Workflows

### 1. Build Workflow (`.github/workflows/build.yml`)

**Triggers:**
- Push to `dev`, `main`, `master`, or `develop` branches
- Pull requests to these branches
- Manual dispatch

**Pipeline Flow:**

```
┌─────────────┐
│    build    │  Compile application
└──────┬──────┘
       │
       ├─────────────────┬─────────────────┐
       │                 │                 │
       ▼                 ▼                 ▼
┌─────────────┐   ┌─────────────┐   
│   package   │   │  test-unit  │   (parallel)
└──────┬──────┘   └─────────────┘   
       │
       ▼
┌─────────────┐
│    test     │  Verify package in clean env
└─────────────┘
```

#### Job Details

**1. build** 
- **Purpose**: Compile the application
- **Environment**: Windows latest
- **Outputs**: build-artifacts (exe + dll files)
- **Key steps**:
  - Install Qt 6.5.0
  - Install OpenCV 4.10.0
  - Configure CMake with `-DBUILD_TESTS=ON`
  - Build Release configuration
  - Upload build artifacts

**2. package** (depends on: build)
- **Purpose**: Create release package with all dependencies
- **Environment**: Windows Server 2022
- **Inputs**: build-artifacts
- **Outputs**: release-package (complete zip)
- **Key steps**:
  - Download build artifacts
  - Install Qt (for windeployqt tool)
  - Install OpenCV (for runtime DLLs)
  - Run windeployqt to deploy Qt dependencies
  - Copy OpenCV DLLs
  - Create zip archive

**3. test** (depends on: package)
- **Purpose**: **Validate packaged application in pristine environment**
- **Environment**: Windows Server 2022 (clean environment)
- **Inputs**: release-package
- **Critical features**: 
  - ✅ Does NOT install development tools (Qt SDK, OpenCV SDK, CMake)
  - ✅ Only installs runtime dependencies (VC++ Runtime)
  - ✅ Simulates real user environment
- **Key steps**:
  - Download packaged zip file
  - Extract and verify package structure
  - Check for required DLLs (Qt, OpenCV)
  - Install Visual C++ Runtime
  - Run application smoke test
  - Verify application launches successfully

**4. test-unit** (depends on: build, parallel with package)
- **Purpose**: Run unit tests
- **Environment**: Windows Server 2022
- **Inputs**: build-artifacts
- **Key steps**:
  - Download build artifacts
  - Install runtime dependencies
  - Execute unit tests
  - Verify test results

**Test Coverage:**
- Brightness calculation algorithm with various test images
- Windows brightness adjustment API (actual API calls with verification)
- Qt UI widget functionality
- Camera input mocking and processing

**Artifacts:**
- `build-artifacts`: Executables and DLLs (retention: 7 days)
- `release-package`: Complete packaged application (retention: 7 days)

### 2. Release Workflow (`.github/workflows/release.yml`)

**Triggers:**
- Push of tags matching `v*.*.*` (e.g., `v1.0.0`)
- Manual dispatch

**Pipeline Flow:**

```
┌─────────────┐
│    build    │  Compile application
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   package   │  Create package
└──────┬──────┘
       │
       ▼
┌─────────────┐
│    test     │  Verify in clean env
└──────┬──────┘
       │
       ▼
┌─────────────┐
│   release   │  Create GitHub Release
└─────────────┘
```

#### Job Details

**1. build**
- Same as build.yml but with 90-day artifact retention

**2. package** (depends on: build)
- Same as build.yml but with 90-day artifact retention

**3. test** (depends on: package)
- Same as build.yml - validates package before release
- Ensures only verified packages are released

**4. release** (depends on: test)
- **Purpose**: Create GitHub Release
- **Environment**: Windows Server 2022
- **Permissions**: contents: write
- **Key steps**:
  - Download verified release-package
  - Create GitHub Release using softprops/action-gh-release
  - Upload zip file as release asset
  - Auto-generate release notes

**Creating a Release:**

```bash
git tag v1.0.0
git push origin v1.0.0
```

The workflow will automatically build, package, test, and release the application.

## Core Advantages

### 1. Ensures Package Validity ✅

**Problem with traditional approach:**
- Tests run in build environment with all dev tools installed
- Packages may be missing dependencies
- Users download packages that fail to run
- "Works on my machine" syndrome

**Our solution:**
- **test job runs in completely clean environment**
- Only minimal runtime dependencies installed
- Simulates actual user environment
- Guarantees package completeness

### 2. Avoids Redundant Builds ⚡

**Problem with traditional approach:**
- Each job rebuilds the application (build, test, package)
- Wastes CI time and resources
- Different jobs may produce different artifacts

**Our solution:**
- **Build only once**
- Share artifacts via GitHub Actions artifacts
- All subsequent jobs use same build output
- Significantly reduces CI time

### 3. Parallel Execution 🚀

- test-unit and package run concurrently
- Faster CI feedback
- Efficient resource utilization

### 4. Clear Responsibilities 📋

Each job has a single, well-defined purpose:
- **build**: Compile
- **package**: Package
- **test**: Validate
- **test-unit**: Unit tests
- **release**: Publish

## Artifact Flow

```
build
  └─> build-artifacts (*.exe, *.dll)
      ├─> package
      │   └─> release-package (*.zip)
      │       └─> test
      │           └─> release (release.yml only)
      └─> test-unit
```

## Environment Details

- **Operating System:** Windows Server 2022 (Windows 11 compatible)
- **Qt Version:** 6.5.0 (MSVC 2019 64-bit)
- **OpenCV Version:** 4.10.0
- **Compiler:** Visual Studio 2022 (MSVC)
- **Architecture:** x64

## Local Testing

Simulate the CI pipeline locally:

```powershell
# 1. Build
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DBUILD_TESTS=ON
cmake --build . --config Release

# 2. Run unit tests
.\Release\AutoBrightnessWidgetTests.exe

# 3. Package (manually)
mkdir ..\release-package
copy Release\*.exe ..\release-package\
windeployqt ..\release-package\AutoBrightnessWidget.exe --release
# Manually copy OpenCV DLLs

# 4. Test package
cd ..\release-package
.\AutoBrightnessWidget.exe
```

## Customization

### Changing Qt Version

Edit the `version` field in the "Install Qt" step:

```yaml
- name: Install Qt
  uses: jurplel/install-qt-action@v3
  with:
    version: '6.6.0'  # Change this
    arch: 'win64_msvc2019_64'
    cache: true
```

### Changing OpenCV Version

Edit the `opencvVersion` variable in the "Install OpenCV" step:

```yaml
- name: Install OpenCV
  run: |
    $opencvVersion = "4.11.0"  # Change this
    ...
```

### Adding Tests

Tests use Qt Test framework. To add more tests:

1. Create new test files in the `tests/` directory
2. Include them in `tests/test_main.cpp`
3. Update `tests/CMakeLists.txt` if needed
4. Test locally with `-DBUILD_TESTS=ON`

## Troubleshooting

### Build Fails

**OpenCV download/install failure:**
- Check OpenCV version availability
- Verify network connectivity

**CMake configuration failure:**
- Check CMakeLists.txt syntax
- Verify dependency paths

### Package Fails

**windeployqt not found:**
- Ensure Qt is correctly installed
- Verify Qt6_DIR environment variable

**OpenCV DLLs not found:**
- Check OpenCV installation path
- Verify VC version directory exists

### Test Fails

**Application won't launch:**
- Check package contains all required DLLs
- Use Dependency Walker to identify missing dependencies

**Smoke test timeout:**
- Increase timeout-minutes value
- Check for application blocking issues

### Release Workflow Doesn't Trigger

- Ensure you're pushing tags, not just creating them locally
- Verify tag matches pattern `v*.*.*`
- Check repository permissions for creating releases

## Manual Workflow Dispatch

All workflows can be triggered manually:

1. Go to the "Actions" tab in GitHub
2. Select the workflow (Build or Release)
3. Click "Run workflow"
4. Select the branch and click "Run workflow"

## Status Badges

Add status badges to your README:

```markdown
[![Build](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/build.yml/badge.svg)](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/build.yml)
[![Release](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/release.yml/badge.svg)](https://github.com/183965983/AutoBrightnessWidget/actions/workflows/release.yml)
```

## Best Practices

1. **Commit frequently**: Let CI catch issues early
2. **Small iterations**: One feature or fix per PR
3. **Monitor CI results**: Don't ignore warnings
4. **Test locally**: Run builds and tests before pushing
5. **Use semantic versioning**: Follow v1.0.0 format for tags

## Future Improvements

- [ ] Add Linux and macOS support
- [ ] Integrate code coverage reporting
- [ ] Add performance benchmarks
- [ ] Implement automatic version management
- [ ] Add more integration test scenarios
- [ ] Cache build dependencies for faster builds

## Related Documentation

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [CMake Documentation](https://cmake.org/documentation/)
- [Qt Documentation](https://doc.qt.io/)
- [OpenCV Documentation](https://docs.opencv.org/)
