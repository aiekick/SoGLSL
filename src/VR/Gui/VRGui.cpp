// NoodlesPlate Copyright (C) 2017-2023 Stephane Cuillerdier aka Aiekick
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

#ifdef USE_VR

#include "VRGui.h"
#include <ImWidgets.h>
#include <Res/CustomFont2.h>
#include <VR/Backend/VRBackend.h>

/////////////////////////////////////////////////////////////////////
///// CTOR/DTOR /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////

VRGui::VRGui()
{

}

VRGui::~VRGui()
{

}

///////////////////////////////////////////////////////
///// IMGUI ///////////////////////////////////////////
///////////////////////////////////////////////////////

void VRGui::Draw()
{
	if (VRBackend::Instance()->IsLoaded())
	{
		if (ImGui::CollapsingHeader("VR"))
		{
			const float aw = ImGui::GetContentRegionAvail().x;

			if (ImGui::ContrastedButton("Restart OpenXR", nullptr, nullptr, aw))
			{
				VRBackend::Instance()->Init(m_CodeTreeWeak.lock());
			}

			ImGui::Separator();

			const auto& xr_info = VRBackend::Instance()->GetOpenXRInfo();

			if (ImGui::BeginFramedGroup("Instance"))
			{
				ImGui::Text("Runtime Name %s", xr_info.runtimeName.c_str());
				ImGui::Text("Runtime Version %s", xr_info.runtimeVersion.c_str());

				ImGui::EndFramedGroup();
			}

			if (ImGui::BeginFramedGroup("System"))
			{
				ImGui::Text("System Name %s", xr_info.systemName.c_str());
				ImGui::Text("Vendor ID %s", xr_info.vendorId.c_str());
				ImGui::Text("Orientation Trackng : %s", xr_info.orientationTracking ? "true" : "false");
				ImGui::Text("Position Tracking : %s", xr_info.positionTracking ? "true" : "false");
				ImGui::Text("max layer Count : %u", xr_info.maxLayerCount);
				ImGui::Text("max SwapChain Size : %u x %u", xr_info.maxSwapChainSize.x, xr_info.maxSwapChainSize.y);

				ImGui::EndFramedGroup();
			}

			if (ImGui::BeginFramedGroup("Extensions"))
			{
				ImGui::Text("Opengl Supported : %s", xr_info.openglSupported ? "true" : "false");
				ImGui::Text("Depth Supported : %s", xr_info.depthSupported ? "true" : "false");

				ImGui::EndFramedGroup();
			}

			if (ImGui::BeginFramedGroup("Views"))
			{
				const auto& xr_views = VRBackend::Instance()->GetOpenXRViews();

				uint32_t idx = 0;
				for (const auto& view : xr_views)
				{
					ImGui::Text("--- view %u ------", idx++);
					ImGui::Text("Pos : %.3f,%.3f,%.3f", view.pose.position.x, view.pose.position.y, view.pose.position.z);
					ImGui::Text("Dir : %.3f,%.3f,%.3f,%.3f", view.pose.orientation.x, view.pose.orientation.y, view.pose.orientation.z, view.pose.orientation.w);
				}

				ImGui::EndFramedGroup();
			}

			if (ImGui::BeginFramedGroup("Params"))
			{
				ImGui::CheckBoxBoolDefault("Can Move Headset", &VRBackend::Instance()->headsetCanMove, true);

				ImGui::EndFramedGroup();
			}

			if (ImGui::BeginFramedGroup("Controllers Actions"))
			{
				const auto& xr_controllerDatas = VRBackend::Instance()->GetXRActionsRef().GetOpenXRControllerDatas();
				size_t idx = 0U;
				for (auto& controllerData : xr_controllerDatas)
				{
					ImGui::Text("--- Controller : %s", (idx == 0) ? "left" : "right");

					ImGui::Text("Pos : %f, %f, %f",
						controllerData.controllerLocation.pose.position.x,
						controllerData.controllerLocation.pose.position.y,
						controllerData.controllerLocation.pose.position.z);
					ImGui::Text("Dir : %f, %f, %f, %f",
						controllerData.controllerLocation.pose.orientation.x,
						controllerData.controllerLocation.pose.orientation.y,
						controllerData.controllerLocation.pose.orientation.z,
						controllerData.controllerLocation.pose.orientation.w);

					ImGui::FramedGroupSeparator();

					ImGui::Text("Grab active %d, current %f, changed %d",
						controllerData.grabValue.isActive,
						controllerData.grabValue.currentState,
						controllerData.grabValue.changedSinceLastSync);

					if (controllerData.grabValue.isActive && 
						controllerData.grabValue.currentState > 0.75f)
					{
						ImGui::Text("Haptic Feedback Sent (Vibration)");
					}

					ImGui::FramedGroupSeparator();

					ImGui::Text("Trigger active %d, current %f, changed %d",
						controllerData.triggerValue.isActive,
						controllerData.triggerValue.currentState,
						controllerData.triggerValue.changedSinceLastSync);

					ImGui::FramedGroupSeparator();

					ImGui::Text("Squeeze active %d, current %f, changed %d",
						controllerData.squeezeValue.isActive,
						controllerData.squeezeValue.currentState,
						controllerData.squeezeValue.changedSinceLastSync);

					ImGui::FramedGroupSeparator();

					ImGui::Text("ThumbSitck X : %f",
						controllerData.thumbstickXValue.currentState);
					ImGui::Text("ThumbSitck Y : %f",
						controllerData.thumbstickYValue.currentState);

					idx++;
				}

				ImGui::EndFramedGroup();
			}
		}
	}
}

void VRGui::SetCodeTree(std::weak_ptr<CodeTree> vCodeTreeWeak)
{
	m_CodeTreeWeak = vCodeTreeWeak;
}

#endif // USE_VR