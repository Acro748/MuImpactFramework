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
        [[nodiscard]] inline bool GetInstanceMode() const noexcept {
            return InstanceMode;
        }
        [[nodiscard]] inline bool GetEnableInanimateObject() const noexcept {
            return EnableInanimateObject;
        }
        [[nodiscard]] inline std::uint32_t GetArtObjectVFXLimit() const noexcept {
            return ArtObjectVFXLimit;
        }
        [[nodiscard]] inline std::uint32_t GetSoundLimit() const noexcept {
            return SoundLimit;
        }
        [[nodiscard]] inline bool GetProxyProjectileEnable() const noexcept {
            return ProxyProjectileEnable;
        }
        [[nodiscard]] inline std::vector<std::uint32_t> GetRemoveList() const noexcept {
            return RemoveList;
        }
        [[nodiscard]] inline bool GetEnableTimeCounter() const noexcept {
            return EnableTimeCounter;
        }
        
    private:
        //Debug
        spdlog::level::level_enum logLevel{ spdlog::level::level_enum::info };
        spdlog::level::level_enum flushLevel{ spdlog::level::level_enum::trace };

        //General
        bool InstanceMode = false;
        bool EnableInanimateObject = false;
        std::uint32_t ArtObjectVFXLimit = 32;
        std::uint32_t SoundLimit = 32;

        bool ProxyProjectileEnable = true;
        std::vector<std::uint32_t> RemoveList;
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

        inline static std::string ws2s(const std::wstring& wstr)
        {
            using convert_typeX = std::codecvt_utf8<wchar_t>;
            std::wstring_convert<convert_typeX, wchar_t> converterX;

            return converterX.to_bytes(wstr);
        }
        inline static std::string ansi2utf8(std::string str)
        {
            std::wstring unicode;
            const char* ansi = str.c_str();
            std::size_t ansi_size = str.size();
            do {
                if ((nullptr == ansi) || (0 == ansi_size)) {
                    break;
                }
                unicode.clear();

                int required_cch = ::MultiByteToWideChar(
                    CP_ACP,
                    0,
                    ansi, static_cast<int>(ansi_size),
                    nullptr, 0
                );

                if (0 == required_cch) {
                    break;
                }
                unicode.resize(required_cch);

                if (0 == ::MultiByteToWideChar(
                    CP_ACP,
                    0,
                    ansi, static_cast<int>(ansi_size),
                    const_cast<wchar_t*>(unicode.c_str()), static_cast<int>(unicode.size())
                )) {
                    break;
                }
            } while (false);
            return ws2s(unicode);
        }
    };

    class MultipleConfig : public Config {
    public:
        bool LoadSetupConfig();

        static inline std::vector<std::string> GetAllFiles(std::string folder)
        {
            std::vector<std::string> names;
            for (const auto& file : std::filesystem::directory_iterator(folder))
            {
                names.emplace_back(file.path().string());
            }
            return names;
        }

        static inline bool stringStartsWith(std::string str, std::string prefix)
        {
            str = lowLetter(str);
            prefix = lowLetter(prefix);
            if (str.find(prefix) == 0)
                return true;
            else
                return false;
        }

        static inline bool stringEndsWith(std::string str, std::string suffix)
        {
            str = lowLetter(str);
            suffix = lowLetter(suffix);
            if (str.length() >= suffix.length())
            {
                return (0 == str.compare(str.length() - suffix.length(), suffix.length(), suffix));
            }
            else
            {
                return false;
            }
        }
    };
}
