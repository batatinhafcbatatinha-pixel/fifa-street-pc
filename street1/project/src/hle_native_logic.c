/* D3D8 logic operations use the D3D11.1 output merger's integer bitwise path. */
#define COBJMACROS
#include <windows.h>
#include <d3d11_1.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "recomp/gen/recomp_types.h"
#include "hle_d3d11_internal.h"

typedef struct LogicState {
    uint64_t normal_key;
    uint32_t operation;
    D3D11_BLEND_DESC normal;
    ID3D11BlendState1 *state;
} LogicState;

static ID3D11Device *s_owner;
static ID3D11Device1 *s_device1;
static ID3D11DeviceContext1 *s_context1;
static LogicState s_cache[256];
static uint32_t s_next;
static DXGI_FORMAT s_checked_format;

static void logic_failure(const char *message, HRESULT hr)
{
    fprintf(stderr, "[HLE-LOGIC] Native logic operations cannot be applied: %s (0x%08lX)\n",
        message, (unsigned long)hr);
    fflush(stderr);
    ExitProcess(1);
}

static void logic_device(void)
{
    if (s_owner == g_hle.dev) return;
    for (uint32_t i = 0; i < 256; i++) {
        if (s_cache[i].state) ID3D11BlendState1_Release(s_cache[i].state);
    }
    memset(s_cache, 0, sizeof s_cache);
    if (s_device1) ID3D11Device1_Release(s_device1);
    if (s_context1) ID3D11DeviceContext1_Release(s_context1);
    s_device1 = NULL; s_context1 = NULL; s_next = 0; s_checked_format = DXGI_FORMAT_UNKNOWN;
    HRESULT hr = ID3D11Device_QueryInterface(g_hle.dev, &IID_ID3D11Device1, (void **)&s_device1);
    if (FAILED(hr)) logic_failure("ID3D11Device1 is unavailable", hr);
    hr = ID3D11DeviceContext_QueryInterface(g_hle.ctx, &IID_ID3D11DeviceContext1, (void **)&s_context1);
    if (FAILED(hr)) logic_failure("ID3D11DeviceContext1 is unavailable", hr);
    D3D11_FEATURE_DATA_D3D11_OPTIONS options;
    memset(&options, 0, sizeof options);
    hr = ID3D11Device_CheckFeatureSupport(g_hle.dev, D3D11_FEATURE_D3D11_OPTIONS, &options, sizeof options);
    if (FAILED(hr) || !options.OutputMergerLogicOp)
        logic_failure("the adapter does not support OutputMergerLogicOp", FAILED(hr) ? hr : E_NOTIMPL);
    s_owner = g_hle.dev;
    fprintf(stderr, "[HLE-LOGIC] D3D11.1 output-merger logic operations enabled\n");
}

static D3D11_LOGIC_OP native_operation(uint32_t value)
{
    static const D3D11_LOGIC_OP operations[16] = {
        D3D11_LOGIC_OP_CLEAR, D3D11_LOGIC_OP_AND, D3D11_LOGIC_OP_AND_REVERSE, D3D11_LOGIC_OP_COPY,
        D3D11_LOGIC_OP_AND_INVERTED, D3D11_LOGIC_OP_NOOP, D3D11_LOGIC_OP_XOR, D3D11_LOGIC_OP_OR,
        D3D11_LOGIC_OP_NOR, D3D11_LOGIC_OP_EQUIV, D3D11_LOGIC_OP_INVERT, D3D11_LOGIC_OP_OR_REVERSE,
        D3D11_LOGIC_OP_COPY_INVERTED, D3D11_LOGIC_OP_OR_INVERTED, D3D11_LOGIC_OP_NAND, D3D11_LOGIC_OP_SET
    };
    if (value < 0x1500u || value > 0x150Fu) logic_failure("invalid Xbox logic operation", E_INVALIDARG);
    return operations[value & 15u];
}

ID3D11BlendState *hle_native_logic_blend(uint64_t normal_key, const D3D11_BLEND_DESC *normal)
{
    uint32_t operation = MEM32(0x2611F8u);
    if (!operation) return NULL;
    logic_device();
    D3D11_LOGIC_OP logic = native_operation(operation);
    ID3D11RenderTargetView *rtv = NULL;
    ID3D11DeviceContext_OMGetRenderTargets(g_hle.ctx, 1, &rtv, NULL);
    if (rtv) {
        D3D11_RENDER_TARGET_VIEW_DESC view;
        ID3D11RenderTargetView_GetDesc(rtv, &view);
        ID3D11RenderTargetView_Release(rtv);
        if (view.Format != s_checked_format) {
            D3D11_FEATURE_DATA_FORMAT_SUPPORT2 format = { view.Format, 0 };
            HRESULT hr = ID3D11Device_CheckFeatureSupport(g_hle.dev, D3D11_FEATURE_FORMAT_SUPPORT2, &format, sizeof format);
            if (FAILED(hr) || !(format.OutFormatSupport2 & D3D11_FORMAT_SUPPORT2_OUTPUT_MERGER_LOGIC_OP))
                logic_failure("the render-target format does not support bitwise logic operations", FAILED(hr) ? hr : E_NOTIMPL);
            s_checked_format = view.Format;
        }
    }
    for (uint32_t i = 0; i < 256; i++) {
        LogicState *entry = &s_cache[i];
        if (entry->state && entry->operation == operation && entry->normal_key == normal_key &&
            !memcmp(&entry->normal, normal, sizeof *normal)) return (ID3D11BlendState *)entry->state;
    }
    D3D11_BLEND_DESC1 desc;
    memset(&desc, 0, sizeof desc);
    desc.AlphaToCoverageEnable = normal->AlphaToCoverageEnable;
    desc.IndependentBlendEnable = FALSE;
    for (uint32_t i = 0; i < 8; i++) {
        const D3D11_RENDER_TARGET_BLEND_DESC *source = &normal->RenderTarget[i];
        D3D11_RENDER_TARGET_BLEND_DESC1 *target = &desc.RenderTarget[i];
        target->BlendEnable = FALSE;
        target->LogicOpEnable = i == 0;
        target->SrcBlend = source->SrcBlend ? source->SrcBlend : D3D11_BLEND_ONE;
        target->DestBlend = source->DestBlend ? source->DestBlend : D3D11_BLEND_ZERO;
        target->BlendOp = source->BlendOp ? source->BlendOp : D3D11_BLEND_OP_ADD;
        target->SrcBlendAlpha = source->SrcBlendAlpha ? source->SrcBlendAlpha : D3D11_BLEND_ONE;
        target->DestBlendAlpha = source->DestBlendAlpha ? source->DestBlendAlpha : D3D11_BLEND_ZERO;
        target->BlendOpAlpha = source->BlendOpAlpha ? source->BlendOpAlpha : D3D11_BLEND_OP_ADD;
        target->LogicOp = logic;
        target->RenderTargetWriteMask = source->RenderTargetWriteMask;
    }
    ID3D11BlendState1 *state = NULL;
    HRESULT hr = ID3D11Device1_CreateBlendState1(s_device1, &desc, &state);
    if (FAILED(hr)) logic_failure("CreateBlendState1 failed", hr);
    LogicState *entry = &s_cache[s_next++ & 255u];
    if (entry->state) ID3D11BlendState1_Release(entry->state);
    entry->normal_key = normal_key; entry->operation = operation; entry->normal = *normal; entry->state = state;
    return (ID3D11BlendState *)state;
}
