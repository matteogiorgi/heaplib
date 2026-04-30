# Build And Packaging

## C Build

Build the C static library:

```sh
make
```

Run the C tests:

```sh
make test
```

## Python Development Build

Build the Python extension in-place:

```sh
make python-build
```

Run the Python binding smoke tests:

```sh
make python-test
```

## Installing From Source

Inside a virtual environment:

```sh
python3 -m pip install /path/to/hpqlib
```

This command compiles the extension during installation. It does not require a
previous manual build, but it does require a compiler and Python development
headers on the user's machine.

## Building A Wheel

Build a wheel for the current platform and Python version:

```sh
make wheel
```

The generated file is written to `dist/` and can be installed with:

```sh
python3 -m pip install dist/hpqlib-*.whl
```

Wheel files are platform-specific for this project because the package contains
a compiled CPython extension.

## Precompiled Wheels

To avoid compiling on end-user machines, release wheel files for each supported
platform, architecture, and Python version. A Linux wheel built for CPython 3.11
does not cover macOS, Windows, or other Python versions.

The project is prepared for `cibuildwheel`, which can generate release wheels in
CI. A typical release process is:

```sh
python3 -m pip install cibuildwheel
python3 -m cibuildwheel --output-dir wheelhouse
```

The resulting `wheelhouse/` directory can be uploaded to a release page or to
PyPI.

## Release Workflow

The Makefile is the primary interface for release work. Build and verify a
local release with:

```sh
make release VERSION=0.1.0
```

This runs:

- `make clean`
- `make test`
- `make python-test`
- `make wheel`

It leaves the generated wheel under `dist/`.

To upload the generated wheel to a GitHub Release, use:

```sh
make release-upload VERSION=0.1.0
```

If the release already exists, the release helper uploads `dist/*.whl` with
`--clobber`. If it does not exist, the helper creates it. The upload mode
requires the GitHub CLI:

```sh
gh auth login
```

For repositories where `gh` cannot infer the remote, pass the repository
explicitly:

```sh
make release-upload VERSION=0.1.0 REPO=OWNER/REPO
```

Internally, the Make targets call `release.sh`. The script keeps the
argument parsing and GitHub Release logic out of the Makefile, while the
Makefile remains the recommended user-facing command surface.
