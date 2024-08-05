#pragma once 

#include <Headers/RenderPackHeaders.h>
#include <ImGuiPack/ImGuiPack.h>

class UniformWidgets {
public:
    static bool checkUniformVisiblity(UniformVariantPtr v, bool vShowUnUsed);
    static void drawUniformComment(UniformVariantPtr vUniPtr);
    static bool drawUniformName(ShaderKeyPtr vKey, UniformVariantPtr vUniPtr, int vComponent = 0, const char* vTxt = nullptr);
    static bool drawImGuiUniformWidgetForPanes(CodeTreePtr vCodeTreePtr,
                                               UniformVariantPtr vUniPtr,
                                               float vMaxWidth,
                                               float vFirstColumnWidth,
                                               RenderPackWeak vRenderPack = RenderPackWeak(),
                                               bool vShowUnUsed = true,
                                               bool vShowCustom = false);

private:
    // vWidths.x : first column width, vWidths.y : max width
    static bool m_drawTimeWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawButtonWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawCheckboxWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawRadioWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawComboBoxWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawColorWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawDeltaTimeWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawFrameWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawDateWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawBufferWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawMouseWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawPictureWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawSampler2DWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawSamplerCubeWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawMatrixWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);
    static bool m_drawTextWidget(const ImVec2& vWidths, CodeTreePtr vCodeTreePtr, ShaderKeyPtr vShaderKeyPtr, UniformVariantPtr vUniPtr);

    static bool m_drawSliderWidgetFloat(const ImVec2& vWidths,
                                      CodeTreePtr vCodeTreePtr,
                                      ShaderKeyPtr vShaderKeyPtr,
                                      UniformVariantPtr vUniPtr,
                                      const bool vShowCustom,
                                      const size_t vChannels);
    static bool m_drawSliderWidgetInt(const ImVec2& vWidths,
                                      CodeTreePtr vCodeTreePtr,
                                      ShaderKeyPtr vShaderKeyPtr,
                                      UniformVariantPtr vUniPtr,
                                      const bool vShowCustom,
                                      const size_t vChannels);
    static bool m_drawSliderWidgetUInt(const ImVec2& vWidths,
                                      CodeTreePtr vCodeTreePtr,
                                      ShaderKeyPtr vShaderKeyPtr,
                                      UniformVariantPtr vUniPtr,
                                      const bool vShowCustom,
                                       const size_t vChannels);

    static bool m_drawInputWidgetFloat(const ImVec2& vWidths,
                                       CodeTreePtr vCodeTreePtr,
                                       ShaderKeyPtr vShaderKeyPtr,
                                       UniformVariantPtr vUniPtr,
                                       const bool vShowCustom,
                                       const size_t vChannels);
    static bool m_drawInputWidgetInt(const ImVec2& vWidths,
                                     CodeTreePtr vCodeTreePtr,
                                     ShaderKeyPtr vShaderKeyPtr,
                                     UniformVariantPtr vUniPtr,
                                     const bool vShowCustom,
                                     const size_t vChannels);
    static bool m_drawInputWidgetUInt(const ImVec2& vWidths,
                                      CodeTreePtr vCodeTreePtr,
                                      ShaderKeyPtr vShaderKeyPtr,
                                      UniformVariantPtr vUniPtr,
                                      const bool vShowCustom,
                                      const size_t vChannels);
};
