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

#include "LineFileErrors.h"

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////

ErrorLineFragment::ErrorLineFragment()
{
	line = 0;
}

ErrorLineFragment::ErrorLineFragment(size_t vLine, std::string vFile, std::string vErr)
{
	line = vLine;
	file = vFile;
	error = vErr;
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////

ErrorLine::ErrorLine()
{
}

ErrorLine::ErrorLine(std::vector<ErrorLineFragment> vFragment)
{
	fragments = vFragment;
}

ErrorLine::ErrorLine(std::string vErr)
{
	ErrorLineFragment frag;
	frag.error = vErr;

	fragments.push_back(frag);
}

ErrorLine::ErrorLine(size_t vLine, std::string vFile, std::string vErr)
{
	ErrorLineFragment frag;
	frag.line = vLine;
	frag.file = vFile;
	frag.error = vErr;

	fragments.push_back(frag);
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////

LineFileErrors::LineFileErrors()
{
}

LineFileErrors::LineFileErrors(const size_t& vLine, const std::string& vFile, const std::string& error)
{
	if (vLine != std::string::npos)
		errLine[vLine][vFile] = ErrorLine(vLine, vFile, error);
}

void LineFileErrors::Set(const size_t& vLine, const std::string& vFile, const std::string& error)
{
	if (vLine != std::string::npos)
		errLine[vLine][vFile] = ErrorLine(vLine, vFile, error);
}

void LineFileErrors::Set(const size_t& vLine, const std::string& vFile, ErrorLine vErrorLineStruct)
{
	if (vLine != std::string::npos)
	{
		bool found = false;
		// merge pour pas ecrire par dessus
		if (errLine.find(vLine) != errLine.end()) // trouvé
		{
			if (errLine[vLine].find(vFile) != errLine[vLine].end()) // trouvé
			{
				found = true;
				for (const auto& it : vErrorLineStruct.fragments)
				{
					errLine[vLine][vFile].fragments.push_back(it);
				}
			}
		}
		if (!found) // on a pas mergé en fait
			errLine[vLine][vFile] = vErrorLineStruct;
	}
}