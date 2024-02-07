#include "Config.h"

#include <direct.h>

using namespace Mus;

namespace Mus {
    bool Config::LoadConfig() {
        std::string configPath = GetRuntimeSKSEDirectory();
        configPath += SKSE::PluginDeclaration::GetSingleton()->GetName().data();
        configPath += ".ini";

        std::ifstream file(configPath);

        if (!file.is_open())
        {
            std::transform(configPath.begin(), configPath.end(), configPath.begin(), ::tolower);
            file.open(configPath);
        }

        if (!file.is_open())
        {
            logger::critical("Unable to {}.ini file.", SKSE::PluginDeclaration::GetSingleton()->GetName().data());
            return false;
        }

        std::string line;
        std::string currentSetting;
        while (std::getline(file, line))
        {
            //trim(line);
            skipComments(line);
            trim(line);
            if (line.length() == 0)
                continue;

            if (line.substr(0, 1) == "[")
            {
                currentSetting = line;
                continue;
            }
            std::string variableName;
            std::string variableValue = GetConfigSettingsStringValue(line, variableName);
            if (currentSetting == "[Debug]")
            {
                if (variableName == "logLevel")
                {
                    logLevel = spdlog::level::from_str(variableValue);
                }
                else if (variableName == "flushLevel")
                {
                    flushLevel = spdlog::level::from_str(variableValue);
                }
            }
            else if (currentSetting == "[General]")
            {
                if (variableName == "EnableInanimateObject")
                {
                    EnableInanimateObject = GetConfigSettingsBoolValue(variableValue);
                }
                else if (variableName == "ArtObjectLimit")
                {
                    ArtObjectVFXLimit = GetConfigSettingsUintValue(variableValue);
                }
                else if (variableName == "SoundLimit")
                {
                    SoundLimit = GetConfigSettingsUintValue(variableValue);
                }
                else if (variableName == "Persistent")
                {
                    Persistent = GetConfigSettingsBoolValue(variableValue);
                }
                else if (variableName == "EnableTimeCounter")
                {
                    EnableTimeCounter = GetConfigSettingsBoolValue(variableValue);
                }
            }
        }
        logger::info("Config loaded done");
        return true;
    }

    bool MultipleConfig::LoadSetupConfig()
    {
        std::string configPath = GetRuntimeSKSEDirectory();
        configPath += SKSE::PluginDeclaration::GetSingleton()->GetName().data();

        auto files = GetAllFiles(configPath);
        concurrency::parallel_for_each(files.begin(), files.end(), [&](auto& file) {
            std::u8string filename_utf8 = file.filename().u8string();
            std::string filename(filename_utf8.begin(), filename_utf8.end());
            if (filename != "." && filename != "..")
            {
                if (stringEndsWith(filename, ".ini"))
                {
                    logger::info("File found: {}", filename);
                    std::ifstream ifile(file);
                    if (ifile.is_open())
                    {
                        ConditionManager::Condition condition;
                        condition.fileName = filename;

                        std::string line;
                        bool isAggressor = false;
                        bool isTarget = false;
                        while (std::getline(ifile, line))
                        {
                            //trim(line);
                            skipComments(line);
                            trim(line);
                            if (line.length() > 0)
                            {
                                std::string variableName;
                                std::string variableValue = GetConfigSettingsStringValue(line, variableName);
                                if (variableName == "ImpactDataSet")
                                {
                                    auto values = split(variableValue, ",");
                                    if (values.size() == 0)
                                        continue;
                                    for (auto& value : values)
                                    {
                                        trim(value);
                                        RE::FormID id;
                                        std::string pluginname;
                                        auto plugins = split(value, "|");
                                        if (plugins.size() == 1)
                                        {
                                            id = getHex(plugins.at(0));
                                        }
                                        else if (plugins.size() == 2)
                                        {
                                            pluginname = plugins.at(0);
                                            id = getHex(plugins.at(1));
                                        }
                                        else
                                            continue;
                                        auto form = GetFormByID<RE::BGSImpactDataSet*>(id, pluginname);
                                        if (form)
                                            condition.ImpactDataSets.emplace_back(form);
                                    }
                                    isAggressor = false;
                                    isTarget = false;
                                }
                                else if (variableName == "Spell")
                                {
                                    auto values = split(variableValue, ",");
                                    if (values.size() == 0)
                                        continue;
                                    for (auto& value : values)
                                    {
                                        trim(value);
                                        RE::FormID id;
                                        std::string pluginname;
                                        auto plugins = split(value, "|");
                                        if (plugins.size() == 1)
                                        {
                                            id = getHex(plugins.at(0));
                                        }
                                        else if (plugins.size() == 2)
                                        {
                                            pluginname = plugins.at(0);
                                            id = getHex(plugins.at(1));
                                        }
                                        else
                                            continue;
                                        auto form = GetFormByID<RE::SpellItem*>(id, pluginname);
                                        if (form)
                                            condition.SpellItems.emplace_back(form);
                                    }
                                    isAggressor = false;
                                    isTarget = false;
                                }
                                else if (variableName == "VFX")
                                {
                                    auto values = split(variableValue, ",");
                                    if (values.size() == 0)
                                        continue;
                                    for (auto& value : values)
                                    {
                                        trim(value);
                                        ConditionManager::VFXInfo vfxInfo;
                                        vfxInfo.vfxPath = value;
                                        vfxInfo.vfxType = ConditionManager::GetVFXType(value);
                                        condition.VFXItems.emplace_back(vfxInfo);
                                    }
                                    isAggressor = false;
                                    isTarget = false;
                                }
                                else if (variableName == "Sound1")
                                {
                                    auto values = split(variableValue, ",");
                                    if (values.size() == 0)
                                        continue;
                                    for (auto& value : values)
                                    {
                                        trim(value);
                                        RE::FormID id;
                                        std::string pluginname;
                                        auto plugins = split(value, "|");
                                        if (plugins.size() == 1)
                                        {
                                            id = getHex(plugins.at(0));
                                        }
                                        else if (plugins.size() == 2)
                                        {
                                            pluginname = plugins.at(0);
                                            id = getHex(plugins.at(1));
                                        }
                                        else
                                            continue;
                                        auto form = GetFormByID<RE::BGSSoundDescriptorForm*>(id, pluginname);
                                        if (form)
                                            condition.SoundDescriptor1Items.emplace_back(form);
                                    }
                                    isAggressor = false;
                                    isTarget = false;
                                }
                                else if (variableName == "Sound2")
                                {
                                    auto values = split(variableValue, ",");
                                    if (values.size() == 0)
                                        continue;
                                    for (auto& value : values)
                                    {
                                        trim(value);
                                        RE::FormID id;
                                        std::string pluginname;
                                        auto plugins = split(value, "|");
                                        if (plugins.size() == 1)
                                        {
                                            id = getHex(plugins.at(0));
                                        }
                                        else if (plugins.size() == 2)
                                        {
                                            pluginname = plugins.at(0);
                                            id = getHex(plugins.at(1));
                                        }
                                        else
                                            continue;
                                        auto form = GetFormByID<RE::BGSSoundDescriptorForm*>(id, pluginname);
                                        if (form)
                                            condition.SoundDescriptor2Items.emplace_back(form);
                                    }
                                    isAggressor = false;
                                    isTarget = false;
                                }
                                else if (variableName == "EffectShader")
                                {
                                    auto values = split(variableValue, ",");
                                    if (values.size() == 0)
                                        continue;
                                    for (auto& value : values)
                                    {
                                        trim(value);
                                        RE::FormID id;
                                        std::string pluginname;
                                        auto plugins = split(value, "|");
                                        if (plugins.size() == 1)
                                        {
                                            id = getHex(plugins.at(0));
                                        }
                                        else if (plugins.size() == 2)
                                        {
                                            pluginname = plugins.at(0);
                                            id = getHex(plugins.at(1));
                                        }
                                        else
                                            continue;
                                        auto form = GetFormByID<RE::TESEffectShader*>(id, pluginname);
                                        if (form)
                                            condition.EffectShaderItems.emplace_back(form);
                                    }
                                    isAggressor = false;
                                    isTarget = false;
                                }
                                else if (variableName == "ArtObject")
                                {
                                    auto values = split(variableValue, ",");
                                    if (values.size() == 0)
                                        continue;
                                    for (auto& value : values)
                                    {
                                        trim(value);
                                        RE::FormID id;
                                        std::string pluginname;
                                        auto plugins = split(value, "|");
                                        if (plugins.size() == 1)
                                        {
                                            id = getHex(plugins.at(0));
                                        }
                                        else if (plugins.size() == 2)
                                        {
                                            pluginname = plugins.at(0);
                                            id = getHex(plugins.at(1));
                                        }
                                        else
                                            continue;
                                        auto form = GetFormByID<RE::BGSArtObject*>(id, pluginname);
                                        if (form)
                                            condition.ArtObjectItems.emplace_back(form);
                                    }
                                    isAggressor = false;
                                    isTarget = false;
                                }
                                else if (variableName == "Aggressor")
                                {
                                    condition.originalCondition[ConditionManager::ConditionOption::Aggressor] = variableValue;
                                    isAggressor = true;
                                    isTarget = false;
                                }
                                else if (variableName == "Target")
                                {
                                    condition.originalCondition[ConditionManager::ConditionOption::Target] = variableValue;
                                    isAggressor = false;
                                    isTarget = true;
                                }
                                else if (variableName == "Option")
                                {
                                    auto options = split(variableValue, ",");
                                    for (auto& option : options)
                                    {
                                        auto option_arg = split(option, ":");
                                        if (option_arg.size() == 1)
                                        {
                                            if (IsSameString(option_arg.at(0), "randomdirection") || option_arg.at(0) == "RD")
                                                condition.option.RandomDirection = true;
                                        }
                                        else if (option_arg.size() == 2)
                                        {
                                            if (IsSameString(option_arg.at(0), "scale") || option_arg.at(0) == "S")
                                                condition.option.Scale = std::stof(option_arg.at(1));
                                            else if (IsSameString(option_arg.at(0), "duration") || option_arg.at(0) == "D")
                                                condition.option.Duration = std::stof(option_arg.at(1));
                                        }
                                    }
                                }
                                else
                                {
                                    if (isAggressor)
                                        condition.originalCondition[ConditionManager::ConditionOption::Aggressor] += " " + variableValue;
                                    else if (isTarget)
                                        condition.originalCondition[ConditionManager::ConditionOption::Target] += " " + variableValue;
                                }
                            }
                        }
                        if (ConditionManager::GetSingleton().RegisterCondition(condition))
                            logger::info("Registered Condition file : {}", filename);
                        else
                            logger::error("There is no valid effect! : {}", condition.fileName);
                    }
                }
            }
        });
        logger::info("Registered Condition : {}", ConditionManager::GetSingleton().ConditionCount());

        return true;
    }
}

