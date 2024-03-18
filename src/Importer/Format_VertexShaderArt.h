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
#include <vector>

#include <ctools/cTools.h>

#include <picojson.h>
#include <fstream>

namespace VertexShaderArtFormat
{
	struct VertexShaderArtStruct
	{
		std::string id;
		std::string creationDate;
		std::string ModificationDate;
		std::string origId;
		std::string name;
		std::string user;
		std::string avatar;
		std::string num;
		std::string mode;
		std::string sound;
		std::string lineSize;
		ct::fColor backColor;
		std::string shader;
		std::string revisionId;

		static std::string GetGoodUrl(const std::string& vBaseUrl, std::string *vId)
		{
			std::string realUrl;

			if (vBaseUrl.find("vertexshaderart.com") != std::string::npos) // vertexshaderart
			{
				const size_t lastSlash = vBaseUrl.find_last_of('/');
				if (lastSlash != std::string::npos)
				{
					*vId = vBaseUrl.substr(lastSlash + 1);
				}
				realUrl = vBaseUrl + "?format=json";
			}

			return realUrl;
		}

		static VertexShaderArtStruct ParseBuffer(const std::string& vBuffer)
		{
			std::string correctedJson = vBuffer;

			//ReplaceString(correctedJson, "\"{", "{");
			//ReplaceString(correctedJson, "}\"", "}");
			/*ReplaceString(correctedJson, "\\\\", "\\");
			ReplaceString(correctedJson, "\\\n", "\n");
			ReplaceString(correctedJson, "\\n", "\n");
			ReplaceString(correctedJson, "\\\t", "\t");
			ReplaceString(correctedJson, "\\t", "\t");
			ReplaceString(correctedJson, "\\\"", "\"");*/
			
			// on sauve le fichier pour la mise au point
			std::ofstream docFile("lastImportBuffer.txt", std::ios::out);
			if (docFile.bad() == false)
			{
				docFile << correctedJson;
				docFile.close();
			}

			VertexShaderArtStruct vsaStr;

			picojson::value json;
			picojson::parse(json, correctedJson);
			std::string err = picojson::get_last_error();
			if (err.empty())
			{
				if (json.contains("_id"))
				{
					picojson::value code = json.get("_id");
					vsaStr.id = code.to_str();
				}
				if (json.contains("createdAt"))
				{
					picojson::value user = json.get("createdAt");
					vsaStr.creationDate = user.to_str();
				}
				if (json.contains("modifiedAt"))
				{
					picojson::value parent = json.get("modifiedAt");
					vsaStr.ModificationDate = parent.to_str();
				}
				if (json.contains("origId"))
				{
					picojson::value parent = json.get("origId");
					vsaStr.origId = parent.to_str();
				}
				if (json.contains("name"))
				{
					picojson::value parent = json.get("name");
					vsaStr.name = parent.to_str();
				}
				if (json.contains("username"))
				{
					picojson::value parent = json.get("username");
					vsaStr.user = parent.to_str();
				}
				if (json.contains("avatarUrl"))
				{
					picojson::value parent = json.get("avatarUrl");
					vsaStr.avatar = parent.to_str();
				}
				if (json.contains("settings"))
				{
					picojson::value valSettings = json.get("settings");
					
					if (valSettings.contains("num"))
					{
						picojson::value parent = valSettings.get("num");
						vsaStr.num = parent.to_str();
					}
					if (valSettings.contains("mode"))
					{
						picojson::value parent = valSettings.get("mode");
						vsaStr.mode = parent.to_str();
					}
					if (valSettings.contains("sound"))
					{
						picojson::value parent = valSettings.get("sound");
						vsaStr.sound = parent.to_str();
					}
					if (valSettings.contains("lineSize"))
					{
						picojson::value parent = valSettings.get("lineSize");
						vsaStr.lineSize = parent.to_str();
					}
					if (valSettings.contains("backgroundColor"))
					{
						if (valSettings.get("backgroundColor").is<picojson::array>())
						{
							int i = 0;
							float col[4];
							picojson::array backColorJson = valSettings.get("backgroundColor").get<picojson::array>();
							for (picojson::array::iterator it_out = backColorJson.begin(); it_out != backColorJson.end(); ++it_out)
							{
								picojson::value channel = *it_out;
								col[i++] = ct::fvariant(channel.to_str()).GetF();
							}
							vsaStr.backColor = ct::fColor(col, 4, 1.0f);
						}
					}
					if (valSettings.contains("shader"))
					{
						picojson::value parent = valSettings.get("shader");
						vsaStr.shader = parent.to_str();
					}
				}
				if (json.contains("revisionId"))
				{
					picojson::value parent = json.get("revisionId");
					vsaStr.revisionId = parent.to_str();
				}
			}

			return vsaStr;
		}
	};
}


// la structure json  de ce shader 
/*
https://www.vertexshaderart.com/art/LPS8BeAeCDomFZXzX?format=json
{
"_id":"LPS8BeAeCDomFZXzX",
"createdAt":"2017-11-28T18:41:16.304Z",
"modifiedAt":"2017-11-28T19:14:21.200Z",
"origId":"qtwQmSqQKkS3ptSYN",
"name":"omg",
"username":"gman",
"avatarUrl":"https://secure.gravatar.com/avatar/dcc0309895c3d6db087631813efaa9d1?default=retro&size=200",
"settings":
"{
\"num\":14909,
\"mode\":\"TRIANGLES\",
\"sound\":\"https://soundcloud.com/dirtybirdrecords/claudevonstroke-dood\",
\"lineSize\":\"NATIVE\",
\"backgroundColor\":[1,1,1,1],
\"shader\":\""
\"revisionId":"WbRMSomP7naZo57Gx"
}
*/

