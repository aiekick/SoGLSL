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

#pragma once

#include <string>

#include <picojson/picojson.h>

namespace GlslSandboxFormat
{
	
struct GlslSandboxStruct
{
	std::string id;
	std::string code;
	std::string user;
	std::string parent;
	
	static std::string GetGoodUrl(const std::string& vBaseUrl, std::string *vId)
	{
		std::string realUrl;

		if (vBaseUrl.find("glslsandbox.com") != std::string::npos) // ShaderToy
		{
			// on va extraire l'id du shader de l'url
			size_t e = vBaseUrl.find("/e#");
			if (e != std::string::npos)
			{
				e += 3;
				*vId = vBaseUrl.substr(e, vBaseUrl.length() - e);
				realUrl = "http://glslsandbox.com/item/" + *vId;
			}
		}

		return realUrl;
	}

	static GlslSandboxStruct ParseBuffer(const std::string& vBuffer)
	{
		GlslSandboxStruct glStr;

		picojson::value json;
		picojson::parse(json, vBuffer);
		const std::string err = picojson::get_last_error();
		if (err.empty())
		{
			if (json.contains("code"))
			{
				const picojson::value code = json.get("code");
				glStr.code = code.to_str();
			}
			if (json.contains("user"))
			{
				const picojson::value user = json.get("user");
				glStr.user = user.to_str();
			}
			if (json.contains("parent"))
			{
				const picojson::value parent = json.get("parent");
				glStr.parent = parent.to_str();
			}	
		}

		return glStr;
	}
};
}


// la structure json  de ce shader 
// http://glslsandbox.com/e#38000.1
// api std::string : http://glslsandbox.com/item/38000.1
// c'est ca : 
		// {"code":"",
		// "user":"6c7ebdb",
		// "parent":"/e#38000.1"}

