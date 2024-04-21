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
#include <unordered_map>
#include <memory>
#include <vector>

struct MidiMessage
{
	std::string name;
	std::vector<uint8_t> bytes;
};

struct MidiStruct
{
	std::string deviceName;
	MidiMessage lastMessage;
	MidiMessage currentMessage;
};

class MidiInterface
{
protected:
	std::vector<MidiStruct> m_MidiDevices;

public:
	bool updateMidiNeeded = false;

public:
	uint32_t GetCountDevices()
	{
		return (uint32_t)m_MidiDevices.size();
	}
	virtual std::vector<MidiMessage> GetMidiMessages() = 0;
	virtual MidiMessage GetMidiMessage(const uint32_t& vPort) = 0;
	virtual MidiMessage GetAndClearMidiMessage(const uint32_t& vPort) = 0;
	virtual MidiMessage ClearMidiMessage(const uint32_t& vPort) = 0;
	virtual MidiStruct GetMidiMessageDB(const uint32_t& vPort) = 0;
	virtual std::string GetMidiDeviceName(const uint32_t& vPort) = 0;
};