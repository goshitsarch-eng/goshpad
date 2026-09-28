# Name of the application's binary.
name := 'goshpad'
# The unique ID of the application.
appid := 'com.goshapps.GoshPad'

# Path to root file system, which defaults to `/`.
rootdir := ''
# The prefix for the `/usr` directory.
prefix := '/usr'
builddir := 'build'

# Application's appstream metadata
appdata := appid + '.metainfo.xml'
# Application's desktop entry
desktop := appid + '.desktop'

# Install destinations
base-dir := absolute_path(clean(rootdir / prefix))
metainfo-dst := base-dir / 'share' / 'metainfo' / appdata
bin-dst := base-dir / 'bin' / name
desktop-dst := base-dir / 'share' / 'applications' / desktop
icon-svg-dst := base-dir / 'share' / 'icons' / 'hicolor' / 'scalable' / 'apps' / appid + '.svg'
license-dst := base-dir / 'share' / 'licenses' / appid

# Default recipe which runs `just build-release`
default: build-release

# Configure the CMake build. Extra CMake Modules may live in ~/.local.
configure profile='Debug':
    cmake -S . -B {{builddir}} -DCMAKE_BUILD_TYPE={{profile}} -DCMAKE_INSTALL_PREFIX={{prefix}} -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-$HOME/.local:/usr}"

# Remove the CMake build tree
clean:
    rm -rf {{builddir}}

# Compiles the debug profile
build-debug: (configure 'Debug')
    cmake --build {{builddir}} -j$(nproc)

# Compiles the release profile
build-release: (configure 'Release')
    cmake --build {{builddir}} -j$(nproc)

# Run unit tests and packaging checks
test: build-debug
    ctest --test-dir {{builddir}} --output-on-failure
    ./tests/packaging.sh

# Packaging identity and metadata checks
test-packaging:
    ./tests/packaging.sh

# Run the application
run *args: build-release
    {{builddir}}/bin/{{name}} {{args}}

# Installs files. Use `sudo just install` for the default /usr prefix.
install: build-release
    cmake --install {{builddir}} --prefix {{prefix}}

# Uninstalls installed files
uninstall:
    rm -f {{bin-dst}} {{desktop-dst}} {{metainfo-dst}} {{icon-svg-dst}} {{ license-dst / 'LICENSE' }} {{ license-dst / 'COPYRIGHT' }}
    rmdir {{license-dst}} 2>/dev/null || true
