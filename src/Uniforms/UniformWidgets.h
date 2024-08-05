#pragma once 

#include <Headers/RenderPackHeaders.h>

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
};
