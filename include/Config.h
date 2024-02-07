#pragma once

namespace Mus {
    class Config {
    public:
        [[nodiscard]] static Config& GetSingleton() {
            static Config instance;
            return instance;
        };

        bool LoadConfig();

        //Debug
        [[nodiscard]] inline spdlog::level::level_enum GetLogLevel() const noexcept {
            return logLevel;
        }
        [[nodiscard]] inline spdlog::level::level_enum GetFlushLevel() const noexcept {
            return flushLevel;
        }

        //General
        [[nodiscard]] inline bool GetEnableInanimateObject() const noexcept {
            return EnableInanimateObject;
        }
        [[nodiscard]] inline std::uint32_t GetArtObjectVFXLimit() const noexcept {
            return ArtObjectVFXLimit;
        }
        [[nodiscard]] inline std::uint32_t GetSoundLimit() const noexcept {
            return SoundLimit;
        }
        [[nodiscard]] inline bool GetPersistent() const noexcept {
            return Persistent;
        }
        [[nodiscard]] inline bool GetEnableTimeCounter() const noexcept {
            return EnableTimeCounter;
        }
        
    private:
        //Debug
        spdlog::level::level_enum logLevel{ spdlog::level::level_enum::info };
        spdlog::level::level_enum flushLevel{ spdlog::level::level_enum::trace };

        //General
        bool EnableInanimateObject = false;
        std::uint32_t ArtObjectVFXLimit = 32;
        std::uint32_t SoundLimit = 32;
        bool Persistent = false;
        bool EnableTimeCounter = false;
    public:
        // trim from start (in place)
        inline static void ltrim(std::string& s)
        {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                [](int c) {return !std::isspace(c); }));
        }

        // trim from end (in place)
        inline static void rtrim(std::string& s)
        {
            s.erase(std::find_if(s.rbegin(), s.rend(),
                [](int c) {return !std::isspace(c); }).base(), s.end());
        }

        inline static void ltrim(std::string& s, char e)
        {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                [&](char c) {return c == e; }) + 1);
        }

        inline static void rtrim(std::string& s, char e)
        {
            s.erase(std::find_if(s.rbegin(), s.rend(),
                [&](char c) {return c == e; }).base() - 1, s.end());
        }

        // trim from both ends (in place)
        inline static void trim(std::string& s)
        {
            ltrim(s);
            rtrim(s);
        }

        inline static std::string trim_copy(std::string s)
        {
            trim(s);
            return s;
        }

        inline static std::vector<std::string> split(const std::string& s, std::string delimiter)
        {
            size_t pos_start = 0, pos_end, delim_len = delimiter.length();
            std::string token;
            std::vector<std::string> res;

            while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos)
            {
                token = s.substr(pos_start, pos_end - pos_start);
                trim(token);
                pos_start = pos_end + delim_len;
                res.emplace_back(token);
            }

            std::string lasttoken = s.substr(pos_start);
            trim(lasttoken);
            res.emplace_back(lasttoken);
            return res;
        }

        inline static std::vector<std::string> splitMulti(const std::string& s, std::string delimiters)
        {
            std::string str = trim_copy(s);

            std::vector<std::string> tokens;
            std::stringstream stringStream(str);
            std::string line;
            while (std::getline(stringStream, line))
            {
                std::size_t prev = 0, pos;
                while ((pos = line.find_first_of(delimiters, prev)) != std::string::npos)
                {
                    if (pos > prev)
                    {
                        std::string token = line.substr(prev, pos - prev);
                        trim(token);
                        tokens.emplace_back(token);
                    }

                    prev = pos + 1;
                }
                if (prev < line.length())
                {
                    std::string token = line.substr(prev, std::string::npos);
                    trim(token);
                    tokens.emplace_back(token);
                }
            }
            return tokens;
        }

        inline static void skipComments(std::string& str, const char* c = "#")
        {
            auto pos = str.find(c);
            if (pos != std::string::npos)
            {
                str.erase(pos);
            }
        }

        inline std::string GetConfigSettingsStringValue(std::string line, std::string& variable)
        {
            std::string value = line;
            std::vector<std::string> splittedLine = split(line, "=");
            variable = "";
            if (splittedLine.size() > 1)
            {
                variable = splittedLine[0];
                trim(variable);

                std::string valuestr = splittedLine[1];
                trim(valuestr);
                value = valuestr;
            }
            return value;
        }

        inline static RE::FormID getHex(std::string hexstr)
        {
            return (RE::FormID)strtoul(hexstr.c_str(), 0, 16);
        }

        inline int GetConfigSettingsIntValue(std::string valuestr)
        {
            int value = 0;
            value = std::stoi(valuestr);
            return value;
        }

        inline std::uint32_t GetConfigSettingsUintValue(std::string valuestr)
        {
            int value = 0;
            value = std::stoul(valuestr);
            return value;
        }

        inline float GetConfigSettingsFloatValue(std::string valuestr)
        {
            float value = 0;
            value = strtof(valuestr.c_str(), 0);
            return value;
        }

        inline bool GetConfigSettingsBoolValue(std::string valuestr)
        {
            bool value = false;
            valuestr = lowLetter(valuestr.c_str());
            value = (valuestr.compare("true") == 0);
            return value;
        }

        inline RE::FormID GetConfigSettingsFormIDValue(std::string valuestr)
        {
            RE::FormID value;
            value = getHex(valuestr.c_str());
            return value;
        }

        inline std::vector<RE::FormID> ConfigLineSplitterFormID(std::string valuestr)
        {
            std::vector<std::string> SplittedFormID = split(valuestr, "|");
            std::vector<RE::FormID> value;
            for (size_t index = 0; index < SplittedFormID.size(); index++)
            {
                trim(SplittedFormID[index]);
                value.emplace_back(getHex(SplittedFormID[index].c_str()));
            }
            return value;
        }
    };

    class MultipleConfig : public Config {
    public:
        bool LoadSetupConfig();

        static inline std::vector<std::filesystem::path> GetAllFiles(std::string folder)
        {
            std::vector<std::filesystem::path> files;

            auto path = std::filesystem::path(folder);
            if (!std::filesystem::exists(path))
                return files;
            if (!std::filesystem::is_directory(path))
                return files;

            for (const auto& file : std::filesystem::directory_iterator(folder))
            {
                files.emplace_back(file.path());
            }
            return files;
        }

        static inline bool stringEndsWith(std::string str, std::string suffix)
        {
            str = lowLetter(str);
            suffix = lowLetter(suffix);
            return str.ends_with(suffix);
        }
    };
}
