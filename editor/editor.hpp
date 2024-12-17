#ifndef LOVEENGINE_EDITOR_HPP
#define LOVEENGINE_EDITOR_HPP

#include "SDL3/SDL.h"
#include "imgui.h"
#include "IconsFontAwesome5.h"

#include <filesystem>


/*
 *  Comment out #define IMGUI_ENABLE_FREETYPE on imconfig.h
 *
 *
 */

namespace love {
    namespace editor {
        enum class Theme {
            First,
            GoldSource,
            EverForest,
            Microsoft,
            Moonlight,
            PurpleComfy,
        };
    }

    class Editor {
    public:
        Editor(SDL_Window *window);

        ~Editor();


        void draw(bool& done);

    private:
        void SetupImGuiStyle(love::editor::Theme theme);
        void changeTheme(editor::Theme theme);


        std::filesystem::path currentPath = "/";
        bool asDetail = true;
        bool hideDots = true;

        bool b_showExplorer = true;
        void showExplorer(bool *p_open);




    };
}




#endif //LOVEENGINE_EDITOR_HPP
