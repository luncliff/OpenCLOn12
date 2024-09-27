// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
#pragma once
#include "platform.hpp"
#include "cache.hpp"
#include <string>
#include <vector>
#include <mutex>

#include <D3D12TranslationLayerIncludes.h>
using ImmCtx = D3D12TranslationLayer::ImmediateContext;

class Task;
class Device;

using Submission = std::vector<::ref_ptr_int<Task>>;

class D3DDevice
{
public:
    ID3D12Device* GetDevice() const noexcept { return m_spDevice.Get(); }
    ShaderCache &GetShaderCache() const noexcept { return m_ShaderCache; }
    ShaderCache &GetDriverShaderCache() const noexcept { return m_DriverShaderCache; }

    ImmCtx& ImmCtx() noexcept { return m_ImmCtx; }
    UINT64 GetTimestampFrequency() const noexcept { return m_TimestampFrequency; }
    INT64 GPUToQPCTimestampOffset() const noexcept { return m_GPUToQPCTimestampOffset; }

    void SubmitTask(Task*, TaskPoolLock const&);
    void ReadyTask(Task*, TaskPoolLock const&);
    void Flush(TaskPoolLock const&);

    //std::unique_ptr<D3D12TranslationLayer::PipelineState> CreatePSO(D3D12TranslationLayer::COMPUTE_PIPELINE_STATE_DESC const& Desc);
    Device &GetParent() const noexcept { return m_Parent; }

protected:
    D3DDevice(Device &parent, ID3D12Device *pDevice, ID3D12CommandQueue *pQueue,
              D3D12_FEATURE_DATA_D3D12_OPTIONS &options, bool IsImportedDevice);
    ~D3DDevice() = default;

    friend class Device;

    void ExecuteTasks(std::unique_ptr<Submission> spTasks);
    unsigned m_ContextCount = 1;
    const bool m_IsImportedDevice;

    Device &m_Parent;
    const ComPtr<ID3D12Device> m_spDevice;
    ::ImmCtx m_ImmCtx;

    std::unique_ptr<Submission> m_RecordingSubmission;

    BackgroundTaskScheduler::Scheduler m_ExecutionScheduler;
    BackgroundTaskScheduler::Scheduler m_CompletionScheduler;
    mutable ShaderCache m_ShaderCache;
    mutable ShaderCache m_DriverShaderCache;

    UINT64 m_TimestampFrequency = 0;
    INT64 m_GPUToQPCTimestampOffset = 0;
};

class Device : public CLChildBase<Device, Platform, cl_device_id>
{
public:
    Device(Platform& parent, IDXCoreAdapter* pAdapter);
    ~Device();

    cl_bool IsAvailable() const noexcept;
    cl_ulong GetGlobalMemSize();
    DXCoreHardwareID const& GetHardwareIds() const noexcept;
    cl_device_type GetType() const noexcept;
    bool IsMCDM() const noexcept;
    bool IsUMA();
    bool SupportsInt16();
    bool SupportsTypedUAVLoad();

    std::string GetDeviceName() const;
    LUID GetAdapterLuid() const;
    D3D_SHADER_MODEL GetShaderModel() const { return m_ShaderModel; }
    std::pair<cl_uint, cl_uint> GetWaveSizes() const
    {
        if (!m_D3D12Options1.WaveOps)
        {
            return { 32, 64 };
        }
        return { m_D3D12Options1.WaveLaneCountMin, m_D3D12Options1.WaveLaneCountMax };
    }

    D3DDevice &InitD3D(ID3D12Device *device = nullptr, ID3D12CommandQueue *queue = nullptr);
    void ReleaseD3D(D3DDevice &device);
    void SetDefaultDevice() { m_DefaultDevice = true; }

    bool HasD3DDevice() const noexcept { return !m_D3DDevices.empty(); }
    void CloseCaches();
    void FlushAllDevices(TaskPoolLock const& Lock);

protected:
    void CacheCaps(std::lock_guard<std::mutex> const&, ComPtr<ID3D12Device> spDevice = {});

    ComPtr<IDXCoreAdapter> m_spAdapter;
    DXCoreHardwareID m_HWIDs;
    std::vector<::D3DDevice *> m_D3DDevices;

    // Lazy-initialized
    std::mutex m_InitLock;
    bool m_CapsValid = false;
    bool m_DefaultDevice = false;
    D3D12_FEATURE_DATA_D3D12_OPTIONS m_D3D12Options = {};
    D3D12_FEATURE_DATA_D3D12_OPTIONS1 m_D3D12Options1 = {};
    D3D12_FEATURE_DATA_D3D12_OPTIONS4 m_D3D12Options4 = {};
    D3D12_FEATURE_DATA_ARCHITECTURE m_Architecture = {};
    D3D_SHADER_MODEL m_ShaderModel = D3D_SHADER_MODEL_6_0;
};

using D3DDeviceAndRef = std::pair<Device::ref_ptr_int, D3DDevice *>;
