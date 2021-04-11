// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#include "context.hpp"
#include "program.hpp"

#pragma warning(disable: 4100)

extern CL_API_ENTRY cl_int CL_API_CALL
clCreateSubDevices(cl_device_id                         in_device,
    const cl_device_partition_property * properties,
    cl_uint                              num_devices,
    cl_device_id *                       out_devices,
    cl_uint *                            num_devices_ret) CL_API_SUFFIX__VERSION_1_2
{
    if (!in_device)
    {
        return CL_INVALID_DEVICE;
    }
    if (properties && properties[0] != 0)
    {
        // We don't support any of the partition modes, so the spec says we should return this
        return CL_INVALID_VALUE;
    }
    return CL_INVALID_DEVICE_PARTITION_COUNT;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clSetDefaultDeviceCommandQueue(cl_context           context_,
    cl_device_id         device_,
    cl_command_queue     command_queue) CL_API_SUFFIX__VERSION_2_1
{
    if (!context_)
    {
        return CL_INVALID_CONTEXT;
    }
    Context& context = *static_cast<Context*>(context_);
    auto ReportError = context.GetErrorReporter();
    if (!device_)
    {
        return ReportError("Device must not be null", CL_INVALID_DEVICE);
    }
    Device& device = *static_cast<Device*>(device_);
    if (!context.ValidDeviceForContext(device))
    {
        return ReportError("Device not valid for this context", CL_INVALID_DEVICE);
    }
    if (!command_queue)
    {
        return ReportError("Queue must not be null", CL_INVALID_COMMAND_QUEUE);
    }
    // We don't support creating on-device queues so it's impossible to call this correctly
    return ReportError("Queue is not an on-device queue", CL_INVALID_COMMAND_QUEUE);
}

extern CL_API_ENTRY cl_mem CL_API_CALL
clCreatePipe(cl_context                 context,
    cl_mem_flags               flags,
    cl_uint                    pipe_packet_size,
    cl_uint                    pipe_max_packets,
    const cl_pipe_properties * properties,
    cl_int *                   errcode_ret) CL_API_SUFFIX__VERSION_2_0
{
    if (!context)
    {
        if (errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return nullptr;
    }
    return static_cast<Context*>(context)->GetErrorReporter(errcode_ret)("Platform does not support pipes", CL_INVALID_OPERATION);
}

extern CL_API_ENTRY cl_int CL_API_CALL
clGetPipeInfo(cl_mem           pipe,
    cl_pipe_info     param_name,
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_2_0
{
    return CL_INVALID_MEM_OBJECT;
}

extern CL_API_ENTRY void * CL_API_CALL
clSVMAlloc(cl_context       context,
    cl_svm_mem_flags flags,
    size_t           size,
    cl_uint          alignment) CL_API_SUFFIX__VERSION_2_0
{
    if (context)
    {
        static_cast<Context*>(context)->GetErrorReporter()("Platform does not support SVM", CL_INVALID_OPERATION);
    }
    return nullptr;
}

extern CL_API_ENTRY void CL_API_CALL
clSVMFree(cl_context        context,
    void *            svm_pointer) CL_API_SUFFIX__VERSION_2_0
{
    if (context)
    {
        static_cast<Context*>(context)->GetErrorReporter()("Platform does not support SVM", CL_INVALID_OPERATION);
    }
}

extern CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithBuiltInKernels(cl_context            context_,
    cl_uint               num_devices,
    const cl_device_id *  device_list,
    const char *          kernel_names,
    cl_int *              errcode_ret) CL_API_SUFFIX__VERSION_1_2
{
    if (!context_)
    {
        if (errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return nullptr;
    }
    Context& context = *static_cast<Context*>(context_);
    auto ReportError = context.GetErrorReporter(errcode_ret);
    if (!device_list || !num_devices)
    {
        return ReportError("Device list must not be null", CL_INVALID_VALUE);
    }
    if (!kernel_names)
    {
        return ReportError("Kernel names must not be null", CL_INVALID_VALUE);
    }
    for (cl_uint i = 0; i < num_devices; ++i)
    {
        if (!device_list[i])
        {
            return ReportError("Device list must not contain null entries", CL_INVALID_DEVICE);
        }
        if (!context.ValidDeviceForContext(*static_cast<Device*>(device_list[i])))
        {
            return ReportError("Device list contains device that's invalid for context", CL_INVALID_DEVICE);
        }
    }
    return ReportError("No builtin kernels are supported by this platform", CL_INVALID_VALUE);
}

extern CL_API_ENTRY cl_program CL_API_CALL
clCreateProgramWithIL(cl_context    context_,
    const void*    il,
    size_t         length,
    cl_int*        errcode_ret) CL_API_SUFFIX__VERSION_2_1
{
    if (!context_)
    {
        if (errcode_ret) *errcode_ret = CL_INVALID_CONTEXT;
        return nullptr;
    }
    Context& context = *static_cast<Context*>(context_);
    auto ReportError = context.GetErrorReporter(errcode_ret);
    if (!il || !length)
    {
        return ReportError("IL must not be null and length must be nonzero", CL_INVALID_VALUE);
    }
    return ReportError("Platform does not yet support IL programs", CL_INVALID_OPERATION);
}

extern CL_API_ENTRY CL_EXT_PREFIX__VERSION_2_2_DEPRECATED cl_int CL_API_CALL
clSetProgramReleaseCallback(cl_program          program,
    void (CL_CALLBACK * pfn_notify)(cl_program program,
        void * user_data),
    void *              user_data) CL_EXT_SUFFIX__VERSION_2_2_DEPRECATED
{
    if (!program)
    {
        return CL_INVALID_PROGRAM;
    }
    Context& context = static_cast<Program*>(program)->m_Parent.get();
    return context.GetErrorReporter()("This platform does not support global program destructors", CL_INVALID_OPERATION);
}

extern CL_API_ENTRY cl_int CL_API_CALL
clSetProgramSpecializationConstant(cl_program  program,
    cl_uint     spec_id,
    size_t      spec_size,
    const void* spec_value) CL_API_SUFFIX__VERSION_2_2
{
    if (!program)
    {
        return CL_INVALID_PROGRAM;
    }
    Context& context = static_cast<Program*>(program)->m_Parent.get();
    return context.GetErrorReporter()("This platform does not yet support SPIR-V programs", CL_INVALID_OPERATION);
}

#ifdef CL_VERSION_2_1

extern CL_API_ENTRY cl_kernel CL_API_CALL
clCloneKernel(cl_kernel     source_kernel,
    cl_int*       errcode_ret) CL_API_SUFFIX__VERSION_2_1
{
    *errcode_ret = CL_INVALID_PLATFORM;
    return nullptr;
}

#endif

#ifdef CL_VERSION_2_0

extern CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArgSVMPointer(cl_kernel    kernel,
    cl_uint      arg_index,
    const void * arg_value) CL_API_SUFFIX__VERSION_2_0
{
    return CL_INVALID_PLATFORM;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clSetKernelExecInfo(cl_kernel            kernel,
    cl_kernel_exec_info  param_name,
    size_t               param_value_size,
    const void *         param_value) CL_API_SUFFIX__VERSION_2_0
{
    return CL_INVALID_PLATFORM;
}

#endif

#ifdef CL_VERSION_2_1

extern CL_API_ENTRY cl_int CL_API_CALL
clGetKernelSubGroupInfo(cl_kernel                   kernel,
    cl_device_id                device,
    cl_kernel_sub_group_info    param_name,
    size_t                      input_value_size,
    const void*                 input_value,
    size_t                      param_value_size,
    void*                       param_value,
    size_t*                     param_value_size_ret) CL_API_SUFFIX__VERSION_2_1
{
    return CL_INVALID_PLATFORM;
}

#endif

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNativeKernel(cl_command_queue  command_queue,
    void (CL_CALLBACK * user_func)(void *),
    void *            args,
    size_t            cb_args,
    cl_uint           num_mem_objects,
    const cl_mem *    mem_list,
    const void **     args_mem_loc,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_0
{
    return CL_INVALID_PLATFORM;
}

#ifdef CL_VERSION_2_0

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMFree(cl_command_queue  command_queue,
    cl_uint           num_svm_pointers,
    void *            svm_pointers[],
    void (CL_CALLBACK * pfn_free_func)(cl_command_queue queue,
        cl_uint          num_svm_pointers,
        void *           svm_pointers[],
        void *           user_data),
    void *            user_data,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_2_0
{
    return CL_INVALID_PLATFORM;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMMemcpy(cl_command_queue  command_queue,
    cl_bool           blocking_copy,
    void *            dst_ptr,
    const void *      src_ptr,
    size_t            size,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_2_0
{
    return CL_INVALID_PLATFORM;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMMemFill(cl_command_queue  command_queue,
    void *            svm_ptr,
    const void *      pattern,
    size_t            pattern_size,
    size_t            size,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_2_0
{
    return CL_INVALID_PLATFORM;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMMap(cl_command_queue  command_queue,
    cl_bool           blocking_map,
    cl_map_flags      flags,
    void *            svm_ptr,
    size_t            size,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_2_0
{
    return CL_INVALID_PLATFORM;
}

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMUnmap(cl_command_queue  command_queue,
    void *            svm_ptr,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_2_0
{
    return CL_INVALID_PLATFORM;
}

#endif

#ifdef CL_VERSION_2_1

extern CL_API_ENTRY cl_int CL_API_CALL
clEnqueueSVMMigrateMem(cl_command_queue         command_queue,
    cl_uint                  num_svm_pointers,
    const void **            svm_pointers,
    const size_t *           sizes,
    cl_mem_migration_flags   flags,
    cl_uint                  num_events_in_wait_list,
    const cl_event *         event_wait_list,
    cl_event *               event) CL_API_SUFFIX__VERSION_2_1
{
    return CL_INVALID_PLATFORM;
}

#endif

#ifdef CL_VERSION_1_2

/* Extension function access
 *
 * Returns the extension function address for the given function name,
 * or NULL if a valid function can not be found.  The client must
 * check to make sure the address is not NULL, before using or
 * calling the returned function address.
 */
extern CL_API_ENTRY void * CL_API_CALL
clGetExtensionFunctionAddressForPlatform(cl_platform_id platform,
    const char *   func_name) CL_API_SUFFIX__VERSION_1_2
{
    return nullptr;
}

#endif

/* Deprecated OpenCL 1.1 APIs */

extern CL_API_ENTRY CL_EXT_PREFIX__VERSION_1_1_DEPRECATED cl_int CL_API_CALL
clUnloadCompiler(void) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED
{
    return CL_INVALID_PLATFORM;
}

