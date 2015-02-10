// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2
// Refer to the license.txt file included.


// HideObjectEngine
// Supports the removal of objects/effects from the rendering loop

#include "Core/ConfigManager.h"
#include "Core/Core.h"
#include "Core/HideObjectEngine.h"

namespace HideObjectEngine
{
	const char *HideObjectTypeStrings[] =
	{
		"8bits",
		"16bits",
		"24bits",
		"32bits",
		"40bits",
		"48bits",
		"56bits",
		"64bits",
		"72bits",
		"80bits",
		"88bits",
		"96bits",
		"104bits",
		"112bits",
		"120bits",
		"128bits"
	};

	static std::vector<HideObject> HideObjectCodes;

	void LoadHideObjectSection(const std::string& section, std::vector<HideObject>& HideObjectects, IniFile& globalIni, IniFile& localIni)
	{
		// Load the name of all enabled HideObjectects
		std::string enabledSectionName = section + "_Enabled";
		std::vector<std::string> enabledLines;
		std::set<std::string> enabledNames;
		localIni.GetLines(enabledSectionName, &enabledLines);
		for (const std::string& line : enabledLines)
		{
			if (line.size() != 0 && line[0] == '$')
			{
				std::string name = line.substr(1, line.size() - 1);
				enabledNames.insert(name);
			}
		}

		const IniFile* inis[2] = { &globalIni, &localIni };

		for (const IniFile* ini : inis)
		{
			std::vector<std::string> lines;
			HideObject currentHideObject;
			ini->GetLines(section, &lines);

			for (std::string& line : lines)
			{
				if (line.empty())
					continue;

				if (line[0] == '$')
				{
					// Take care of the previous code
					if (currentHideObject.name.size())
					{
						HideObjectects.push_back(currentHideObject);
					}
					currentHideObject.entries.clear();

					// Set active and name
					currentHideObject.name = line.substr(1, line.size() - 1);
					currentHideObject.active = enabledNames.find(currentHideObject.name) != enabledNames.end();
					currentHideObject.user_defined = (ini == &localIni);
				}
				else
				{
					std::string::size_type loc = line.find('=');

					if (loc != std::string::npos)
					{
						line[loc] = ':';
					}

					std::vector<std::string> items;
					SplitString(line, ':', items);

					if (items.size() >= 3)
					{
						HideObjectEntry pE;
						bool success = true;
						success &= TryParse(items[1], &pE.value_upper);
						success &= TryParse(items[2], &pE.value_lower);

						pE.type = HideObjectType(std::find(HideObjectTypeStrings, HideObjectTypeStrings + 16, items[0]) - HideObjectTypeStrings);
						success &= (pE.type != (HideObjectType)16);
						if (success)
						{
							currentHideObject.entries.push_back(pE);
						}
					}
				}
			}

			if (currentHideObject.name.size() && currentHideObject.entries.size())
			{
				HideObjectects.push_back(currentHideObject);
			}
		}
	}

	void LoadHideObjects()
	{
		IniFile merged = SConfig::GetInstance().m_LocalCoreStartupParameter.LoadGameIni();
		IniFile globalIni = SConfig::GetInstance().m_LocalCoreStartupParameter.LoadDefaultGameIni();
		IniFile localIni = SConfig::GetInstance().m_LocalCoreStartupParameter.LoadLocalGameIni();

		LoadHideObjectSection("HideObjectCodes", HideObjectCodes, globalIni, localIni);
	}

	void ApplyHideObjects(const std::vector<HideObject> &HideObjectects)
	{
		SConfig::GetInstance().m_LocalCoreStartupParameter.object_removal_codes.clear();

		for (const HideObject& HideObjectect : HideObjectects)
		{
			if (HideObjectect.active)
			{
				for (const HideObjectEntry& entry : HideObjectect.entries)
				{
					u64 value_add_lower = entry.value_lower;
					u64 value_add_upper = entry.value_upper;
					SkipEntry skipEntry;
					int size = GetHideObjectTypeCharLength(entry.type) >> 1;

					if (size > 8)
					{
						size -= 8;
						for (int j = size; j > 0; --j)
						{
							skipEntry.push_back((0xFF & (value_add_upper >> ((j - 1) * 8))));
						}
						size = 8;
					}

					for (int j = size; j > 0; --j)
					{
						skipEntry.push_back((0xFF & (value_add_lower >> ((j - 1) * 8))));
					}

					SConfig::GetInstance().m_LocalCoreStartupParameter.object_removal_codes.push_back(skipEntry);
				}
			}
		}
		SConfig::GetInstance().m_LocalCoreStartupParameter.num_object_removal_codes = SConfig::GetInstance().m_LocalCoreStartupParameter.object_removal_codes.size();
	}

	void ApplyFrameHideObjects()
	{
		ApplyHideObjects(HideObjectCodes);
	}

	void Shutdown()
	{
		HideObjectCodes.clear();
	}

}  // namespace
