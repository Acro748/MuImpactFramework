#include "Config.h"

#include <articuno/archives/ryml/ryml.h>
#include <direct.h>

using namespace articuno::ryml;
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
                if (variableName == "ImpactMode")
                {
                    ImpactMode = GetConfigSettingsIntValue(variableValue);
                }
                else if (variableName == "InstanceMode")
                {
                    InstanceMode = GetConfigSettingsBoolValue(variableValue);
                }
                else if (variableName == "NodeDiscoveryMode")
                {
                    NodeDiscoveryMode = GetConfigSettingsIntValue(variableValue);
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
        configPath += "\\";

        auto configList = get_all_files_names_within_folder(configPath.c_str());
        concurrency::parallel_for(std::size_t(0), configList.size(), [&](std::size_t i) {
            std::string filename = configList.at(i);

            if (filename != "." && filename != "..")
            {
                if (stringEndsWith(filename, ".ini"))
                {
                    logger::info("File found: {}", filename);

                    std::string filepath = configPath;
                    filepath.append(filename);
                    std::ifstream file(filepath);

                    if (!file.is_open())
                    {
                        transform(filepath.begin(), filepath.end(), filepath.begin(), ::tolower);
                        file.open(filepath);
                    }

                    if (file.is_open())
                    {
                        ConditionManager::Condition condition;
                        std::string presetfile;

                        std::string line;
                        while (std::getline(file, line))
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
                                        ConditionManager::PluginsInfo pluginInfo;
                                        auto plugins = split(value, "|");
                                        if (plugins.size() == 1)
                                        {
                                            pluginInfo.id = getHex(plugins.at(0));
                                        }
                                        else if (plugins.size() == 2)
                                        {
                                            pluginInfo.pluginName = plugins.at(0);
                                            pluginInfo.id = getHex(plugins.at(1));
                                        }
                                        else
                                            continue;
                                        condition.ImpactDataSets.emplace_back(pluginInfo);
                                    }
                                }
                                else if (variableName == "Spell")
                                {
                                    auto values = split(variableValue, ",");
                                    if (values.size() == 0)
                                        continue;
                                    for (auto& value : values)
                                    {
                                        trim(value);
                                        ConditionManager::PluginsInfo pluginInfo;
                                        auto plugins = split(value, "|");
                                        if (plugins.size() == 1)
                                        {
                                            pluginInfo.id = getHex(plugins.at(0));
                                        }
                                        else if (plugins.size() == 2)
                                        {
                                            pluginInfo.pluginName = plugins.at(0);
                                            pluginInfo.id = getHex(plugins.at(1));
                                        }
                                        else
                                            continue;
                                        condition.SpellItems.emplace_back(pluginInfo);
                                    }
                                }
                                else if (variableName == "Aggressor")
                                {
                                    condition.originalCondition[ConditionManager::ConditionOption::Aggressor] = variableValue;
                                }
                                else if (variableName == "Target")
                                {
                                    condition.originalCondition[ConditionManager::ConditionOption::Target] = variableValue;
                                }
                            }
                        }
                        if (ConditionManager::GetSingleton().RegisterCondition(condition, presetfile))
                            logger::info("Registered Condition file : {}", filename);
                    }
                }
            }
            });
        return true;
    }
}

