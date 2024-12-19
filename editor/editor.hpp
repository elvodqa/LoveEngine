#ifndef LOVEENGINE_EDITOR_HPP
#define LOVEENGINE_EDITOR_HPP


#include <stb_image.h>

#include "SDL3/SDL.h"
#include "imgui.h"
#include "IconsFontAwesome5.h"
#include "volk.h"
#include <filesystem>
#include <string.h>
#include "../Renderer/Renderer.h"
#include "../debug_panic.h"


/*
 *  Comment out #define IMGUI_ENABLE_FREETYPE on imconfig.h
 *
 *
 */

struct MyTextureData
{
    VkDescriptorSet DS;         // Descriptor set: this is what you'll pass to Image()
    int             Width;
    int             Height;
    int             Channels;

    // Need to keep track of these to properly cleanup
    VkImageView     ImageView;
    VkImage         Image;
    VkDeviceMemory  ImageMemory;
    VkSampler       Sampler;
    VkBuffer        UploadBuffer;
    VkDeviceMemory  UploadBufferMemory;

    MyTextureData() { memset(this, 0, sizeof(*this)); }
};

namespace love {
    namespace editor {
        // dummy
        struct ImageAsset {
            MyTextureData data;
            std::string path;
            std::string name;
        };

        enum class LogType {
            Trace,
            Info,
            Warn,
            Error,

            Debug,
        };

        enum class Theme {
            First,
            GoldSource,
            EverForest,
            Microsoft,
            Moonlight,
            PurpleComfy,
            Default,
        };


        struct LogItem {
            std::string message;
            love::editor::LogType type;
        };
    }


    class Editor {
    public:
        Editor(SDL_Window *window);

        ~Editor();


        void draw(bool& done);
        void check_events(const SDL_Event* _event);
        std::vector<love::editor::LogItem> consoleItems;

        void log(love::editor::LogType type, std::string msg);
    private:
        SDL_Renderer* renderer;
        std::vector<love::editor::ImageAsset> assetsImage;


        void SetupImGuiStyle(love::editor::Theme theme);
        void changeTheme(editor::Theme theme);


        std::filesystem::path currentPath = "/";
        bool asDetail = true;
        bool hideDots = true;

        bool b_eventFileDropped = false;
        char* c_eventFileDroppedName = nullptr;

        bool b_explorerShow = true;
        bool b_explorerSearchInputing = false;
        bool b_assetBrowserHovered = false;
        char* c_explorerSearchBuffer;
        char* c_assetSearchBuffer;
        const size_t t_assetSearchBufferSize = 1024;

        bool b_assetBrowserShow = true;
        bool b_consoleShow = true;
        bool b_scrollToBottom = false;
        char* c_consoleInputBuffer;
        const size_t t_consoleInputBufferSize = 1024;


        void showExplorer(bool *p_open);
        void ShowAssetBrowser(bool *p_open);
        void showConsole(bool *p_open);



    };
}




#endif //LOVEENGINE_EDITOR_HPP
