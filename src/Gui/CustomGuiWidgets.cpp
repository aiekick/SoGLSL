// NoodlesPlate Copyright (C) 2017-2024 Stephane Cuillerdier aka Aiekick
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "CustomGuiWidgets.h"
#include <Texture/TextureCube.h>
#include <Texture/Texture2D.h>

ImVec4 ImGui::GetUniformLocColor(int vLoc)
{
	if (vLoc >= 0)
		return ImVec4(0.2f, 0.4f, 0.2f, 1.00f); // good loc

	return ImVec4(0.4f, 0.2f, 0.2f, 1.00f); // bad loc	
}

void ImGui::Texture(ctTexturePtr vTex, float vWidth, ImVec4 vColor, float /*vBorderThick*/) {
	if (vTex == nullptr)
		return;

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	if (!window->ScrollbarY)
	{
		vWidth -= ImGui::GetStyle().ScrollbarSize;
	}

	const ImVec2 uv0 = ImVec2(0, 0);
	const ImVec2 uv1 = ImVec2(1, 1);

	ImVec2 size = ImVec2(vWidth, vWidth);

	const float ratioX = (float)vTex->h / (float)vTex->w;
	const float y = size.x * ratioX;
	if (y > size.y)
		size.x = size.y / ratioX;
	else
		size.y = y;

	size.x = ct::clamp(size.x, 1.0f, vWidth);
	size.y = ct::clamp(size.y, 1.0f, vWidth);

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	if (vColor.w > 0.0f)
		bb.Max += ImVec2(2, 2);
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, 0))
		return;

	if (vColor.w > 0.0f)
	{
		window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(vColor), 0.0f);
		window->DrawList->AddImage((ImTextureID)(size_t)vTex->glTex, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
	}
	else
	{
		window->DrawList->AddImage((ImTextureID)(size_t)vTex->glTex, bb.Min, bb.Max, uv0, uv1, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
	}

#ifdef _DEBUG
	if (ImGui::IsItemHovered())
	{
		char arr[3];
		if (snprintf(arr, 3, "%i", (int)vTex->glTex))
		{
			ImGui::SetTooltip(arr);
		}
	}
#endif
}

void ImGui::Texture(ctTexturePtr vTex, float vWidth, float vHeight, ImVec4 vColor, float /*vBorderThick*/) {
	if (vTex == nullptr)
		return;

	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return;

	if (!window->ScrollbarY)
	{
		vWidth -= ImGui::GetStyle().ScrollbarSize;
	}

	const ImVec2 uv0 = ImVec2(0, 0);
	const ImVec2 uv1 = ImVec2(1, 1);

	ImVec2 size = ImVec2(vWidth, vHeight);

	ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
	if (vColor.w > 0.0f)
		bb.Max += ImVec2(2, 2);
	ImGui::ItemSize(bb);
	if (!ImGui::ItemAdd(bb, 0))
		return;

	if (vColor.w > 0.0f)
	{
		window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(vColor), 0.0f);
		window->DrawList->AddImage((ImTextureID)(size_t)vTex->glTex, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
	}
	else
	{
		window->DrawList->AddImage((ImTextureID)(size_t)vTex->glTex, bb.Min, bb.Max, uv0, uv1, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
	}

#ifdef _DEBUG
	if (ImGui::IsItemHovered())
	{
		char arr[3];
		if (snprintf(arr, 3, "%i", (int)vTex->glTex))
		{
			ImGui::SetTooltip(arr);
		}
	}
#endif
}

bool ImGui::TextureButton(ctTexturePtr vTex, float vWidth, ImVec4 /*vColor*/, float /*vBorderThick*/)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	if (!window->ScrollbarY)
	{
		vWidth -= GetStyle().ScrollbarSize;
	}

	const ImVec2 uv0 = ImVec2(0, 0);
	const ImVec2 uv1 = ImVec2(1, 1);

	ImVec2 size = ImVec2(vWidth, vWidth);

	if (vTex)
	{
		const float ratioX = (float)vTex->h / (float)vTex->w;
		const float y = size.x * ratioX;
		if (y > size.y)
			size.x = size.y / ratioX;
		else
			size.y = y;

		size.x = ct::clamp(size.x, 1.0f, vWidth);
		size.y = ct::clamp(size.y, 1.0f, vWidth);
	}

	bool pressed = false;

	if (vTex)
	{
		const ImVec4 bg_col = ImVec4(1, 1, 1, 1);
		const ImTextureID texid = (ImTextureID)(size_t)vTex->glTex;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		// Default to using texture ID as ID. User can still push string/integer prefixes.
		// We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
		PushID(ImGui::IncPUSHID());

		const ImGuiID id = window->GetID("#image");

		PopID();

		const ImVec2 padding = style.FramePadding;
        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
		const ImRect image_bb(window->DC.CursorPos + padding, window->DC.CursorPos + padding + size);
		ItemSize(bb);
		if (!ItemAdd(bb, id))
			return false;

		bool hovered, held;
		pressed = ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnClick);

		// Render
		const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		RenderNavHighlight(bb, id);
		RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
		if (bg_col.w > 0.0f)
			window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));
		window->DrawList->AddImage(texid, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(bg_col));

	}
	else
	{
		PushID(ImGui::IncPUSHID());

		pressed = Button("+", size);

		PopID();
	}

#ifdef _DEBUG
	if (IsItemHovered())
	{
		char arr[5];
		if (snprintf(arr, 5, "%i", (int)ImGui::GetPUSHID()))
		{
			SetTooltip(arr);
		}
	}
#endif

	return pressed;
}

bool ImGui::TextureButton(ctTexturePtr vTex, float vWidth, ImVec4 /*vColor*/, ImVec4 vBorderColor)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	if (!window->ScrollbarY)
	{
		vWidth -= GetStyle().ScrollbarSize;
	}

	const ImVec2 uv0 = ImVec2(0, 0);
	const ImVec2 uv1 = ImVec2(1, 1);

	ImVec2 size = ImVec2(vWidth, vWidth);

	if (vTex)
	{
		const float ratioX = (float)vTex->h / (float)vTex->w;
		const float y = size.x * ratioX;
		if (y > size.y)
			size.x = size.y / ratioX;
		else
			size.y = y;

		size.x = ct::clamp(size.x, 1.0f, vWidth);
		size.y = ct::clamp(size.y, 1.0f, vWidth);
	}

	bool pressed = false;

	if (vTex)
	{
		//ImTextureID texid = (ImTextureID)(size_t)vTex->glTex;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		// Default to using texture ID as ID. User can still push string/integer prefixes.
		// We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
		PushID(ImGui::IncPUSHID());

		const ImGuiID id = window->GetID("#image");

		PopID();

		const ImVec2 padding = style.FramePadding;
		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size + padding * 2);
		//const ImRect image_bb(window->DC.CursorPos + padding, window->DC.CursorPos + padding + size);
		ItemSize(bb);
		if (!ItemAdd(bb, id))
			return false;

		bool hovered, held;
		pressed = ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_PressedOnClick);

		// Render
		//const ImU32 col = GetColorU32((held && hovered) ? ImVec4(1.0f,1.0f,1.0f,1.0f) : hovered ? ImVec4(0.5f, 0.5f, 0.5f, 0.5f) : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
		RenderNavHighlight(bb, id);
		//RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
		//if (bg_col.w > 0.0f)
		//	window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

		if (vBorderColor.w > 0.0f)
		{
			window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(vBorderColor), 0.0f, ImDrawFlags_RoundCornersAll, vBorderColor.w);
			window->DrawList->AddImage((ImTextureID)(size_t)vTex->glTex, bb.Min + ImVec2(1, 1), bb.Max - ImVec2(1, 1), uv0, uv1, GetColorU32(ImVec4(1, 1, 1, 1)));
		}
		else
		{
			window->DrawList->AddImage((ImTextureID)(size_t)vTex->glTex, bb.Min, bb.Max, uv0, uv1, GetColorU32(ImVec4(1, 1, 1, 1)));
		}
		//window->DrawList->AddImage(texid, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(bg_col));

		if (hovered)
		{
			window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(ImVec4(0.5f, 0.5f, 0.5f, 0.5f)));
		}
	}
	else
	{
		PushID(ImGui::IncPUSHID());

		pressed = Button("+", size);

		PopID();
	}

#ifdef _DEBUG
	if (IsItemHovered())
	{
		char arr[5];
		if (snprintf(arr, 5, "%i", (int)ImGui::GetPUSHID()))
		{
			SetTooltip(arr);
		}
	}
#endif

	return pressed;
}

void ImGui::Texture(float vWidth, const char* /*vName*/, TextureCubePtr vCubeMap, GLuint vLoc)
{
	if (vCubeMap != nullptr)
	{
		ImGui::BeginGroup();

		const ImVec4 col = GetUniformLocColor(vLoc);

		const float w = vWidth / 3.0f;

		TextureOverLay(w, vCubeMap->GetTexture2D(0)->getBack().get(), col, "X+", ImVec4(0.0f, 0.0f, 0.0f, 1.0f), ImVec4(0.7f, 0.7f, 0.7f, 0.7f));
		ImGui::SameLine();
        TextureOverLay(w, vCubeMap->GetTexture2D(2)->getBack().get(), col, "Y+", ImVec4(0.0f, 0.0f, 0.0f, 1.0f), ImVec4(0.7f, 0.7f, 0.7f, 0.7f));
		ImGui::SameLine();
        TextureOverLay(w, vCubeMap->GetTexture2D(4)->getBack().get(), col, "Z+", ImVec4(0.0f, 0.0f, 0.0f, 1.0f), ImVec4(0.7f, 0.7f, 0.7f, 0.7f));

		TextureOverLay(w, vCubeMap->GetTexture2D(1)->getBack().get(), col, "X-", ImVec4(0.0f, 0.0f, 0.0f, 1.0f), ImVec4(0.7f, 0.7f, 0.7f, 0.7f));
		ImGui::SameLine();
        TextureOverLay(w, vCubeMap->GetTexture2D(3)->getBack().get(), col, "Y-", ImVec4(0.0f, 0.0f, 0.0f, 1.0f), ImVec4(0.7f, 0.7f, 0.7f, 0.7f));
		ImGui::SameLine();
        TextureOverLay(w, vCubeMap->GetTexture2D(5)->getBack().get(), col, "Z-", ImVec4(0.0f, 0.0f, 0.0f, 1.0f), ImVec4(0.7f, 0.7f, 0.7f, 0.7f));

		ImGui::EndGroup();
	}
}

void ImGui::PlotFVec4Histo(const char* vLabel, ct::fvec4* vDatas, int vDataCount, bool* vShowChannel, ImVec2 frame_size, ct::fvec4 scale_min,
                           ct::fvec4 scale_max, int* vHoveredIdx) {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(vLabel);

    const ImVec2 label_size = ImGui::CalcTextSize(vLabel, NULL, true);
    if (IS_FLOAT_EQUAL(frame_size.x, 0.0f))
        frame_size.x = ImGui::CalcItemWidth();
    if (IS_FLOAT_EQUAL(frame_size.y, 0.0f))
        frame_size.y = label_size.y + (style.FramePadding.y * 2);

    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    const ImRect inner_bb(frame_bb.Min + style.FramePadding, frame_bb.Max - style.FramePadding);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0));
    ImGui::ItemSize(total_bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(total_bb, 0, &frame_bb))
        return;

    ImGui::PushID(ImGui::IncPUSHID());

    const bool hovered = ImGui::ItemHoverable(frame_bb, id, ImGuiItemFlags_None);

    // Determine scale from values if not specified
    for (int chan = 0; chan < 4; chan++) {
        if (IS_FLOAT_EQUAL(scale_min[chan], FLT_MAX) || IS_FLOAT_EQUAL(scale_max[chan], FLT_MAX)) {
            float v_min = FLT_MAX;
            float v_max = -FLT_MAX;
            for (int i = 0; i < vDataCount; ++i) {
                const float v = vDatas[i][chan];
                v_min = ct::mini(v_min, v);
                v_max = ct::maxi(v_max, v);
            }
            if (scale_min[chan] == FLT_MAX)
                scale_min[chan] = v_min;
            if (scale_max[chan] == FLT_MAX)
                scale_max[chan] = v_max;
        }
    }

    ImGui::RenderFrame(frame_bb.Min, frame_bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

    const int values_count_min = 2;
    if (vDataCount >= values_count_min) {
        int res_w = ct::mini((int)frame_size.x, (int)vDataCount) - 1;
        int item_count = vDataCount - 1;

        // Tooltip on hover
        int v_hovered = -1;
        if (vHoveredIdx)
            *vHoveredIdx = v_hovered;

        if (hovered && inner_bb.Contains(g.IO.MousePos)) {
            const float t = ImClamp((g.IO.MousePos.x - inner_bb.Min.x) / (inner_bb.Max.x - inner_bb.Min.x), 0.0f, 0.9999f);
            const int v_idx = (int)(t * item_count);
            IM_ASSERT(v_idx >= 0 && v_idx < vDataCount);

            const ct::fvec4 v0 = vDatas[v_idx % vDataCount];
            ImGui::BeginTooltip();
            if (!vShowChannel || vShowChannel[0]) {
                ImGui::Text("r %d: %8.4g", v_idx, v0.x);
            }
            if (!vShowChannel || vShowChannel[1]) {
                ImGui::Text("g %d: %8.4g", v_idx, v0.y);
            }
            if (!vShowChannel || vShowChannel[2]) {
                ImGui::Text("b %d: %8.4g", v_idx, v0.z);
            }
            if (!vShowChannel || vShowChannel[3]) {
                ImGui::Text("a %d: %8.4g", v_idx, v0.w);
            }
            ImGui::EndTooltip();
            v_hovered = v_idx;

            if (vHoveredIdx)
                *vHoveredIdx = v_hovered;
        }

        float t_step = 1.0f / (float)res_w;
        ct::fvec4 inv_scale;
        inv_scale.x = (scale_min.x == scale_max.x) ? 0.0f : (1.0f / (scale_max.x - scale_min.x));
        inv_scale.y = (scale_min.y == scale_max.y) ? 0.0f : (1.0f / (scale_max.y - scale_min.y));
        inv_scale.z = (scale_min.z == scale_max.z) ? 0.0f : (1.0f / (scale_max.z - scale_min.z));
        inv_scale.w = (scale_min.w == scale_max.w) ? 0.0f : (1.0f / (scale_max.w - scale_min.w));

        const ct::fvec4 v0 = (vDatas[0] - scale_min) * inv_scale;
        ct::fvec4 yt0;
        yt0.x = 1.0f - ct::clamp(v0.x);
        yt0.y = 1.0f - ct::clamp(v0.y);
        yt0.z = 1.0f - ct::clamp(v0.z);
        yt0.w = 1.0f - ct::clamp(v0.w);
        float xt0 = 0.0f;

        ct::fvec4 histogram_zero_line_t;
        histogram_zero_line_t.x = (scale_min.x * scale_max.x < 0.0f) ? (-scale_min.x * inv_scale.x)
                                                                     : (scale_min.x < 0.0f ? 0.0f : 1.0f);  // Where does the zero line stands
        histogram_zero_line_t.y = (scale_min.y * scale_max.y < 0.0f) ? (-scale_min.y * inv_scale.y)
                                                                     : (scale_min.y < 0.0f ? 0.0f : 1.0f);  // Where does the zero line stands
        histogram_zero_line_t.z = (scale_min.z * scale_max.z < 0.0f) ? (-scale_min.z * inv_scale.z)
                                                                     : (scale_min.z < 0.0f ? 0.0f : 1.0f);  // Where does the zero line stands
        histogram_zero_line_t.w = (scale_min.w * scale_max.w < 0.0f) ? (-scale_min.w * inv_scale.w)
                                                                     : (scale_min.w < 0.0f ? 0.0f : 1.0f);  // Where does the zero line stands

        for (int n = 0; n < res_w; n++) {
            float xt1 = xt0 + t_step;
            int v1_idx = (int)(xt0 * item_count + 0.5f);
            IM_ASSERT(v1_idx >= 0 && v1_idx < vDataCount);
            const ct::fvec4 v1 = (vDatas[(v1_idx + 1) % vDataCount] - scale_min) * inv_scale;
            ct::fvec4 yt1;  // = 1.0f - ct::clamp(v1);
            yt1.x = 1.0f - ct::clamp(v1.x);
            yt1.y = 1.0f - ct::clamp(v1.y);
            yt1.z = 1.0f - ct::clamp(v1.z);
            yt1.w = 1.0f - ct::clamp(v1.w);

            if (!vShowChannel || vShowChannel[0]) {
                ImVec2 p0x = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt0, yt0.x));
                ImVec2 p1x = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt1, yt1.x));
                window->DrawList->AddLine(p0x, p1x, ImGui::GetColorU32(ImVec4(1, 0, 0, 1)));
            }

            if (!vShowChannel || vShowChannel[1]) {
                ImVec2 p0y = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt0, yt0.y));
                ImVec2 p1y = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt1, yt1.y));
                window->DrawList->AddLine(p0y, p1y, ImGui::GetColorU32(ImVec4(0, 1, 0, 1)));
            }

            if (!vShowChannel || vShowChannel[2]) {
                ImVec2 p0z = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt0, yt0.z));
                ImVec2 p1z = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt1, yt1.z));
                window->DrawList->AddLine(p0z, p1z, ImGui::GetColorU32(ImVec4(0, 0, 1, 1)));
            }

            if (!vShowChannel || vShowChannel[3]) {
                ImVec2 p0w = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt0, yt0.w));
                ImVec2 p1w = ImLerp(inner_bb.Min, inner_bb.Max, ImVec2(xt1, yt1.w));
                window->DrawList->AddLine(p0w, p1w, ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
            }

            xt0 = xt1;
            yt0 = yt1;
        }

        if (!vShowChannel || vShowChannel[0]) {
            float y = ImLerp(inner_bb.Min.y, inner_bb.Max.y, yt0.x);
            char buf[20];
            snprintf(buf, 20, "r %.3f", yt0.x);
            window->DrawList->AddText(ImVec2(inner_bb.Max.x + style.FramePadding.x, y + style.FramePadding.y), ImGui::GetColorU32(ImVec4(1, 0, 0, 1)),
                                      buf);
        }

        if (!vShowChannel || vShowChannel[1]) {
            float y = ImLerp(inner_bb.Min.y, inner_bb.Max.y, yt0.y);
            char buf[20];
            snprintf(buf, 20, "g %.3f", yt0.y);
            window->DrawList->AddText(ImVec2(inner_bb.Max.x + style.FramePadding.x, y + style.FramePadding.y), ImGui::GetColorU32(ImVec4(0, 1, 0, 1)),
                                      buf);
        }

        if (!vShowChannel || vShowChannel[2]) {
            float y = ImLerp(inner_bb.Min.y, inner_bb.Max.y, yt0.z);
            char buf[20];
            snprintf(buf, 20, "b %.3f", yt0.z);
            window->DrawList->AddText(ImVec2(inner_bb.Max.x + style.FramePadding.x, y + style.FramePadding.y), ImGui::GetColorU32(ImVec4(0, 0, 1, 1)),
                                      buf);
        }

        if (!vShowChannel || vShowChannel[3]) {
            float y = ImLerp(inner_bb.Min.y, inner_bb.Max.y, yt0.w);
            char buf[20];
            snprintf(buf, 20, "a %.3f", yt0.w);
            window->DrawList->AddText(ImVec2(inner_bb.Max.x + style.FramePadding.x, y + style.FramePadding.y), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)),
                                      buf);
        }
    }

    ImGui::PopID();
}
