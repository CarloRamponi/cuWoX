#include <nv_uvm_ioctl.h>
#include <log.h>

int print_uvm_call(uint8_t nr, size_t size, void *arg)
{
    switch (nr)
    {
        case UVM_RESERVE_VA:
        {
            LOG("UVM_RESERVE_VA\n");
            UVM_RESERVE_VA_PARAMS *params = (UVM_RESERVE_VA_PARAMS *)arg;
            LOG_VERBOSE("requestedBase: 0x%016lx, length: 0x%016lx, rmStatus: %d (0x%08x)\n", params->requestedBase, params->length, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_RELEASE_VA:
        {
            LOG("UVM_RELEASE_VA\n");
            UVM_RELEASE_VA_PARAMS *params = (UVM_RELEASE_VA_PARAMS *)arg;
            LOG_VERBOSE("requestedBase: 0x%016lx, length: 0x%016lx, rmStatus: %d (0x%08x)\n", params->requestedBase, params->length, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_REGION_COMMIT:
        {
            LOG("UVM_REGION_COMMIT\n");
            UVM_REGION_COMMIT_PARAMS *params = (UVM_REGION_COMMIT_PARAMS *)arg;
            LOG_VERBOSE("requestedBase: 0x%016lx, length: 0x%016lx, streamId: %d, gpuUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmStatus: %d (0x%08x)\n", params->requestedBase, params->length, params->streamId, params->gpuUuid.uuid[0], params->gpuUuid.uuid[1], params->gpuUuid.uuid[2], params->gpuUuid.uuid[3], params->gpuUuid.uuid[4], params->gpuUuid.uuid[5], params->gpuUuid.uuid[6], params->gpuUuid.uuid[7], params->gpuUuid.uuid[8], params->gpuUuid.uuid[9], params->gpuUuid.uuid[10], params->gpuUuid.uuid[11], params->gpuUuid.uuid[12], params->gpuUuid.uuid[13], params->gpuUuid.uuid[14], params->gpuUuid.uuid[15], params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_REGION_DECOMMIT:
        {
            LOG("UVM_REGION_DECOMMIT\n");
            UVM_REGION_DECOMMIT_PARAMS *params = (UVM_REGION_DECOMMIT_PARAMS *)arg;
            LOG_VERBOSE("requestedBase: 0x%016lx, length: 0x%016lx, rmStatus: %d (0x%08x)\n", params->requestedBase, params->length, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_REGION_SET_STREAM:
        {
            LOG("UVM_REGION_SET_STREAM\n");
            UVM_REGION_SET_STREAM_PARAMS *params = (UVM_REGION_SET_STREAM_PARAMS *)arg;
            LOG_VERBOSE("requestedBase: 0x%016lx, length: 0x%016lx, newStreamId: %d, gpuUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmStatus: %d (0x%08x)\n", params->requestedBase, params->length, params->newStreamId, params->gpuUuid.uuid[0], params->gpuUuid.uuid[1], params->gpuUuid.uuid[2], params->gpuUuid.uuid[3], params->gpuUuid.uuid[4], params->gpuUuid.uuid[5], params->gpuUuid.uuid[6], params->gpuUuid.uuid[7], params->gpuUuid.uuid[8], params->gpuUuid.uuid[9], params->gpuUuid.uuid[10], params->gpuUuid.uuid[11], params->gpuUuid.uuid[12], params->gpuUuid.uuid[13], params->gpuUuid.uuid[14], params->gpuUuid.uuid[15], params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_SET_STREAM_RUNNING:
        {
            LOG("UVM_SET_STREAM_RUNNING\n");
            UVM_SET_STREAM_RUNNING_PARAMS *params = (UVM_SET_STREAM_RUNNING_PARAMS *)arg;
            LOG_VERBOSE("streamId: %d, rmStatus: %d (0x%08x)\n", params->streamId, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_SET_STREAM_STOPPED:
        {
            LOG("UVM_SET_STREAM_STOPPED\n");
            UVM_SET_STREAM_STOPPED_PARAMS *params = (UVM_SET_STREAM_STOPPED_PARAMS *)arg;
            LOG_VERBOSE("nStreams: %d, rmStatus: %d (0x%08x)\n", params->nStreams, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_RUN_TEST:
        {
            LOG("UVM_RUN_TEST\n");
            UVM_RUN_TEST_PARAMS *params = (UVM_RUN_TEST_PARAMS *)arg;
            LOG_VERBOSE("gpuUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, test: %d, peerGpuUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, peerId: %d, rmStatus: %d (0x%08x)\n", params->gpuUuid.uuid[0], params->gpuUuid.uuid[1], params->gpuUuid.uuid[2], params->gpuUuid.uuid[3], params->gpuUuid.uuid[4], params->gpuUuid.uuid[5], params->gpuUuid.uuid[6], params->gpuUuid.uuid[7], params->gpuUuid.uuid[8], params->gpuUuid.uuid[9], params->gpuUuid.uuid[10], params->gpuUuid.uuid[11], params->gpuUuid.uuid[12], params->gpuUuid.uuid[13], params->gpuUuid.uuid[14], params->gpuUuid.uuid[15], params->test, params->multiGpu.peerGpuUuid.uuid[0], params->multiGpu.peerGpuUuid.uuid[1], params->multiGpu.peerGpuUuid.uuid[2], params->multiGpu.peerGpuUuid.uuid[3], params->multiGpu.peerGpuUuid.uuid[4], params->multiGpu.peerGpuUuid.uuid[5], params->multiGpu.peerGpuUuid.uuid[6], params->multiGpu.peerGpuUuid.uuid[7], params->multiGpu.peerGpuUuid.uuid[8], params->multiGpu.peerGpuUuid.uuid[9], params->multiGpu.peerGpuUuid.uuid[10], params->multiGpu.peerGpuUuid.uuid[11], params->multiGpu.peerGpuUuid.uuid[12], params->multiGpu.peerGpuUuid.uuid[13], params->multiGpu.peerGpuUuid.uuid[14], params->multiGpu.peerGpuUuid.uuid[15], params->multiGpu.peerId, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_ADD_SESSION:
        {
            LOG("UVM_ADD_SESSION\n");
            UVM_ADD_SESSION_PARAMS *params = (UVM_ADD_SESSION_PARAMS *)arg;
            LOG_VERBOSE("pidTarget: %d, countersBaseAddress: 0x%016lx, sessionIndex: %d, rmStatus: %d (0x%08x)\n", params->pidTarget, params->countersBaseAddress, params->sessionIndex, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_REMOVE_SESSION:
        {
            LOG("UVM_REMOVE_SESSION\n");
            UVM_REMOVE_SESSION_PARAMS *params = (UVM_REMOVE_SESSION_PARAMS *)arg;
            LOG_VERBOSE("sessionIndex: %d, rmStatus: %d (0x%08x)\n", params->sessionIndex, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_ENABLE_COUNTERS:
        {
            LOG("UVM_ENABLE_COUNTERS\n");
            UVM_ENABLE_COUNTERS_PARAMS *params = (UVM_ENABLE_COUNTERS_PARAMS *)arg;
            LOG_VERBOSE("sessionIndex: %d, count: %d, rmStatus: %d (0x%08x)\n", params->sessionIndex, params->count, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_MAP_COUNTER:
        {
            LOG("UVM_MAP_COUNTER\n");
            UVM_MAP_COUNTER_PARAMS *params = (UVM_MAP_COUNTER_PARAMS *)arg;
            LOG_VERBOSE("sessionIndex: %d, scope: %d, counterName: %d, gpuUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, addr: 0x%016lx, rmStatus: %d (0x%08x)\n", params->sessionIndex, params->scope, params->counterName, params->gpuUuid.uuid[0], params->gpuUuid.uuid[1], params->gpuUuid.uuid[2], params->gpuUuid.uuid[3], params->gpuUuid.uuid[4], params->gpuUuid.uuid[5], params->gpuUuid.uuid[6], params->gpuUuid.uuid[7], params->gpuUuid.uuid[8], params->gpuUuid.uuid[9], params->gpuUuid.uuid[10], params->gpuUuid.uuid[11], params->gpuUuid.uuid[12], params->gpuUuid.uuid[13], params->gpuUuid.uuid[14], params->gpuUuid.uuid[15], params->addr, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_CREATE_EVENT_QUEUE:
        {
            LOG("UVM_CREATE_EVENT_QUEUE\n");
            UVM_CREATE_EVENT_QUEUE_PARAMS *params = (UVM_CREATE_EVENT_QUEUE_PARAMS *)arg;
            LOG_VERBOSE("eventQueueIndex: %d, queueSize: 0x%016lx, notificationCount: 0x%016lx, timeStampType: %d, rmStatus: %d (0x%08x)\n", params->eventQueueIndex, params->queueSize, params->notificationCount, params->timeStampType, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_REMOVE_EVENT_QUEUE:
        {
            LOG("UVM_REMOVE_EVENT_QUEUE\n");
            UVM_REMOVE_EVENT_QUEUE_PARAMS *params = (UVM_REMOVE_EVENT_QUEUE_PARAMS *)arg;
            LOG_VERBOSE("sessionIndex: %d, eventQueueIndex: %d, rmStatus: %d (0x%08x)\n", params->sessionIndex, params->eventQueueIndex, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_MAP_EVENT_QUEUE:
        {
            LOG("UVM_MAP_EVENT_QUEUE\n");
            UVM_MAP_EVENT_QUEUE_PARAMS *params = (UVM_MAP_EVENT_QUEUE_PARAMS *)arg;
            LOG_VERBOSE("sessionIndex: %d, eventQueueIndex: %d, userRODataAddr: 0x%016lx, userRWDataAddr: 0x%016lx, readIndexAddr: 0x%016lx, writeIndexAddr: 0x%016lx, queueBufferAddr: 0x%016lx, rmStatus: %d (0x%08x)\n", params->sessionIndex, params->eventQueueIndex, params->userRODataAddr, params->userRWDataAddr, params->readIndexAddr, params->writeIndexAddr, params->queueBufferAddr, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_EVENT_CTRL:
        {
            LOG("UVM_EVENT_CTRL\n");
            UVM_EVENT_CTRL_PARAMS *params = (UVM_EVENT_CTRL_PARAMS *)arg;
            LOG_VERBOSE("sessionIndex: %d, eventQueueIndex: %d, eventType: %d, enable: %d, rmStatus: %d (0x%08x)\n", params->sessionIndex, params->eventQueueIndex, params->eventType, params->enable, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_REGISTER_MPS_SERVER:
        {
            LOG("UVM_REGISTER_MPS_SERVER\n");
            UVM_REGISTER_MPS_SERVER_PARAMS *params = (UVM_REGISTER_MPS_SERVER_PARAMS *)arg;
            LOG_VERBOSE("numGpus: %d, serverId: 0x%016lx, rmStatus: %d (0x%08x)\n", params->numGpus, params->serverId, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_REGISTER_MPS_CLIENT:
        {
            LOG("UVM_REGISTER_MPS_CLIENT\n");
            UVM_REGISTER_MPS_CLIENT_PARAMS *params = (UVM_REGISTER_MPS_CLIENT_PARAMS *)arg;
            LOG_VERBOSE("serverId: 0x%016lx, rmStatus: %d (0x%08x)\n", params->serverId, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_GET_GPU_UUID_TABLE:
        {
            LOG("UVM_GET_GPU_UUID_TABLE\n");
            UVM_GET_GPU_UUID_TABLE_PARAMS *params = (UVM_GET_GPU_UUID_TABLE_PARAMS *)arg;
            LOG_VERBOSE("validCount: %d, rmStatus: %d (0x%08x)\n", params->validCount, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_CREATE_RANGE_GROUP:
        {
            LOG("UVM_CREATE_RANGE_GROUP\n");
            UVM_CREATE_RANGE_GROUP_PARAMS *params = (UVM_CREATE_RANGE_GROUP_PARAMS *)arg;
            LOG_VERBOSE("rangeGroupId: 0x%016lx, rmStatus: %d (0x%08x)\n", params->rangeGroupId, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_DESTROY_RANGE_GROUP:
        {
            LOG("UVM_DESTROY_RANGE_GROUP\n");
            UVM_DESTROY_RANGE_GROUP_PARAMS *params = (UVM_DESTROY_RANGE_GROUP_PARAMS *)arg;
            LOG_VERBOSE("rangeGroupId: 0x%016lx, rmStatus: %d (0x%08x)\n", params->rangeGroupId, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_REGISTER_GPU_VASPACE:
        {
            LOG("UVM_REGISTER_GPU_VASPACE\n");
            UVM_REGISTER_GPU_VASPACE_PARAMS *params = (UVM_REGISTER_GPU_VASPACE_PARAMS *)arg;
            LOG_VERBOSE("gpuUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmCtrlFd: %d, hClient: %d, hVaSpace: %d, rmStatus: %d (0x%08x)\n", params->gpuUuid.uuid[0], params->gpuUuid.uuid[1], params->gpuUuid.uuid[2], params->gpuUuid.uuid[3], params->gpuUuid.uuid[4], params->gpuUuid.uuid[5], params->gpuUuid.uuid[6], params->gpuUuid.uuid[7], params->gpuUuid.uuid[8], params->gpuUuid.uuid[9], params->gpuUuid.uuid[10], params->gpuUuid.uuid[11], params->gpuUuid.uuid[12], params->gpuUuid.uuid[13], params->gpuUuid.uuid[14], params->gpuUuid.uuid[15], params->rmCtrlFd, params->hClient, params->hVaSpace, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_UNREGISTER_GPU_VASPACE:
        {
            LOG("UVM_UNREGISTER_GPU_VASPACE\n");
            UVM_UNREGISTER_GPU_VASPACE_PARAMS *params = (UVM_UNREGISTER_GPU_VASPACE_PARAMS *)arg;
            LOG_VERBOSE("gpuUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmStatus: %d (0x%08x)\n", params->gpuUuid.uuid[0], params->gpuUuid.uuid[1], params->gpuUuid.uuid[2], params->gpuUuid.uuid[3], params->gpuUuid.uuid[4], params->gpuUuid.uuid[5], params->gpuUuid.uuid[6], params->gpuUuid.uuid[7], params->gpuUuid.uuid[8], params->gpuUuid.uuid[9], params->gpuUuid.uuid[10], params->gpuUuid.uuid[11], params->gpuUuid.uuid[12], params->gpuUuid.uuid[13], params->gpuUuid.uuid[14], params->gpuUuid.uuid[15], params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_REGISTER_CHANNEL:
        {
            LOG("UVM_REGISTER_CHANNEL\n");
            UVM_REGISTER_CHANNEL_PARAMS *params = (UVM_REGISTER_CHANNEL_PARAMS *)arg;
            LOG_VERBOSE("gpuUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmCtrlFd: %d, hClient: %d, hChannel: %d, base: 0x%016lx, length: 0x%016lx, rmStatus: %d (0x%08x)\n", params->gpuUuid.uuid[0], params->gpuUuid.uuid[1], params->gpuUuid.uuid[2], params->gpuUuid.uuid[3], params->gpuUuid.uuid[4], params->gpuUuid.uuid[5], params->gpuUuid.uuid[6], params->gpuUuid.uuid[7], params->gpuUuid.uuid[8], params->gpuUuid.uuid[9], params->gpuUuid.uuid[10], params->gpuUuid.uuid[11], params->gpuUuid.uuid[12], params->gpuUuid.uuid[13], params->gpuUuid.uuid[14], params->gpuUuid.uuid[15], params->rmCtrlFd, params->hClient, params->hChannel, params->base, params->length, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_UNREGISTER_CHANNEL:
        {
            LOG("UVM_UNREGISTER_CHANNEL\n");
            UVM_UNREGISTER_CHANNEL_PARAMS *params = (UVM_UNREGISTER_CHANNEL_PARAMS *)arg;
            LOG_VERBOSE("gpuUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, hClient: %d, hChannel: %d, rmStatus: %d (0x%08x)\n", params->gpuUuid.uuid[0], params->gpuUuid.uuid[1], params->gpuUuid.uuid[2], params->gpuUuid.uuid[3], params->gpuUuid.uuid[4], params->gpuUuid.uuid[5], params->gpuUuid.uuid[6], params->gpuUuid.uuid[7], params->gpuUuid.uuid[8], params->gpuUuid.uuid[9], params->gpuUuid.uuid[10], params->gpuUuid.uuid[11], params->gpuUuid.uuid[12], params->gpuUuid.uuid[13], params->gpuUuid.uuid[14], params->gpuUuid.uuid[15], params->hClient, params->hChannel, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_ENABLE_PEER_ACCESS:
        {
            LOG("UVM_ENABLE_PEER_ACCESS\n");
            UVM_ENABLE_PEER_ACCESS_PARAMS *params = (UVM_ENABLE_PEER_ACCESS_PARAMS *)arg;
            LOG_VERBOSE("gpuUuidA: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, gpuUuidB: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmStatus: %d (0x%08x)\n", params->gpuUuidA.uuid[0], params->gpuUuidA.uuid[1], params->gpuUuidA.uuid[2], params->gpuUuidA.uuid[3], params->gpuUuidA.uuid[4], params->gpuUuidA.uuid[5], params->gpuUuidA.uuid[6], params->gpuUuidA.uuid[7], params->gpuUuidA.uuid[8], params->gpuUuidA.uuid[9], params->gpuUuidA.uuid[10], params->gpuUuidA.uuid[11], params->gpuUuidA.uuid[12], params->gpuUuidA.uuid[13], params->gpuUuidA.uuid[14], params->gpuUuidA.uuid[15], params->gpuUuidB.uuid[0], params->gpuUuidB.uuid[1], params->gpuUuidB.uuid[2], params->gpuUuidB.uuid[3], params->gpuUuidB.uuid[4], params->gpuUuidB.uuid[5], params->gpuUuidB.uuid[6], params->gpuUuidB.uuid[7], params->gpuUuidB.uuid[8], params->gpuUuidB.uuid[9], params->gpuUuidB.uuid[10], params->gpuUuidB.uuid[11], params->gpuUuidB.uuid[12], params->gpuUuidB.uuid[13], params->gpuUuidB.uuid[14], params->gpuUuidB.uuid[15], params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_DISABLE_PEER_ACCESS:
        {
            LOG("UVM_DISABLE_PEER_ACCESS\n");
            UVM_DISABLE_PEER_ACCESS_PARAMS *params = (UVM_DISABLE_PEER_ACCESS_PARAMS *)arg;
            LOG_VERBOSE("gpuUuidA: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, gpuUuidB: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmStatus: %d (0x%08x)\n", params->gpuUuidA.uuid[0], params->gpuUuidA.uuid[1], params->gpuUuidA.uuid[2], params->gpuUuidA.uuid[3], params->gpuUuidA.uuid[4], params->gpuUuidA.uuid[5], params->gpuUuidA.uuid[6], params->gpuUuidA.uuid[7], params->gpuUuidA.uuid[8], params->gpuUuidA.uuid[9], params->gpuUuidA.uuid[10], params->gpuUuidA.uuid[11], params->gpuUuidA.uuid[12], params->gpuUuidA.uuid[13], params->gpuUuidA.uuid[14], params->gpuUuidA.uuid[15], params->gpuUuidB.uuid[0], params->gpuUuidB.uuid[1], params->gpuUuidB.uuid[2], params->gpuUuidB.uuid[3], params->gpuUuidB.uuid[4], params->gpuUuidB.uuid[5], params->gpuUuidB.uuid[6], params->gpuUuidB.uuid[7], params->gpuUuidB.uuid[8], params->gpuUuidB.uuid[9], params->gpuUuidB.uuid[10], params->gpuUuidB.uuid[11], params->gpuUuidB.uuid[12], params->gpuUuidB.uuid[13], params->gpuUuidB.uuid[14], params->gpuUuidB.uuid[15], params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_SET_RANGE_GROUP:
        {
            LOG("UVM_SET_RANGE_GROUP\n");
            UVM_SET_RANGE_GROUP_PARAMS *params = (UVM_SET_RANGE_GROUP_PARAMS *)arg;
            LOG_VERBOSE("rangeGroupId: 0x%016lx, requestedBase: 0x%016lx, length: 0x%016lx, rmStatus: %d (0x%08x)\n", params->rangeGroupId, params->requestedBase, params->length, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_MAP_EXTERNAL_ALLOCATION:
        {
            LOG("UVM_MAP_EXTERNAL_ALLOCATION\n");
            UVM_MAP_EXTERNAL_ALLOCATION_PARAMS *params = (UVM_MAP_EXTERNAL_ALLOCATION_PARAMS *)arg;
            LOG_VERBOSE("base: 0x%016lx, length: 0x%016lx, offset: 0x%016lx, gpuAttributesCount: 0x%016lx, rmCtrlFd: %d, hClient: %d, hMemory: 0x%08x, rmStatus: %d (0x%08x)\n", params->base, params->length, params->offset, params->gpuAttributesCount, params->rmCtrlFd, params->hClient, params->hMemory, params->rmStatus, params->rmStatus);
            // for(int i = 0; i < params->gpuAttributesCount; i++) {
            //     LOG_VERBOSE("GPU %d:\n\tgpuCachingType: 0x%08x\n\tgpuCompressionType: 0x%08x\n\tgpuElementBits: 0x%08x\n\tgpuFormatType: 0x%08x\n\tgpuMappingType: 0x%08x\n",
            //         i,
            //         params->perGpuAttributes[i].gpuCachingType,
            //         params->perGpuAttributes[i].gpuCompressionType,
            //         params->perGpuAttributes[i].gpuElementBits,
            //         params->perGpuAttributes[i].gpuFormatType,
            //         params->perGpuAttributes[i].gpuMappingType
            //     );
            // }

            return 1;
        }
        case UVM_FREE:
        {
            LOG("UVM_FREE\n");
            UVM_FREE_PARAMS *params = (UVM_FREE_PARAMS *)arg;
            LOG_VERBOSE("base: 0x%016lx, length: 0x%016lx, rmStatus: %d (0x%08x)\n", params->base, params->length, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_MEM_MAP:
        {
            LOG("UVM_MEM_MAP\n");
            UVM_MEM_MAP_PARAMS *params = (UVM_MEM_MAP_PARAMS *)arg;
            LOG_VERBOSE("regionBase: 0x%016lx, regionLength: 0x%016lx, rmStatus: %d (0x%08x)\n", params->regionBase, params->regionLength, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_DEBUG_ACCESS_MEMORY:
        {
            LOG("UVM_DEBUG_ACCESS_MEMORY\n");
            UVM_DEBUG_ACCESS_MEMORY_PARAMS *params = (UVM_DEBUG_ACCESS_MEMORY_PARAMS *)arg;
            LOG_VERBOSE("baseAddress: 0x%016lx, sizeInBytes: 0x%016lx, accessType: %d, buffer: 0x%016lx, isBitmaskSet: %d, bitmask: 0x%016lx, rmStatus: %d (0x%08x)\n", params->baseAddress, params->sizeInBytes, params->accessType, params->buffer, params->isBitmaskSet, params->bitmask, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_REGISTER_GPU:
        {
            LOG("UVM_REGISTER_GPU\n");
            UVM_REGISTER_GPU_PARAMS *params = (UVM_REGISTER_GPU_PARAMS *)arg;
            LOG_VERBOSE("gpu_uuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, numaEnabled: %d, numaNodeId: %d, rmCtrlFd: %d, hClient: %d, hSmcPartRef: %d, rmStatus: %d (0x%08x)\n", params->gpu_uuid.uuid[0], params->gpu_uuid.uuid[1], params->gpu_uuid.uuid[2], params->gpu_uuid.uuid[3], params->gpu_uuid.uuid[4], params->gpu_uuid.uuid[5], params->gpu_uuid.uuid[6], params->gpu_uuid.uuid[7], params->gpu_uuid.uuid[8], params->gpu_uuid.uuid[9], params->gpu_uuid.uuid[10], params->gpu_uuid.uuid[11], params->gpu_uuid.uuid[12], params->gpu_uuid.uuid[13], params->gpu_uuid.uuid[14], params->gpu_uuid.uuid[15], params->numaEnabled, params->numaNodeId, params->rmCtrlFd, params->hClient, params->hSmcPartRef, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_UNREGISTER_GPU:
        {
            LOG("UVM_UNREGISTER_GPU\n");
            UVM_UNREGISTER_GPU_PARAMS *params = (UVM_UNREGISTER_GPU_PARAMS *)arg;
            LOG_VERBOSE("gpu_uuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmStatus: %d (0x%08x)\n", params->gpu_uuid.uuid[0], params->gpu_uuid.uuid[1], params->gpu_uuid.uuid[2], params->gpu_uuid.uuid[3], params->gpu_uuid.uuid[4], params->gpu_uuid.uuid[5], params->gpu_uuid.uuid[6], params->gpu_uuid.uuid[7], params->gpu_uuid.uuid[8], params->gpu_uuid.uuid[9], params->gpu_uuid.uuid[10], params->gpu_uuid.uuid[11], params->gpu_uuid.uuid[12], params->gpu_uuid.uuid[13], params->gpu_uuid.uuid[14], params->gpu_uuid.uuid[15], params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_PAGEABLE_MEM_ACCESS_ON_GPU:
        {
            LOG("UVM_PAGEABLE_MEM_ACCESS_ON_GPU\n");
            UVM_PAGEABLE_MEM_ACCESS_ON_GPU_PARAMS *params = (UVM_PAGEABLE_MEM_ACCESS_ON_GPU_PARAMS *)arg;
            LOG_VERBOSE("gpu_uuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, pageableMemAccess: %d, rmStatus: %d (0x%08x)\n", params->gpu_uuid.uuid[0], params->gpu_uuid.uuid[1], params->gpu_uuid.uuid[2], params->gpu_uuid.uuid[3], params->gpu_uuid.uuid[4], params->gpu_uuid.uuid[5], params->gpu_uuid.uuid[6], params->gpu_uuid.uuid[7], params->gpu_uuid.uuid[8], params->gpu_uuid.uuid[9], params->gpu_uuid.uuid[10], params->gpu_uuid.uuid[11], params->gpu_uuid.uuid[12], params->gpu_uuid.uuid[13], params->gpu_uuid.uuid[14], params->gpu_uuid.uuid[15], params->pageableMemAccess, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_PREVENT_MIGRATION_RANGE_GROUPS:
        {
            LOG("UVM_PREVENT_MIGRATION_RANGE_GROUPS\n");
            UVM_PREVENT_MIGRATION_RANGE_GROUPS_PARAMS *params = (UVM_PREVENT_MIGRATION_RANGE_GROUPS_PARAMS *)arg;
            LOG_VERBOSE("rangeGroupIds: %p, numGroupIds: %ld, rmStatus: %d (0x%08x)\n", params->rangeGroupIds, params->numGroupIds, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_ALLOW_MIGRATION_RANGE_GROUPS:
        {
            LOG("UVM_ALLOW_MIGRATION_RANGE_GROUPS\n");
            UVM_ALLOW_MIGRATION_RANGE_GROUPS_PARAMS *params = (UVM_ALLOW_MIGRATION_RANGE_GROUPS_PARAMS *)arg;
            LOG_VERBOSE("rangeGroupIds: %p, numGroupIds: %ld, rmStatus: %d (0x%08x)\n", params->rangeGroupIds, params->numGroupIds, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_SET_PREFERRED_LOCATION:
        {
            LOG("UVM_SET_PREFERRED_LOCATION\n");
            // UVM_SET_PREFERRED_LOCATION_PARAMS *params = (UVM_SET_PREFERRED_LOCATION_PARAMS *)arg;
            // LOG_VERBOSE("requestedBase: 0x%016lx, length: 0x%016lx, preferredLocation: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, preferredCpuNumaNode: %d, rmStatus: %d (0x%08x)\n", params->requestedBase, params->length, params->preferredLocation.uuid[0], params->preferredLocation.uuid[1], params->preferredLocation.uuid[2], params->preferredLocation.uuid[3], params->preferredLocation.uuid[4], params->preferredLocation.uuid[5], params->preferredLocation.uuid[6], params->preferredLocation.uuid[7], params->preferredLocation.uuid[8], params->preferredLocation.uuid[9], params->preferredLocation.uuid[10], params->preferredLocation.uuid[11], params->preferredLocation.uuid[12], params->preferredLocation.uuid[13], params->preferredLocation.uuid[14], params->preferredLocation.uuid[15], params->preferredCpuNumaNode, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_UNSET_PREFERRED_LOCATION:
        {
            LOG("UVM_UNSET_PREFERRED_LOCATION\n");
            // UVM_UNSET_PREFERRED_LOCATION_PARAMS *params = (UVM_UNSET_PREFERRED_LOCATION_PARAMS *)arg;
            // LOG_VERBOSE("requestedBase: 0x%016lx, length: 0x%016lx, rmStatus: %d (0x%08x)\n", params->requestedBase, params->length, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_ENABLE_READ_DUPLICATION:
        {
            LOG("UVM_ENABLE_READ_DUPLICATION\n");
            UVM_ENABLE_READ_DUPLICATION_PARAMS *params = (UVM_ENABLE_READ_DUPLICATION_PARAMS *)arg;
            LOG_VERBOSE("requestedBase: 0x%016lx, length: 0x%016lx, rmStatus: %d (0x%08x)\n", params->requestedBase, params->length, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_DISABLE_READ_DUPLICATION:
        {
            LOG("UVM_DISABLE_READ_DUPLICATION\n");
            UVM_DISABLE_READ_DUPLICATION_PARAMS *params = (UVM_DISABLE_READ_DUPLICATION_PARAMS *)arg;
            LOG_VERBOSE("requestedBase: 0x%016lx, length: 0x%016lx, rmStatus: %d (0x%08x)\n", params->requestedBase, params->length, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_UNSET_ACCESSED_BY:
        {
            LOG("UVM_UNSET_ACCESSED_BY\n");
            UVM_UNSET_ACCESSED_BY_PARAMS *params = (UVM_UNSET_ACCESSED_BY_PARAMS *)arg;
            LOG_VERBOSE("requestedBase: 0x%016lx, length: 0x%016lx, accessedByUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmStatus: %d (0x%08x)\n", params->requestedBase, params->length, params->accessedByUuid.uuid[0], params->accessedByUuid.uuid[1], params->accessedByUuid.uuid[2], params->accessedByUuid.uuid[3], params->accessedByUuid.uuid[4], params->accessedByUuid.uuid[5], params->accessedByUuid.uuid[6], params->accessedByUuid.uuid[7], params->accessedByUuid.uuid[8], params->accessedByUuid.uuid[9], params->accessedByUuid.uuid[10], params->accessedByUuid.uuid[11], params->accessedByUuid.uuid[12], params->accessedByUuid.uuid[13], params->accessedByUuid.uuid[14], params->accessedByUuid.uuid[15], params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_MIGRATE:
        {
            LOG("UVM_MIGRATE\n");
            UVM_MIGRATE_PARAMS *params = (UVM_MIGRATE_PARAMS *)arg;
            LOG_VERBOSE("base: 0x%016lx, length: 0x%016lx, destinationUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, flags: %d, semaphoreAddress: 0x%016lx, semaphorePayload: %d, cpuNumaNode: %d, userSpaceStart: 0x%016lx, userSpaceLength: 0x%016lx, rmStatus: %d (0x%08x)\n", params->base, params->length, params->destinationUuid.uuid[0], params->destinationUuid.uuid[1], params->destinationUuid.uuid[2], params->destinationUuid.uuid[3], params->destinationUuid.uuid[4], params->destinationUuid.uuid[5], params->destinationUuid.uuid[6], params->destinationUuid.uuid[7], params->destinationUuid.uuid[8], params->destinationUuid.uuid[9], params->destinationUuid.uuid[10], params->destinationUuid.uuid[11], params->destinationUuid.uuid[12], params->destinationUuid.uuid[13], params->destinationUuid.uuid[14], params->destinationUuid.uuid[15], params->flags, params->semaphoreAddress, params->semaphorePayload, params->cpuNumaNode, params->userSpaceStart, params->userSpaceLength, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_MIGRATE_RANGE_GROUP:
        {
            LOG("UVM_MIGRATE_RANGE_GROUP\n");
            UVM_MIGRATE_RANGE_GROUP_PARAMS *params = (UVM_MIGRATE_RANGE_GROUP_PARAMS *)arg;
            LOG_VERBOSE("rangeGroupId: 0x%016lx, destinationUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmStatus: %d (0x%08x)\n", params->rangeGroupId, params->destinationUuid.uuid[0], params->destinationUuid.uuid[1], params->destinationUuid.uuid[2], params->destinationUuid.uuid[3], params->destinationUuid.uuid[4], params->destinationUuid.uuid[5], params->destinationUuid.uuid[6], params->destinationUuid.uuid[7], params->destinationUuid.uuid[8], params->destinationUuid.uuid[9], params->destinationUuid.uuid[10], params->destinationUuid.uuid[11], params->destinationUuid.uuid[12], params->destinationUuid.uuid[13], params->destinationUuid.uuid[14], params->destinationUuid.uuid[15], params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_ENABLE_SYSTEM_WIDE_ATOMICS:
        {
            LOG("UVM_ENABLE_SYSTEM_WIDE_ATOMICS\n");
            UVM_ENABLE_SYSTEM_WIDE_ATOMICS_PARAMS *params = (UVM_ENABLE_SYSTEM_WIDE_ATOMICS_PARAMS *)arg;
            LOG_VERBOSE("gpu_uuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmStatus: %d (0x%08x)\n", params->gpu_uuid.uuid[0], params->gpu_uuid.uuid[1], params->gpu_uuid.uuid[2], params->gpu_uuid.uuid[3], params->gpu_uuid.uuid[4], params->gpu_uuid.uuid[5], params->gpu_uuid.uuid[6], params->gpu_uuid.uuid[7], params->gpu_uuid.uuid[8], params->gpu_uuid.uuid[9], params->gpu_uuid.uuid[10], params->gpu_uuid.uuid[11], params->gpu_uuid.uuid[12], params->gpu_uuid.uuid[13], params->gpu_uuid.uuid[14], params->gpu_uuid.uuid[15], params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_DISABLE_SYSTEM_WIDE_ATOMICS:
        {
            LOG("UVM_DISABLE_SYSTEM_WIDE_ATOMICS\n");
            UVM_DISABLE_SYSTEM_WIDE_ATOMICS_PARAMS *params = (UVM_DISABLE_SYSTEM_WIDE_ATOMICS_PARAMS *)arg;
            LOG_VERBOSE("gpu_uuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmStatus: %d (0x%08x)\n", params->gpu_uuid.uuid[0], params->gpu_uuid.uuid[1], params->gpu_uuid.uuid[2], params->gpu_uuid.uuid[3], params->gpu_uuid.uuid[4], params->gpu_uuid.uuid[5], params->gpu_uuid.uuid[6], params->gpu_uuid.uuid[7], params->gpu_uuid.uuid[8], params->gpu_uuid.uuid[9], params->gpu_uuid.uuid[10], params->gpu_uuid.uuid[11], params->gpu_uuid.uuid[12], params->gpu_uuid.uuid[13], params->gpu_uuid.uuid[14], params->gpu_uuid.uuid[15], params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_TOOLS_INIT_EVENT_TRACKER:
        {
            LOG("UVM_TOOLS_INIT_EVENT_TRACKER\n");
            UVM_TOOLS_INIT_EVENT_TRACKER_PARAMS *params = (UVM_TOOLS_INIT_EVENT_TRACKER_PARAMS *)arg;
            LOG_VERBOSE("queueBuffer: 0x%016lx, queueBufferSize: 0x%016lx, controlBuffer: 0x%016lx, processor: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, allProcessors: %d, uvmFd: %d, version: %d, rmStatus: %d (0x%08x)\n", params->queueBuffer, params->queueBufferSize, params->controlBuffer, params->processor.uuid[0], params->processor.uuid[1], params->processor.uuid[2], params->processor.uuid[3], params->processor.uuid[4], params->processor.uuid[5], params->processor.uuid[6], params->processor.uuid[7], params->processor.uuid[8], params->processor.uuid[9], params->processor.uuid[10], params->processor.uuid[11], params->processor.uuid[12], params->processor.uuid[13], params->processor.uuid[14], params->processor.uuid[15], params->allProcessors, params->uvmFd, params->version, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_TOOLS_SET_NOTIFICATION_THRESHOLD:
        {
            LOG("UVM_TOOLS_SET_NOTIFICATION_THRESHOLD\n");
            UVM_TOOLS_SET_NOTIFICATION_THRESHOLD_PARAMS *params = (UVM_TOOLS_SET_NOTIFICATION_THRESHOLD_PARAMS *)arg;
            LOG_VERBOSE("notificationThreshold: %d, rmStatus: %d (0x%08x)\n", params->notificationThreshold, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_TOOLS_EVENT_QUEUE_ENABLE_EVENTS:
        {
            LOG("UVM_TOOLS_EVENT_QUEUE_ENABLE_EVENTS\n");
            UVM_TOOLS_EVENT_QUEUE_ENABLE_EVENTS_PARAMS *params = (UVM_TOOLS_EVENT_QUEUE_ENABLE_EVENTS_PARAMS *)arg;
            LOG_VERBOSE("eventTypeFlags: 0x%016lx, rmStatus: %d (0x%08x)\n", params->eventTypeFlags, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_TOOLS_EVENT_QUEUE_DISABLE_EVENTS:
        {
            LOG("UVM_TOOLS_EVENT_QUEUE_DISABLE_EVENTS\n");
            UVM_TOOLS_EVENT_QUEUE_DISABLE_EVENTS_PARAMS *params = (UVM_TOOLS_EVENT_QUEUE_DISABLE_EVENTS_PARAMS *)arg;
            LOG_VERBOSE("eventTypeFlags: 0x%016lx, rmStatus: %d (0x%08x)\n", params->eventTypeFlags, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_TOOLS_ENABLE_COUNTERS:
        {
            LOG("UVM_TOOLS_ENABLE_COUNTERS\n");
            UVM_TOOLS_ENABLE_COUNTERS_PARAMS *params = (UVM_TOOLS_ENABLE_COUNTERS_PARAMS *)arg;
            LOG_VERBOSE("counterTypeFlags: 0x%016lx, rmStatus: %d (0x%08x)\n", params->counterTypeFlags, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_TOOLS_DISABLE_COUNTERS:
        {
            LOG("UVM_TOOLS_DISABLE_COUNTERS\n");
            UVM_TOOLS_DISABLE_COUNTERS_PARAMS *params = (UVM_TOOLS_DISABLE_COUNTERS_PARAMS *)arg;
            LOG_VERBOSE("counterTypeFlags: 0x%016lx, rmStatus: %d (0x%08x)\n", params->counterTypeFlags, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_TOOLS_READ_PROCESS_MEMORY:
        {
            LOG("UVM_TOOLS_READ_PROCESS_MEMORY\n");
            UVM_TOOLS_READ_PROCESS_MEMORY_PARAMS *params = (UVM_TOOLS_READ_PROCESS_MEMORY_PARAMS *)arg;
            LOG_VERBOSE("buffer: 0x%016lx, size: 0x%016lx, targetVa: 0x%016lx, bytesRead: 0x%016lx, rmStatus: %d (0x%08x)\n", params->buffer, params->size, params->targetVa, params->bytesRead, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_TOOLS_WRITE_PROCESS_MEMORY:
        {
            LOG("UVM_TOOLS_WRITE_PROCESS_MEMORY\n");
            UVM_TOOLS_WRITE_PROCESS_MEMORY_PARAMS *params = (UVM_TOOLS_WRITE_PROCESS_MEMORY_PARAMS *)arg;
            LOG_VERBOSE("buffer: 0x%016lx, size: 0x%016lx, targetVa: 0x%016lx, bytesWritten: 0x%016lx, rmStatus: %d (0x%08x)\n", params->buffer, params->size, params->targetVa, params->bytesWritten, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_TOOLS_GET_PROCESSOR_UUID_TABLE:
        {
            LOG("UVM_TOOLS_GET_PROCESSOR_UUID_TABLE\n");
            UVM_TOOLS_GET_PROCESSOR_UUID_TABLE_PARAMS *params = (UVM_TOOLS_GET_PROCESSOR_UUID_TABLE_PARAMS *)arg;
            LOG_VERBOSE("tablePtr: 0x%016lx, version: %d, rmStatus: %d (0x%08x)\n", params->tablePtr, params->version, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_MAP_DYNAMIC_PARALLELISM_REGION:
        {
            LOG("UVM_MAP_DYNAMIC_PARALLELISM_REGION\n");
            UVM_MAP_DYNAMIC_PARALLELISM_REGION_PARAMS *params = (UVM_MAP_DYNAMIC_PARALLELISM_REGION_PARAMS *)arg;
            LOG_VERBOSE("base: 0x%016lx, length: 0x%016lx, gpuUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmStatus: %d (0x%08x)\n", params->base, params->length, params->gpuUuid.uuid[0], params->gpuUuid.uuid[1], params->gpuUuid.uuid[2], params->gpuUuid.uuid[3], params->gpuUuid.uuid[4], params->gpuUuid.uuid[5], params->gpuUuid.uuid[6], params->gpuUuid.uuid[7], params->gpuUuid.uuid[8], params->gpuUuid.uuid[9], params->gpuUuid.uuid[10], params->gpuUuid.uuid[11], params->gpuUuid.uuid[12], params->gpuUuid.uuid[13], params->gpuUuid.uuid[14], params->gpuUuid.uuid[15], params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_UNMAP_EXTERNAL:
        {
            LOG("UVM_UNMAP_EXTERNAL\n");
            UVM_UNMAP_EXTERNAL_PARAMS *params = (UVM_UNMAP_EXTERNAL_PARAMS *)arg;
            LOG_VERBOSE("base: 0x%016lx, length: 0x%016lx, gpuUuid: %02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x-%02x%02x%02x%02x, rmStatus: %d (0x%08x)\n", params->base, params->length, params->gpuUuid.uuid[0], params->gpuUuid.uuid[1], params->gpuUuid.uuid[2], params->gpuUuid.uuid[3], params->gpuUuid.uuid[4], params->gpuUuid.uuid[5], params->gpuUuid.uuid[6], params->gpuUuid.uuid[7], params->gpuUuid.uuid[8], params->gpuUuid.uuid[9], params->gpuUuid.uuid[10], params->gpuUuid.uuid[11], params->gpuUuid.uuid[12], params->gpuUuid.uuid[13], params->gpuUuid.uuid[14], params->gpuUuid.uuid[15], params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_TOOLS_FLUSH_EVENTS:
        {
            LOG("UVM_TOOLS_FLUSH_EVENTS\n");
            UVM_TOOLS_FLUSH_EVENTS_PARAMS *params = (UVM_TOOLS_FLUSH_EVENTS_PARAMS *)arg;
            LOG_VERBOSE("rmStatus: %d (0x%08x)\n", params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_ALLOC_SEMAPHORE_POOL:
        {
            LOG("UVM_ALLOC_SEMAPHORE_POOL\n");
            UVM_ALLOC_SEMAPHORE_POOL_PARAMS *params = (UVM_ALLOC_SEMAPHORE_POOL_PARAMS *)arg;
            LOG_VERBOSE("base: 0x%016lx, length: 0x%016lx, gpuAttributesCount: 0x%016lx, rmStatus: %d (0x%08x)\n", params->base, params->length, params->gpuAttributesCount, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_CLEAN_UP_ZOMBIE_RESOURCES:
        {
            LOG("UVM_CLEAN_UP_ZOMBIE_RESOURCES\n");
            UVM_CLEAN_UP_ZOMBIE_RESOURCES_PARAMS *params = (UVM_CLEAN_UP_ZOMBIE_RESOURCES_PARAMS *)arg;
            LOG_VERBOSE("rmStatus: %d (0x%08x)\n", params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_CREATE_EXTERNAL_RANGE:
        {
            LOG("UVM_CREATE_EXTERNAL_RANGE\n");
            UVM_CREATE_EXTERNAL_RANGE_PARAMS *params = (UVM_CREATE_EXTERNAL_RANGE_PARAMS *)arg;
            LOG_VERBOSE("base: 0x%016lx, length: 0x%016lx, rmStatus: %d (0x%08x)\n", params->base, params->length, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_MM_INITIALIZE:
        {
            LOG("UVM_MM_INITIALIZE\n");
            UVM_MM_INITIALIZE_PARAMS *params = (UVM_MM_INITIALIZE_PARAMS *)arg;
            LOG_VERBOSE("rmStatus: %d (0x%08x)\n", params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_PAGEABLE_MEM_ACCESS:
        {
            LOG("UVM_PAGEABLE_MEM_ACCESS\n");
            UVM_PAGEABLE_MEM_ACCESS_PARAMS *params = (UVM_PAGEABLE_MEM_ACCESS_PARAMS *)arg;
            LOG_VERBOSE("pageableMemAccess %u, rmStatus: %d (0x%08x)\n", params->pageableMemAccess, params->rmStatus, params->rmStatus);
            return 1;
        }
        case UVM_VALIDATE_VA_RANGE:
        {
            LOG("UVM_VALIDATE_VA_RANGE\n");
            UVM_VALIDATE_VA_RANGE_PARAMS *params = (UVM_VALIDATE_VA_RANGE_PARAMS *)arg;
            LOG_VERBOSE("base: 0x%016lx, length: 0x%016lx, rmStatus: %d (0x%08x)\n", params->base, params->length, params->rmStatus, params->rmStatus);
            return 1;
        }
    }
    return 0;
}