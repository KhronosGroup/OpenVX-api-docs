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

static void create_data_refs(vx_context context, vx_image *in_refs, vx_image *out_refs, vx_uint32 num_in_refs, vx_uint32 num_out_refs, vx_uint32 width, vx_uint32 height)
{
    vx_uint32 i;

    for(i=0; i<num_in_refs; i++)
    {
        in_refs[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_RGB);
    }
    for(i=0; i<num_out_refs; i++)
    {
        out_refs[i] = vxCreateImage(context, width, height, VX_DF_IMAGE_S16);
    }
}

static void release_data_refs(vx_image *in_refs, vx_image *out_refs, vx_uint32 num_in_refs, vx_uint32 num_out_refs)
{
    vx_uint32 i;

    for(i=0; i<num_in_refs; i++)
    {
        vxReleaseImage(&in_refs[i]);
    }
    for(i=0; i<num_out_refs; i++)
    {
        vxReleaseImage(&out_refs[i]);
    }
}

// tag::graph_pipeline[]

/*
 * index of graph parameter data reference which is used to provide input to the graph
 */
#define GRAPH_PARAMETER_IN  (0u)
/*
 * index of graph parameter data reference which is used to provide output to the graph
 */
#define GRAPH_PARAMETER_OUT (1u)
/*
 * max parameters to this graph
 */
#define GRAPH_PARAMETER_MAX (2u)

/*
 * Utility API used to add a graph parameter from a node, node parameter index
 */
void add_graph_parameter_by_node_index(vx_graph graph, vx_node node,
                                       vx_uint32 node_parameter_index)
{
    vx_parameter parameter = vxGetParameterByIndex(node, node_parameter_index);
    vxAddParameterToGraph(graph, parameter);
    vxReleaseParameter(&parameter);
}

/*
 * Utility API used to create graph with graph parameter for input and output
 *
 * The following graph is created,
 * IN_IMG -> EXTRACT_NODE -> TMP_IMG -> CONVERT_DEPTH_NODE -> OUT_IMG
 *                                          ^
 *                                          |
 *                                      SHIFT_SCALAR
 *
 * IN_IMG and OUT_IMG are graph parameters.
 * TMP_IMG is a virtual image
 */
static vx_graph create_graph(vx_context context, vx_uint32 width, vx_uint32 height)
{
    vx_graph graph;
    vx_node n0, n1;
    vx_image tmp_img;
    vx_int32 shift;
    vx_scalar s0;

    graph = vxCreateGraph(context);

    /* create intermediate virtual image */
    tmp_img = vxCreateVirtualImage(graph, 0, 0, VX_DF_IMAGE_VIRT);

    /* create first node, input is NULL this will be made as graph parameter */
    n0 = vxChannelExtractNode(graph, NULL, VX_CHANNEL_G, tmp_img);

    /* create a scalar object required for second node */
    shift = 8;
    s0 = vxCreateScalar(context, VX_TYPE_INT32, &shift);

    /* create second node, output is NULL since this will be made as graph parameter
     */
    n1 = vxConvertDepthNode(graph, tmp_img, NULL, VX_CONVERT_POLICY_SATURATE, s0);

    /* add graph parameters */
    add_graph_parameter_by_node_index(graph, n0, 0);
    add_graph_parameter_by_node_index(graph, n1, 1);

    vxReleaseScalar(&s0);
    vxReleaseNode(&n0);
    vxReleaseNode(&n1);
    vxReleaseImage(&tmp_img);

    return graph;
}

/*
 * Utility API used to fill data and enqueue input to graph
 */
static void enqueue_input(vx_graph graph,
                          vx_uint32 width, vx_uint32 height, vx_image in_img)
{
    vx_rectangle_t rect = { 0, 0, width, height};
    vx_imagepatch_addressing_t imagepatch_addr;
    vx_map_id map_id;
    void *user_ptr;

    if(in_img!=NULL)
    {
        /* Fill input data using Copy/Map/SwapHandles */
        vxMapImagePatch(in_img, &rect, 0, &map_id, &imagepatch_addr, &user_ptr,
                        VX_WRITE_ONLY, VX_MEMORY_TYPE_NONE, VX_NOGAP_X);
        /* ... */
        vxUnmapImagePatch(in_img, map_id);
        vxGraphParameterEnqueueReadyRef(graph, GRAPH_PARAMETER_IN,
                                        (vx_reference*)&in_img, 1);
    }
}

/*
 * Utility API used to fill input to graph
 */
static void dequeue_input(vx_graph graph, vx_image *in_img)
{
    vx_uint32 num_refs;

    *in_img = NULL;

    /* Get consumed input reference */
    vxGraphParameterDequeueDoneRef(graph, GRAPH_PARAMETER_IN,
                                   (vx_reference*)in_img, 1, &num_refs);
}

/*
 * Utility API used to enqueue output to graph
 */
static void enqueue_output(vx_graph graph, vx_image out_img)
{
    if(out_img!=NULL)
    {
        vxGraphParameterEnqueueReadyRef(graph, GRAPH_PARAMETER_OUT,
                                        (vx_reference*)&out_img, 1);
    }
}

static vx_bool is_output_available(vx_graph graph)
{
    vx_uint32 num_refs;

    vxGraphParameterCheckDoneRef(graph, GRAPH_PARAMETER_OUT, &num_refs);

    return (num_refs > 0);
}

/*
 * Utility API used to dequeue output and consume it
 */
static void dequeue_output(vx_graph graph,
                           vx_uint32 width, vx_uint32 height, vx_image *out_img)
{
    vx_rectangle_t rect = { 0, 0, width, height};
    vx_imagepatch_addressing_t imagepatch_addr;
    vx_map_id map_id;
    void *user_ptr;
    vx_uint32 num_refs;

    *out_img = NULL;

    /* Get output reference and consume new data,
     * waits until a reference is available
     */
    vxGraphParameterDequeueDoneRef(graph, GRAPH_PARAMETER_OUT,
                                   (vx_reference*)out_img, 1, &num_refs);
    if(*out_img!=NULL)
    {
        /* Consume output data using Copy/Map/SwapHandles */
        vxMapImagePatch(*out_img, &rect, 0, &map_id, &imagepatch_addr, &user_ptr,
                        VX_READ_ONLY, VX_MEMORY_TYPE_NONE, VX_NOGAP_X);
        /* ... */
        vxUnmapImagePatch(*out_img, map_id);
    }
}

/* Max number of input references */
#define GRAPH_PARAMETER_IN_MAX_REFS   (2u)
/* Max number of output references */
#define GRAPH_PARAMETER_OUT_MAX_REFS   (2u)

/* execute graph in a pipelined manner
 */
void vx_khr_pipelining()
{
    vx_uint32 width = 640, height = 480, i;
    vx_context context;
    vx_graph graph;
    vx_image in_refs[GRAPH_PARAMETER_IN_MAX_REFS];
    vx_image out_refs[GRAPH_PARAMETER_IN_MAX_REFS];
    vx_image in_img, out_img;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[GRAPH_PARAMETER_MAX];

    context = vxCreateContext();
    graph = create_graph(context, width, height);

    create_data_refs(context, in_refs, out_refs, GRAPH_PARAMETER_IN_MAX_REFS,
                     GRAPH_PARAMETER_OUT_MAX_REFS, width, height);

    graph_parameters_queue_params_list[0].graph_parameter_index =
            GRAPH_PARAMETER_IN;
    graph_parameters_queue_params_list[0].refs_list_size =
            GRAPH_PARAMETER_IN_MAX_REFS;
    graph_parameters_queue_params_list[0].refs_list =
            (vx_reference*)&in_refs[0];
    graph_parameters_queue_params_list[1].graph_parameter_index =
            GRAPH_PARAMETER_OUT;
    graph_parameters_queue_params_list[1].refs_list_size =
            GRAPH_PARAMETER_OUT_MAX_REFS;
    graph_parameters_queue_params_list[1].refs_list =
            (vx_reference*)&out_refs[0];

    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            GRAPH_PARAMETER_MAX,
            graph_parameters_queue_params_list
            );

    vxVerifyGraph(graph);

    /* enqueue input and output to trigger graph */
    for(i=0; i<GRAPH_PARAMETER_IN_MAX_REFS; i++)
    {
        enqueue_input(graph, width, height, in_refs[i]);
    }
    for(i=0; i<GRAPH_PARAMETER_OUT_MAX_REFS; i++)
    {
        enqueue_output(graph, out_refs[i]);
    }

    while(1)
    {
        /* wait for input to be available, dequeue it -
         * BLOCKs until input can be dequeued
         */
        dequeue_input(graph, &in_img);

        /* wait for output to be available, dequeue output and process it -
         * BLOCKs until output can be dequeued
         */
        dequeue_output(graph, width, height, &out_img);

        /* recycle input - fill new data and re-enqueue*/
        enqueue_input(graph, width, height, in_img);

        /* recycle output */
        enqueue_output(graph, out_img);

        if(CheckExit())
        {
            /* App wants to exit, break from main loop */
            break;
        }
    }

    /*
     * wait until all previous graph executions have completed
     */
    vxWaitGraph(graph);

    /* flush output references, only required
     * if need to consume last few references
     */
    while( is_output_available(graph) )
    {
      dequeue_output(graph, width, height, &out_img);
    }

    vxReleaseGraph(&graph);
    release_data_refs(in_refs, out_refs, GRAPH_PARAMETER_IN_MAX_REFS,
                      GRAPH_PARAMETER_OUT_MAX_REFS);
    vxReleaseContext(&context);
}
// end::graph_pipeline[]

// tag::graph_events[]
/* Utility API to clear any pending events */
static void clear_pending_events(vx_context context)
{
    vx_event_t event;

    /* do not block */
    while(vxWaitEvent(context, &event, vx_true_e)==VX_SUCCESS)
        ;
}

/* execute graph in a pipelined manner with events used
 * to schedule the graph execution
 */
void vx_khr_pipelining_with_events()
{
    vx_uint32 width = 640, height = 480, i;
    vx_context context;
    vx_graph graph;
    vx_image in_refs[GRAPH_PARAMETER_IN_MAX_REFS];
    vx_image out_refs[GRAPH_PARAMETER_IN_MAX_REFS];
    vx_image in_img, out_img;
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[GRAPH_PARAMETER_MAX];

    context = vxCreateContext();
    graph = create_graph(context, width, height);

    create_data_refs(context, in_refs, out_refs, GRAPH_PARAMETER_IN_MAX_REFS,
                     GRAPH_PARAMETER_OUT_MAX_REFS, width, height);

    graph_parameters_queue_params_list[0].graph_parameter_index = GRAPH_PARAMETER_IN;
    graph_parameters_queue_params_list[0].refs_list_size =
                    GRAPH_PARAMETER_IN_MAX_REFS;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&in_refs[0];
    graph_parameters_queue_params_list[1].graph_parameter_index = GRAPH_PARAMETER_OUT;
    graph_parameters_queue_params_list[1].refs_list_size =
                    GRAPH_PARAMETER_OUT_MAX_REFS;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&out_refs[0];

    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_AUTO,
            GRAPH_PARAMETER_MAX,
            graph_parameters_queue_params_list
            );

    /* register events for input consumed and output consumed */
    vxRegisterEvent((vx_reference)graph, VX_EVENT_GRAPH_PARAMETER_CONSUMED,
                    GRAPH_PARAMETER_IN);
    vxRegisterEvent((vx_reference)graph, VX_EVENT_GRAPH_PARAMETER_CONSUMED,
                    GRAPH_PARAMETER_OUT);

    vxVerifyGraph(graph);

    /* disable events generation */
    vxEnableEvents(context);
    /* clear pending events.
     * Not strictly required but- it's a good practice to clear any
     * pending events from last execution before waiting on new events */
    clear_pending_events(context);

    /* enqueue input and output to trigger graph */
    for(i=0; i<GRAPH_PARAMETER_IN_MAX_REFS; i++)
    {
        enqueue_input(graph, width, height, in_refs[i]);
    }
    for(i=0; i<GRAPH_PARAMETER_OUT_MAX_REFS; i++)
    {
        enqueue_output(graph, out_refs[i]);
    }

    while(1)
    {
        vx_event_t event;

        /* wait for events, block until event is received */
        vxWaitEvent(context, &event, vx_false_e);

        /* event for input data ready for recycling, i.e early input release */
        if(event.type == VX_EVENT_GRAPH_PARAMETER_CONSUMED
            && event.event_info.graph_parameter_consumed.graph == graph
            && event.event_info.graph_parameter_consumed.graph_parameter_index
                    == GRAPH_PARAMETER_IN
            )
        {
            /* dequeue consumed input, fill new data and re-enqueue */
            dequeue_input(graph, &in_img);
            enqueue_input(graph, width, height, in_img);
        }
        else
        /* event for output data ready for recycling, i.e output release */
        if(event.type == VX_EVENT_GRAPH_PARAMETER_CONSUMED
            && event.event_info.graph_parameter_consumed.graph == graph
            && event.event_info.graph_parameter_consumed.graph_parameter_index
                    == GRAPH_PARAMETER_OUT
            )
        {
            /* dequeue output reference, consume generated data and
             * re-enqueue output reference
             */
            dequeue_output(graph, width, height, &out_img);
            enqueue_output(graph, out_img);
        }
        else
        if(event.type == VX_EVENT_USER && event.event_info.user_event.user_event_id
                                               == 0xDEADBEEF /* app code for exit */
            )
        {
            /* App wants to exit, break from main loop */
            break;
        }
    }

    /*
     * wait until all previous graph executions have completed
     */
    vxWaitGraph(graph);

    /* flush output references, only required if need to consume last few references
     */
    do {
      dequeue_output(graph, width, height, &out_img);
    } while(out_img!=NULL);

    vxReleaseGraph(&graph);
    release_data_refs(in_refs, out_refs, GRAPH_PARAMETER_IN_MAX_REFS,
                      GRAPH_PARAMETER_OUT_MAX_REFS);

    /* disable events generation */
    vxDisableEvents(context);
    /* clear pending events.
     * Not strictly required but- it's a good practice to clear any
     * pending events from last execution before exiting application */
    clear_pending_events(context);

    vxReleaseContext(&context);
}
// end::graph_events[]

// tag::graph_batch_processing[]

/* Max batch size supported by application */
#define GRAPH_PARAMETER_MAX_BATCH_SIZE  (10u)

/* execute graph in a batch-processing manner
 */
void vx_khr_batch_processing()
{
    vx_uint32 width = 640, height = 480, actual_batch_size;
    vx_context context;
    vx_graph graph;
    vx_image in_refs[GRAPH_PARAMETER_MAX_BATCH_SIZE];
    vx_image out_refs[GRAPH_PARAMETER_MAX_BATCH_SIZE];
    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[GRAPH_PARAMETER_MAX];

    context = vxCreateContext();
    graph = create_graph(context, width, height);

    create_data_refs(context, in_refs, out_refs, GRAPH_PARAMETER_MAX_BATCH_SIZE,
                     GRAPH_PARAMETER_MAX_BATCH_SIZE, width, height);

    graph_parameters_queue_params_list[0].graph_parameter_index = GRAPH_PARAMETER_IN;
    graph_parameters_queue_params_list[0].refs_list_size =
            GRAPH_PARAMETER_MAX_BATCH_SIZE;
    graph_parameters_queue_params_list[0].refs_list = (vx_reference*)&in_refs[0];
    graph_parameters_queue_params_list[1].graph_parameter_index = GRAPH_PARAMETER_OUT;
    graph_parameters_queue_params_list[1].refs_list_size =
            GRAPH_PARAMETER_MAX_BATCH_SIZE;
    graph_parameters_queue_params_list[1].refs_list = (vx_reference*)&out_refs[0];

    vxSetGraphScheduleConfig(graph,
            VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL,
            GRAPH_PARAMETER_MAX,
            graph_parameters_queue_params_list
            );

    vxVerifyGraph(graph);

    while(1)
    {
        /* read next batch of input and output */
        get_input_output_batch(in_refs, out_refs,
                GRAPH_PARAMETER_MAX_BATCH_SIZE,
                &actual_batch_size);

        vxGraphParameterEnqueueReadyRef(graph,
            GRAPH_PARAMETER_IN,
            (vx_reference*)&in_refs[0],
            actual_batch_size);

        vxGraphParameterEnqueueReadyRef(
            graph,
            GRAPH_PARAMETER_OUT,
            (vx_reference*)&out_refs[0],
            actual_batch_size);

        /* trigger processing of previously enqueued input and output */
        vxScheduleGraph(graph);
        /* wait for the batch processing to complete */
        vxWaitGraph(graph);

        /* dequeue the processed input and output data */
        vxGraphParameterDequeueDoneRef(graph,
            GRAPH_PARAMETER_IN,
            (vx_reference*)&in_refs[0],
            GRAPH_PARAMETER_MAX_BATCH_SIZE,
            &actual_batch_size);

        vxGraphParameterDequeueDoneRef(
            graph,
            GRAPH_PARAMETER_OUT,
            (vx_reference*)&out_refs[0],
            GRAPH_PARAMETER_MAX_BATCH_SIZE,
            &actual_batch_size);

        if(CheckExit())
        {
            /* App wants to exit, break from main loop */
            break;
        }
    }

    vxReleaseGraph(&graph);
    release_data_refs(in_refs, out_refs, GRAPH_PARAMETER_MAX_BATCH_SIZE,
                      GRAPH_PARAMETER_MAX_BATCH_SIZE);
    vxReleaseContext(&context);
}

// end::graph_batch_processing[]
