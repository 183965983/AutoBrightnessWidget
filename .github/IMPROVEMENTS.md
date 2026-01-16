# Future Improvements for CI/CD Pipeline

This document tracks potential improvements for the CI/CD workflows.

## Security Enhancements

### SHA Pinning for Third-Party Actions

**Current State**: Using version tags (e.g., `@v3`, `@v4`)
**Recommendation**: Pin to specific commit SHAs for supply chain security

**Actions to update**:
- `jurplel/install-qt-action@v3` → `jurplel/install-qt-action@<SHA>`
- `actions/upload-artifact@v4` → `actions/upload-artifact@<SHA>`
- `softprops/action-gh-release@v1` → `softprops/action-gh-release@<SHA>`

**How to find SHAs**:
```bash
# Example for install-qt-action v3
# Visit: https://github.com/jurplel/install-qt-action/releases/tag/v3.X.X
# Use the full commit SHA from that release
```

**Trade-off**: SHA pinning provides better security but requires manual updates when new versions are released.

## Code Quality Improvements

### Extract OpenCV Installation to Reusable Action

**Current State**: OpenCV installation logic is duplicated in all three workflows

**Recommendation**: Create a composite action in `.github/actions/install-opencv/action.yml`

**Benefits**:
- Single source of truth for OpenCV installation
- Easier to maintain and update
- Reduces duplication across workflows

**Example Structure**:
```yaml
# .github/actions/install-opencv/action.yml
name: 'Install OpenCV'
description: 'Installs OpenCV on Windows'
inputs:
  version:
    description: 'OpenCV version to install'
    required: false
    default: '4.8.1'
outputs:
  opencv-dir:
    description: 'OpenCV installation directory'
    value: ${{ steps.install.outputs.dir }}
runs:
  using: "composite"
  steps:
    - name: Install OpenCV
      id: install
      shell: pwsh
      run: |
        # Installation logic here
```

**Usage in workflows**:
```yaml
- name: Install OpenCV
  uses: ./.github/actions/install-opencv
  with:
    version: '4.8.1'
```

## Workflow Enhancements

### Add Caching for Dependencies

**Recommendation**: Cache OpenCV downloads to speed up builds

```yaml
- name: Cache OpenCV
  uses: actions/cache@v3
  with:
    path: ~/opencv-cache
    key: opencv-4.8.1-windows
```

### Add Build Matrix for Multiple Configurations

**Future Enhancement**: Test multiple Qt/OpenCV versions

```yaml
strategy:
  matrix:
    qt-version: [6.5.0, 6.6.0]
    opencv-version: [4.8.1, 4.9.0]
```

### Add Code Quality Checks

**Potential additions**:
- Static analysis (cppcheck, clang-tidy)
- Code formatting validation (clang-format)
- Documentation generation

### Separate Build and Deploy Jobs

**Current**: Release workflow builds and deploys in one job
**Enhancement**: Separate into build → package → deploy stages for better control

## Testing Improvements

### Add Unit Tests

**Current**: Test workflow has placeholder for tests
**Enhancement**: Add Qt Test framework tests

**Example**:
```cmake
# In CMakeLists.txt
enable_testing()
add_subdirectory(tests)

# tests/CMakeLists.txt
qt_add_test(TestAutoBrightness test_autobrightness.cpp)
```

### Add Integration Tests

**Enhancement**: Test the full application workflow
- Camera detection
- Brightness adjustment
- UI interactions

## Documentation Enhancements

### Add Workflow Status Page

Create a dashboard showing all workflow statuses

### Add Release Notes Template

Create `.github/release-template.md` for consistent release notes

## Priority

1. **High Priority**: Unit tests, caching
2. **Medium Priority**: SHA pinning, reusable actions
3. **Low Priority**: Build matrix, code quality checks

## Implementation Timeline

- **Phase 1** (Current): Basic CI/CD workflows ✓
- **Phase 2** (Next): Add unit tests, caching
- **Phase 3** (Future): Security hardening, code quality
- **Phase 4** (Optional): Advanced features, matrix builds
