#pragma once

namespace Mus {
    inline std::string GetRuntimeDirectory()
    {
        char RuntimeDirectory[4096];
        if (_getcwd(RuntimeDirectory, 4096) == NULL)
        {
            SKSE::stl::report_and_fail("Unable to lookup Runtime directory.");
        }
        std::string Directory = RuntimeDirectory;

        return Directory;
    }

    inline std::string GetRuntimeDataDirectory()
    {
        std::string Directory = GetRuntimeDirectory();
        Directory += "\\Data\\";

        return Directory;
    }

    inline std::string GetRuntimeMeshesDirectory()
    {
        std::string Directory = GetRuntimeDirectory();
        Directory += "\\Data\\Meshes\\";

        return Directory;
    }

    inline std::string GetRuntimeTexturesDirectory()
    {
        std::string Directory = GetRuntimeDirectory();
        Directory += "\\Data\\Textures\\";

        return Directory;
    }

    inline std::string GetRuntimeSKSEDirectory()
    {
        std::string Directory = GetRuntimeDirectory();
        Directory += "\\Data\\SKSE\\Plugins\\";

        return Directory;
    }

    inline RE::TESForm* GetFormByID(RE::FormID id, std::string_view modname = "")
    {
        if (!modname.empty())
        {
            RE::TESDataHandler* DataHandler = RE::TESDataHandler::GetSingleton();
            if (!DataHandler)
                return nullptr;

            if (REL::Module::IsVR())
            {
                auto esp = DataHandler->LookupModByName(modname);
                if (!esp)
                    return nullptr;

                RE::FormID index = 0;
                RE::FormID Compileindex = esp->GetCompileIndex();
                index = (Compileindex << 24);
                id += index;
            }
            else
            {
                id = DataHandler->LookupFormID(id, modname);
            }
        }
        RE::TESForm* result = RE::TESForm::LookupByID(id);
        return result ? result : nullptr;
    }

    template <typename T>
    inline T GetFormByID(RE::FormID id, std::string_view modname = "")
    {
        return skyrim_cast<T>(GetFormByID(id, modname));
    }

    inline bool IsThereTheESP(std::string_view modname)
    {
        RE::TESDataHandler* DataHandler = RE::TESDataHandler::GetSingleton();
        if (!DataHandler)
            return false;
        auto esp = DataHandler->LookupModByName(modname);
        return esp ? true : false;
    }

    inline RE::FormID GetIndexByPluginName(std::string_view modname)
    {
        RE::TESDataHandler* DataHandler = RE::TESDataHandler::GetSingleton();
        if (!DataHandler)
            return 0;
        auto esp = DataHandler->LookupModByName(modname);
        if (!esp)
            return 0;
        std::uint32_t index = esp->GetPartialIndex();
        return !esp->IsLight() ? index << 24 : index << 16;
    }

    inline std::uint32_t GetActorBaseFormID(RE::Actor* actor)
    {
        if (!actor)
            return 0;
        RE::TESNPC* actorBase = actor->GetActorBase();
        return actorBase ? actorBase->formID : 0;
    }

    inline std::uint8_t GetModIndex(RE::FormID id)
    {
        return (id >> 24);
    }

    inline std::uint16_t GetLightModIndex(RE::FormID id)
    {
        return ((id - (0xFE << 24)) >> 16);
    }

    inline bool IsLightMod(RE::FormID id)
    {
        return REL::Module::IsVR() ? false : ((id >> 24) == 0xFE); //VR hasn't esl
    }

    inline std::string_view GetModNameByIndex(std::uint8_t index)
    {
        RE::TESDataHandler* DataHandler = RE::TESDataHandler::GetSingleton();
        if (!DataHandler)
            return "NULL";
        const RE::TESFile* esp = DataHandler->LookupLoadedModByIndex(index);
        if (!esp)
            return "NULL";
        return DataHandler->LookupLoadedModByIndex(index)->GetFilename();
    }

    inline std::string_view GetLightModNameByIndex(std::uint16_t index)
    {
        RE::TESDataHandler* DataHandler = RE::TESDataHandler::GetSingleton();
        if (!DataHandler)
            return "NULL";
        const RE::TESFile* esp = DataHandler->LookupLoadedLightModByIndex(index);
        if (!esp)
            return "NULL";
        return DataHandler->LookupLoadedLightModByIndex(index)->GetFilename();
    }

    inline std::string_view GetModNameByID(RE::FormID id) //if modindex is 0xFF then it'll return "Template"
    {
        if (REL::Module::IsVR()) //why doesn't work on VR?
            return "VR";

        if ((id >> 24) == 0xFF)
            return "Template";

        try {
            if (!IsLightMod(id))
                return GetModNameByIndex(GetModIndex(id));
            else
                return GetLightModNameByIndex(GetLightModIndex(id));
        }
        catch (...) {
            return "ERROR";
        }
    }

    inline std::string_view GetModNameByForm(RE::TESForm* form)
    {
        if (!form)
            return "NULL";
        RE::TESFile* file = form->GetFile();
        return file ? file->GetFilename() : "Template";
    }

    inline std::string spaces(int n) {
        auto s = std::string(n, ' ');
        return s;
    }

    inline bool isPlayer(RE::FormID id) { return (id == 0x14 || id == 0x7); };

    inline RE::SEX GetPlayerSex() {
        RE::SEX sex = RE::SEX::kNone;
        auto Player = RE::PlayerCharacter::GetSingleton();
        if (!Player)
            return sex;
        auto PlayerBase = Player->GetActorBase();
        if (!PlayerBase)
            return sex;
        sex = PlayerBase->GetSex();
        return sex;
    }

    inline bool IsEqual(float a_value, float b_value)
    {
        return (a_value - b_value) >= -0.0001f && (a_value - b_value) <= 0.0001f;
    }
    inline bool IsEqual(RE::NiPoint3 a_value, RE::NiPoint3 b_value)
    {
        return IsEqual(a_value.x, b_value.x) && IsEqual(a_value.y, b_value.y) && IsEqual(a_value.z, b_value.z);
    }

    inline std::string lowLetter(std::string Letter)
    {
        for (size_t i = 0; i < Letter.size(); i++) {
            Letter[i] = tolower(Letter[i]);
        }
        return Letter;
    }

    inline bool IsContainString(std::string base, std::string find)
    {
        base = lowLetter(base);
        find = lowLetter(find);

        return (base.find(find) != std::string::npos);
    }

    inline bool IsSameString(std::string str1, std::string str2)
    {
        str1 = lowLetter(str1);
        str2 = lowLetter(str2);

        return (str1 == str2);
    }

    inline bool IsVirtual(RE::TESFile* file)
    {
        return file ? true : false;
    }

    inline bool IsArmorSlot(RE::TESObjectARMO* thisArmor, RE::BIPED_MODEL::BipedObjectSlot slotMask)
    {
        return thisArmor->bipedModelData.bipedObjectSlots.underlying() & std::to_underlying(slotMask);
    }

    inline void TimeLogger(bool isEnd = false, bool logging = true)
    {
        if (!logging)
            return;
        static LARGE_INTEGER startingTime, endingTime, elapsedMicroseconds, frequency;
        static std::uint32_t count = 0;
        QueryPerformanceFrequency(&frequency);
        if (!isEnd)
            QueryPerformanceCounter(&startingTime);
        else
        {
            QueryPerformanceCounter(&endingTime);
            elapsedMicroseconds.QuadPart = endingTime.QuadPart - startingTime.QuadPart;
            elapsedMicroseconds.QuadPart *= 1000000000LL;
            elapsedMicroseconds.QuadPart /= frequency.QuadPart;
            float us = (float)elapsedMicroseconds.QuadPart * 0.001f;
            float ms = us * 0.001f;
            float s = ms * 0.001f;
            logger::info("{}. {}ns / {:.6f}us / {:.6f}ms / {:.6f}s", count, elapsedMicroseconds.QuadPart, us, ms, s);
            count++;
        }
    }
}
