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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

vx_status vx_example_extensions(vx_context context) {
// tag::extensions[]
    vx_char *tmp, *extensions = NULL;
    vx_size size = 0;
    vxQueryContext(context,VX_CONTEXT_EXTENSIONS_SIZE,&size,sizeof(size));
    extensions = malloc(size);
    vxQueryContext(context,VX_CONTEXT_EXTENSIONS,
                   extensions, size);
// end::extensions[]
    tmp = strtok(extensions, " ");
    do {
        if (tmp)
            printf("Extension: %s\n", tmp);
        tmp = strtok(NULL, " ");
    } while (tmp);
    free(extensions);
    return VX_SUCCESS;
}
