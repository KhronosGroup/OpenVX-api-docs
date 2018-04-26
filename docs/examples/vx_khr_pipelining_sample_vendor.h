/*
 * Copyright (c) 2012-2017 The Khronos Group Inc.
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
#ifndef _APP_H_
#define _APP_H_

#ifdef  __cplusplus
extern "C" {
#endif

/* This file has stub APIs and vendor specific API place holders
 * Actual implementation of these API will be vendor specific or
 * application specific
 */


/* Open and start Display device */
void DisplayDeviceOpen(int *fd, vx_uint32 width, vx_uint32 height, vx_enum df);
/* Stop and close Display device */
void DisplayDeviceClose(int *fd);
/* Swap buffer with a display device */
void DisplayDeviceSwapHandles(int fd, vx_reference new_buf, vx_reference *old_buf);

/* Open capture device */
void CaptureDeviceOpen(int *fd, vx_uint32 width, vx_uint32 height, vx_enum df);
/* Start capture device */
void CaptureDeviceStart(int fd);
/* Stop capture device */
void CaptureDeviceStop(int fd);
/* Close capture device */
void CaptureDeviceClose(int *fd);
/* Swap buffer with capture device */
void CaptureDeviceSwapHandles(int fd, vx_reference empty_buf, vx_reference *full_buf);
/* alloc buffers for capturing with capture device */
void CaptureDeviceAllocHandles(int fd, vx_reference bufs[], int num_bufs);
/* free buffers previously allocated */
void CaptureDeviceFreeHandles(int fd, vx_reference bufs[], int num_bufs);

/* API to check if application loop should exit or not */
vx_bool CheckExit();
/* API to wait/pend until user defined exit condition occurs */
void WaitExit();

/* get input and output buffers for next batch processing */
void get_input_output_batch(vx_image *in_refs, vx_image *out_refs,
                vx_uint32 max_batch_size,
                vx_uint32 *actual_batch_size);

#ifdef  __cplusplus
}
#endif

#endif
