# OpenVX API Documentation

This repository contains the source for the [OpenVX](https://www.khronos.org/openvx/) API specification, header files, and extension documents maintained by the [Khronos Group](https://www.khronos.org/).

## Repository Structure

```
api-docs/
├── include/VX/                  # OpenVX public header files
│   ├── vx.h                     # Core OpenVX header
│   ├── vx_api.h                 # API function declarations
│   ├── vx_kernels.h             # Kernel enumeration
│   ├── vx_nodes.h               # Graph node functions
│   ├── vx_types.h               # Type definitions
│   ├── vxu.h                    # Immediate mode functions
│   └── vx_khr_*.h               # KHR extension headers
├── docs/
│   ├── OpenVX_Specification.adoc  # Main specification source (AsciiDoc)
│   ├── vx_khr_*.adoc              # KHR extension specification sources
│   ├── api/                       # Auto-generated API include files
│   ├── config/                    # Build configuration, stylesheets, scripts
│   ├── examples/                  # Example C code referenced in the spec
│   ├── images/                    # Diagrams and figures
│   ├── katex/                     # KaTeX math rendering library (HTML)
│   ├── xml/                       # API registry (vx.xml) and generators
│   ├── doxygen/                   # Doxygen build for header documentation
│   ├── Makefile                   # Spec build makefile
│   └── README.adoc                # Detailed build instructions
├── .gitlab-ci.yml               # CI/CD pipeline configuration
├── Makefile                     # Top-level makefile (Doxygen)
├── LICENSE                      # Apache 2.0 License
└── CODE_OF_CONDUCT.md
```

## Prerequisites

The build runs on **Ubuntu 22.04** (used in CI). The following packages and tools are required:

### System Packages

```bash
sudo apt-get update
sudo apt-get install -y \
    wget unzip build-essential python3 git cmake bison flex \
    libffi-dev zip fonts-lyx libgmp-dev libxml2-dev \
    libgdk-pixbuf2.0-dev libcairo2-dev libpango1.0-dev \
    gtk-doc-tools ghostscript ruby-dev default-jre \
    libwebp-dev libzstd-dev

# For Doxygen / PDF builds
sudo apt-get install -y texlive-latex-extra texlive-font-utils inkscape
```

### Ruby Gems

```bash
gem install asciidoctor-diagram -v 1.5.18
gem install asciidoctor-pdf --pre -v 1.6.2
gem install asciidoctor-mathematical -v 0.3.5
gem install coderay --pre -v 1.1.3
```

> For platform-specific setup (macOS, Windows/WSL, Cygwin, MinGW), see [docs/README.adoc](docs/README.adoc).

## Building

All build commands are run from the `docs/` directory.

### Build the main specification (HTML + PDF)

```bash
cd docs
make
```

This builds the OpenVX 1.3.2 specification and outputs to `docs/out/OpenVX_Specification_1_3_2/`.

### Build all specifications and extensions

```bash
cd docs
make all
```

### Build individual targets

```bash
# HTML only
make html

# PDF only
make pdf

# Specific extension
make SPECBASE=vx_khr_nn html pdf SPECREVISION='1.3' SPECSUFFIX=1_3
```

### Build output locations

| Target | Output |
|--------|--------|
| HTML | `docs/out/<SPECBASE>_<SPECSUFFIX>/<SPECBASE>_<SPECSUFFIX>.html` |
| PDF | `docs/out/<SPECBASE>_<SPECSUFFIX>.pdf` |

### Clean build artifacts

```bash
cd docs
make clean
```

## Extensions

The following KHR extensions are included in the repository:

| Extension | Description |
|-----------|-------------|
| `vx_khr_nn` | Neural Networks |
| `vx_khr_pipelining` | Graph Pipelining |
| `vx_khr_ix` | Export and Import |
| `vx_khr_icd` | Installable Client Driver |
| `vx_khr_feature_sets` | Feature Sets |
| `vx_khr_import_kernel` | Import Kernel |
| `vx_khr_opencl_interop` | OpenCL Interoperability |
| `vx_khr_class` | Object Array Class |
| `vx_khr_tiling` | Block Tiling |
| `vx_khr_xml` | XML Schema |
| `vx_khr_s16` | S16 Extension |
| `vx_khr_buffer_aliasing` | Buffer Aliasing |
| `vx_khr_user_data_object` | User Data Object |
| `vx_khr_raw_image` | Raw Image |
| `vx_khr_tensor_from_image` | Tensor from Image |
| `vx_khr_node_send_command` | Node Send Command |
| `vx_khr_sub_image_arrays` | Sub-Image Arrays |
| `vx_khr_swap_move` | Swap and Move |
| `vx_khr_bidirectional_parameters` | Bidirectional Parameters |
| `vx_khr_supplementary_data` | Supplementary Data |
| `vx_khr_safe_casts` | Safe Casts |

## Updating Specification Tags

Requirements tags follow the pattern `` `[*REQ-NNNN*]` ``. To add a new tag without an ID:

```
`[*REQ*]`
```

To auto-assign unique IDs to untagged requirements:

```bash
cd docs
python config/spec-tags.py update OpenVX_Specification.adoc
```

## Utility Scripts

```bash
# Check for missing API functions in headers vs spec
python docs/config/check-missing-apis.py

# Verbose output
python docs/config/check-missing-apis.py -v
```

## CI/CD

The GitLab CI pipeline (`.gitlab-ci.yml`) runs on Ubuntu 22.04 and:
1. Installs all system dependencies and Ruby gems
2. Runs `make all` from the `docs/` directory
3. Archives PDF and HTML outputs as artifacts (retained for 2 weeks)

## License

Licensed under the [Apache License, Version 2.0](LICENSE).

Copyright (c) 2012-2025 The Khronos Group Inc.
