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

#pragma once

#include <string>
#include <vector>

#include <ctools/cTools.h>

#include <picojson.h>

namespace ShaderToyFormat
{

struct ShaderToyInfo
{
	std::string id;
	std::string date;
	std::string viewed;
	std::string name;
	std::string username;
	std::string description;
	std::string likes;
	std::string published;
	std::string flags;
	std::vector<std::string> tags;
	std::string hasliked;
};

struct ShaderToySampler
{
	std::string filter;
	std::string wrap;
	std::string vflip;
	std::string srgb;
	std::string internal;
};
struct ShaderToyInput
{
	std::string id;
	std::string filepath;
	std::string type;
	std::string chan;
	ShaderToySampler sampler;
	std::string published;
};
struct ShaderToyOutput
{
	std::string id;
	std::string chan;
};
struct ShaderToyRenderPass
{
	std::list<ShaderToyInput> inputs;
	std::list<ShaderToyOutput> outputs;
	std::string code;
	std::string name;
	std::string desc;
	std::string type; // image ou buffer
};
struct ShadertoyStruct
{
	std::string version;
	ShaderToyInfo info;
	std::list<ShaderToyRenderPass> renderpass;
	std::string error;

	static std::string GetGoodUrl(const std::string& vBaseUrl, const std::string& vApiKey, std::string *vId)
	{
		std::string realUrl;

		if (vBaseUrl.find("shadertoy.com") != std::string::npos) // ShaderToy
		{
			// on va extraire l'id du shader de l'url
			size_t e = vBaseUrl.find("/view/");
			if (e != std::string::npos)
			{
				e += 6;
				const std::string base = "https://www.shadertoy.com/api/v1/shaders/";
				*vId = vBaseUrl.substr(e, vBaseUrl.length() - e);
				const std::string apikey = ct::toStr("?key=") + vApiKey;
				realUrl = base + *vId + apikey;
			}
		}

		return realUrl;
	}

	static ShadertoyStruct ParseShader(const picojson::value& vJsonShader)
	{
		ShadertoyStruct shStr;

		if (vJsonShader.contains("info"))
		{
			picojson::value info = vJsonShader.get("info");

			shStr.info.id = info.get("id").to_str();
			shStr.info.date = info.get("date").to_str();
			shStr.info.viewed = info.get("viewed").to_str();
			shStr.info.name = info.get("name").to_str();
			shStr.info.username = info.get("username").to_str();
			shStr.info.description = info.get("description").to_str();
			shStr.info.likes = info.get("likes").to_str();
			shStr.info.published = info.get("published").to_str();
			shStr.info.flags = info.get("flags").to_str();
			shStr.info.hasliked = info.get("hasliked").to_str();

			if (info.get("tags").is<picojson::array>())
			{
				picojson::array tags = info.get("tags").get<picojson::array>();
				for (picojson::array::iterator it_tags = tags.begin(); it_tags != tags.end(); ++it_tags)
				{
					picojson::value var_tag = *it_tags;

					std::string tag = var_tag.to_str();
					shStr.info.tags.emplace_back(tag);
				}
			}
		}

		if (vJsonShader.contains("renderpass"))
		{
			if (vJsonShader.get("renderpass").is<picojson::array>())
			{
				picojson::array renderpass = vJsonShader.get("renderpass").get<picojson::array>();
				for (picojson::array::iterator it_rp = renderpass.begin(); it_rp != renderpass.end(); ++it_rp)
				{
					picojson::value var_rp = *it_rp;

					ShaderToyRenderPass rpass;

					if (var_rp.get("inputs").is<picojson::array>())
					{
						picojson::array inputs = var_rp.get("inputs").get<picojson::array>();
						for (picojson::array::iterator it_in = inputs.begin(); it_in != inputs.end(); ++it_in)
						{
							picojson::value var_in = *it_in;

							ShaderToyInput input;
							input.id = var_in.get("id").to_str();
							input.type = var_in.get("type").to_str();
							if (input.type.empty()) {
								input.type = var_in.get("ctype").to_str();
							}

							input.chan = var_in.get("channel").to_str();
							input.filepath = "https://www.shadertoy.com" + var_in.get("filepath").to_str();

							picojson::value sampler = var_in.get("sampler");
							input.sampler.filter = sampler.get("filter").to_str();
							input.sampler.wrap = sampler.get("wrap").to_str();
							input.sampler.vflip = sampler.get("vflip").to_str();
							input.sampler.srgb = sampler.get("srgb").to_str();
							input.sampler.internal = sampler.get("internal").to_str();

							input.published = var_in.get("published").to_str();
							rpass.inputs.emplace_front(input);
						}
					}

					if (var_rp.get("outputs").is<picojson::array>())
					{
						picojson::array outputs = var_rp.get("outputs").get<picojson::array>();
						for (picojson::array::iterator it_out = outputs.begin(); it_out != outputs.end(); ++it_out)
						{
							picojson::value var_out = *it_out;

							ShaderToyOutput output;
							output.id = var_out.get("id").to_str();
							output.chan = var_out.get("channel").to_str();
							rpass.outputs.emplace_front(output);
						}
					}

					std::string type = var_rp.get("type").to_str();
					std::string name = var_rp.get("name").to_str();
					std::string code = var_rp.get("code").to_str();

					std::string endFragmentCode;
					if (code.find("mainVR") != std::string::npos)
					{
						if (type == "image")
							endFragmentCode = getFragmentTemplateEndForImageVR();
						else if (type == "buffer")
							endFragmentCode = getFragmentTemplateEndForBufferVR();
					}
					else
					{
						if (type == "image")
							endFragmentCode = getFragmentTemplateEndForImage();
						else if (type == "buffer")
							endFragmentCode = getFragmentTemplateEndForBuffer();
					}

					ct::replaceString(name, " ", "_");
					rpass.name = name;
					rpass.type = type;
					rpass.code = code + "\n\n" + endFragmentCode;

					if (type != "common")
						rpass.code = getFragmentTemplateStart() + "\n\n" + rpass.code;

					shStr.renderpass.emplace_back(rpass);
				}
			}
		}

		return shStr;
	}

	static ShadertoyStruct ParseBuffer(const std::string& vBuffer)
	{
		ShadertoyStruct shStr;

		size_t first_accolade = vBuffer.find('{');
		if (first_accolade != std::string::npos)
		{
			std::string buf = vBuffer.substr(first_accolade);

			picojson::value json;
			picojson::parse(json, buf);
			std::string err = picojson::get_last_error();
			if (err.empty())
			{
				if (json.contains("Shader"))
				{
					picojson::value shader = json.get("Shader");

					shStr = ParseShader(shader);
				}
				else if (json.contains("Error"))
				{
					shStr.error = json.get("Error").to_str();
				}
			}
		}

		return shStr;
	}

	static std::string getVertexTemplate()
	{
		return "void main()\n{\n\tgl_Position = vec4(a_position.xy, 0.0, 1.0);\n}\n";
	}

	static std::string getFragmentTemplateStart()
	{
		std::string frag;
		frag += "struct Channel {vec3 resolution; float time;};\n";
		frag += "void mainImage(inout vec4 c, in vec2 f);\n";
		frag += "uniform vec3 iResolution;\n";
		frag += "uniform float iTime;\n";
		frag += "uniform float iChannelTime[4];\n";
		frag += "uniform vec4 iMouse;\n";
		frag += "uniform vec4 iDate;\n";
		frag += "uniform float iSampleRate;\n";
		frag += "uniform vec3 iChannelResolution[4];\n";
		frag += "uniform int iFrame;\n";
		frag += "uniform float iTimeDelta;\n";
		frag += "uniform float iFrameRate;\n";
		frag += "uniform Channel iChannel[4];\n";
		frag += "uniform sampler2D iChannel0;\n";
		frag += "uniform sampler2D iChannel1;\n";
		frag += "uniform sampler2D iChannel2;\n";
		frag += "uniform sampler2D iChannel3;\n";
		return frag;
	}

	static std::string getFragmentTemplateEndForBuffer()
	{
		std::string frag = "void main(void)\n{\n\tvec4 color = vec4(0.0, 0.0, 0.0, 1.0);\n\tmainImage(color, gl_FragCoord.xy);\n\t//color.a = 1.0;\n\tgl_FragColor = color;\n}\n";
		return frag;
	}
	static std::string getFragmentTemplateEndForImage()
	{
		std::string frag = "void main(void)\n{\n\tvec4 color = vec4(0.0, 0.0, 0.0, 1.0);\n\tmainImage(color, gl_FragCoord.xy);\n\tcolor.a = 1.0;\n\tgl_FragColor = color;\n}\n";
		return frag;
	}
	static std::string getFragmentTemplateEndForBufferVR()
	{
		std::string frag = u8R"(
#include "space3d.glsl"
void main(void)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	if (useVR < 0.5)
	{	
		mainImage(color, gl_FragCoord.xy);
		//color.a = 1.0;
	}
	else
	{	
		vec3 ro = getRayOrigin();
		vec3 rd = getRayDirection();
		mainVR(color, gl_FragCoord.xy, ro, rd);
		//color.a = 1.0;
	}
	gl_FragColor = color;
}
)";
		return frag;
	}
	static std::string getFragmentTemplateEndForImageVR()
	{
		std::string frag = u8R"(
#include "space3d.glsl"
void main(void)
{
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	if (useVR < 0.5)
	{	
		mainImage(color, gl_FragCoord.xy);
		color.a = 1.0;
	}
	else
	{	
		vec3 ro = getRayOrigin();
		vec3 rd = getRayDirection();
		mainVR(color, gl_FragCoord.xy, ro, rd);
		color.a = 1.0;
	}
	gl_FragColor = color;
}
)";
		return frag;
	}
};
}


// la structure json  de ce shader 
// https://www.shadertoy.com/view/4sK3RD
// api std::string : https://www.shadertoy.com/api/v1/shaders/4sK3RD?key=rdrtw4 with api key rdrtw4
// c'est ca : version 0.1
		// t:12.24s {
		/* "Shader":
		{
			"ver":"0.1",
			"info" : 
			{
				"id":"4l3SzH",
				"date" : "1478916114",
				"viewed" : 1537,
				"name" : "Tunnel Beauty 6",
				"username" : "aiekick",
				"description" : "i have the limit of the float after a certain time, and some noise appears :)\nyou can increase the iteration at line 86 to have better shape but more slow too :) ",
				"likes" : 28,
				"published" : 3,
				"flags" : 0,
				"tags" : ["tunnelbeauty6"],
				"hasliked" : 0
			},
			"renderpass":
			[{
				"inputs":
				[{
					"id":30,
					"filepath" : "\/presets\/tex16.png", 
					"type" : "texture",
					"channel" : 0,
					"sampler" : 
					{
						"filter":"mipmap",
						"wrap" : "repeat",
						"vflip" : "false",
						"srgb" : "false",
						"internal" : "byte"
					}
				}],
				"outputs":
				[{
					"id":37,
					"channel" : 0
				}],
				"code" : "\/\/ Created by Stephane Cuillerdier - @Aiekick\/2016\n\/\/ License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.\n\nmat3 RotZ(float a)\n{\n    return mat3(cos(a),-sin(a),0.,sin(a),cos(a),0.,0.,0.,1.);\n}\n\nfloat df(vec3 p)\n{\n\tvec3 q = p;\n    q.xy += cos(iGlobalTime * 0.2);\n\tq *= RotZ(q.z * 0.1);\n    q += sin(q.zxy * 0.5) * 0.5;\n\tq *= RotZ(q.z * 0.2);\n    q = sin(q.zxy * 0.2) * 1.5;\n    p += q;\n\tp *= RotZ(p.z * 0.045);\n    return 10. - abs(p.y*0.5);\n}\n\nvec3 nor( vec3 pos, float prec )\n{\n\tvec3 eps = vec3( prec, 0., 0. );\n\tvec3 nor = vec3(\n\t    df(pos+eps.xyy) - df(pos-eps.xyy),\n\t    df(pos+eps.yxy) - df(pos-eps.yxy),\n\t    df(pos+eps.yyx) - df(pos-eps.yyx) );\n\treturn normalize(nor);\n}\n\n\/\/ return color from temperature \n\/\/http:\/\/www.physics.sfasu.edu\/astro\/color\/blackbody.html\n\/\/http:\/\/www.vendian.org\/mncharity\/dir3\/blackbody\/\n\/\/http:\/\/www.vendian.org\/mncharity\/dir3\/blackbody\/UnstableURLs\/bbr_color.html\nvec3 blackbody(float Temp)\n{\n\tvec3 col = vec3(255.);\n    col.x = 56100000. * pow(Temp,(-3. \/ 2.)) + 148.;\n   \tcol.y = 100.04 * log(Temp) - 623.6;\n   \tif (Temp > 6500.) col.y = 35200000. * pow(Temp,(-3. \/ 2.)) + 184.;\n   \tcol.z = 194.18 * log(Temp) - 1448.6;\n   \tcol = clamp(col, 0., 255.)\/255.;\n    if (Temp < 1000.) col *= Temp\/1000.;\n   \treturn col;\n}\n\n\/\/ get density of the df at surfPoint\n\/\/ ratio between constant step and df value\nfloat SubDensity(vec3 surfPoint, float prec, float ms) \n{\n\tvec3 n;\n\tfloat s = 0.;\n    const int iter = 10;\n\tfor (int i=0;i<iter;i++)\n\t{\n\t\tn = nor(surfPoint,prec); \n\t\tsurfPoint = surfPoint - n * ms; \n\t\ts += df(surfPoint);\n\t}\n\t\n\treturn 1.-s\/(ms*float(iter)); \/\/ s < 0. => inside df\n}\n\nfloat SubDensity(vec3 p, float s) \n{\n\tvec3 n = nor(p,s); \t\t\t\t\t\t\t\/\/ precise normale at surf point\n\treturn df(p - n * s);\t\t\t\t\t\t\/\/ ratio between df step and constant step\n}\n\nvoid mainImage( out vec4 fragColor, in vec2 fragCoord )\n{\n\tvec2 g = fragCoord;\n\tvec2 si = iResolution.xy;\n\tvec2 uv = (g+g-si)\/si.y;\n\tvec3 ro = vec3(0,0, iGlobalTime * 15.); \n    vec3 cv = ro + vec3(0,0,1); \n\tvec3 cu = normalize(vec3(0,1,0));\n  \tvec3 z = normalize(cv-ro);\n    vec3 x = normalize(cross(cu,z));\n  \tvec3 y = cross(z,x);\n    float fov = .9;\n  \tvec3 rd = normalize(fov * (uv.x * x + uv.y * y) + z);\n\t\n\tfloat s = 1., d = 0.;\n\tfor (int i=0; i<90; i++) \n\t{\n\t\tif (log(d*d\/s\/1e5)>0.) break;\n\t\td += (s=df(ro+rd*d))*.2;\n\t}\n\t\n\tvec3 p = ro + rd * d;\t\t\t\t\t\t\t\t\t\t\t\/\/ surface point\n\tvec3 lid = normalize(ro-p); \t\t\t\t\t\t\t\t\t\/\/ light dir\n\tvec3 n = nor(p, 0.1);\t\t\t\t\t\t\t\t\t\t\t\/\/ normal at surface point\n\tvec3 refl = reflect(rd,n);\t\t\t\t\t\t\t\t\t\t\/\/ reflected ray dir at surf point \n\tfloat diff = clamp( dot( n, lid ), 0.0, 1.0 ); \t\t\t\t\t\/\/ diffuse\n\tfloat fre = pow( clamp( 1. + dot(n,rd),0.0,1.0), 4. ); \t\t\t\/\/ fresnel\n\tfloat spe = pow(clamp( dot( refl, lid ), 0.0, 1.0 ),16.);\t\t\/\/ specular\n\tvec3 col = vec3(.8,.5,.2);\n    \n    \/\/ here the magic happen\n\tfloat sss = df(p - n*0.001)\/0.1;\t\t\t\t\t\t\t\t\/\/ quick sss 0.001 of subsurface\n\t\n\tfloat sb = SubDensity(p, 0.01, 0.1);\t\t\t\t\t\t\t\/\/ deep subdensity from 0.01 to 0.1 (10 iterations)\n\tvec3 bb = clamp(blackbody(200. * sb),0.,1.);\t\t\t\t\t\/\/ blackbody color\n\tfloat sss2 = 0.8 - SubDensity(p, 3.); \t\t\t\t\t\t\t\/\/ one step sub density of df of 3 of subsurface\n\t\n    fragColor.rgb = (diff + fre + bb * sss2 * .8 + col * sss * .2) * 0.25 + spe * 1.2;\n\n\t\/\/ vigneting from iq Shader Mike : https:\/\/www.shadertoy.com\/view\/MsXGWr\n    vec2 q = g\/si;\n    fragColor.rgb *= 0.5 + 0.5*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.55 );\n}\n",
				"name" : "Image",
				"description" : "",
				"type" : "image"
			}]
		}*/

		/* format full avec buffers

		"Shader":
		{
			"ver":"0.1",
			"info":
			{
				"id":"4sK3RD",
				"date":"1453591467",
				"viewed":4738,
				"name":"Rock-Paper-Scissor-4D",
				"username":"Flexi",
				"description":"Multi-scale \"Milkdrop2\" Gaussian blur diffusion, mouse drag and drop vortex pair plane deformation, cyclic rgba reaction, gradient lookups for expansive flow and edgy color map",
				"likes":165,
				"published":3,
				"flags":32,
				"tags":
				[
					"multiscale",
					"reactiondiffusion"
				],
				"hasliked":0
			},
			"renderpass":
			[
				{
					"inputs":
					[
						{
							"id":257,
							"src":"\/presets\/previz\/buffer00.png",
							"type":"buffer",
							"channel":0,
							"sampler":
							{
								"filter":"linear",
								"wrap":"clamp",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						},
						{
							"id":258,
							"src":"\/presets\/previz\/buffer01.png",
							"type":"buffer",
							"channel":1,
							"sampler":
							{
								"filter":"linear",
								"wrap":"clamp",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						},
						{
							"id":260,
							"src":"\/presets\/previz\/buffer03.png",
							"type":"buffer",
							"channel":3,
							"sampler":
							{
								"filter":"linear",
								"wrap":"clamp",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						}
					],
					"outputs":
					[
						{
							"id":37,
							"channel":0
						}
					],
					"code":"vec2 lower_left(vec2 uv)\n{\n    return fract(uv * 0.5);\n}\n\nvec2 lower_right(vec2 uv)\n{\n    return fract((uv - vec2(1, 0.)) * 0.5);\n}\n\nvec2 upper_left(vec2 uv)\n{\n    return fract((uv - vec2(0., 1)) * 0.5);\n}\n\nvec2 upper_right(vec2 uv)\n{\n    return fract((uv - 1.) * 0.5);\n}\n\nvec4 BlurA(vec2 uv, int level)\n{\n    if(level <= 0)\n    {\n        return texture2D(iChannel0, fract(uv));\n    }\n\n    uv = upper_left(uv);\n    for(int depth = 1; depth < 8; depth++)\n    {\n        if(depth >= level)\n        {\n            break;\n        }\n        uv = lower_right(uv);\n    }\n\n    return texture2D(iChannel3, uv);\n}\n\nvec4 BlurB(vec2 uv, int level)\n{\n    if(level <= 0)\n    {\n        return texture2D(iChannel1, fract(uv));\n    }\n\n    uv = lower_left(uv);\n    for(int depth = 1; depth < 8; depth++)\n    {\n        if(depth >= level)\n        {\n            break;\n        }\n        uv = lower_right(uv);\n    }\n\n    return texture2D(iChannel3, uv);\n}\n\nvec2 GradientA(vec2 uv, vec2 d, vec4 selector, int level){\n    vec4 dX = 0.5*BlurA(uv + vec2(1.,0.)*d, level) - 0.5*BlurA(uv - vec2(1.,0.)*d, level);\n    vec4 dY = 0.5*BlurA(uv + vec2(0.,1.)*d, level) - 0.5*BlurA(uv - vec2(0.,1.)*d, level);\n    return vec2( dot(dX, selector), dot(dY, selector) );\n}\n\nvoid mainImage( out vec4 fragColor, in vec2 fragCoord )\n{\n    vec2 uv = fragCoord.xy \/ iResolution.xy;\n    vec2 pixelSize = 1. \/ iResolution.xy;\n    vec2 aspect = vec2(1.,iResolution.y\/iResolution.x);\n\n    vec2 d = pixelSize*2.;\n    vec4 dx = (BlurA(uv + vec2(1,0)*d, 1) - BlurA(uv - vec2(1,0)*d, 1))*0.5;\n    vec4 dy = (BlurA(uv + vec2(0,1)*d, 1) - BlurA(uv - vec2(0,1)*d, 1))*0.5;\n\n    d = pixelSize*1.;\n    dx += BlurA(uv + vec2(1,0)*d, 0) - BlurA(uv - vec2(1,0)*d, 0);\n    dy += BlurA(uv + vec2(0,1)*d, 0) - BlurA(uv - vec2(0,1)*d, 0);\n    vec2 lightSize=vec2(0.5);\n\n    fragColor = BlurA(uv+vec2(dx.x,dy.x)*pixelSize*8., 0).x * vec4(0.7,1.66,2.0,1.0) - vec4(0.3,1.0,1.0,1.0);\n    fragColor = mix(fragColor,vec4(8.0,6.,2.,1.), BlurA(uv + vec2(dx.x,dy.x)*lightSize, 3).y*0.4*0.75*vec4(1.-BlurA(uv+vec2(dx.x,dy.x)*pixelSize*8., 0).x)); \n    fragColor = mix(fragColor, vec4(0.1,0.,0.4,0.), BlurA(uv, 1).a*length(GradientA(uv, pixelSize*2., vec4(0.,0.,0.,1.), 0))*5.);\n    fragColor = mix(fragColor, vec4(1.25,1.35,1.4,0.), BlurA(uv, 0).x*BlurA(uv + GradientA(uv, pixelSize*2.5, vec4(-256.,32.,-128.,32.), 1)*pixelSize, 2).y);\n    fragColor = mix(fragColor, vec4(0.25,0.75,1.,0.), BlurA(uv, 1).x*length(GradientA(uv+GradientA(uv, pixelSize*2., vec4(0.,0.,128.,0.), 1)*pixelSize, pixelSize*2., vec4(0.,0.,0.,1.), 0))*5.);\n    fragColor = mix(fragColor, vec4(1.,1.25,1.5,0.), 0.5*(1.-BlurA(uv, 0)*1.).a*length(GradientA(uv+GradientA(uv, pixelSize*2., vec4(0.,128.,0.,0.), 1)*pixelSize, pixelSize*1.5, vec4(0.,0.,16.,0.), 0)));\n\n    \/\/    fragColor = BlurA(uv, 0); \/\/ simple bypass\n    \/\/    fragColor = BlurB(uv, 0); \/\/ simple bypass\n    \/\/    fragColor = texture2D(iChannel3, uv); \/\/ raw Gaussian pyramid\n\n}",
					"name":"Image",
					"description":"",
					"type":"image"
				},
				{
					"inputs":
					[
						{
							"id":30,
							"src":"\/presets\/tex16.png",
							"type":"texture",
							"channel":2,
							"sampler":
							{
								"filter":"mipmap",
								"wrap":"repeat",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						},
						{
							"id":257,
							"src":"\/presets\/previz\/buffer00.png",
							"type":"buffer",
							"channel":0,
							"sampler":
							{
								"filter":"linear",
								"wrap":"clamp",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						},
						{
							"id":260,
							"src":"\/presets\/previz\/buffer03.png",
							"type":"buffer",
							"channel":3,
							"sampler":
							{
								"filter":"linear",
								"wrap":"clamp",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						}
					],
					"outputs":
					[
						{
							"id":257,
							"channel":0
						}
					],
					"code":"#define pi2_inv 0.159154943091895335768883763372\n\nvec2 lower_left(vec2 uv)\n{\n    return fract(uv * 0.5);\n}\n\nvec2 lower_right(vec2 uv)\n{\n    return fract((uv - vec2(1, 0.)) * 0.5);\n}\n\nvec2 upper_left(vec2 uv)\n{\n    return fract((uv - vec2(0., 1)) * 0.5);\n}\n\nvec2 upper_right(vec2 uv)\n{\n    return fract((uv - 1.) * 0.5);\n}\n\nvec2 mouseDelta(){\n    vec2 pixelSize = 1. \/ iResolution.xy;\n    float eighth = 1.\/8.;\n    vec4 oldMouse = texture2D(iChannel3, vec2(7.5 * eighth, 2.5 * eighth));\n    vec4 nowMouse = vec4(iMouse.xy \/ iResolution.xy, iMouse.zw \/ iResolution.xy);\n    if(oldMouse.z > pixelSize.x && oldMouse.w > pixelSize.y && \n       nowMouse.z > pixelSize.x && nowMouse.w > pixelSize.y)\n    {\n        return nowMouse.xy - oldMouse.xy;\n    }\n    return vec2(0.);\n}\n\nvec4 BlurA(vec2 uv, int level)\n{\n    if(level <= 0)\n    {\n        return texture2D(iChannel0, fract(uv));\n    }\n\n    uv = upper_left(uv);\n    for(int depth = 1; depth < 8; depth++)\n    {\n        if(depth >= level)\n        {\n            break;\n        }\n        uv = lower_right(uv);\n    }\n\n    return texture2D(iChannel3, uv);\n}\n\t\nvec2 GradientA(vec2 uv, vec2 d, vec4 selector, int level){\n\tvec4 dX = 0.5*BlurA(uv + vec2(1.,0.)*d, level) - 0.5*BlurA(uv - vec2(1.,0.)*d, level);\n\tvec4 dY = 0.5*BlurA(uv + vec2(0.,1.)*d, level) - 0.5*BlurA(uv - vec2(0.,1.)*d, level);\n\treturn vec2( dot(dX, selector), dot(dY, selector) );\n}\n\nvec2 rot90(vec2 vector){\n\treturn vector.yx*vec2(1,-1);\n}\n\nvec2 complex_mul(vec2 factorA, vec2 factorB){\n    return vec2( factorA.x*factorB.x - factorA.y*factorB.y, factorA.x*factorB.y + factorA.y*factorB.x);\n}\n\nvec2 spiralzoom(vec2 domain, vec2 center, float n, float spiral_factor, float zoopufactor, vec2 pos){\n    vec2 uv = domain - center;\n    float d = length(uv);\n    return vec2( atan(uv.y, uv.x)*n*pi2_inv + d*spiral_factor, -log(d)*zoopufactor) + pos;\n}\n\nvec2 complex_div(vec2 numerator, vec2 denominator){\n    return vec2( numerator.x*denominator.x + numerator.y*denominator.y,\n                numerator.y*denominator.x - numerator.x*denominator.y)\/\n        vec2(denominator.x*denominator.x + denominator.y*denominator.y);\n}\n\nfloat circle(vec2 uv, vec2 aspect, float scale){\n    return clamp( 1. - length((uv-0.5)*aspect*scale), 0., 1.);\n}\n\nfloat sigmoid(float x) {\n    return 2.\/(1. + exp2(-x)) - 1.;\n}\n\nfloat smoothcircle(vec2 uv, vec2 aspect, float radius, float ramp){\n    return 0.5 - sigmoid( ( length( (uv - 0.5) * aspect) - radius) * ramp) * 0.5;\n}\n\nfloat conetip(vec2 uv, vec2 pos, float size, float min)\n{\n    vec2 aspect = vec2(1.,iResolution.y\/iResolution.x);\n    return max( min, 1. - length((uv - pos) * aspect \/ size) );\n}\n\nfloat warpFilter(vec2 uv, vec2 pos, float size, float ramp)\n{\n    return 0.5 + sigmoid( conetip(uv, pos, size, -16.) * ramp) * 0.5;\n}\n\nvec2 vortex_warp(vec2 uv, vec2 pos, float size, float ramp, vec2 rot)\n{\n    vec2 aspect = vec2(1.,iResolution.y\/iResolution.x);\n\n    vec2 pos_correct = 0.5 + (pos - 0.5);\n    vec2 rot_uv = pos_correct + complex_mul((uv - pos_correct)*aspect, rot)\/aspect;\n    float filter = warpFilter(uv, pos_correct, size, ramp);\n    return mix(uv, rot_uv, filter);\n}\n\nvec2 vortex_pair_warp(vec2 uv, vec2 pos, vec2 vel)\n{\n    vec2 aspect = vec2(1.,iResolution.y\/iResolution.x);\n    float ramp = 4.;\n\n    float d = 0.125;\n\n    float l = length(vel);\n    vec2 p1 = pos;\n    vec2 p2 = pos;\n\n    if(l > 0.){\n        vec2 normal = normalize(vel.yx * vec2(-1., 1.))\/aspect;\n        p1 = pos - normal * d \/ 2.;\n        p2 = pos + normal * d \/ 2.;\n    }\n\n    float w = l \/ d * 2.;\n\n    \/\/ two overlapping rotations that would annihilate when they were not displaced.\n    vec2 circle1 = vortex_warp(uv, p1, d, ramp, vec2(cos(w),sin(w)));\n    vec2 circle2 = vortex_warp(uv, p2, d, ramp, vec2(cos(-w),sin(-w)));\n    return (circle1 + circle2) \/ 2.;\n}\n\nvoid mainImage( out vec4 fragColor, in vec2 fragCoord )\n{\n    vec2 uv = fragCoord.xy \/ iResolution.xy;\n    vec4 noise = (texture2D(iChannel2, fragCoord.xy \/ iChannelResolution[2].xy + fract(vec2(42,56)*iGlobalTime))-0.5)*2.;\n\n    if(iFrame<10)\n    {\n        fragColor = noise;\n        return;\n    }\n\n    vec2 mouseV = mouseDelta();\n    vec2 aspect = vec2(1.,iResolution.y\/iResolution.x);\n    vec2 pixelSize = 1. \/ iResolution.xy;\n\n    uv = vortex_pair_warp(uv, iMouse.xy*pixelSize, mouseV*aspect*1.4);\n    \n    \/\/ expansion\n    vec2 gradientLookupDistance = pixelSize*3.;\n    float expansionFactor = 1.;\n    \n    \/\/ reaction-diffusion  \n    float differentialFactor = 12.\/256.;\n    float increment = - 3.\/256.;\n    float noiseFactor = 2.\/256.;\n    \n    \/\/ rock-paper-scissor\n    float feedBack = 6.\/256.;\n    float feedForward = 6.\/256.;\n\n\tfragColor.r = BlurA(uv + GradientA(uv, gradientLookupDistance, vec4(4.,0.,-2.,0.), 1)*pixelSize*expansionFactor, 0).r;\n\tfragColor.g = BlurA(uv + GradientA(uv, gradientLookupDistance, vec4(0.,4.,0.,-2.), 1)*pixelSize*expansionFactor, 0).g;\n\tfragColor.b = BlurA(uv + GradientA(uv, gradientLookupDistance, vec4(-2.,0.,4.,0.), 1)*pixelSize*expansionFactor, 0).b;\n    fragColor.a = BlurA(uv + GradientA(uv, gradientLookupDistance, vec4(0.,-2.,0.,4.), 1)*pixelSize*expansionFactor, 0).a;\n\n   \tfragColor += (BlurA(uv, 1) - BlurA(uv, 2))*differentialFactor;\n\n    fragColor += increment + noise * noiseFactor;\n\n    fragColor -= fragColor.argb * feedBack;\n    fragColor += fragColor.gbar * feedForward;\n    \n    fragColor = clamp(fragColor, 0., 1.);\n\n\/\/    fragColor = noise; \/\/ reset\n}",
					"name":"Buf A",
					"description":"",
					"type":"buffer"
				},
				{
					"inputs":
					[
						{
							"id":30,
							"src":"\/presets\/tex16.png",
							"type":"texture",
							"channel":2,
							"sampler":
							{
								"filter":"mipmap",
								"wrap":"repeat",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						},
						{
							"id":258,
							"src":"\/presets\/previz\/buffer01.png",
							"type":"buffer",
							"channel":1,
							"sampler":
							{
								"filter":"linear",
								"wrap":"clamp",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						},
						{
							"id":260,
							"src":"\/presets\/previz\/buffer03.png",
							"type":"buffer",
							"channel":3,
							"sampler":
							{
								"filter":"linear",
								"wrap":"clamp",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						}
					],
					"outputs":
					[
						{
							"id":258,
							"channel":0
						}
					],
					"code":"#define pi2_inv 0.159154943091895335768883763372\n\nvec2 lower_left(vec2 uv)\n{\n    return fract(uv * 0.5);\n}\n\nvec2 lower_right(vec2 uv)\n{\n    return fract((uv - vec2(1, 0.)) * 0.5);\n}\n\nvec2 upper_left(vec2 uv)\n{\n    return fract((uv - vec2(0., 1)) * 0.5);\n}\n\nvec2 upper_right(vec2 uv)\n{\n    return fract((uv - 1.) * 0.5);\n}\n\nvec2 mouseDelta(){\n    vec2 pixelSize = 1. \/ iResolution.xy;\n    float eighth = 1.\/8.;\n    vec4 oldMouse = texture2D(iChannel3, vec2(7.5 * eighth, 2.5 * eighth));\n    vec4 nowMouse = vec4(iMouse.xy \/ iResolution.xy, iMouse.zw \/ iResolution.xy);\n    if(oldMouse.z > pixelSize.x && oldMouse.w > pixelSize.y && \n       nowMouse.z > pixelSize.x && nowMouse.w > pixelSize.y)\n    {\n        return nowMouse.xy - oldMouse.xy;\n    }\n    return vec2(0.);\n}\n\nvec4 BlurA(vec2 uv, int level)\n{\n    if(level <= 0)\n    {\n        return texture2D(iChannel0, fract(uv));\n    }\n\n    uv = upper_left(uv);\n    for(int depth = 1; depth < 8; depth++)\n    {\n        if(depth >= level)\n        {\n            break;\n        }\n        uv = lower_right(uv);\n    }\n\n    return texture2D(iChannel3, uv);\n}\n\nvec4 BlurB(vec2 uv, int level)\n{\n    if(level <= 0)\n    {\n        return texture2D(iChannel1, fract(uv));\n    }\n\n    uv = lower_left(uv);\n    for(int depth = 1; depth < 8; depth++)\n    {\n        if(depth >= level)\n        {\n            break;\n        }\n        uv = lower_right(uv);\n    }\n\n    return texture2D(iChannel3, uv);\n}\n\nvec2 GradientA(vec2 uv, vec2 d, vec4 selector, int level){\n    vec4 dX = 0.5*BlurA(uv + vec2(1.,0.)*d, level) - 0.5*BlurA(uv - vec2(1.,0.)*d, level);\n    vec4 dY = 0.5*BlurA(uv + vec2(0.,1.)*d, level) - 0.5*BlurA(uv - vec2(0.,1.)*d, level);\n    return vec2( dot(dX, selector), dot(dY, selector) );\n}\n\nvec2 rot90(vec2 vector){\n    return vector.yx*vec2(1,-1);\n}\n\nvec2 complex_mul(vec2 factorA, vec2 factorB){\n    return vec2( factorA.x*factorB.x - factorA.y*factorB.y, factorA.x*factorB.y + factorA.y*factorB.x);\n}\n\nvec2 spiralzoom(vec2 domain, vec2 center, float n, float spiral_factor, float zoopufactor, vec2 pos){\n    vec2 uv = domain - center;\n    float d = length(uv);\n    return vec2( atan(uv.y, uv.x)*n*pi2_inv + d*spiral_factor, -log(d)*zoopufactor) + pos;\n}\n\nvec2 complex_div(vec2 numerator, vec2 denominator){\n    return vec2( numerator.x*denominator.x + numerator.y*denominator.y,\n                numerator.y*denominator.x - numerator.x*denominator.y)\/\n        vec2(denominator.x*denominator.x + denominator.y*denominator.y);\n}\n\nfloat circle(vec2 uv, vec2 aspect, float scale){\n    return clamp( 1. - length((uv-0.5)*aspect*scale), 0., 1.);\n}\n\nfloat sigmoid(float x) {\n    return 2.\/(1. + exp2(-x)) - 1.;\n}\n\nfloat smoothcircle(vec2 uv, vec2 aspect, float radius, float ramp){\n    return 0.5 - sigmoid( ( length( (uv - 0.5) * aspect) - radius) * ramp) * 0.5;\n}\n\nfloat conetip(vec2 uv, vec2 pos, float size, float min)\n{\n    vec2 aspect = vec2(1.,iResolution.y\/iResolution.x);\n    return max( min, 1. - length((uv - pos) * aspect \/ size) );\n}\n\nfloat warpFilter(vec2 uv, vec2 pos, float size, float ramp)\n{\n    return 0.5 + sigmoid( conetip(uv, pos, size, -16.) * ramp) * 0.5;\n}\n\nvec2 vortex_warp(vec2 uv, vec2 pos, float size, float ramp, vec2 rot)\n{\n    vec2 aspect = vec2(1.,iResolution.y\/iResolution.x);\n\n    vec2 pos_correct = 0.5 + (pos - 0.5);\n    vec2 rot_uv = pos_correct + complex_mul((uv - pos_correct)*aspect, rot)\/aspect;\n    float filter = warpFilter(uv, pos_correct, size, ramp);\n    return mix(uv, rot_uv, filter);\n}\n\nvec2 vortex_pair_warp(vec2 uv, vec2 pos, vec2 vel)\n{\n    vec2 aspect = vec2(1.,iResolution.y\/iResolution.x);\n    float ramp = 4.;\n\n    float d = 0.125;\n\n    float l = length(vel);\n    vec2 p1 = pos;\n    vec2 p2 = pos;\n\n    if(l > 0.){\n        vec2 normal = normalize(vel.yx * vec2(-1., 1.))\/aspect;\n        p1 = pos - normal * d \/ 2.;\n        p2 = pos + normal * d \/ 2.;\n    }\n\n    float w = l \/ d * 2.;\n\n    \/\/ two overlapping rotations that would annihilate when they were not displaced.\n    vec2 circle1 = vortex_warp(uv, p1, d, ramp, vec2(cos(w),sin(w)));\n    vec2 circle2 = vortex_warp(uv, p2, d, ramp, vec2(cos(-w),sin(-w)));\n    return (circle1 + circle2) \/ 2.;\n}\n\nvoid mainImage( out vec4 fragColor, in vec2 fragCoord )\n{\n    vec2 uv = fragCoord.xy \/ iResolution.xy;\n    vec4 noise = texture2D(iChannel2, fragCoord.xy \/ iChannelResolution[2].xy + fract(vec2(42,56)*iGlobalTime));\n\n    if(iFrame<10)\n    {\n        fragColor = noise;\n        return;\n    }\n\n\n    uv = 0.5 + (uv - 0.5)*0.99;\n    vec2 pixelSize = 1.\/iResolution.xy;\n    vec2 mouseV = mouseDelta();\n    vec2 aspect = vec2(1.,iResolution.y\/iResolution.x);\n    uv = vortex_pair_warp(uv, iMouse.xy*pixelSize, mouseV*aspect*1.4);\n\n    float time = float(iFrame)\/60.;\n    uv = uv + vec2(sin(time*0.1 + uv.x*2. +1.) - sin(time*0.214 + uv.y*2. +1.), sin(time*0.168 + uv.x*2. +1.) - sin(time*0.115 +uv.y*2. +1.))*pixelSize*1.5;\n\n    fragColor = BlurB(uv, 0);\n    fragColor += ((BlurB(uv, 1) - BlurB(uv, 2))*0.5 + (noise-0.5) * 0.004); \n\n    fragColor = clamp(fragColor, 0., 1.);\n\n    \/\/fragColor = noise; \/\/ reset\n}",
					"name":"Buf B",
					"description":"",
					"type":"buffer"
				},
				{
					"inputs":
					[
						{
							"id":257,
							"src":"\/presets\/previz\/buffer00.png",
							"type":"buffer",
							"channel":0,
							"sampler":
							{
								"filter":"linear",
								"wrap":"clamp",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						},
						{
							"id":258,
							"src":"\/presets\/previz\/buffer01.png",
							"type":"buffer",
							"channel":1,
							"sampler":
							{
								"filter":"linear",
								"wrap":"clamp",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						},
						{
							"id":260,
							"src":"\/presets\/previz\/buffer03.png",
							"type":"buffer",
							"channel":3,
							"sampler":
							{
								"filter":"linear",
								"wrap":"clamp",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						}
					],
					"outputs":
					[
						{
							"id":259,
							"channel":0
						}
					],
					"code":"\/\/ resolution reduction and horizontal blur\n\nvec2 lower_left(vec2 uv)\n{\n    return fract(uv * 0.5);\n}\n\nvec2 lower_right(vec2 uv)\n{\n    return fract((uv - vec2(1, 0.)) * 0.5);\n}\n\nvec2 upper_left(vec2 uv)\n{\n    return fract((uv - vec2(0., 1)) * 0.5);\n}\n\nvec2 upper_right(vec2 uv)\n{\n    return fract((uv - 1.) * 0.5);\n}\n\nvec4 blur_horizontal(sampler2D channel, vec2 uv, float scale)\n{\n    float h = scale \/ iResolution.x;\n    vec4 sum = vec4(0.0);\n\n    sum += texture2D(channel, fract(vec2(uv.x - 4.0*h, uv.y)) ) * 0.05;\n    sum += texture2D(channel, fract(vec2(uv.x - 3.0*h, uv.y)) ) * 0.09;\n    sum += texture2D(channel, fract(vec2(uv.x - 2.0*h, uv.y)) ) * 0.12;\n    sum += texture2D(channel, fract(vec2(uv.x - 1.0*h, uv.y)) ) * 0.15;\n    sum += texture2D(channel, fract(vec2(uv.x + 0.0*h, uv.y)) ) * 0.16;\n    sum += texture2D(channel, fract(vec2(uv.x + 1.0*h, uv.y)) ) * 0.15;\n    sum += texture2D(channel, fract(vec2(uv.x + 2.0*h, uv.y)) ) * 0.12;\n    sum += texture2D(channel, fract(vec2(uv.x + 3.0*h, uv.y)) ) * 0.09;\n    sum += texture2D(channel, fract(vec2(uv.x + 4.0*h, uv.y)) ) * 0.05;\n\n    return sum\/0.98; \/\/ normalize\n}\n\nvec4 blur_horizontal_left_column(vec2 uv, int depth)\n{\n    float h = pow(2., float(depth)) \/ iResolution.x;    \n    vec2 uv1, uv2, uv3, uv4, uv5, uv6, uv7, uv8, uv9;\n\n    uv1 = fract(vec2(uv.x - 4.0 * h, uv.y) * 2.);\n    uv2 = fract(vec2(uv.x - 3.0 * h, uv.y) * 2.);\n    uv3 = fract(vec2(uv.x - 2.0 * h, uv.y) * 2.);\n    uv4 = fract(vec2(uv.x - 1.0 * h, uv.y) * 2.);\n    uv5 = fract(vec2(uv.x + 0.0 * h, uv.y) * 2.);\n    uv6 = fract(vec2(uv.x + 1.0 * h, uv.y) * 2.);\n    uv7 = fract(vec2(uv.x + 2.0 * h, uv.y) * 2.);\n    uv8 = fract(vec2(uv.x + 3.0 * h, uv.y) * 2.);\n    uv9 = fract(vec2(uv.x + 4.0 * h, uv.y) * 2.);\n\n    if(uv.y > 0.5)\n    {\n        uv1 = upper_left(uv1);\n        uv2 = upper_left(uv2);\n        uv3 = upper_left(uv3);\n        uv4 = upper_left(uv4);\n        uv5 = upper_left(uv5);\n        uv6 = upper_left(uv6);\n        uv7 = upper_left(uv7);\n        uv8 = upper_left(uv8);\n        uv9 = upper_left(uv9);\n    }\n    else{\n        uv1 = lower_left(uv1);\n        uv2 = lower_left(uv2);\n        uv3 = lower_left(uv3);\n        uv4 = lower_left(uv4);\n        uv5 = lower_left(uv5);\n        uv6 = lower_left(uv6);\n        uv7 = lower_left(uv7);\n        uv8 = lower_left(uv8);\n        uv9 = lower_left(uv9);\n    }\n\n    for(int level = 0; level < 8; level++)\n    {\n        if(level >= depth)\n        {\n            break;\n        }\n\n        uv1 = lower_right(uv1);\n        uv2 = lower_right(uv2);\n        uv3 = lower_right(uv3);\n        uv4 = lower_right(uv4);\n        uv5 = lower_right(uv5);\n        uv6 = lower_right(uv6);\n        uv7 = lower_right(uv7);\n        uv8 = lower_right(uv8);\n        uv9 = lower_right(uv9);\n    }\n\n    vec4 sum = vec4(0.0);\n\n    sum += texture2D(iChannel3, uv1) * 0.05;\n    sum += texture2D(iChannel3, uv2) * 0.09;\n    sum += texture2D(iChannel3, uv3) * 0.12;\n    sum += texture2D(iChannel3, uv4) * 0.15;\n    sum += texture2D(iChannel3, uv5) * 0.16;\n    sum += texture2D(iChannel3, uv6) * 0.15;\n    sum += texture2D(iChannel3, uv7) * 0.12;\n    sum += texture2D(iChannel3, uv8) * 0.09;\n    sum += texture2D(iChannel3, uv9) * 0.05;\n\n    return sum\/0.98; \/\/ normalize\n}\n\nvoid mainImage( out vec4 fragColor, in vec2 fragCoord )\n{\n    vec2 uv = fragCoord.xy \/ iResolution.xy;\n\n    if(uv.x < 0.5)\n    {\n        vec2 uv_half = fract(uv*2.);\n        if(uv.y > 0.5)\n        {\n            fragColor = blur_horizontal(iChannel0, uv_half, 1.);\n        }\n        else\n        {\n            fragColor = blur_horizontal(iChannel1, uv_half, 1.);\n        }\n    }\n    else\n    {\n        for(int level = 0; level < 8; level++)\n        {\n            if((uv.x > 0.5 && uv.y > 0.5) || (uv.x <= 0.5))\n            {\n                break;\n            }\n            vec2 uv_half = fract(uv*2.);\n            fragColor = blur_horizontal_left_column(uv_half, level);\n            uv = uv_half;\n        }\n    }\n}",
					"name":"Buf C",
					"description":"",
					"type":"buffer"
				},
				{
					"inputs":
					[
						{
							"id":259,
							"src":"\/presets\/previz\/buffer02.png",
							"type":"buffer",
							"channel":2,
							"sampler":
							{
								"filter":"linear",
								"wrap":"clamp",
								"vflip":"true",
								"srgb":"false",
								"internal":"byte"
							}
						}
					],
					"outputs":
					[
						{
							"id":260,
							"channel":0
						}
					],
					"code":"\/\/ vertical blur (second pass)\n\nvec2 lower_left(vec2 uv)\n{\n    return fract(uv * 0.5);\n}\n\nvec2 lower_right(vec2 uv)\n{\n    return fract((uv - vec2(1, 0.)) * 0.5);\n}\n\nvec2 upper_left(vec2 uv)\n{\n    return fract((uv - vec2(0., 1)) * 0.5);\n}\n\nvec2 upper_right(vec2 uv)\n{\n    return fract((uv - 1.) * 0.5);\n}\n\nvec4 blur_vertical_upper_left(sampler2D channel, vec2 uv)\n{\n    float v = 1. \/ iResolution.y;\n    vec4 sum = vec4(0.0);\n    sum += texture2D(channel, upper_left(vec2(uv.x, uv.y - 4.0*v)) ) * 0.05;\n    sum += texture2D(channel, upper_left(vec2(uv.x, uv.y - 3.0*v)) ) * 0.09;\n    sum += texture2D(channel, upper_left(vec2(uv.x, uv.y - 2.0*v)) ) * 0.12;\n    sum += texture2D(channel, upper_left(vec2(uv.x, uv.y - 1.0*v)) ) * 0.15;\n    sum += texture2D(channel, upper_left(vec2(uv.x, uv.y + 0.0*v)) ) * 0.16;\n    sum += texture2D(channel, upper_left(vec2(uv.x, uv.y + 1.0*v)) ) * 0.15;\n    sum += texture2D(channel, upper_left(vec2(uv.x, uv.y + 2.0*v)) ) * 0.12;\n    sum += texture2D(channel, upper_left(vec2(uv.x, uv.y + 3.0*v)) ) * 0.09;\n    sum += texture2D(channel, upper_left(vec2(uv.x, uv.y + 4.0*v)) ) * 0.05;\n    return sum\/0.98; \/\/ normalize\n}\n\nvec4 blur_vertical_lower_left(sampler2D channel, vec2 uv)\n{\n    float v = 1. \/ iResolution.y;\n    vec4 sum = vec4(0.0);\n    sum += texture2D(channel, lower_left(vec2(uv.x, uv.y - 4.0*v)) ) * 0.05;\n    sum += texture2D(channel, lower_left(vec2(uv.x, uv.y - 3.0*v)) ) * 0.09;\n    sum += texture2D(channel, lower_left(vec2(uv.x, uv.y - 2.0*v)) ) * 0.12;\n    sum += texture2D(channel, lower_left(vec2(uv.x, uv.y - 1.0*v)) ) * 0.15;\n    sum += texture2D(channel, lower_left(vec2(uv.x, uv.y + 0.0*v)) ) * 0.16;\n    sum += texture2D(channel, lower_left(vec2(uv.x, uv.y + 1.0*v)) ) * 0.15;\n    sum += texture2D(channel, lower_left(vec2(uv.x, uv.y + 2.0*v)) ) * 0.12;\n    sum += texture2D(channel, lower_left(vec2(uv.x, uv.y + 3.0*v)) ) * 0.09;\n    sum += texture2D(channel, lower_left(vec2(uv.x, uv.y + 4.0*v)) ) * 0.05;\n    return sum\/0.98; \/\/ normalize\n}\n\nvec4 blur_vertical_left_column(vec2 uv, int depth)\n{\n    float v = pow(2., float(depth)) \/ iResolution.y;\n\n    vec2 uv1, uv2, uv3, uv4, uv5, uv6, uv7, uv8, uv9;\n\n    uv1 = fract(vec2(uv.x, uv.y - 4.0*v) * 2.);\n    uv2 = fract(vec2(uv.x, uv.y - 3.0*v) * 2.);\n    uv3 = fract(vec2(uv.x, uv.y - 2.0*v) * 2.);\n    uv4 = fract(vec2(uv.x, uv.y - 1.0*v) * 2.);\n    uv5 = fract(vec2(uv.x, uv.y + 0.0*v) * 2.);\n    uv6 = fract(vec2(uv.x, uv.y + 1.0*v) * 2.);\n    uv7 = fract(vec2(uv.x, uv.y + 2.0*v) * 2.);\n    uv8 = fract(vec2(uv.x, uv.y + 3.0*v) * 2.);\n    uv9 = fract(vec2(uv.x, uv.y + 4.0*v) * 2.);\n\n    if(uv.y > 0.5)\n    {\n        uv1 = upper_left(uv1);\n        uv2 = upper_left(uv2);\n        uv3 = upper_left(uv3);\n        uv4 = upper_left(uv4);\n        uv5 = upper_left(uv5);\n        uv6 = upper_left(uv6);\n        uv7 = upper_left(uv7);\n        uv8 = upper_left(uv8);\n        uv9 = upper_left(uv9);\n    }\n    else{\n        uv1 = lower_left(uv1);\n        uv2 = lower_left(uv2);\n        uv3 = lower_left(uv3);\n        uv4 = lower_left(uv4);\n        uv5 = lower_left(uv5);\n        uv6 = lower_left(uv6);\n        uv7 = lower_left(uv7);\n        uv8 = lower_left(uv8);\n        uv9 = lower_left(uv9);\n    }\n\n    for(int level = 0; level < 8; level++)\n    {\n        if(level > depth)\n        {\n            break;\n        }\n\n        uv1 = lower_right(uv1);\n        uv2 = lower_right(uv2);\n        uv3 = lower_right(uv3);\n        uv4 = lower_right(uv4);\n        uv5 = lower_right(uv5);\n        uv6 = lower_right(uv6);\n        uv7 = lower_right(uv7);\n        uv8 = lower_right(uv8);\n        uv9 = lower_right(uv9);\n    }\n\n    vec4 sum = vec4(0.0);\n\n    sum += texture2D(iChannel2, uv1) * 0.05;\n    sum += texture2D(iChannel2, uv2) * 0.09;\n    sum += texture2D(iChannel2, uv3) * 0.12;\n    sum += texture2D(iChannel2, uv4) * 0.15;\n    sum += texture2D(iChannel2, uv5) * 0.16;\n    sum += texture2D(iChannel2, uv6) * 0.15;\n    sum += texture2D(iChannel2, uv7) * 0.12;\n    sum += texture2D(iChannel2, uv8) * 0.09;\n    sum += texture2D(iChannel2, uv9) * 0.05;\n\n    return sum\/0.98; \/\/ normalize\n}\n\nvoid mainImage( out vec4 fragColor, in vec2 fragCoord )\n{\n    vec2 uv = fragCoord.xy \/ iResolution.xy;\n    vec2 uv_orig = uv;\n    vec2 uv_half = fract(uv*2.);\n    if(uv.x < 0.5)\n    {\n        if(uv.y > 0.5)\n        {\n            fragColor = blur_vertical_upper_left(iChannel2, uv_half);\n        }\n        else\n        {\n            fragColor = blur_vertical_lower_left(iChannel2, uv_half);\n        }\n    }\n    else\n    {\n        for(int level = 0; level < 8; level++)\n        {\n            if((uv.x > 0.5 && uv.y >= 0.5) || (uv.x < 0.5))\n            {\n                break;\n            }\n            vec2 uv_half = fract(uv*2.);\n            fragColor = blur_vertical_left_column(uv_half, level);\n            uv = uv_half;\n        }  \n    }\n    uv = uv_orig;\n    float eighth = 1.\/8.;\n    if(uv.x > 7.*eighth && uv.x < 8.*eighth && uv.y > 2.*eighth && uv.y < 3.*eighth)\n    {\n        fragColor = vec4(iMouse.xy \/ iResolution.xy, iMouse.zw \/ iResolution.xy);\n    }\n}",
					"name":"Buf D",
					"description":"",
					"type":"buffer"
				}
			]
		}

		*/

