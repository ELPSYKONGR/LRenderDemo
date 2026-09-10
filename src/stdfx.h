/**
 * @file Project precompiled header for frequently used C++ and Windows declarations.
 * @author Codex
 * @created 2026-09-07
 */
#pragma once

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <d3d11.h>
#include <dxgi.h>
#include <windows.h>
#include <wrl/client.h>

#include <SimpleMath.h>
#include <CommonStates.h>

#include "render/CommonConstantBuffers.h"
#include "render/Dx11ConstantBuffer.h"
#include "render/IRenderEffect.h"
#include "render/EffectCubeMapResource.h"
#include "render/viewManager.h"
//
#define FrameInfoSLOT 0
#define ObjectInfoSLOT 1
#define MaterialInfoSLOT 2
#define LightInfoSLOT 3


//
#define SkyTextureCubeSLOT 0


//
#define LinearClampSamplerSLOT 0