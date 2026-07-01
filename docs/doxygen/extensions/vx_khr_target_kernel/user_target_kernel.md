@mainpage

# Introduction

## Purpose

This document details an extension to any OpenVX version from 1.1 to 1.3, and references some APIs
and symbols that may be found in those APIs: https://www.khronos.org/registry/OpenVX/.

This extension is intended to define support for the user kernels which can be executed on remote
targets in the system, not just the host core.

## Acknowledgements

This specification would not be possible without the contributions from this partial list of the
following individuals from the Khronos Working Group and the companies that they represented at the
time:

* Kiriti Nagesh Gowda - AMD
* Raphael Cano - Robert Bosch GmbH
* Jesse Villarreal - TI
* Isaac Wong - Ambarella International LP
* Viktor Gyenes - AI Motive

## Background

The main OpenVX specification includes APIs to support [User
Kernels](https://registry.khronos.org/OpenVX/specs/1.3_08Aug2019/html/OpenVX_Specification_1_3.html#sec_user_kernels),
which enables users to plugin their own kernels. However, this support is limited to user kernels
which are **executed on the host core**, including host core calls into an API such as
OpenCL/Vulkan/OpenGL to dispatch to a hardware accelerator such as a GPU.

Some systems include a variety of heterogeneous CPUs and hardware accelerators which can be
leveraged by nodes in OpenVX graphs.  In such systems, the host can offload processing to dedicated
hardware targets in the system, freeing up cycles on the host processor for other tasks.  This
extension provides support for users to add kernels on other targets in such systems.

The diagram below shows an example system which has several targets where user kernels can run.  The
left side of this diagram can make use of the existing user kernel functionality from the main
specification, where the host CPU can either run CPU-based kernels, or potentially run an OpenCL
kernel which can offload to a GPU, for example. The right side of this diagram shows how OpenVX
nodes can be executed on remote CPUs.  These remote CPUs can execute kernels which can run
algorithms or host drivers to hardware accelerators.

@anchor DesignOverviewDiagram

![Heterogeneous SoC Model](OVX-heterogeneous-SoC-model.drawio.svg){html: width=40%, latex: width=0.8\textwidth}

**For this purpose, a new set of "Target Kernel" APIs has been designed.**

# Design Overview

## User Target Kernel Callbacks

This extension builds on the existing pattern from the OpenVX specification. In the main
specification, the [vxAddUserKernel][vxAddUserKernel] API is defined, which allows the user to
register the kernel (by name and enum) to the framework, along with four kernel callback functions
which are compiled and linked to run on the host processor:
- **init**: Kernel initialization function
- **validate**: Validation function, which validates parameters to the kernel
- **func_ptr**: Main processing function which is called each time the graph is executed.
- **deinit**: Kernel deinitialization function

This extension also leverages this same function to register the kernel and host-side callbacks on
the host CPU, however, if the user intends this kernel to execute on a remote target instead of the
host CPU, the user should pass a **NULL** pointer to the **func_ptr** callback pointer.  This
signals the framework that this kernel is expected to also be registered on a remote CPU in the
system using the [vxAddTargetKernel](#vxAddTargetKernel) or
[vxAddTargetKernelByName](#vxAddTargetKernelByName) APIs, which registers up to four additional
target-side callbacks:
- **create_func**: Called during graph verification, to perform any local memory setup or one-time
  configuration.
- **process_func**: Main processing function which is called each time the graph is executed.
- **control_func**: Can optionally be called asynchronously via [vxNodeSendCommand][vxNodeSendCommand] from the
  application.
- **delete_func**: Called during graph release, to release local memory or tear-down any local
  setup.

The following diagram depicts these two sets of callbacks: one for the host, and one for the target.
The reason for the separation of these callbacks and associated registration functions is due to
the fact that the target callbacks typically are linked into the executable binaries of one or
more remote core firmwares separate from the host callbacks.

![User Kernel Callbacks](user-kernel-callbacks.drawio.svg){html: width=30%, latex: width=0.6\textwidth}

During graph verification, before the creation of individual nodes on the remote core(s) (see the
relevant callbacks), the host may validate the kernel parameters used to create each node. For this
validation, the host may need to access the complete set of OpenVX objects and this can be done within the
original user node validate callback which resides on the host.

The following call sequence shows the relative interaction between the host application and the
target kernel callbacks:

\mscfile usertarget_and_ovx_states.msc "Sequence of Host/Target Kernel Interaction" height=\textheight

## User Target Kernel Registration

As mentioned earlier, the [vxAddUserKernel][vxAddUserKernel] function requires both a kernel name
and a kernel enumeration on the host.  The enumeration may be statically defined, as in the case for
the standard OpenVX vision kernels, or it may be dynamically allocated using
[vxAllocateUserKernelId][vxAllocateUserKernelId].  These two ways of obtaining kernel enumerations
are the reason why this extension provides two separate options for registering user target kernels
as indicated below:

- [vxAddTargetKernel](#vxAddTargetKernel) : Can be used when the kernel enumeration is known on the
  target CPU at the time of target kernel registration (for example, if it is statically assigned at
  build time)

- [vxAddTargetKernelByName](#vxAddTargetKernelByName) : Should be used when the kernel enumeration
  is **NOT** known on the target CPU at the time of target kernel registration (for example, if it
  is dynamically assigned at run time on the host via
  [vxAllocateUserKernelId][vxAllocateUserKernelId]).
  
Kernel names are usually statically defined, so technically
[vxAddTargetKernelByName](#vxAddTargetKernelByName) can also be used instead of
[vxAddTargetKernel](#vxAddTargetKernel) even if the kernel enumeration is known on the target CPU.
However, both functions are available in case a framework or kernel implementation is more optimal
using [vxAddTargetKernel](#vxAddTargetKernel) when it can.

## Target-side Opaque Objects

As illustrated in the [Heterogeneous SoC Model](@ref DesignOverviewDiagram) from the "Background"
section, the host and target model requires sharing only a minimal set of data to allow the remote
kernel to access essential information, such as:
- Inputs
- Outputs
- Kernel parameters
- Other relevant metadata

While it is technically possible to exchange complete set of OpenVX objects between the host and targets,
doing so may significantly increase memory usage and result in unnecessary data transfer. To
address this, this extension prioritizes minimizing the memory footprint by exchanging only the
essential subset of information:

- [vx_target_kernel](#vx_target_kernel) : Opaque target kernel object (target-side equivalent of
  vx_kernel object)
  - Used when adding and removing the target kernel
- [vx_target_kernel_instance](#vx_target_kernel_instance) : Opaque target kernel instance passed to
  the kernel callbacks (target-side equivalent of vx_node object)
  - Used, if needed, to set and get the kernel instance context that can be shared between callbacks
    (described below)
- [vx_object_desc](#vx_object_desc) : Opaque target input and output data object information
  (target-side equivalent to vx_reference object for data objects)
  - Used to access node parameters within the target callbacks
  - Must be shared between the host and (possibly multiple) remote cores in a manner that prevents
    concurrent read/write access
- The number of parameters.
- And some additional optional data.

These objects are designed to contain just enough data to enable the invocation and execution of the
four callback functions described earlier, specifically on a dedicated target. As a trade-off, the
target kernel will not have access to the full set of OpenVX APIs. However, this is not required,
since kernel implementations do not interact with the graph or other abstract objects.

### Accessing data from Objects

The actual contents of [vx_target_kernel_instance](#vx_target_kernel_instance) and
[vx_object_desc](#vx_object_desc), and the mechanism by which the writers of the user target kernels
can access required information (such as image width/height, buffer addresses, etc) are not
specified in this extension.  These details are implementation dependent.  The rationale for this
are as follows:

- The priority for this extension is a lightweight implementation on targets to optimize for memory
  and speed.  Therefore, reusing the existing data object access functions, or creating target-side
  equivalents may violate this priority.
- Unlike host-only user kernels, target kernels are more often then not, vendor specific.  For
  example, they include categories of algorithms optimized for remote targets like specific DSPs, or
  hardware drivers specific to vendors' IP.  Therefore, portability of the actual user target kernels
  is not as high of a priority for this extension as compared to remote core code size.

Therefore, this extension focuses on providing the high level mechanism of registration functions
for specific callbacks, saving and accessing context between callbacks, and memory allocations
specific for the target.

## Target Kernel Instance Context

In many cases, there may be some context information that needs to be shared between the callbacks
of a kernel.  For example, if a kernel instance needs some scratch memory, the create callback can
allocate it, the process callback can utilize it, and the delete callback can free it.  Additionally,
there may be some parameters that are calculated or setup in the create callback, and are tracked
and updated in the process callback. This context is kernel-specific, typically using a custom
structure that can contain all the pointers, sizes, and parameters that needs to be maintained and
shared between the callbacks.  Once a buffer of the size of this context structure is allocated in the
create callback, it can be initialized and then finally registered and retrieved using the following calls:
- [vxSetTargetKernelInstanceContext](#vxSetTargetKernelInstanceContext) typically called from the
  create callback to save off the pointer to an instance context that can be retrieved from other
  callbacks
- [vxGetTargetKernelInstanceContext](#vxGetTargetKernelInstanceContext) called from kernel callbacks
  to retrieve the kernel instance context for reading or updating

## Memory Allocation

OpenVX does not dictate any requirements on memory allocation methods or the layout of opaque memory
objects and it does not dictate byte packing or alignment for structures on architectures. This
extension introduces a feature that allows users to implement specific functions for dedicated
hardware accelerators or CPUs, which may run remotely on separate operating systems and hardware
infrastructures. In such cases, it can be beneficial to allocate "local" memory dedicated to these
subsystems.

@anchor MemoryDiagram

![Example SoC Memory Hierarchy Diagram](OVX-SoC-memory-hierarchy-model.drawio.svg){html: width=40%, latex: width=0.7\textwidth}

Therefore, it is necessary to add a standard generic OpenVX memory allocator to support new types of
memory pools tailored for target-specific allocations. This enables efficient and flexible memory
management for remote cores, ensuring that each subsystem can utilize memory resources optimized for
its unique requirements. This extension introduces a memory allocator function,
[vxMemTargetAlloc](#vxMemTargetAlloc), which is intended for exclusive use by the user node. It allows
the allocation of memory blocks of specific sizes from designated memory pools.

This extension only defines a single generic enumeration to be used with this allocator:
[VX_MEM_POOL_ANY](#VX_MEM_POOL_ANY). Since the specific available memory pools in a system are highly
system specific, it is expected that a vendor may define extension memory pool enumerations which
can be used by user kernels implemented for their systems.

For example, the following table could be an example of a vendor extension list of memory pools
based on the arbitrary example given in the above diagram: [Example SoC Memory Hierarchy Diagram](@ref MemoryDiagram)

> [!note]
> This table is just an example for how a specific system may choose to define different mempools.

| Mempool Categories                                                                   | Mempool Name                       | Comments                                                                               |
| ------------------------------------------------------------------------------------ | ---------------------------------- | ---------------------------------------------------------------------------------------|
| Standard/Portable Memory Pool                                                        | [VX_MEM_POOL_ANY](#VX_MEM_POOL_ANY) | Unspecified memory pool                                                                |
| CPU-Dedicated Memory Pools (memory mapped to specific cores they are dedicated to)   | XYZ_MEM_POOL_L2                   | Memory pool associated with the L2 RAM of a specific CPU core                          |
| ^                                                                                    | XYZ_MEM_POOL_L3_SCRATCH           | Scratchpad memory in L3 RAM, typically used for temporary or high-speed data storage   |
| ^                                                                                    | XYZ_MEM_POOL_EXT_PERSISTENT       | External persistent memory pool for long-term data storage                             |
| Remote/Shared Memory Pools (Memory pools shared across remote cores or systems)      | XYZ_MEM_POOL_L3_SHARED            | Shared L3 cache memory pool accessible by remote cores/systems                         |
| ^                                                                                    | XYZ_MEM_POOL_EXT_SHARED_CACHED    | External shared memory pool with caching enabled                                       |
| ^                                                                                    | XYZ_MEM_POOL_P_EXT_SHARED_NONCACHED | External shared memory pool with caching disabled                                      |

During release graph, the memory allocated on each remote core must be freed via the
[vxMemTargetFree](#vxMemTargetFree) within the corresponding node **delete_func** callback.

# Kernel Callback Developer Guidelines

When a framework includes user callbacks, there are usually assumptions that the framework makes
about how those callbacks are implemented. OpenVX is no exception. The following sections contain
guidelines and assumptions that User Target Kernel callback implementers should follow for proper
usage when using both the default behavior of the OpenVX framework as well as some additional
considerations when using graph or node level timeouts.

## create_func

### Thread/blocking Implications

The [vxVerifyGraph][vxVerifyGraph] function is a blocking function which runs to completion
before returning. It calls the **create_func** callback for each node and then doesn't return until
all node create function callbacks return. Therefore, the following guidelines shall be followed:

- There shall not be any dependency on another node's create callback since it may not have executed
  yet in the sequence of calls to each node.

- There shall not be any dependency on some action that the application does after returning from
  [vxVerifyGraph][vxVerifyGraph]. For example, a blocking call called from within the
  **create_func** will result in blocking the full [vxVerifyGraph][vxVerifyGraph], potentially
  causing a deadlock if the **create_func** is waiting for further action from the same thread in
  the application which called [vxVerifyGraph][vxVerifyGraph], or from another node's create
  function.

### Memory Implications

If there is some context which needs to be accessed for the other target-side callbacks for a
given kernel, it should be created in the **create_func** since memory allocations are not allowed
in any callback except the **create_func** callback.

- The context pointer can be allocated using [vxMemTargetAlloc](#vxMemTargetAlloc). This allocator
  allows the user to allocate memory on the remote core where the kernel will run and will be seen
  by this specific kernel instance only. The following is an example of the allocation of the
  vxCannyParams data structure context:
  \code{.c}
  vxCannyParams prms = vxMemTargetAlloc(sizeof(vxCannyParams), VX_MEM_POOL_ANY);
  \endcode

- The allocated context shall be added to the target kernel instance using the
  [vxSetTargetKernelInstanceContext](#vxSetTargetKernelInstanceContext) function (so the other
  callbacks can retrieve it via [vxGetTargetKernelInstanceContext](#vxGetTargetKernelInstanceContext).
  For example for the canny edge parameters:
  \code{.c}
  vx_status status = vxSetTargetKernelInstanceContext(kernel, prms, sizeof(vxCannyParams));
  \endcode

- If the node instance needs additional memory, then it should allocate it in the **create_func**
  callback using the [vxMemTargetAlloc](#vxMemTargetAlloc) with the appropriate memory pool
  needed based on what is made available in the system by the vendor. Then the corresponding
  pointers and sizes can be added to the context structure to be accessed by the other callbacks.

## process_func

### Thread/blocking Implications

The **process_func** callback is called for each node in order of graph dependency. Therefore, upon
returning from a process function, the framework shall assume that the operations are
complete and the inputs are no longer being read, and the outputs are no longer being updated. Therefore
the process function should ensure completion of the job before returning.

### Memory Implications

- No memory allocations should be made in the **process_func** callback. Any memory
  allocations should have been created in the **create_func** (see above).

- If there is some context which needs to be accessed/updated from the **create_func** callback, it
  can be retrieved from the kernel instance using the
  [vxGetTargetKernelInstanceContext](#vxGetTargetKernelInstanceContext) function:
  \code{.c}
  status = vxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);
  \endcode

## control_func

### Data Object Verification Implications

Since the objects being used with the control callback are not necessarily node parameters, the
parameters are not subject to the validate callback checks being done during the call to
[vxVerifyGraph][vxVerifyGraph]. Therefore, if an object is to be used within a control
callback, it is necessary for the control callback (or some other mechanism within the
application, etc) to perform validation of the parameters being used within the callback.

### Thread/blocking Implications:

The **control_func** callback (if implemented) is triggered from the application by calling
[vxNodeSendCommand][vxNodeSendCommand]. The call to [vxNodeSendCommand][vxNodeSendCommand]
is blocked until the target can complete execution of the corresponding
**control_func** callback.

> [!note]
> Since the call to [vxNodeSendCommand][vxNodeSendCommand] is made asynchronous to the
> **process_func**, there is no guarantee on the order or exact time the command will get executed
> (i.e. it could get executed a few frames after it was called depending on the implementation).

### Memory Implications

- No memory allocations should happen in the **control_func** callback. Any memory
  allocations should have been created in the **create_func** (see above).

- If there is some context which needs to be accessed/updated from the **create_func** callback, it
  can be retrieved from the kernel instance using the
  [vxGetTargetKernelInstanceContext](#vxGetTargetKernelInstanceContext) function:
  \code{.c}
  status = vxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);
  \endcode

- The **control_func** callback (if implemented) should only be called after
  [vxVerifyGraph][vxVerifyGraph] and before [vxReleaseGraph][vxReleaseGraph], since it may need
  to access the kernel instance context, which only exists in the time between these two calls.
    
## delete_func

### Thread/blocking Implications

The [vxReleaseGraph][vxReleaseGraph] function is a blocking function which runs to completion before returning. It calls the **delete_func** callback for each node one by one (sequentially) and then
doesn't return until all node delete function callbacks return. Therefore, the following guidelines
shall be followed:

- There shall not be any dependency on another node's delete function since it may not have executed
  yet in the sequence of calls to each node.

- There shall not be any dependency on some action that the application does after returning from
  [vxReleaseGraph][vxReleaseGraph]. For example, a blocking call called from within the **delete_func**
  will result in blocking the full [vxReleaseGraph][vxReleaseGraph], potentially causing a deadlock
  if the **delete_func** is waiting for further action from the same thread in the application which
  called [vxReleaseGraph][vxReleaseGraph], or from another node's delete function.

### Memory Implications

All memory buffers allocated during the **create_func** should be freed in the **delete_func**:
- If there is some context which was allocated in the **create_func** callback, it can be retrieved
from the kernel instance using the
[vxGetTargetKernelInstanceContext](#vxGetTargetKernelInstanceContext) function:
\code{.c}
status = vxGetTargetKernelInstanceContext(kernel, (void **)&prms, &size);
\endcode

- If the kernel instance context included pointers/sizes to additional scratch or persistent memory
  allocated in the **create_func** callback, it should be freed in the **delete_func** callback
  using the [vxMemTargetFree](#vxMemTargetFree) function with the dedicated mempool.

- If the **create_func** allocated kernel instance context, it should be freed from the kernel
instance using the [vxMemTargetFree](#vxMemTargetFree) function:
\code{.c}
vxMemTargetFree(prms, sizeof(vxCannyParams), VX_MEM_POOL_ANY);
\endcode

[vxAddUserKernel]:
    https://registry.khronos.org/OpenVX/specs/1.3_08Aug2019/html/OpenVX_Specification_1_3.html#vxAddUserKernel
[vxAllocateUserKernelId]:
    https://registry.khronos.org/OpenVX/specs/1.3_08Aug2019/html/OpenVX_Specification_1_3.html#vxAllocateUserKernelId
[vxVerifyGraph]:
    https://registry.khronos.org/OpenVX/specs/1.3_08Aug2019/html/OpenVX_Specification_1_3.html#vxVerifyGraph
[vxReleaseGraph]:
    https://registry.khronos.org/OpenVX/specs/1.3_08Aug2019/html/OpenVX_Specification_1_3.html#vxReleaseGraph
[vxNodeSendCommand]:
    https://registry.khronos.org/OpenVX/extensions/vx_khr_node_send_command/1.0/vx_khr_node_send_command_1_0_0/vx_khr_node_send_command_1_0_0.html#vxNodeSendCommand

\defgroup group_vx_target_kernel User Target Kernel Extension
\brief This section lists the APIs required for User Target Kernels
