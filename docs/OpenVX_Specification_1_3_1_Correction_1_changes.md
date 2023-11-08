## Correction to OpenVX Specification 1.3.1 Section 7.6.3: vxAddUserKernel API

The data type of the `name` parameter changed from `const vx_char name[VX_MAX_KERNEL_NAME]` to `const vx_char * name`. Below is corrected `vxAddUserKernel` API declaration:

```
vx_kernel vxAddUserKernel(
    vx_context                                  context,
    const vx_char *                             name,
    vx_enum                                     enumeration,
    vx_kernel_f                                 func_ptr,
    vx_uint32                                   numParams,
    vx_kernel_validate_f                        validate,
    vx_kernel_initialize_f                      init,
    vx_kernel_deinitialize_f                    deinit);
```

The above change is reflected in the latest header package on Khronos OpenVX registry dated Nov 2023.
