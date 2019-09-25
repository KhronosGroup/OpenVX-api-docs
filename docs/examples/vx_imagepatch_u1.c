/*
 * Copyright (c) 2012-2017 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <VX/vx.h>

#include <assert.h>
#define PATCH_DIM 16

vx_status example_imagepatch_u1(vx_context context)
{
    // tag::imagepatch_u1[]
    vx_status status = VX_SUCCESS;
    void *base_ptr = NULL;
    vx_uint32 width = 640, height = 480, plane = 0;
    vx_image image = vxCreateImage(context, width, height, VX_DF_IMAGE_U1);
    vx_rectangle_t rect;
    vx_imagepatch_addressing_t addr;
    vx_map_id map_id;

    rect.start_x = rect.start_y = 0;
    rect.end_x = rect.end_y = PATCH_DIM;

    /* special restrictions when addressing image patches in U1 images */
    assert(rect.start_x % 8 == 0);

    status = vxMapImagePatch(image, &rect, plane, &map_id,
                                &addr, &base_ptr,
                                VX_READ_AND_WRITE, VX_MEMORY_TYPE_HOST, 0);

    /* special restrictions on binary images */
    assert(addr.stride_x == 0);
    assert(addr.stride_x_bits == 1);

    if (status == VX_SUCCESS)
    {
        vx_uint32 x, y, i, j, xb;
        vx_uint8 pixel = 0;
        vx_uint8 mask, value;

        /* a couple addressing options */

        /* use linear addressing function/macro to address bytes */
        for (i = 0; i < addr.dim_x*addr.dim_y; i += 8) {
            vx_uint8 *ptr2 = (vx_uint8*)vxFormatImagePatchAddress1d(base_ptr, i, &addr);

            /* address and set individual bits/pixels within the byte */
            for (xb = 0; xb < 8; xb++) {
                mask = 1 << xb;
                value = pixel << xb;
                *ptr2 = (*ptr2 & ~mask) | value;
            }
        }

        /* 2d addressing option */
        for (y = 0; y < addr.dim_y; y+=addr.step_y) {

            /* address bytes */
            for (x = 0; x < addr.dim_x; x += 8) {
                vx_uint8 *ptr2 = (vx_uint8*)vxFormatImagePatchAddress2d(base_ptr, x, y, &addr);

                /* address and set individual bits/pixels within the byte */
                for (xb = 0; xb < 8; xb++) {
                    mask = 1 << xb;
                    value = pixel << xb;
                    *ptr2 = (*ptr2 & ~mask) | value;
                }
            }
        }

        /* direct addressing by client */
        for (y = 0; y < addr.dim_y; y+=addr.step_y) {
            j = addr.stride_y*y;

            /* address bytes */
            for (x = 0; x < addr.dim_x; x += 8) {
                vx_uint8 *tmp = (vx_uint8 *)base_ptr;
                i = j + x/8;
                vx_uint8 *ptr2 = &tmp[i];

                /* address and set individual bits/pixels within the byte */
                for (xb = 0; xb < 8; xb++) {
                    mask = 1 << xb;
                    value = pixel << xb;
                    *ptr2 = (*ptr2 & ~mask) | value;
                }
            }
        }

        /* this commits the data back to the image */
        status = vxUnmapImagePatch(image, map_id);
    }
    vxReleaseImage(&image);
    // end::imagepatch_u1[]
    return status;
}
