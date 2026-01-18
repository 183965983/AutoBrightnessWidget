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
- **Outputs**: app-screenshot (PNG image)
- **Critical features**: 
  - ✅ Does NOT install development tools (Qt SDK, OpenCV SDK, CMake)
  - ✅ Only installs runtime dependencies (VC++ Runtime)
  - ✅ Simulates real user environment
  - ✅ Captures application screenshot for visual verification
  - ✅ Posts screenshot info as PR comment (on pull requests)
- **Key steps**:
  - Download packaged zip file
  - Extract and verify package structure
  - Check for required DLLs (Qt, OpenCV)
  - Install Visual C++ Runtime
  - Run application smoke test
  - Verify application launches successfully
  - **Capture screenshot of running application**
  - **Upload screenshot as artifact**
  - **Comment on PR with screenshot download link (PR only)**

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
- `app-screenshot`: Screenshot of running application UI (retention: 7 days)
  - Captured during test job
  - Available for download from workflow run
  - Automatically linked in PR comments

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
      │           ├─> app-screenshot (*.png) [NEW]
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

## Screenshot Feature

### Overview

The test job automatically captures a screenshot of the running application to provide visual verification of the UI. This feature helps reviewers quickly verify that the application launches correctly and displays the expected interface.

### How It Works

1. **Capture**: After the application launches successfully (with a 5-second initialization delay), the test script:
   - Waits an additional 2 seconds for the window to fully render
   - Uses .NET libraries (System.Windows.Forms and System.Drawing) to capture the entire primary screen
   - Saves the screenshot as `screenshots/app-screenshot.png`

2. **Upload to Image Hosting**: The screenshot is automatically uploaded to Imgur (free image hosting service):
   - Converts image to base64 format
   - Uses Imgur's anonymous upload API
   - Returns a publicly accessible URL

3. **Upload as Artifact**: The screenshot is also uploaded as a GitHub Actions artifact named `app-screenshot` with a 7-day retention period (backup option).

4. **PR Comment**: When the workflow is triggered by a pull request:
   - A comment is automatically posted to the PR
   - **The screenshot is embedded directly in the comment** (when Imgur upload succeeds)
   - Falls back to artifact download link if upload fails
   - Includes a link to the workflow run for full details

### Implementation Details

**PowerShell Screenshot Code:**
```powershell
# Load .NET assemblies
Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# Capture screen
$bounds = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds
$bitmap = New-Object System.Drawing.Bitmap $bounds.Width, $bounds.Height
$graphics = [System.Drawing.Graphics]::FromImage($bitmap)
$graphics.CopyFromScreen($bounds.Location, [System.Drawing.Point]::Empty, $bounds.Size)

# Save and cleanup
$bitmap.Save("screenshots/app-screenshot.png", [System.Drawing.Imaging.ImageFormat]::Png)
$graphics.Dispose()
$bitmap.Dispose()
```

**GitHub Actions Configuration:**
```yaml
# Upload screenshot as artifact (backup)
- name: Upload screenshot
  if: always()
  uses: actions/upload-artifact@v4
  with:
    name: app-screenshot
    path: screenshots/app-screenshot.png
    retention-days: 7

# Upload to Imgur for embedding in PR comments
- name: Upload screenshot to Imgur
  if: github.event_name == 'pull_request'
  id: imgur_step
  continue-on-error: true
  run: |
    # Convert to base64 and upload to Imgur
    $bytes = [System.IO.File]::ReadAllBytes($screenshotPath)
    $base64 = [System.Convert]::ToBase64String($bytes)
    $headers = @{ "Authorization" = "Client-ID 546c25a59c58ad7" }
    $response = Invoke-RestMethod -Uri "https://api.imgur.com/3/image" ...
  shell: pwsh

# Post comment with embedded screenshot
- name: Comment screenshot on PR
  if: github.event_name == 'pull_request'
  uses: actions/github-script@v7
  with:
    script: |
      // Embed screenshot if Imgur upload succeeded
      if (imgurSuccess && imgurUrl) {
        comment = `![Screenshot](${imgurUrl})`;
      }
```

### Required Permissions

The test job requires `pull-requests: write` permission to post comments on PRs:

```yaml
test:
  permissions:
    contents: read
    actions: read
    pull-requests: write
```

### Viewing Screenshots

**For Pull Requests:**
- **The screenshot is displayed directly in the PR comment** 🎉
- No need to download - just scroll through the PR comments
- If Imgur upload fails, a download link to the artifact is provided as fallback

**For Direct Branch Pushes:**
- Go to the Actions tab
- Select the workflow run
- Download the `app-screenshot` artifact from the Artifacts section

### PR Comment Example

When the workflow runs on a PR, you'll see a comment like this:

```markdown
## 🖼️ Application Screenshot

✅ Application launched successfully!

![Screenshot](https://i.imgur.com/xxxxx.png)

**Test Run:** [Workflow #12345](workflow-url)

---
*Screenshot captured during automated testing*
```

The screenshot is embedded inline, making it easy to visually verify the UI without downloading files.

### Benefits

- **Visual Verification**: Screenshot is **embedded directly in PR comments** for instant viewing
- **No Download Required**: Review UI changes without leaving the PR page
- **Regression Detection**: Compare screenshots across commits to spot UI changes
- **Documentation**: Screenshots serve as visual documentation of the application state
- **Debugging**: Helps diagnose UI issues in CI environment
- **Reliable**: Falls back to artifact download if image hosting fails

### Technical Details

**Image Hosting:**
- Uses Imgur's free anonymous upload API
- No account or authentication required
- Images are publicly accessible via direct links
- Perfect for embedding in GitHub comments

**Fallback Mechanism:**
- If Imgur upload fails (`continue-on-error: true`), the workflow continues
- PR comment will include an artifact download link instead
- Ensures screenshots are always available in some form

### Limitations

- Screenshot captures the entire primary screen, not just the application window
- Only available on Windows runners (uses Windows-specific .NET APIs)
- Requires GUI application to be visible on screen
- May capture background applications if they overlap
- **Imgur uploads are public** - anyone with the link can view the screenshot
- Imgur has rate limits for anonymous uploads (though generous for typical usage)

## Related Documentation

- [GitHub Actions Documentation](https://docs.github.com/en/actions)
- [CMake Documentation](https://cmake.org/documentation/)
- [Qt Documentation](https://doc.qt.io/)
- [OpenCV Documentation](https://docs.opencv.org/)
