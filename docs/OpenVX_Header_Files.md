## OpenVX Header Files

OpenVX is defined as an API in the C99 language.
Khronos provides a corresponding set of header files for applications using the API, which may be used in either C or C++ code.
The interface descriptions in the OpenVX specification are the same as the interfaces defined in these header files.

### OpenVX Combined API Header "VX/vx.h" (Informative)

Applications normally will include the header file `VX/vx.h`. In turn, `VX/vx.h` always includes the following header files:

| Header File | Details |
| :----------- | :------ |
| VX/vx_types.h     | The type definitions required by OpenVX Library. |
| VX/vx_api.h       | The API definition for OpenVX. |
| VX/vx_kernels.h   | The list of supported kernels in the OpenVX standard. |
| VX/vx_nodes.h     | The "Simple" API interface for OpenVX. These APIs are just wrappers around the more verbose functions defined in "VX/vx_api.h". |
| VX/vx_vendors.h   | The OpenVX Vendor ID list. As new vendors submit their implementations, this enumeration will grow. |

### OpenVX Extension Header Control (Informative)

OpenVX extensions are defined in separate header files allowing applications to decide whether or not to include them.
The header files for OpenVX extensions are of the form `VX/<extension-name>.h`.

#### OpenVX KHR Extensions

| Header File | Details |
| :----------- | :------ |
| VX/vx_khr_nn.h | OpenVX KHR Neural Network Extension |
| VX/vx_khr_pipelining.h | OpenVX KHR Pipelining Extension |
| VX/vx_khr_ix.h | OpenVX KHR Export And Import Extension |
| VX/vx_khr_user_data_object.h | OpenVX KHR User Data Object Extension |

#### OpenVX KHR Provisional Extensions

| Header File | Details |
| :----------- | :------ |
| VX/vx_khr_bidirectional_parameters.h | OpenVX KHR Bidirectional Parameters Extension |
| VX/vx_khr_raw_image.h | OpenVX KHR Raw Image Extension |
| VX/vx_khr_swap_move.h | OpenVX KHR Swap And Move kernel Extension |
| VX/vx_khr_import_kernel.h | OpenVX KHR Import Kernel Extension |
| VX/vx_khr_buffer_aliasing.h | OpenVX KHR Buffer Aliasing Extension |
| VX/vx_khr_class.h | OpenVX KHR Classifier Extension |
| VX/vx_khr_icd.h | OpenVX KHR Installable Client Driver Extension |
| VX/vx_khr_opencl_interop.h | OpenVX KHR OpenCL Interop Extension |
| VX/vx_khr_tiling.h | OpenVX KHR Tiling Extension |
| VX/vx_khr_xml.h | OpenVX KHR XML Extension |
