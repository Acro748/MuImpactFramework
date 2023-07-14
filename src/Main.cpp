#include <direct.h>
using namespace RE;
using namespace RE::BSScript;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;
using namespace Mus;

namespace {
    /**
     * Setup logging.
     *
     * <p>
     * Logging is important to track issues. CommonLibSSE bundles functionality for spdlog, a common C++ logging
     * framework. Here we initialize it, using values from the configuration file. This includes support for a debug
     * logger that shows output in your IDE when it has a debugger attached to Skyrim, as well as a file logger which
     * writes data to the standard SKSE logging directory at <code>Documents/My Games/Skyrim Special Edition/SKSE</code>
     * (or <code>Skyrim VR</code> if you are using VR).
     * </p>
     */
    void InitializeLogging() 
    {
        std::string LogDirectory = GetRuntimeSKSEDirectory();
        
        std::optional<std::filesystem::path> path(LogDirectory);
        *path /= PluginDeclaration::GetSingleton()->GetName();
        *path += L".log";

        std::shared_ptr<spdlog::logger> log;
        if (IsDebuggerPresent()) 
        {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        } 
        else 
        {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
        }

        log->set_level(Config::GetSingleton().GetLogLevel());
        log->flush_on(Config::GetSingleton().GetFlushLevel());

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
    }

    /**
     * Initialize the SKSE cosave system for our plugin.
     *
     * <p>
     * SKSE comes with a feature called a <em>cosave</em>, an additional save file kept alongside the original Skyrim
     * save file. SKSE plugins can write their own data to this file, and load it again when the save game is loaded,
     * allowing them to keep custom data along with a player's save. Each plugin must have a unique ID, which is four
     * characters long (similar to the record names used by forms in ESP files). Note however this is little-endian, so
     * technically the 'SMPL' here ends up as 'LPMS' in the save file, unless we use a byte order swap.
     * </p>
     *
     * <p>
     * There can only be one serialization callback for save, revert (called on new game and before a load), and load
     * for the entire plugin.
     * </p>
     */
    void AllSave(SKSE::SerializationInterface* serde)
    {
        ImpactManager::Save(serde);
    }

    void AllLoad(SKSE::SerializationInterface* serde)
    {
        std::uint32_t type;
        std::uint32_t size;
        std::uint32_t version;
        while (serde->GetNextRecordInfo(type, version, size)) {
            ImpactManager::Load(serde, type);
        }
    }

    void InitializeSerialization()
    {
        log::info("Initializing cosave serialization...");
        auto* serde = GetSerializationInterface();
        serde->SetUniqueID(_byteswap_ulong('MUIF'));
        serde->SetSaveCallback(AllSave);
        serde->SetLoadCallback(AllLoad);
    }

    /**
     * Initialize our Papyrus extensions.
     *
     * <p>
     * A common use of SKSE is to add new Papyrus functions. You can call a registration callback to do this. This
     * callback will not necessarily be called immediately, if the Papyrus VM has not been initialized yet (in that case
     * it's execution is delayed until the VM is available).
     * </p>
     *
     * <p>
     * You can call the <code>Register</code> function as many times as you want and at any time you want to register
     * additional functions.
     * </p>
     */
    void InitializePapyrus() 
    {
        log::info("Initializing Papyrus binding...");
        if (GetPapyrusInterface()->Register(Mus::Papyrus::RegisterPapyrusFunctions))
        {
            log::info("Papyrus functions bound.");
        } 
        else 
        {
            stl::report_and_fail("Failure to register Papyrus bindings.");
        }
    }

    /**
     * Initialize the trampoline space for function hooks.
     *
     * <p>
     * Function hooks are one of the most powerful features available to SKSE developers, allowing you to replace
     * functions with your own, or replace a function call with a call to another function. However, to do this, we
     * need a code snippet that replicates the first instructions of the original code we overwrote, in order to be
     * able to call back to the original control flow with all the same functionality.
     * </p>
     *
     * <p>
     * CommonLibSSE comes with functionality to allocate trampoline space, including a common singleton space we can
     * access from anywhere. While this is not necessarily the most advanced use of trampolines and hooks, this will
     * suffice for our demo project.
     * </p>
     */
    void InitializeHooking() 
    {
        log::info("Building hook...");
    }

    class ConfigLoader : //make late load in case that create a virtual form in other mods
        public RE::BSTEventSink<RE::MenuOpenCloseEvent> {
    public:
        [[nodiscard]] static ConfigLoader& GetSingleton() {
            static ConfigLoader instance;
            return instance;
        }

    protected:
        EventResult ProcessEvent(const RE::MenuOpenCloseEvent* evn, RE::BSTEventSource<RE::MenuOpenCloseEvent>*) override {
            if (!evn || evn->menuName.empty())
                return EventResult::kContinue;

            if (!evn->opening && IsSameString(evn->menuName.c_str(), "Main Menu"))
            {
                static_cast<MultipleConfig>(Config::GetSingleton()).LoadSetupConfig();
                return EventResult::kStop;
            }
            return EventResult::kContinue;
        };
    };

    void kDataloadedFunction()
    {
        if (const auto Menu = RE::UI::GetSingleton(); Menu) {
            Menu->AddEventSink<RE::MenuOpenCloseEvent>(&ConfigLoader::GetSingleton());
        }
        if (const auto EventHolder = RE::ScriptEventSourceHolder::GetSingleton(); EventHolder) {
            EventHolder->AddEventSink<RE::TESHitEvent>(&ImpactManager::GetSingleton());
        }
        ConditionManager::GetSingleton().InitialConditionMap();
    }

    void InitializeMessaging() 
    {
        if (!GetMessagingInterface()->RegisterListener([](MessagingInterface::Message* message) 
        {
            switch (message->type) 
            {
                // Skyrim lifecycle events.
                case MessagingInterface::kPostLoad: // Called after all plugins have finished running SKSEPlugin_Load.
                    // It is now safe to do multithreaded operations, or operations against other plugins.
                    break;
                case MessagingInterface::kPostPostLoad: // Called after all kPostLoad message handlers have run.
                    break;
                case MessagingInterface::kInputLoaded: // Called when all game data has been found.
                    break;
                case MessagingInterface::kDataLoaded: // All ESM/ESL/ESP plugins have loaded, main menu is now active.
                    // It is now safe to access form data.
                    kDataloadedFunction();
                    break;

                // Skyrim game events.
                case MessagingInterface::kNewGame: // Player starts a new game from main menu.
                    break;
                case MessagingInterface::kPreLoadGame: // Player selected a game to load, but it hasn't loaded yet.
                    // Data will be the name of the loaded save.
                    break;
                case MessagingInterface::kPostLoadGame: // Player's selected save game has finished loading.
                    // Data will be a boolean indicating whether the load was successful.
                    break;
                case MessagingInterface::kSaveGame: // The player has saved a game.
                    // Data will be the save name.
                    break;
                case MessagingInterface::kDeleteGame: // The player deleted a saved game from within the load menu.
                    break;
            }
        })) {
            stl::report_and_fail("Unable to register message listener.");
        }
    }
}

/**
 * This if the main callback for initializing your SKSE plugin, called just before Skyrim runs its main function.
 *
 * <p>
 * This is your main entry point to your plugin, where you should initialize everything you need. Many things can't be
 * done yet here, since Skyrim has not initialized and the Windows loader lock is not released (so don't do any
 * multithreading). But you can register to listen for messages for later stages of Skyrim startup to perform such
 * tasks.
 * </p>
 */


SKSEPluginLoad(const LoadInterface* skse) 
{
    Config::GetSingleton().LoadConfig();
    InitializeLogging();

    auto* plugin = PluginDeclaration::GetSingleton();
    auto version = plugin->GetVersion();
    log::info("{} {} is loading...", plugin->GetName(), version);

    auto runtime = REL::Module::get().version();
    log::info("Working on skyrim version : {}.{}.{}.{}", runtime.major(), runtime.minor(), runtime.patch(), runtime.build());

    Init(skse);
    InitializeMessaging();
    InitializeSerialization();
    InitializePapyrus();
    InitializeHooking();

    log::info("{} has finished loading.", plugin->GetName());
    return true;
}
