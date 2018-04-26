/*
 * Copyright (c) 2012-2018 The Khronos Group Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and/or associated documentation files (the
 * "Materials"), to deal in the Materials without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Materials, and to
 * permit persons to whom the Materials are furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Materials.
 *
 * MODIFICATIONS TO THIS FILE MAY MEAN IT NO LONGER ACCURATELY REFLECTS
 * KHRONOS STANDARDS. THE UNMODIFIED, NORMATIVE VERSIONS OF KHRONOS
 * SPECIFICATIONS AND HEADER INFORMATION ARE LOCATED AT
 *    https://www.khronos.org/registry/
 *
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * MATERIALS OR THE USE OR OTHER DEALINGS IN THE MATERIALS.
 */

#ifndef _OPENVX_OPENCL_INTEROP_H_
#define _OPENVX_OPENCL_INTEROP_H_

#include <VX/vx.h>
#if __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

/*!
 * \file
 * \brief The OpenVX OpenCL interop extension API.
 */
#define OPENVX_KHR_OPENCL_INTEROP  "vx_khr_opencl_interop"

/*! \brief The constants added by OpenCL interop extension.
 *  \ingroup group_opencl_interop
 */

/* The vx_memory_type_e enum to import from the OpenCL buffer.
 */
#define VX_MEMORY_TYPE_OPENCL_BUFFER    (VX_ENUM_BASE(VX_ID_KHRONOS, VX_ENUM_MEMORY_TYPE) + 0x2)

/* vx_context attribute to query the OpenCL context associated with the OpenVX context.
 * Read-only.
 */
#define VX_CONTEXT_CL_CONTEXT           (VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_CONTEXT) + 0xF)

/* vx_context attribute to query the coordination command queue associated with the OpenVX context.
 * Read-only.
 */
#define VX_CONTEXT_CL_COMMAND_QUEUE     (VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_CONTEXT) + 0x10)

/* vx_node attribute to query the cl_command_queue associated with a user kernel node.
 * Read-only.
 */
#define VX_NODE_CL_COMMAND_QUEUE        (VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_NODE) + 0x9)

/* vx_kernel attribute to specify and query whether a user kernel is using the vx_khr_opencl_interop API.
 * Return value is vx_bool. The default value of this attribute is vx_false_e.
 * This attribute is read-only after the vxFinalizeKernel call.
 */
#define VX_KERNEL_USE_OPENCL            (VX_ATTRIBUTE_BASE(VX_ID_KHRONOS, VX_TYPE_KERNEL) + 0x4)


#ifdef  __cplusplus
extern "C" {
#endif

/*! \brief Create an OpenVX context with specified OpenCL context and global coordination command queue.
 *
 * This function creates a top-level object context for OpenVX and uses the OpenCL context and
 * global coordination command queue created by the application for the interop.
 *
 * This OpenCL context and global coordination command queue can be queried using
 * the VX_CONTEXT_CL_CONTEXT and VX_CONTEXT_CL_COMMAND_QUEUE attributes of vx_context.
 *
 * If the OpenVX context is created using vxCreateContext or vxCreateContextFromCL with
 * opencl_context as NULL, the OpenCL context used by OpenVX is implementation dependent.
 * If the opencl_command_queue is NULL, the global coordination command queue used by
 * OpenVX is implementation dependent.
 *
 * The global coordination command queue must be created using the OpenCL context used by OpenVX.
 *
 * \param opencl_context [in] The OpenCL context
 * \param opencl_command_queue [in] The global coordination command queue
 *
 * \retval On success, a valid vx_context object. Calling vxGetStatus with the return value
 *         as a parameter will return VX_SUCCESS if the function was successful.
 *
 * \ingroup group_opencl_interop
 */
VX_API_ENTRY vx_context VX_API_CALL vxCreateContextFromCL(
        cl_context opencl_context,
        cl_command_queue opencl_command_queue
    );

#ifdef  __cplusplus
}
#endif

#endif
