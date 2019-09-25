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

/*
 * This file has sample API usage of pipelining extention API
 *
 * NOTE: the implementation in this file will work even on a
 *       vendor implementation which does not support pipelining.
 *       No change in below implementation is required.
 *
 */

#include <VX/vx.h>
#include <VX/vx_khr_pipelining.h>
#include <vx_khr_pipelining_sample_vendor.h>

/* number of image references to prime for capture device */
#define MAX_CAPTURE_REFS_PRIME      (3u)
/* input image references */
static vx_reference capture_refs_prime[MAX_CAPTURE_REFS_PRIME];

/* source kernel handle */
static vx_kernel user_node_source_kernel;
/* source kernel id */
static vx_enum user_node_source_kernel_id;

/* sink kernel handle */
static vx_kernel user_node_sink_kernel;
/* sink kernel id */
static vx_enum user_node_sink_kernel_id;

/* V4L2 camera device file handle */
static int capture_dev = -1;
/* V4L2 display device file handle */
static int display_dev = -1;

// tag::streaming_source_node[]
static vx_status user_node_source_validate(
                vx_node node,
                const vx_reference parameters[],
                vx_uint32 num,
                vx_meta_format metas[])
{
    /* if any verification checks do here */
    return VX_SUCCESS;
}

static vx_status user_node_source_init(
                vx_node node,
                const vx_reference parameters[],
                vx_uint32 num)
{
    vx_image img = (vx_image)parameters[0];
    vx_uint32 width, height, i;
    vx_enum df;

    vxQueryImage(img, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(img, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(img, VX_IMAGE_FORMAT, &df, sizeof(vx_enum));

    CaptureDeviceOpen(&capture_dev, width, height, df);
    /* allocate images for priming the capture device.
     * Typically capture devices need some image references to be
     * primed in order to start capturing data.
     */
    CaptureDeviceAllocHandles(capture_dev, capture_refs_prime,
                              MAX_CAPTURE_REFS_PRIME);
    /* prime image references to capture device */
    for(i=0; i<MAX_CAPTURE_REFS_PRIME; i++)
    {
        CaptureDeviceSwapHandles(capture_dev, capture_refs_prime[i], NULL);
    }
    /* start capturing data to primed image references */
    CaptureDeviceStart(capture_dev);

    return VX_SUCCESS;
}

static vx_status user_node_source_run(
                    vx_node node,
                    vx_reference parameters[],
                    vx_uint32 num)
{
    vx_reference empty_ref, full_ref;

    empty_ref = parameters[0];

    /* swap a 'empty' image reference with a captured image reference filled with data
     * If this is one of the first few calls to CaptureDeviceSwapHandle, then full_buf
     * would be one of the image references primed during user_node_source_init
     */
    CaptureDeviceSwapHandles(capture_dev, empty_ref, &full_ref);

    parameters[0] = full_ref;

    return VX_SUCCESS;
}

static vx_status user_node_source_deinit(
                    vx_node node,
                    const vx_reference parameters[],
                    vx_uint32 num)
{
    CaptureDeviceStop(capture_dev);
    CaptureDeviceFreeHandles(capture_dev, capture_refs_prime, MAX_CAPTURE_REFS_PRIME);
    CaptureDeviceClose(&capture_dev);

    return VX_SUCCESS;
}

/* Add user node as streaming node */
static void user_node_source_add(vx_context context)
{
    vxAllocateUserKernelId(context, &user_node_source_kernel_id);

    user_node_source_kernel = vxAddUserKernel(
            context,
            "user_kernel.source",
            user_node_source_kernel_id,
            (vx_kernel_f)user_node_source_run,
            1,
            user_node_source_validate,
            user_node_source_init,
            user_node_source_deinit
            );

    vxAddParameterToKernel(user_node_source_kernel,
        0,
        VX_OUTPUT,
        VX_TYPE_IMAGE,
        VX_PARAMETER_STATE_REQUIRED
        );

    vxFinalizeKernel(user_node_source_kernel);
}

/* Boiler plate code of standard OpenVX API, nothing specific to streaming API */
static void user_node_source_remove()
{
    vxRemoveKernel(user_node_source_kernel);
}

/* Boiler plate code of standard OpenVX API, nothing specific to streaming API */
static vx_node user_node_source_create_node(vx_graph graph, vx_image output)
{
    vx_node node = NULL;

    node = vxCreateGenericNode(graph, user_node_source_kernel);
    vxSetParameterByIndex(node, 0, (vx_reference)output);

    return node;
}

// end::streaming_source_node[]

// tag::streaming_source_pipeup_node[]
static vx_status user_node_source_init(
                vx_node node,
                const vx_reference parameters[],
                vx_uint32 num)
{
    vx_image img = (vx_image)parameters[0];
    vx_uint32 width, height, i;
    vx_enum df;

    vxQueryImage(img, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(img, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(img, VX_IMAGE_FORMAT, &df, sizeof(vx_enum));

    CaptureDeviceOpen(&capture_dev, width, height, df);

    /* start capturing data (actual start happens later when it gets references) */
    CaptureDeviceStart(capture_dev);

    return VX_SUCCESS;
}

static vx_status user_node_source_run(
                    vx_node node,
                    vx_reference parameters[],
                    vx_uint32 num)
{
    uint32_t state;
    vx_reference empty_ref, full_ref;

    vxQueryNode(node, VX_NODE_STATE, &state, sizeof(state));

    empty_ref = parameters[0];

    if (state == VX_NODE_STATE_STEADY)
    {
        /* swap a 'empty' image reference with a captured image reference filled with data
         * If this is one of the first few calls to CaptureDeviceSwapHandle, then full_buf
         * would be one of the image references primed during VX_NODE_STATE_PIPEUP
         */
        CaptureDeviceSwapHandles(capture_dev, empty_ref, &full_ref);
    }
    else
    {
        /* prime image reference to capture device */
        CaptureDeviceSwapHandles(capture_dev, empty_ref, NULL);
    }

    parameters[0] = full_ref;

    return VX_SUCCESS;
}

static vx_status user_node_source_deinit(
                    vx_node node,
                    const vx_reference parameters[],
                    vx_uint32 num)
{
    CaptureDeviceStop(capture_dev);
    CaptureDeviceClose(&capture_dev);

    return VX_SUCCESS;
}

/* Add user node as streaming node */
static void user_node_source_add(vx_context context)
{
    vx_uint32 pipeup_depth = 3;

    vxAllocateUserKernelId(context, &user_node_source_kernel_id);

    user_node_source_kernel = vxAddUserKernel(
            context,
            "user_kernel.source",
            user_node_source_kernel_id,
            (vx_kernel_f)user_node_source_run,
            1,
            user_node_source_validate,
            user_node_source_init,
            user_node_source_deinit
            );

    vxAddParameterToKernel(user_node_source_kernel,
        0,
        VX_OUTPUT,
        VX_TYPE_IMAGE,
        VX_PARAMETER_STATE_REQUIRED
        );

    vxSetKernelAttribute(user_node_source_kernel, VX_KERNEL_PIPEUP_DEPTH,
                         &pipeup_depth, sizeof(pipeup_depth));

    vxFinalizeKernel(user_node_source_kernel);
}

/* Boiler plate code of standard OpenVX API, nothing specific to streaming API */
static void user_node_source_remove()
{
    vxRemoveKernel(user_node_source_kernel);
}

/* Boiler plate code of standard OpenVX API, nothing specific to streaming API */
static vx_node user_node_source_create_node(vx_graph graph, vx_image output)
{
    vx_node node = NULL;

    node = vxCreateGenericNode(graph, user_node_source_kernel);
    vxSetParameterByIndex(node, 0, (vx_reference)output);

    return node;
}
// end::streaming_source_pipeup_node[]


// tag::streaming_sink_node[]
/* Boiler plate code of standard OpenVX API, nothing specific to streaming API */
static vx_status user_node_sink_validate(
                    vx_node node,
                    const vx_reference parameters[],
                    vx_uint32 num,
                    vx_meta_format metas[])
{
    /* if any verification checks do here */
    return VX_SUCCESS;
}

static vx_status user_node_sink_init(
                    vx_node node,
                    const vx_reference parameters[],
                    vx_uint32 num)
{
    vx_image img = (vx_image)parameters[0];
    vx_uint32 width, height;
    vx_enum df;

    vxQueryImage(img, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(img, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(img, VX_IMAGE_FORMAT, &df, sizeof(vx_enum));

    DisplayDeviceOpen(&display_dev, width, height, df);
    /* allocate images for priming the display device.
     * Typically display devices need to retain one or more
     * filled buffers until a new filled buffer is given.
     */
    DisplayDeviceAllocHandles(display_dev, display_refs_prime,
                              MAX_DISPLAY_REFS_PRIME);
    /* prime image references to display device */
    for(i=0; i<MAX_DISPLAY_REFS_PRIME; i++)
    {
        DisplayDeviceSwapHandles(display_dev, display_refs_prime[i], NULL);
    }

    return VX_SUCCESS;
}

static vx_status user_node_sink_run(
                    vx_node node,
                    vx_reference parameters[],
                    vx_uint32 num)
{
    vx_reference new_ref, old_ref;

    new_ref = parameters[0];

    /* Swap input reference with reference currently held by display
     * Return parameters to framework to be recycled
     * for subsequent graph execution
     */
    DisplayDeviceSwapHandles(display_dev, new_ref, &old_ref);

    parameters[0] = old_ref;

    return VX_SUCCESS;
}

static vx_status user_node_sink_deinit(
                    vx_node node,
                    const vx_reference parameters[],
                    vx_uint32 num)
{
    DisplayDeviceFreeHandles(display_dev, display_refs_prime, MAX_DISPLAY_REFS_PRIME);
    DisplayDeviceClose(&display_dev);

    return VX_SUCCESS;
}

/* Add user node as streaming node */
static void user_node_sink_add(vx_context context)
{
    vxAllocateUserKernelId(context, &user_node_sink_kernel_id);

    user_node_sink_kernel = vxAddUserKernel(
            context,
            "user_kernel.sink",
            user_node_sink_kernel_id,
            (vx_kernel_f)user_node_sink_run,
            1,
            user_node_sink_validate,
            user_node_sink_init,
            user_node_sink_deinit
            );

    vxAddParameterToKernel(user_node_sink_kernel,
        0,
        VX_INPUT,
        VX_TYPE_IMAGE,
        VX_PARAMETER_STATE_REQUIRED
        );

    vxFinalizeKernel(user_node_sink_kernel);
}

/* Boiler plate code of standard OpenVX API, nothing specific to streaming API */
static void user_node_sink_remove()
{
    vxRemoveKernel(user_node_sink_kernel);
}

/* Boiler plate code of standard OpenVX API, nothing specific to streaming API */
static vx_node user_node_sink_create_node(vx_graph graph, vx_image input)
{
    vx_node node = NULL;

    node = vxCreateGenericNode(graph, user_node_sink_kernel);
    vxSetParameterByIndex(node, 0, (vx_reference)input);

    return node;
}
// end::streaming_sink_node[]

// tag::streaming_sink_pipeup_node[]
/* Boiler plate code of standard OpenVX API, nothing specific to streaming API */
static vx_status user_node_sink_validate(
                    vx_node node,
                    const vx_reference parameters[],
                    vx_uint32 num,
                    vx_meta_format metas[])
{
    /* if any verification checks do here */
    return VX_SUCCESS;
}

static vx_status user_node_sink_init(
                    vx_node node,
                    const vx_reference parameters[],
                    vx_uint32 num)
{
    vx_image img = (vx_image)parameters[0];
    vx_uint32 width, height;
    vx_enum df;

    vxQueryImage(img, VX_IMAGE_WIDTH, &width, sizeof(vx_uint32));
    vxQueryImage(img, VX_IMAGE_HEIGHT, &height, sizeof(vx_uint32));
    vxQueryImage(img, VX_IMAGE_FORMAT, &df, sizeof(vx_enum));

    DisplayDeviceOpen(&display_dev, width, height, df);

    return VX_SUCCESS;
}

static vx_status user_node_sink_run(
                    vx_node node,
                    vx_reference parameters[],
                    vx_uint32 num)
{
    uint32_t state;
    vx_reference new_ref, old_ref = NULL;

    vxQueryNode(node, VX_NODE_STATE, &state, sizeof(state));

    new_ref = parameters[0];

    if (state == VX_NODE_STATE_STEADY)
    {
        /* Swap input reference with reference currently held by display
         * Return parameters to framework to be recycled
         * for subsequent graph execution
         */
        DisplayDeviceSwapHandles(display_dev, new_ref, &old_ref);
    }
    else
    {
        /* Send image reference to display device without getting one in return*/
        DisplayDeviceSwapHandles(display_dev, new_ref, NULL);
    }

    parameters[0] = old_ref;

    return VX_SUCCESS;
}

static vx_status user_node_sink_deinit(
                    vx_node node,
                    const vx_reference parameters[],
                    vx_uint32 num)
{
    DisplayDeviceClose(&display_dev);

    return VX_SUCCESS;
}

/* Add user node as streaming node */
static void user_node_sink_add(vx_context context)
{
    vx_uint32 pipeup_depth = 2;

    vxAllocateUserKernelId(context, &user_node_sink_kernel_id);

    user_node_sink_kernel = vxAddUserKernel(
            context,
            "user_kernel.sink",
            user_node_sink_kernel_id,
            (vx_kernel_f)user_node_sink_run,
            1,
            user_node_sink_validate,
            user_node_sink_init,
            user_node_sink_deinit
            );

    vxAddParameterToKernel(user_node_sink_kernel,
        0,
        VX_INPUT,
        VX_TYPE_IMAGE,
        VX_PARAMETER_STATE_REQUIRED
        );

    vxSetKernelAttribute(user_node_sink_kernel, VX_KERNEL_PIPEUP_DEPTH,
                         &pipeup_depth, sizeof(pipeup_depth));

    vxFinalizeKernel(user_node_sink_kernel);
}

/* Boiler plate code of standard OpenVX API, nothing specific to streaming API */
static void user_node_sink_remove()
{
    vxRemoveKernel(user_node_sink_kernel);
}

/* Boiler plate code of standard OpenVX API, nothing specific to streaming API */
static vx_node user_node_sink_create_node(vx_graph graph, vx_image input)
{
    vx_node node = NULL;

    node = vxCreateGenericNode(graph, user_node_sink_kernel);
    vxSetParameterByIndex(node, 0, (vx_reference)input);

    return node;
}
// end::streaming_sink_pipeup_node[]

// tag::streaming_application[]
/*
 * Utility API used to create graph with source and sink nodes
 */
static vx_graph create_graph(vx_context context, vx_uint32 width, vx_uint32 height)
{
    vx_graph graph;
    vx_node n0, n1, node_source, node_sink;
    vx_image in_img, tmp_img, out_img;
    vx_int32 shift;
    vx_scalar s0;

    graph = vxCreateGraph(context);

    in_img = vxCreateVirtualImage(graph, width, height, VX_DF_IMAGE_RGB);

    /* create source node */
    node_source = user_node_source_create_node(graph, in_img);

    /* Enable streaming */
    vxEnableGraphStreaming(graph, node_source);

    /* create intermediate virtual image */
    tmp_img = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT);

    /* create first node, input is NULL since this will be made as graph parameter */
    n0 = vxChannelExtractNode(graph, in_img, VX_CHANNEL_G, tmp_img);

    out_img = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_S16);

    /* create a scalar object required for second node */
    shift = 8;
    s0 = vxCreateScalar(context, VX_TYPE_INT32, &shift);

    /* create second node, output is NULL since this will be made as graph parameter
     */
    n1 = vxConvertDepthNode(graph, tmp_img, out_img, VX_CONVERT_POLICY_SATURATE, s0);

    /* create sink node */
    node_sink = user_node_sink_create_node(graph, out_img);

    vxReleaseScalar(&s0);
    vxReleaseNode(&n0);
    vxReleaseNode(&n1);
    vxReleaseNode(&node_source);
    vxReleaseNode(&node_sink);
    vxReleaseImage(&tmp_img);
    vxReleaseImage(&in_img);
    vxReleaseImage(&out_img);

    return graph;
}

void vx_khr_streaming_sample()
{
    vx_uint32 width = 640, height = 480;
    vx_context context = vxCreateContext();
    vx_graph graph;

    /* add user kernels to context */
    user_node_source_add(context);
    user_node_sink_add(context);

    graph = create_graph(context, width, height);

    vxVerifyGraph(graph);

    /* execute graph in streaming mode,
     * graph is retriggered when input reference is consumed by a graph execution
     */
    vxStartGraphStreaming(graph);

    /* wait until user wants to exit */
    WaitExit();

    /* stop graph streaming */
    vxStopGraphStreaming(graph);

    vxReleaseGraph(&graph);

    /* remove user kernels from context */
    user_node_source_remove();
    user_node_sink_remove();

    vxReleaseContext(&context);
}
// end::streaming_application[]
