/*
 *  Copyright (C) 2005-2023 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "DVDCodecs/Video/DXVA.h"
#include "VideoRenderers/Windows/RendererBase.h"
#include "guilib/D3DResource.h"

#include <mutex>

#include <wrl/client.h>

namespace DXVA
{

using namespace Microsoft::WRL;

// ProcAmp filters d3d11 filters
struct ProcAmpFilter
{
  D3D11_VIDEO_PROCESSOR_FILTER filter;
  D3D11_VIDEO_PROCESSOR_FILTER_CAPS cap;
  const char* name;
};

// clang-format off
const ProcAmpFilter PROCAMP_FILTERS[] = {
    {D3D11_VIDEO_PROCESSOR_FILTER_BRIGHTNESS,
    D3D11_VIDEO_PROCESSOR_FILTER_CAPS_BRIGHTNESS, "Brightness"},
    {D3D11_VIDEO_PROCESSOR_FILTER_CONTRAST,
    D3D11_VIDEO_PROCESSOR_FILTER_CAPS_CONTRAST, "Contrast"},
    {D3D11_VIDEO_PROCESSOR_FILTER_HUE,
    D3D11_VIDEO_PROCESSOR_FILTER_CAPS_HUE, "Hue"},
    {D3D11_VIDEO_PROCESSOR_FILTER_SATURATION,
    D3D11_VIDEO_PROCESSOR_FILTER_CAPS_SATURATION, "Saturation"},
    {D3D11_VIDEO_PROCESSOR_FILTER_NOISE_REDUCTION,
    D3D11_VIDEO_PROCESSOR_FILTER_CAPS_NOISE_REDUCTION, "Noise Reduction"},
    {D3D11_VIDEO_PROCESSOR_FILTER_EDGE_ENHANCEMENT,
     D3D11_VIDEO_PROCESSOR_FILTER_CAPS_EDGE_ENHANCEMENT, "Edge Enhancement"},
    {D3D11_VIDEO_PROCESSOR_FILTER_ANAMORPHIC_SCALING,
     D3D11_VIDEO_PROCESSOR_FILTER_CAPS_ANAMORPHIC_SCALING, "Anamorphic Scaling"},
    {D3D11_VIDEO_PROCESSOR_FILTER_STEREO_ADJUSTMENT,
     D3D11_VIDEO_PROCESSOR_FILTER_CAPS_STEREO_ADJUSTMENT, "Stereo Adjustment"}};
// clang-format on

constexpr size_t NUM_FILTERS = ARRAYSIZE(PROCAMP_FILTERS);

struct ProcAmpInfo
{
  bool bSupported;
  D3D11_VIDEO_PROCESSOR_FILTER_RANGE Range;
};

struct ProcessorCapabilities
{
  bool m_valid{false};

  uint32_t m_procIndex{0};
  D3D11_VIDEO_PROCESSOR_CAPS m_vcaps{};
  D3D11_VIDEO_PROCESSOR_RATE_CONVERSION_CAPS m_rateCaps{};
  ProcAmpInfo m_Filters[NUM_FILTERS]{};
  bool m_hasMetadataHDR10Support{false};
};

enum InputFormat
{
  None,
  TopLeft,
  Left
};

struct ProcessorFormats
{
  std::vector<DXGI_FORMAT> m_input;
  std::vector<DXGI_FORMAT> m_output;
  bool m_valid{false};
};

struct ProcessorConversion
{
  DXGI_FORMAT m_inputFormat{DXGI_FORMAT_UNKNOWN};
  DXGI_COLOR_SPACE_TYPE m_inputCS{DXGI_COLOR_SPACE_RESERVED};
  DXGI_FORMAT m_outputFormat{DXGI_FORMAT_UNKNOWN};
  DXGI_COLOR_SPACE_TYPE m_outputCS{DXGI_COLOR_SPACE_RESERVED};

  ProcessorConversion() = default;
  ProcessorConversion(const DXGI_FORMAT& inputFormat,
                      const DXGI_COLOR_SPACE_TYPE& inputCS,
                      const DXGI_FORMAT& outputFormat,
                      const DXGI_COLOR_SPACE_TYPE& outputCS)
    : m_inputFormat(inputFormat),
      m_inputCS(inputCS),
      m_outputFormat(outputFormat),
      m_outputCS(outputCS)
  {
  }

  const std::string ToString() const;
};

using ProcessorConversions = std::vector<ProcessorConversion>;

const std::vector<DXGI_FORMAT> RenderingOutputFormats = {DXGI_FORMAT_B8G8R8A8_UNORM,
                                                         DXGI_FORMAT_R10G10B10A2_UNORM};

struct SupportedConversionsArgs
{
  AVColorPrimaries m_colorPrimaries{AVCOL_PRI_UNSPECIFIED};
  AVColorTransferCharacteristic m_colorTransfer{AVCOL_TRC_UNSPECIFIED};
  bool m_fullRange{false};
  bool m_hdrOutput{false};

  SupportedConversionsArgs(const VideoPicture& picture, bool isHdrOutput)
  {
    m_colorPrimaries = static_cast<AVColorPrimaries>(picture.color_primaries);
    m_colorTransfer = static_cast<AVColorTransferCharacteristic>(picture.color_transfer);
    m_fullRange = picture.color_range == 1;
    m_hdrOutput = isHdrOutput;
  }

  SupportedConversionsArgs(const AVColorPrimaries& colorPrimaries,
                           const AVColorTransferCharacteristic& colorTransfer,
                           bool fullRange,
                           bool hdrOutput)
    : m_colorPrimaries(colorPrimaries),
      m_colorTransfer(colorTransfer),
      m_fullRange(fullRange),
      m_hdrOutput(hdrOutput)
  {
  }
};

class CEnumeratorHD : public ID3DResource
{
public:
  CEnumeratorHD();
  virtual ~CEnumeratorHD();

  bool Open(unsigned int width, unsigned int height, DXGI_FORMAT input_dxgi_format);
  void Close();

  // ID3DResource overrides
  void OnCreateDevice() override
  {
    std::unique_lock<CCriticalSection> lock(m_section);
    if (m_width > 0 && m_height > 0)
      OpenEnumerator();
  }

  void OnDestroyDevice(bool) override
  {
    std::unique_lock<CCriticalSection> lock(m_section);
    UnInit();
  }

  ProcessorCapabilities ProbeProcessorCaps();
  /*!
   * \brief Check if a conversion is supported by the dxva processor.
   * \param inputFormat the input format
   * \param inputCS the input color space
   * \param outputFormat the output format
   * \param outputCS the output color space
   * \return true when the conversion is supported, false when it is not
   * or the API used to validate is not availe (Windows < 10)
  */
  bool CheckConversion(DXGI_FORMAT inputFormat,
                       DXGI_COLOR_SPACE_TYPE inputCS,
                       DXGI_FORMAT outputFormat,
                       DXGI_COLOR_SPACE_TYPE outputCS);
  /*!
   * \brief Check dxva processor for support of the format as input texture
   * \param format the format
   * \return true supported, false not supported
   */
  bool IsFormatSupportedInput(DXGI_FORMAT format);
  /*!
   * \brief Check dxva processor for support of the format as output texture
   * \param format the format
   * \return true supported, false not supported
   */
  bool IsFormatSupportedOutput(DXGI_FORMAT format);
  /*!
   * \brief Outputs in the log a list of conversions supported by the DXVA processor.
   * \param inputFormat the source format
   * \param inputNativeCS the input color space that would be used with a direct mapping
   * from avcodec to D3D11, without any workarounds or tricks.
   */
  void LogSupportedConversions(const DXGI_FORMAT inputFormat,
                               const DXGI_COLOR_SPACE_TYPE inputNativeCS);

  bool IsInitialized() const { return m_pEnumerator; }
  /*!
   * \brief Returns the availability of the interface ID3D11VideoProcessorEnumerator1
   * (Windows 10 supporting HDR and above)
   * \return true when the interface is available and initialized, false otherwise
   */
  bool IsEnumerator1Available() { return m_pEnumerator1; }

  ComPtr<ID3D11VideoProcessor> CreateVideoProcessor(UINT RateConversionIndex);
  ComPtr<ID3D11VideoProcessorInputView> CreateVideoProcessorInputView(
      ID3D11Resource* pResource, const D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC* pDesc);
  ComPtr<ID3D11VideoProcessorOutputView> CreateVideoProcessorOutputView(
      ID3D11Resource* pResource, const D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC* pDesc);

  /*!
   * \brief Return the conversions supported by the processor to play HDR material as HDR.
   * \param isSourceFullRange Is the source limited or full range
   * \return conversion list of supported conversions
   */
  ProcessorConversions QueryHDRConversions(const bool isSourceFullRange);
  /*!
   * \brief Return the conversions supported by the processor to play HDR material as SDR
   * These conversions avoid tonemapping by the processor and require post processing.
   * \param isSourceFullRange Is the source limited or full range
   * \return conversion list of supported conversions
   */
  ProcessorConversions QueryHDRtoSDRConversions(const bool isSourceFullRange);
  /*!
   * \brief Return the conversions supported by the processor for SDR material.
   * Support is assumed to exist on systems that don't support the
   * ID3D11VideoProcessorEnumerator1 interface.
   * \param isSourceFullRange Is the source limited or full range
   * \param colorPrimaries color primaries of the source
   * \param colorTransfer transfer function of the source
   * \return conversion list of supported conversions
   */
  ProcessorConversions QuerySDRConversions(const bool isSourceFullRange,
                                           AVColorPrimaries colorPrimaries,
                                           AVColorTransferCharacteristic colorTransfer);
  /*!
   * \brief Return a list of conversions supported by the processor for the given parameters.
   * \param args parameters
   * \return list of conversions
   */
  ProcessorConversions SupportedConversions(const SupportedConversionsArgs& args);

protected:
  void UnInit();
  bool OpenEnumerator();

  /*!
   * \brief Retrieve the list of DXGI_FORMAT supported by the DXVA processor
   * \param inputFormats yes/no populate the input formats vector of the returned structure
   * \param outputFormats yes/no populate the output formats vector of the returned structure
   * \return requested list of input and/or output formats.
   */
  ProcessorFormats GetProcessorFormats(bool inputFormats, bool outputFormats) const;
  /*!
   * \brief Retrieve the list of RGB DXGI_FORMAT supported as output by the DXVA
   * processor \return Vector of formats
   */
  std::vector<DXGI_FORMAT> GetProcessorRGBOutputFormats() const;
  /*!
   * \brief Check if a conversion is supported by the dxva processor.
   * \param inputFormat the input format
   * \param inputCS the input color space
   * \param outputFormat the output format
   * \param outputCS the output color space
   * \return true when the conversion is supported, false when it is not
   * or the API used to validate is not available (Windows < 10)
  */
  bool CheckConversionInternal(DXGI_FORMAT inputFormat,
                               DXGI_COLOR_SPACE_TYPE inputCS,
                               DXGI_FORMAT outputFormat,
                               DXGI_COLOR_SPACE_TYPE outputCS) const;
  /*!
   * \brief Check dxva processor for support of the format as input texture
   * \param format the format
   * \return true supported, false not supported
   */
  bool IsFormatSupportedInternal(DXGI_FORMAT format,
                                 D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT support) const;

  /*!
   * \brief Iterate over all combinations of the input parameters and return a list of
   * the combinations that are supported conversions.
   * \param inputFormat The input format
   * \param inputColorSpaces The possible source color spaces
   * \param outputFormats The possible output formats
   * \param outputColorSpaces The possible output color spaces
   * \return List of the supported conversion.
   */
  ProcessorConversions ListConversions(
      DXGI_FORMAT inputFormat,
      const std::vector<DXGI_COLOR_SPACE_TYPE>& inputColorSpaces,
      const std::vector<DXGI_FORMAT>& outputFormats,
      const std::vector<DXGI_COLOR_SPACE_TYPE>& outputColorSpaces) const;

  CCriticalSection m_section;

  uint32_t m_width = 0;
  uint32_t m_height = 0;

  ComPtr<ID3D11VideoDevice> m_pVideoDevice;
  ComPtr<ID3D11VideoProcessorEnumerator> m_pEnumerator;
  ComPtr<ID3D11VideoProcessorEnumerator1> m_pEnumerator1;
  DXGI_FORMAT m_input_dxgi_format{DXGI_FORMAT_UNKNOWN};
};
}; // namespace DXVA
