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

#include "HelpManager.h"
#include <ctools/cTools.h>
#include <Uniforms/UniformVariant.h>
#include <ctools/FileHelper.h>
#include <ImGuiPack.h>

// Following includes for Windows LinkCallback
#define WIN32_LEAN_AND_MEAN
#ifdef WIN32
#include <Windows.h>
#include "Shellapi.h"
#elif defined(LINUX) or defined(APPLE)
#endif
#include <string>

//#include "fonts/comicbd_ttf_compressed.h"

void LinkCallback(ImGui::MarkdownLinkCallbackData data_);
inline ImGui::MarkdownImageData ImageCallback(ImGui::MarkdownLinkCallbackData data_);
static ImGui::MarkdownConfig mdConfig{ LinkCallback, nullptr, ImageCallback, "", { {nullptr, true }, {nullptr, true }, {nullptr, false } } };

static std::string _selectedPath = "";

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void LinkCallback(ImGui::MarkdownLinkCallbackData data_)
{
	const std::string url(data_.link, data_.linkLength);
	if (!data_.isImage)
	{
        FileHelper::Instance()->OpenUrl(url);
#ifdef WIN32
		ShellExecuteA(nullptr, "open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(LINUX) or defined(APPLE)

#endif
	}
}

inline ImGui::MarkdownImageData ImageCallback(ImGui::MarkdownLinkCallbackData /*data_*/)
{
	// In your application you would load an image based on data_ input. Here we just use the imgui font texture.
	const ImTextureID image = ImGui::GetIO().Fonts->TexID;
	const ImGui::MarkdownImageData imageData{ true, false, image, ImVec2(40.0f, 20.0f) };
	return imageData;
}

void Markdown(const std::string& markdown_)
{
	// fonts for, respectively, headings H1, H2, H3 and beyond
	ImGui::Markdown(markdown_.c_str(), markdown_.length(), mdConfig);
}

ImFont* HelpManager::AddFont(std::string vFontFileName, float vFontSize)
{
	ImFont* res = nullptr;

	ImGuiIO& io = ImGui::GetIO();

	res = io.Fonts->AddFontFromFileTTF(vFontFileName.c_str(), vFontSize);
	
	if (res)
	{
		/*// Build texture atlas
		unsigned char* pixels;
		int width, height;

		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

		// Upload texture to graphics system
		GLint last_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		glGenTextures(1, &g_FontTexture);
		glBindTexture(GL_TEXTURE_2D, g_FontTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		// Store our identifier
		io.Fonts->TexID = (ImTextureID)(intptr_t)g_FontTexture;

		// Restore state
		glBindTexture(GL_TEXTURE_2D, last_texture);*/
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

HelpManager::HelpManager()
{
	
}

HelpManager::~HelpManager()
{

}

static ImFont* contentFont = nullptr;


void HelpManager::InitFont()
{
	ImGuiIO& io = ImGui::GetIO();

	io.Fonts->Clear();

	io.Fonts->AddFontDefault();

	ImFont* font = nullptr;

	const float fontSize = 13.0f;

	font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(proggy_clean_ttf_compressed_data_base85, fontSize * 2.0f); //io.Fonts->AddFontFromFileTTF("comicbd.ttf", fontSize * 2.0f);
	if (font == nullptr) font = io.Fonts->Fonts[0];
	mdConfig.headingFormats[0].font = font;

	font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(proggy_clean_ttf_compressed_data_base85, fontSize * 1.75f);
	if (font == nullptr) font = io.Fonts->Fonts[0];
	mdConfig.headingFormats[1].font = font;

	font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(proggy_clean_ttf_compressed_data_base85, fontSize * 1.5f);
	if (font == nullptr) font = io.Fonts->Fonts[0];
	mdConfig.headingFormats[2].font = font;

	font = io.Fonts->AddFontFromMemoryCompressedBase85TTF(proggy_clean_ttf_compressed_data_base85, fontSize * 1.25f);
	if (font == nullptr) font = io.Fonts->Fonts[0];
	contentFont = font;
}

void HelpManager::Init(std::string vAppName)
{
	puAppName = vAppName;

	if (puAppName == "NOODLESPLATE")
	{
		AddUniformHelp("0.NoodlesPlate ?", "", "", "", "");
	}

	if (puAppName == "SDFFONTDESIGNER")
	{
		AddUniformHelp("0.SdfFontDesigner ?", "", "", "", "");
	}

	AddUniformHelp("0.Requirement", "", "", "", "");
	AddUniformHelp("1.Issue, Features Request, Discuss", "", "", "", "");
	AddUniformHelp("2.Features", "", "", "", "");

	if (puAppName == "NOODLESPLATE")
	{
		AddUniformHelp("2.Features", "Shader Import", "From GlslSandbox", "", "");
		AddUniformHelp("2.Features", "Shader Import", "From ShaderToy", "", "");
		AddUniformHelp("2.Features", "Shader Import", "From VertexShaderArt", "", "");
		//AddUniformHelp("2.Features", "Node Graph", "", "", "");
	}

	if (puAppName == "NOODLESPLATE")
	{
		AddUniformHelp("3.Shader Scripting", "Shader Structure", "FrameBuffer Section", "", "");
	}

	AddUniformHelp("3.Shader Scripting", "Shader Structure", "Uniforms Section", "", "");
	AddUniformHelp("3.Shader Scripting", "Shader Structure", "Note Section", "", "");
	AddUniformHelp("3.Shader Scripting", "Shader Structure", "Common Section", "", "");

	if (puAppName == "NOODLESPLATE")
	{
		AddUniformHelp("3.Shader Scripting", "Shader Structure", "Vertex Section", "", "");
		AddUniformHelp("3.Shader Scripting", "Shader Structure", "Geometry Section", "", "");
	}

	AddUniformHelp("3.Shader Scripting", "Shader Structure", "Fragment Section", "", "");

	if (puAppName == "NOODLESPLATE")
	{
		AddUniformHelp("3.Shader Scripting", "Shader Structure", "Compute Section", "", "");
	}

	AddUniformHelp("3.Shader Scripting", "Files Inclusion", "", "", "");
	AddUniformHelp("3.Shader Scripting", "In-Code Configuration", "On-Section Config Name", "", "");
	AddUniformHelp("3.Shader Scripting", "In-Code Configuration", "In-Section Config Name", "", "");

	AddUniformHelp("3.Shader Scripting", "Uniforms", "Sections(arrangement)", "", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Buffer", "");

	if (puAppName == "NOODLESPLATE")
	{
		AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Camera", "");
	}

	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Button", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Checkbox", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Radio", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Color", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "ComboBox", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Cubemap", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Date", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "DeltaTime", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Frame Counter", "");

	if (puAppName == "NOODLESPLATE")
	{
		AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Gizmo", "");
	}

	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Midi", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Mouse", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Sliders", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Texture 2D", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Texture 3D", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Time", "");
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "Sound", "");
#ifdef USE_VR
	AddUniformHelp("3.Shader Scripting", "Uniforms", "Widgets", "VR", "");
#endif
}

void HelpManager::AddUniformHelp(std::string vCat0, std::string vCat1, std::string vCat2, std::string vCat3, std::string vTags)
{
	if (!vCat0.empty())
		puTree[vCat0];
	if (!vCat1.empty())
		puTree[vCat0][vCat1];
	if (!vCat2.empty())
		puTree[vCat0][vCat1][vCat2];
	if (!vCat3.empty())
		puTree[vCat0][vCat1][vCat2].emplace_back(vCat3);
	if (!vTags.empty())
	{
		std::string path = vCat0;
		if (!vCat1.empty())
			path += "/" + vCat1;
		if (!vCat2.empty())
			path += "/" + vCat2;
		if (!vCat3.empty())
			path += "/" + vCat3;

		auto arr = ct::splitStringToVector(vTags, " ", false);
		for (auto it = arr.begin(); it != arr.end(); ++it)
		{
			puPathForTags[*it] = path;
		}
	}
}

void HelpManager::DrawImGui()
{
	DrawTree();
	
	ImGui::SameLine();

	DrawContent();
}

void HelpManager::DrawTree()
{
	ImGui::BeginChild("##HelpTreePane", ImVec2(300,0));

	ImGui::PushFont(contentFont);

	for (auto it0 = puTree.begin(); it0 != puTree.end(); ++it0)
	{
		std::string path = it0->first;

		if (ImGui::Selectable(it0->first.c_str(), _selectedPath == path))
		{
			_selectedPath = path;
		}

		ImGui::Indent();

		for (auto it1 = it0->second.begin(); it1 != it0->second.end(); ++it1)
		{
			std::string path2 = path + "/" + it1->first;
			
			if (ImGui::Selectable(it1->first.c_str(), _selectedPath == path2))
			{
				_selectedPath = path2;
			}

			ImGui::Indent();

			for (auto it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
			{
				std::string path3 = path2 + "/" + it2->first;

				if (ImGui::Selectable(it2->first.c_str(), _selectedPath == path3))
				{
					_selectedPath = path3;
				}

				ImGui::Indent();

				for (auto it3 = it2->second.begin(); it3 != it2->second.end(); ++it3)
				{
					std::string path4 = path3 + "/" + *it3;

					if (ImGui::Selectable((*it3).c_str(), _selectedPath == path4))
					{
						_selectedPath = path4;
					}
				}

				ImGui::Unindent();
			}

			ImGui::Unindent();
		}

		ImGui::Unindent();
	}

	ImGui::PopFont();

	ImGui::EndChild();
}

void HelpManager::DrawContent()
{
	ImGui::BeginChild("##helpcontent", ImVec2(0, 0));
	
	//ImGui::PushFont(contentFont);

	std::string markdownText = u8R"()";

	if (_selectedPath == "0.NoodlesPlate ?")
	{
		markdownText = u8R"(
NoodlesPlate is a GLSL Shader Editor (almost an IDE) :
  * Offline : run only on Windows (for the moment), tested on win 7. Not tested on other version, but there is no reason for it not to work)
  * LightWeight : near 5 MO and no lib, just an exe, auto installed at first execution, in the directory where you put it.
  * Easy : Focused on the display and the tuning, leave editing to an external text editor of your choice (associated with .glsl file extention). i use [NotePad++](https://notepad-plus-plus.org/fr/)
  * Powerfull : you have many advanced features likes these ones :
    * Vertex, Geometry, and fragment Shader in one file
    * Can Import from url, ShaderToy (partially), VertexShaderArt (partially), GlslSandbox (Fully), ex : [here](https://twitter.com/aiekick/status/1097303744717438983)
    * a lot of widgets available for easy tuning of your uniforms, like, [sliders, checkbox, combobox](https://twitter.com/aiekick/status/1099803274180521984), [gizmo](https://twitter.com/aiekick/status/1095428778908680192), etc..
    * Support of Multi Pass Buffers
    * Can play with FrammeBuffer attachment (until 8) switchable in the ui.
    * Many primitive type : Quad (like on ShaderToy and GlslSanbox), Point (like on VertexShaderArt), and Mesh (Support only Obj File for the moment)
    * Include File Support (you can put in include file you tools, the include file can contain each Shader Sections, car ovveride parent section type, for complex imbrication if needed)
    * In-Code Config Systeme Selectable via ComboBox, let you switch between functionnality of your shader easily
    * Compute Shader Support, let you fill texture 2d on texture 3d by example (not very stable for the moment)
    * Support of mesh instancing
    * NodeGraphSystem (experimental and not stable so pay attention when using it, i have also auto layout bug) you can see a quick vid of it [here](https://twitter.com/aiekick/status/1095138951181017088) 

I Discovered Shaders in 2014 for a game, and im in love with all the procedural stuff we can create with that since this moment.
I have designed this soft for have an easy way to tune shaders via widgets. but As a cad Designer, i not work in the GFX industry, 
so this soft, was a way i choosed for learn all the things around the gfx programming.
So maybe the way's i used here for all these features, are not the best. but i want learn, 
so if you have idea's, issue's, new feature's, please let me discuss with you about your request's on GitHub :)

I have designed this soft by passions for passionates, so i hope you will have a lot of pleasure to use it. 
Don't' hesitate to speak of it around you, its very pleasant to hear, that what we design is used and maybe enjoyed.

I have posted many video on twitter about NoodePlates, if you want to see how some features are working : [here](https://twitter.com/hashtag/noodlesplate?src=hash)

NoodlesPlate is designed with the help of many free libs, you can find the full list in the About Dialog.

Happys Codding. :)

AieKick.

)";
	}
	else if (_selectedPath == "0.SdfFontDesigner ?")
	{
		markdownText = u8R"(
SdfFontDesigner is a GLSL Shader Editor for create font bitmap :
  * Offline : run only on Windows (for the moment), tested on win 7. Not tested on other version, but there is no reason for it not to work)
  * LightWeight : near 5 MO and no lib, just an exe, auto installed at first execution, in the directory where you put it.
  * Easy : Focused on the display and the tuning, leave editing to an external text editor of your choice (associated with .glsl file extention). i use [NotePad++](https://notepad-plus-plus.org/fr/)
  * Powerfull : you have many advanced features likes these ones :
    * a lot of widgets available for easy tuning of your uniforms, like, sliders, checkbox, combobox, etc..
    * Support of Multi Pass Buffers
    * Include File Support (you can put in include file you tools, the include file can contain each Shader Sections, car ovveride parent section type, for complex imbrication if needed)
    * In-Code Config Systeme Selectable via ComboBox, let you switch between functionnality of your shader easily
    
I Discovered Shaders in 2014 for a game, and im in love with all the procedural stuff we can create with that since this moment.
I have designed this soft for have an easy way to tune shaders via widgets. but As a cad Designer, i not work in the GFX industry, 
so this soft, was a way i choosed for learn all the things around the gfx programming.
So maybe the way's i used here for all these features, are not the best. but i want learn, 
so if you have idea's, issue's, new feature's, please let me discuss with you about your request's on GitHub :)

I have designed this soft by passions for passionates, so i hope you will have a lot of pleasure to use it. 
Don't' hesitate to speak of it around you, its very pleasant to hear, that what we design is used and maybe enjoyed.

SdfFontDesigner is designed with the help of many free libs, you can find the full list in the About Dialog.

Happys Codding. :)

AieKick.

)";
	}
	else if (_selectedPath == "0.Requirement")
	{
		markdownText = u8R"(
the minimum requirement i thinck is :
* window operating sytem 32 or 64 bits dependant of the version :)
* a modern gpu with opengl 3.3 mini
)";
	}
	else if (_selectedPath == "1.Issue, Features Request, Discuss")
	{
		markdownText = u8R"(
you can post an issue, ask a feature of just discuss about my soft => [here](https://github.com/aiekick/SdfFontDesigner/issues)
)";
	}
	else if (_selectedPath == "2.Features")
	{
		markdownText = u8R"(In this section yiu will find informations about specifics features.)";
	}
	else if (_selectedPath == "2.Features/Shader Import")
	{
		markdownText = u8R"(In this section yiu will find informations about Import of Shaders By url
for the moment, three web site are supported (partially) :
  * [GlslSandbox](http://glslsandbox.com/) (fully)
  * [ShaderToy](https://www.shadertoy.com) (partially)
  * [VertexShaderArt](https://www.vertexshaderart.com/) (partially)

for Import a Shader you need to :
  * have a url fo the shader you want to import, the site will be recognized automatically
  * go To File/Import, here you have two choices :
    * Custom Url : is for your url
    * From Smaples Url : is a library of all links you/me have imported. this list will grow up with time :) already filled with some of the shaders im loving

for some platforms you need to do some specific things before import.

please go in the subsection for informations.
)";
	}
	else if (_selectedPath == "2.Features/Shader Import/From GlslSandbox")
	{
		markdownText = u8R"(Nothing special for the moment)";
	}
	else if (_selectedPath == "2.Features/Shader Import/From ShaderToy")
	{
		markdownText = u8R"(
For Import url from Shadertoy, you need to have an [Api Key](https://www.shadertoy.com/api)

You can create this Api Key, in your ShaderToy profile / Config / Manage Your Apps.
After that you just need to create this api key and put it in the ApiKey field in the import Bar.

You need to know another things. some shader can't be imported. A shader can be imported if he have been plublished in "public + api".
if not, NoodesPlate will say, 'he can't found the shader at url'.

After that, if you have good knowledge about the file structure, you have no other choice, than, create import manually yourself.

)";
	}
	else if (_selectedPath == "2.Features/Shader Import/From VertexShaderArt")
	{
		markdownText = u8R"(Nothing special for the moment)";
	}
	else if (_selectedPath == "2.Features/Node Graph")
	{
		markdownText = u8R"(
the Node Graph is an experimental feature, let you tune function of the current buffer, and replace some input who have the same signature only.
a little buggy for the moment.

the node graph is automatically generated form the analysis of the shader :)

you need first to parse the good shader in teh curreent buffer ( i know the layout is buggy, if you want recalc it, unsselect/select one checkbox)
after that you can move a like to another. and click compil for have the result

see a demo [here](https://twitter.com/aiekick/status/1095138951181017088) 

need improveing / debug, i let it be in the frist beat release, but will chnage in futrue updates.

it need to be a system for manipulate buffers, and assmeble cutom fucntion from a library.
the goal of this is not to become a full nodegraph creation systeme, but a mixed, classical code / and quick tuning :)

)";
	}
	else if (_selectedPath == "3.Shader Scripting")
	{
		markdownText = u8R"(
Here i will describe the shader scripting. its more than just glsl.
my system is designed around glsl, and extend it with cool feature
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Shader Structure")
	{
		markdownText = u8R"(
A shader file integrate many sections in one file :

the possibles sections are listed in the tree

each section can have some params after.

the general syntax si :

@SECTION_NAME SECTION(name) KEY(params) KEY2(params) etc

the key SECTION() is a special section. you can have many section with a name. 
like that is the left panel, you have a combobox when you can choose what name you want use

all theses key can be commented by // or with /* */

you can have many section in the order you want.

@FRAGMENT

@UNIFORMS

@FRAGMENT

is valid :)

you have some samples available on the repo : [here](https://github.com/aiekick/NoodlesPlate)
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Shader Structure/FrameBuffer Section")
	{
		markdownText = u8R"(
this section correspond to the FRAMEBUFFER. this one control the size and the format of the FBO.

it is written like that :

@FRAMEBUFFER //FORMAT() SIZE() RATIO()

here we have 3 possibles keys :

  * FORMAT(params) with params can be 'float' or 'byte'. GlslSanbox use byte texture. ShaderToy use float Texture
  * SIZE(params) with params who correspond to size x,y or a picture file with the params (picture:toto.png or toto.jpg)
  * RATIO(prams) with params who correspont to a screen ratio or a picture file with the params (picture:toto.png or toto.jpg)

with no key the default it FORMAT(float) and no size or ratio defined, so its adjusted to screen

)";
	}
	else if (_selectedPath == "3.Shader Scripting/Shader Structure/Uniforms Section")
	{
		markdownText = u8R"(
this section will contain all the uniforms

it is written like that :

@UNIFORMS

there is no key or params for this one

when you need an uniforms you need to put in after this Section

if you want to place an uniforms on top of a function you can apply this trick

@FRAGMENT

code

@UNIFORME

uniform float toto;

@FRAGMENT

code

:)
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Shader Structure/Note Section")
	{
		markdownText = u8R"(
this section correspond to the nota.

it is written like that :

@NOTE

you can put what you want like text, description etc. its a standard text format. 
when you import a shader, the infos of the shader are set in this section

you can show in the ide the content of note via tne button Note

)";
	}
	else if (_selectedPath == "3.Shader Scripting/Shader Structure/Common Section")
	{
		markdownText = u8R"(
this section ia a common section. so is content can be merge for the three type of shaders, Vertes, Geometry and fragment

it is written like that :

@COMMON

no key or params for this one

its useful when located in include files.
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Shader Structure/Vertex Section")
	{
		markdownText = u8R"(
this section correspond to the vertex shader.

it is written like that :

@VERTEX. the possible key and parmas are QUAD() POINTS(params) MESH(params) INSTANCES(params) DISPLAY(params)

  * QUAD() it the defualt key, when you have nothing or specified. its the quad primitive like in GlslSandbox and ShaderToy

  * POINTS() it the primitve point like in VertexShaderArt. the params can be :
    * a number who correspond to the total point count : POINTS(VALUE) => POINTS(1000)
    * a slider notation, for have a slider in the left pane, for tune it : POINTS(INF:SUP:DEFAULT) => POINTS(1000:100000:50000)
    * you have a sepcific uniform for get the point count : uniform float(maxpoints) name;

  * MESH() is the primtive mesh. you can load a mesh. at this moment you can uplaod only a mesh file obj with triangular faces.
    * if you sepcify file like MESH(file.obj). the mesh will be loaded auto matically after section header modification
    * if you specify params or nothings, you always loa a file after  (in the left pane)

  * INSTANCES() is the instance count, like point you can sepcify a number or a slider notation, and in this case you have a slider in the left pane
    * you have a specific uniform for get the instances count : uniform float(maxinstances) name;
    * see this sample if you want : [SdfInstancing](https://github.com/aiekick/NoodlesPlate/tree/master/SdfInstancing)

  * DISPLAY() is a params for permit the render mode of the meshs. you have many possibilities :
    * DISPLAY(POINT�S,LINES,TRIANGLES) like that you have a combobox in the left pane for choice the render mode in this list
    * DISPLAY() you have the full choice, LINES / LINE_STRIP / LOOP / POINTS / TRIANGLES / TRIANGLES_FAN / TRIANGLES_STRIP. its the defualt if you have write nothings
    * DISPLAY(TRIANGLES), no combobox, here, the only possible render mode is fixed like that

after the section you need to write that (classic glsl) :

for QUAD :
layout(location = 0) in vec2 name;

for POINTs :
layout(location = 0) in float vertexIdName;

for mesh : maybe you mesh not have vertex color or normal bu if yes you need that
layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 normalname;
layout(location = 2) in vec4 colorName;

you can define what you want for these names, the important things is the location number
)";
	}
	
	else if (_selectedPath == "3.Shader Scripting/Shader Structure/Geometry Section")
	{
		markdownText = u8R"(
this section correspond to the geometry shader

it is written like that

@GEOMETRY, no params for this one

you have in the left pane, a checkbox for activate or not the geometry shader
so you have also a specific uniform available for get in glsl is we use or note the geometry shader : uniform float(usegeometry) name;

in geometry shader you nedd ot wirte that after the section header :

layout(location = 0) in float vertexIdName[]; the name can be waht you want

here you will put the input primitive type
layout(triangles) in; // input primitive(s) (must match the current render mode) (points / lines / triangles / lines_adjacency / triangles_adjacency)

and output primitve type. its not specific to NoodlesPlate its classic glsl
layout(triangle_strip, max_vertices=3) out; // output primitive(s) (points / line_strip / triangle_strip)

for have many test case you can write some In-Code Config or Section Config, see in tree
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Shader Structure/Fragment Section")
	{
		markdownText = u8R"(
this section correspond to the fragment shader

it is written like that :

@FRAGMENT, no params here

after that you need to specifiy the vec4 you wna ttu write into.

the location correspond to one of the attachment 0 to 7 :

layout(location = 0) out vec4 fragColor;

for declare other attachment, this notation is not sufficient, go to the tree : Uniforms / Buffer
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Shader Structure/Compute Section")
	{
		markdownText = u8R"(
this section corrspond to the coomptue shader

it is written like that :

@COMPUTE, no params.

you cant have this section in the main buffer, only in a sub buffer.

the only section you can have with the Compute, are COMMON, UNIFORMS and NOTE
fot the classi shader, you can put all except this COMPUTE.

i will comeback to this feature in a futrue version, because the systeme is not stable and complex to explain
 
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Files Inclusion")
	{
		markdownText = u8R"(
you can put in code : #include "file.glsl"

the content of this include file will be merged by NoodlesPlate for compiling in the parent shader

by the way in this include file, you can define Sections :

@UNIFORMS
@FRAMEBUFFER
@COMMON
@FRAGMENT
@VERTEX
@GEOMETRY
@NOTE

by example if your main shader is like that

/////////////

@UNIFORMS

code

@VERTEX

#include "toto.glsl"
code

@FRAGMENT

#include "toto.glsl"
code

////////////

and if toto.glsl contain :

@UNIFORMS

code_toto_unif

@FRAGMENT

code_toto_frag

////////////

as you see toto.glsl not contian veretx section

so the only section whill be merge are UNIFORMS in Parent VERTEX and Parent FRAGMENT, and FRAGMENT in Parent Fragment, like that :

@UNIFORMS

code_toto_unif
code

@VERTEX

code

@FRAGMENT

code_toto_frag
code

teh include file can have his own uniforms and as a include file can be common to many buffer, all the unfirosma re grouped.
so you tune this include file for all buufer who use it :) powerfull no ? :)
)";
	}
	else if (_selectedPath == "3.Shader Scripting/In-Code Configuration/On-Section Config Name")
	{
		markdownText = u8R"(
like said before, you have a general KEY for all sections.

its written like that :

@SECTION_NAME, and after you can put the key SECTION(params), with params who contain a name
with that you have in the left pane a combobox. let you selct what section name you want to apply

the section name will be used for select all section all over the whole file, with includes

if we have :

@UNIFORMS SECTION(toto)

@UNIFORMS SECTION(toto2)

@VERTEX

@FRAGMENT SECTION(toto)

@FRAGMENT SECTION(toto2)

we can select within a combobox who will containt the lsit toto, and toto2

if we select toto

we will obtain the shader :

@UNIFORMS SECTION(toto)
@VERTEX
@FRAGMENT SECTION(toto)

and if we select toto2 we will obtain :

@UNIFORMS SECTION(toto2)
@VERTEX
@FRAGMENT SECTION(toto2)

the interesting things, is, with include files.

all the section defined in an includes files will overid the defintion section of the parent

like that you can handle wiht a combobox many vertex primitive by example.

see this sample if you want : [FluidHeightMap](https://github.com/aiekick/NoodlesPlate/tree/master/FluidHeightMap)
)";
	}
	else if (_selectedPath == "3.Shader Scripting/In-Code Configuration/In-Section Config Name")
	{
		markdownText = u8R"(
You have also the possibily to have In-Code Configs;, selectable for each section in a combobox in the left pane

you need to write that :

@CONFIG_START CONFIG_NAME, with config_NAMLE is what you want

put code here

@CONFIG_END

by example :

if you have a fragment code like that :

@FRAGMENT

	vec3 p = vec3(0);

@CONFIG_START Scaled_1
	p.y = 0.;
	p.xz *= chan.r*2.0-1.0;
@CONFIG_END

@CONFIG_START Scaled_2
	p.y = 0.;
	p.xz *= clamp(chan.r+0.5,0.,1.);
@CONFIG_END

@CONFIG_START Elevated
	p.y = chan.r * altitude ;
@CONFIG_END

by selecting Scaled_1, you will obtain :

@FRAGMENT
	vec3 p = vec3(0);
	p.y = 0.;
	p.xz *= chan.r*2.0-1.0;

by selecting Elevated, you will obtain :
 
@FRAGMENT
	vec3 p = vec3(0);
	p.y = chan.r * altitude ;

etc..

you will ahev the choice fo each section shader type :

@UNIFORMS
@VERTEX
@FRAGMENT
@GEOMETRY
@COMPUTE

if you put config in the section @COMMON, you will have the config name where your Common Will be merged.
is he will emrge in VERTEx and FRAGMENT, you will get the choice the the both sections

see this sample if you want : [FluidHeightMap](https://github.com/aiekick/NoodlesPlate/tree/master/FluidHeightMap)
)";
	}

	else if (_selectedPath == "3.Shader Scripting/Uniforms")
	{
		markdownText = u8R"(
the general syntax of an uniform in NoodlesPlate is :

uniform(section) type(params) name; // comment
* params can contain any category separated by a ':'
* each category can have many value separated by a ','
* the commetn can be shonw by let the mouse cursor ont eh uniform name in the unfiroms pane

example :

uniform vec2(0:1:0.5,0.8) slider;
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Sections(arrangement)")
	{
		markdownText = u8R"(
each uniform are attached to a default section called 'default'

but you can define your own :
* uniform(hidden) => will hide the uniform, 'hidden' is a sepcific section name for that
* uniform(toto) => will appear uniform under section toto

define the display order : (order must >= 0)
* uniform(0) float a;
* uniform(1) float b;
* these ,order, will display b after a.

or define the display condition : (currently only work with checkbox and combobox uniform
* with checkbox :
  * uniform(checkName==true)
  * uniform(checkName!=true)
* with combobox :
  * uniform(comboName==optionA)
  * uniform(comboName!=optionB)

you can put these 3 section params together, or separated, or what you want, in any order you want  :
* examples :
  * uniform(toot:0:checkName==true)
  * uniform(0:toto:checkName==true)
  * uniform(checkName==true:0:toto)
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets")
	{
		static UniformVariant uniformSlider;
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Buffer")
	{
		markdownText = u8R"(
Buffer Uniform are the way for have multiPass feature :

* you can load another shader file result attachment in a sampler2D
* you can load another attachment of the current buffer

ex :
access to the current fbo, id of the attachment(0 to 7) = > fragColor, fragColor1 to fragColor7, but you can name that like you want. all is defiend in glsl
* uniform sampler2D(buffer:target=0-7) name;

same but access to the fbo of another shader, filename must be without extention
work only if a shader have a fbo attached;
* uniform sampler2D(buffer:target=id:file=filename) name;

same logic for these two, but get buffer size");
* uniform vec2(buffer:target=id) name;");
* uniform vec2(buffer:target=id:file=filename) name;

you have option like wraping, filtering, mipmap, filp on sampler2D :
* uniform sampler2d(buffer:target=id:file=filename:flip=true:mipmap=true:wrap=clamp:filter=linear) name;

if you define nothign, the defualt is like this :
* uniform sampler2d(buffer:target=id:file=filename:flip=false:mipmap=true:wrap=clamp:filter=linear) name;

pay attention to never use the name buffer ofr an uniform, 
its a reserved name, and the resulting erros are weird and not related to that name, so difficult to know what is rely :)

)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Camera")
	{
		markdownText = u8R"(

when you use mouse on screen you manipulate the cam, without know it :)

get the cam mvp
* uniform mat4(camera:mvp) name;
get the cam mvp with params. replace << rotX,rotY:zoom:translateX,translateY >> by values
* uniform mat4(camera:mvp:rotX,rotY:zoom:translateX,translateY) name;

get the cam proj
* uniform mat4(camera:p) name;

get the cam view
* uniform mat4(camera:v) name;

get the cam model
* uniform mat4(camera:m) name;

get the cam normal matrix
* uniform mat4(camera:nm) name;
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Button")
	{
		markdownText = u8R"(
you can define an uniform button (pressed when released) :
many channel can be true at a time
only for float, vec2, vec3, vec4, bool, bvec2, bvec3, bvec4

the syntax is : 
uniform float(button: press me) uButtonF; // when the button is pressed the value of uButtonF is 1.0 else 0.0
uniform vec2(button: press me 1, press me 2) uButtonV2;
uniform vec3(button: press me 1, press me 2, press me 3) uButtonV3;
uniform vec4(button: press me 1, press me 2, press me 3, press me 4) uButtonV4;

uniform bool(button: press me) uButtonB; // when the button is pressed the value of uButtonB is true else false
uniform bvec2(button: press me 1, press me 2) uButtonBV2;
uniform bvec3(button: press me 1, press me 2, press me 3) uButtonBV3;
uniform bvec4(button: press me 1, press me 2, press me 3, press me 4) uButtonBV4;

for test this uniform in glsl, you need to know, that true == 1.0 and false = 0.0
if float base type, so you can do that, by ex :
if (uButtonF > 0.5) do true stuff
if (uButtonF < 0.5) do false stuff
if (uButtonV3.y > 0.5) do true stuff
if (uButtonV3.z < 0.5) do false stuff

if bool base type, so you can do that, by ex :
if (uButtonB == true) do true stuff
if (uButtonB == false) do false stuff
if (uButtonBV3.x == true) do true stuff
if (uButtonBV2.y == false) do false stuff
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Checkbox")
	{
		markdownText = u8R"(
you can define uniform checkbox :
many channel can be true at a time
only for float, vec2, vec3, vec4, bool, bvec2, bvec3, bvec4

the syntax is : 
uniform float(checkbox:true or false) name; // the default parma is true
uniform float(checkbox) name; // the default state is false
uniform vec2(checkbox:1) name; // the default param is 1 and correspond to the channel id to be true
uniform vec3(checkbox:1) name; // the default param is 1 and correspond to the channel id to be true
uniform vec4(checkbox:1) name; // the default param is 1 and correspond to the channel id to be true
uniform bool(checkbox) name; // teh default state is false
uniform bool(checkbox:true) name; // the default param here is true
uniform bvec2(checkbox:1) name; // the default param is 1 and correspond to the channel id to be true
uniform bvec3(checkbox:1) name; // the default param is 1 and correspond to the channel id to be true
uniform bvec4(checkbox:1) name; // the default param is 1 and correspond to the channel id to be true

for test this uniform in glsl, you need to know, that true == 1.0 and false = 0.0
if float base type, so you can do that :
if (name > 0.5) do true stuff
if (name < 0.5) do false stuff
if (name.y > 0.5) do true stuff
if (name.y < 0.5) do false stuff

if bool base type, so you can do that :
if (name == true) do true stuff
if (name == false) do false stuff
if (name.y == true) do true stuff
if (name.y == false) do false stuff
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Radio")
	{
		markdownText = u8R"(
you can define uniform radio : 
only one channel can be true at a time
only for vec2, vec3, vec4, bvec2, bvec3, bvec4

the syntax is : 
uniform vec2(radio:0) name; // the default param is 0 and correspond to the channel to be true
uniform vec3(radio:2) name; // the default param is 2 and correspond to the channel to be true
uniform vec4(radio:3) name; // the default param is 3 and correspond to the channel to be true
uniform bvec2(radio:0) name; // the default param is 0 and correspond to the channel to be true
uniform bvec3(radio:2) name; // the default param is 2 and correspond to the channel to be true
uniform bvec4(radio:3) name; // the default param is 3 and correspond to the channel to be true

x is 0
y is 1
z is 2
w is 3

for test this uniform in glsl, you need to know, that true == 1.0 and false = 0.0
so you can do that :
if (name.x > 0.5) do true stufff for x channel 
if (name.x < 0.5) do false stuff

if bool base type, so you can do that :
if (name.x == true) do true stuff
if (name.x == false) do false stuff
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Color")
	{
		markdownText = u8R"(
you can have uniform color widget :

the syntax is : 
* uniform vec3(color:default0,default1,default2) name;
* uniform vec4(color:default0,default1,default2,default3) name;

)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/ComboBox")
	{
		markdownText = u8R"(
you can have a combobox :

the syntax is : 
* uniform int(combobox:choice0,choice1,choice2,choice3:default_choice) name;
default_choice is optional
in glsl : 
* choice0 will be the value 0
* choice1 will be the value 0
etc..
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Cubemap")
	{
		markdownText = u8R"(
you can have a cubemap uniform :
use 6 texture filename with ext separated by ','
uniform samplerCube(POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z) name;
will be improved/refactored in the future.
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Date")
	{
		markdownText = u8R"(
you can have a date uniform for get date (year, month, day, seconds (epoc time)):

the syntax is : 
*uniform vec4(date) name;
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/DeltaTime")
	{
		markdownText = u8R"(
you can have a deltatime widget who get the render time of the last frame :

the syntax is : uniform float(deltatime) name;
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Frame Counter")
	{
		markdownText = u8R"(
you can have a frame counter uniform.

the syntax is : uniform int(frame) name;
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Gizmo")
	{
		markdownText = u8R"(
you can have gizmo shown on screen. thanks to Cedric Guillemet for his [ImGuizmo](https://github.com/CedricGuillemet/ImGuizmo) :
this gizmo will appear on the screnn, but for using it, you need to use the camera perpspective :

you have in the menu File/GizmoRelated some Shaders for explain how you can use it, by example merge a sdf in a space3d.

when you have done that, this is the syntax of the gizmo :

uniform mat4(gizmo) name;

like its a mat4, you can control, position rotation and scale. it depend and how you use it in your code :)

you have a button, for hide the gizmo, on top right if you want

you need maybe to activate the space3d : Settings/Show 3d Space (maybe you need to reset the cam, in the left panel)
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Mouse")
	{
		markdownText = u8R"(
you can have a mouse uniform, compatible with ShaderToy, GlslSandobx and VertexShaderArt

mouse x,y == gl_fragCoord.xy;
uniform vec2(mouse) name;

mouse x,y == gl_fragCoord.xy / z,w = mouse click pos (ShaderToy Like)
uniform vec4(mouse:2pos_2click) name;

mouse x,y == gl_fragCoord.xy / z,w = mouse click initial pos
uniform vec4(mouse:2pos_2keepclick) name;

mouse x,y == gl_fragCoord.xy / screensize => 0 to 1 (normalized)
uniform vec2(mouse:normalized) name;

mouse x,y == gl_fragCoord.xy / z,w = mouse click pos => 0 to 1 (normalized for eachs)
uniform vec2(mouse:normalized_2pos_2click) name;

mouse x,y == gl_fragCoord.xy / z,w = mouse click initial pos => 0 to 1 (normalized for eachs)
uniform vec2(mouse:normalized_2pos_2keepclick) name;
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Sliders")
	{
		markdownText = u8R"(
you can have usefull Slider uniform widgets :

the syntax is :

uniform float(inf:sup:default) name");
uniform vec2(inf0,inf1:sup0,sup1:default0,default1) name");
uniform vec3(inf0,inf1,inf2:sup0,sup1,sup2:default0,default1,default2) name");
uniform vec4(inf0,inf1,inf2,inf3:sup0,sup1,sup2,sup3:default0,default1,default2,default3) name");

## each group between ':' can be expressed with one value or many :
* if you define 0.5 with a vec3, its like vec3(0.5,0.5,0.5), but you can also define 0.5,0.5,0.5 directly
* you can mix, by ex : vec3(0:1:0.8,0.2,0.5) work well :)
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Texture 2D")
	{
		markdownText = u8R"(
you can have picture widget :

the possible syntax's are :

uniform sampler2d(picture:file=filename:flip=true:mipmap=true:wrap=repeat:filter=linear) name;

you can get alos the size of the texture
uniform vec2(picture:file=filename) name;

you can also have a choose box :

replace 'file' by 'choice'. you cand efin a default file or not
uniform sampler2d(picture:choice=toto.png:flip=true:mipmap=true:wrap=repeat:filter=linear) name;
by right click on the choose box, you can remap the options is not defined.
* if you want redefine all do only :  uniform sampler2d(picture:choice=toto.png)
* if you want only to redefine wrap and filter do only : uniform sampler2d(picture:choice=toto.png:flip=true:mipmap=true) name;
			
after that if you want of get the size on this choose texture, you can target this uniform :
uniform vec2(picture:target=uniforpuname) name;
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Texture 3D")
	{
		markdownText = u8R"( 
you can use texture 3D, for the moment the texture 3d format used by shadertoy

the syntax is : 

uniform sampler3D(volume:file=toto.bin:flip=true:mipmap=true:wrap=repeat:filter=linear) name;

noi choice available for the moment
)";
	}

	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Time")
	{
		markdownText = u8R"(
you can also have a time widget :

the syntax is : 

// basic timer disabled by default
uniform float(time) name;
// timer enabled with ture or disabled with false by default
uniform float(time:true or false) name;
// timer with a number for period, the range will be 0 to 1.59 in cycle
uniform float(time:1.59) name;
// you can put params in any order you want after the widget type, by ex :
uniform float(time:1.59:false) name;
uniform float(time:true:1.59) name;

)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Sound")
	{
		markdownText = u8R"(
you can also have a sound widget :

the syntax is : 

for just get spectrum
uniform sampler1D(sound:file=toto.ext:loop=true) name;

replace file by choice, if you would like to have a button for choose a file :

uniform sampler2D(sound:choice=toto.ext:loop=true:histo=128) name;

only wav and mp3 files are supproted for the moment
)";
//	with history
//	uniform sampler2D(sound:file = toto.ext : loop = true : histo = 128) name;
//	the number after histo is the samples count of the sound historic(fft)

	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/Midi")
	{
	markdownText = u8R"(
you can have a midi uniform for control sliders via midi device.
you can only have float uniform for the moment

the syntax is a bit complex, but you have a helper in the settings dialog with the midi tab,
for detect and get the uniform syntax

the syntax is similar to float sliders but with a midi id before : 
*uniform vec4(midi:id=deviceName,byte0,byte1,byteN,etc..:value=pos_of_the_value_byte:inf:sup:def:step) name;

byte0,1,N are device bytes
the value indicate the index of the bytes who is the value. and this bytes will be set to -1 in the id field

inf, sup, def, step are optional. the default range for midi sliders are 0.0 o 1.0

ex of a uniform with my midi device :
uniform float(midi:id=WORLDEeasycontrol0,176,3,-1:value=2:-1:1:0.2) uCX_x;

the device is : WORLDEeasycontrol0
the bytes are : 176,3,-1 (the 3rd byte is the value, the only byte who change, so set here to -1)
the value are : 2, so the byte -1
)";
	}
	else if (_selectedPath == "3.Shader Scripting/Uniforms/Widgets/VR")
	{
	markdownText = u8R"(
this the uniforms available for VR

* know when the vr is enabled or not.
  by enabled i mean, when you can see images in the headset.

  the syntax is like a checkbox :
    uniform float(vr:use) useVR; [0.0f => 1.0f]
    uniform int(vr:use) useVR; [0 => 1]
    uniform uint(vr:use) useVR; [0U => 1U]
    uniform bool(vr:use) useVR; [false => true]

* get the pose and direction of each eyes :

    uniform vec3(vr:pos) posVR;
    uniform vec3(vr:dir) dirVR;

* when the VR Images can be displayed in the headset, the view and proj matrics 
  of the camera are filled with infos of each eyes.

  so you can access them like that :
  uniform mat4(camera:p) projMatrix;
  uniform mat4(camera:v) viewMatrix;

  and you can also include in your code the space3d.glsl header,
  like that you can get ray_origin point and ray_direction vectors.
  the ray_direction dpeend of the uv coords, so you can use it directly in your code as a camera ray 
  
  so pay attention to the camera settings, can be the cause of weird transform with you headset
  maybe you need to reset it

* the uniforms for the VR controllers (support for generic, occulus touch and valve index)
    uniform vec2(vr:thumb:left) leftThumbstick;
    uniform vec2(vr:thumb:right) rightThumbstick;
	uniform float(vr:trigger:left) leftTrigger;
	uniform float(vr:trigger:right) rightTrigger;
	uniform float(vr:squeeze:left) leftSqueeze;
	uniform float(vr:squeeze:right) rightSqueeze;
)";
	}

	//ImGui::Text(markdownText.c_str());
	Markdown(markdownText);

	//ImGui::PopFont();

	ImGui::EndChild();
}
