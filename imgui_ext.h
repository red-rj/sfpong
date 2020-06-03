// imgui helpers and extensions
#pragma once
#define IMGUI_USER_CONFIG "imconfig-sfml.h"
#include <imgui.h>


// baseado em https://github.com/sethk/imgui/blob/raii/misc/cpp/imgui_scoped.h
namespace ImScoped
{
    /* Move and copy are not allowed */
    struct BaseGui
    {
        BaseGui(BaseGui&&) = delete;
        BaseGui& operator=(BaseGui&&) = delete;

    protected:
        BaseGui() = default;
    };
    struct VisibleGui : BaseGui
    {
        bool IsContentVisible;
        explicit operator bool() const { return IsContentVisible; }
    };
    struct OpenableGui : BaseGui
    {
        bool IsOpen;
        explicit operator bool() const { return IsOpen; }
    };


    struct Window : VisibleGui
    {
        Window(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0) {
            IsContentVisible = ImGui::Begin(name, p_open, flags);
        }
        ~Window() { ImGui::End(); }
    };

    struct Child : VisibleGui
    {
        Child(const char* str_id, const ImVec2& size = ImVec2(0,0), bool border = false, ImGuiWindowFlags flags = 0) {
            IsContentVisible = ImGui::BeginChild(str_id, size, border, flags);
        }
        Child(ImGuiID id, const ImVec2& size = ImVec2(0,0), bool border = false, ImGuiWindowFlags flags = 0) {
            IsContentVisible = ImGui::BeginChild(id, size, border, flags);
        }
        ~Child() { ImGui::EndChild(); }
    };

    struct Font : BaseGui
    {
        Font(ImFont* font) { ImGui::PushFont(font); }
        ~Font() { ImGui::PopFont(); }
    };

    struct StyleColor : BaseGui
    {
        StyleColor(ImGuiCol idx, ImU32 col) { ImGui::PushStyleColor(idx, col); }
        StyleColor(ImGuiCol idx, const ImVec4& col) { ImGui::PushStyleColor(idx, col); }
        ~StyleColor() { ImGui::PopStyleColor(); }
    };

    struct StyleVar : BaseGui
    {
        StyleVar(ImGuiStyleVar idx, float val) { ImGui::PushStyleVar(idx, val); }
        StyleVar(ImGuiStyleVar idx, const ImVec2& val) { ImGui::PushStyleVar(idx, val); }
        ~StyleVar() { ImGui::PopStyleVar(); }
    };

    struct ItemWidth : BaseGui
    {
        ItemWidth(float item_width) { ImGui::PushItemWidth(item_width); }
        ~ItemWidth() { ImGui::PopItemWidth(); }
    };

    struct TextWrapPos : BaseGui
    {
        TextWrapPos(float wrap_pos_x = 0.0f) { ImGui::PushTextWrapPos(wrap_pos_x); }
        ~TextWrapPos() { ImGui::PopTextWrapPos(); }
    };

    struct AllowKeyboardFocus : BaseGui
    {
        AllowKeyboardFocus(bool allow_keyboard_focus) { ImGui::PushAllowKeyboardFocus(allow_keyboard_focus); }
        ~AllowKeyboardFocus() { ImGui::PopAllowKeyboardFocus(); }
    };

    struct ButtonRepeat : BaseGui
    {
        ButtonRepeat(bool repeat) { ImGui::PushButtonRepeat(repeat); }
        ~ButtonRepeat() { ImGui::PopButtonRepeat(); }
    };

    struct Group : BaseGui
    {
        Group() { ImGui::BeginGroup(); }
        ~Group() { ImGui::EndGroup(); }
    };

    struct ID : BaseGui
    {
        ID(const char* str_id) { ImGui::PushID(str_id); }
        ID(const char* str_id_begin, const char* str_id_end) { ImGui::PushID(str_id_begin, str_id_end); }
        ID(const void* ptr_id) { ImGui::PushID(ptr_id); }
        ID(int int_id) { ImGui::PushID(int_id); }
        ~ID() { ImGui::PopID(); }
    };

    struct Combo : OpenableGui
    {
        Combo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0) {
            IsOpen = ImGui::BeginCombo(label, preview_value, flags);
        }
        ~Combo() { if (IsOpen) ImGui::EndCombo(); }
    };

    struct TreeNode : OpenableGui
    {
        TreeNode(const char* label) {
            IsOpen = ImGui::TreeNode(label);
        }
        TreeNode(const char* str_id, const char* fmt, ...) IM_FMTARGS(3) {
            va_list ap;
            va_start(ap, fmt);
            IsOpen = ImGui::TreeNodeV(str_id, fmt, ap);
            va_end(ap);
        }
        TreeNode(const void* ptr_id, const char* fmt, ...) IM_FMTARGS(3) {
            va_list ap;
            va_start(ap, fmt);
            IsOpen = ImGui::TreeNodeV(ptr_id, fmt, ap); va_end(ap);
        }
        ~TreeNode() { if (IsOpen) ImGui::TreePop(); }
    };

    struct TreeNodeV : OpenableGui
    {
        TreeNodeV(const char* str_id, const char* fmt, va_list args) IM_FMTLIST(3) { IsOpen = ImGui::TreeNodeV(str_id, fmt, args); }
        TreeNodeV(const void* ptr_id, const char* fmt, va_list args) IM_FMTLIST(3) { IsOpen = ImGui::TreeNodeV(ptr_id, fmt, args); }
        ~TreeNodeV() { if (IsOpen) ImGui::TreePop(); }
    };

    struct TreeNodeEx : OpenableGui
    {
        TreeNodeEx(const char* label, ImGuiTreeNodeFlags flags = 0) {
            IM_ASSERT(!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen));
            IsOpen = ImGui::TreeNodeEx(label, flags);
        }
        TreeNodeEx(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, ...) IM_FMTARGS(4) {
            va_list ap; va_start(ap, fmt);
            IM_ASSERT(!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen));
            IsOpen = ImGui::TreeNodeExV(str_id, flags, fmt, ap);
            va_end(ap);
        }
        TreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...) IM_FMTARGS(4) { 
            va_list ap;
            va_start(ap, fmt);
            IM_ASSERT(!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen));
            IsOpen = ImGui::TreeNodeExV(ptr_id, flags, fmt, ap); va_end(ap);
        }
        ~TreeNodeEx() { if (IsOpen) ImGui::TreePop(); }
    };

    struct TreeNodeExV : OpenableGui
    {
        TreeNodeExV(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args) IM_FMTLIST(4) {
            IM_ASSERT(!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen));
            IsOpen = ImGui::TreeNodeExV(str_id, flags, fmt, args);
        }
        TreeNodeExV(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args) IM_FMTLIST(4) { 
            IM_ASSERT(!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen));
            IsOpen = ImGui::TreeNodeExV(ptr_id, flags, fmt, args);
        }
        ~TreeNodeExV() { if (IsOpen) ImGui::TreePop(); }
    };

    struct MainMenuBar : OpenableGui
    {
        MainMenuBar() { IsOpen = ImGui::BeginMainMenuBar(); }
        ~MainMenuBar() { if (IsOpen) ImGui::EndMainMenuBar(); }
    };

    struct MenuBar : OpenableGui
    {
        MenuBar() { IsOpen = ImGui::BeginMenuBar(); }
        ~MenuBar() { if (IsOpen) ImGui::EndMenuBar(); }
    };

    struct Menu : OpenableGui
    {
        Menu(const char* label, bool enabled = true) { IsOpen = ImGui::BeginMenu(label, enabled); }
        ~Menu() { if (IsOpen) ImGui::EndMenu(); }
    };

    struct Tooltip
    {
        Tooltip() { ImGui::BeginTooltip(); }
        ~Tooltip() { ImGui::EndTooltip(); }
    };

    struct Popup : OpenableGui
    {
        Popup(const char* str_id, ImGuiWindowFlags flags = 0) { IsOpen = ImGui::BeginPopup(str_id, flags); }
        ~Popup() { if (IsOpen) ImGui::EndPopup(); }
    };

    struct PopupContextItem : OpenableGui
    {
        PopupContextItem(const char* str_id = NULL, int mouse_button = 1) {
            IsOpen = ImGui::BeginPopupContextItem(str_id, mouse_button);
        }
        ~PopupContextItem() { if (IsOpen) ImGui::EndPopup(); }
    };

    struct PopupContextWindow : OpenableGui
    {
        PopupContextWindow(const char* str_id = NULL, int mouse_button = 1, bool also_over_items = true) {
            IsOpen = ImGui::BeginPopupContextWindow(str_id, mouse_button, also_over_items);
        }
        ~PopupContextWindow() { if (IsOpen) ImGui::EndPopup(); }
    };

    struct PopupContextVoid : OpenableGui
    {
        PopupContextVoid(const char* str_id = NULL, int mouse_button = 1) {
            IsOpen = ImGui::BeginPopupContextVoid(str_id, mouse_button);
        }
        ~PopupContextVoid() { if (IsOpen) ImGui::EndPopup(); }
    };

    struct PopupModal : OpenableGui
    {
        PopupModal(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0) {
            IsOpen = ImGui::BeginPopupModal(name, p_open, flags);
        }
        ~PopupModal() { if (IsOpen) ImGui::EndPopup(); }

        void Close() { ImGui::CloseCurrentPopup(); }
    };

    struct DragDropSource : OpenableGui
    {
        DragDropSource(ImGuiDragDropFlags flags = 0) { IsOpen = ImGui::BeginDragDropSource(flags); }
        ~DragDropSource() { if (IsOpen) ImGui::EndDragDropSource(); }
    };

    struct DragDropTarget : OpenableGui
    {
        DragDropTarget() { IsOpen = ImGui::BeginDragDropTarget(); }
        ~DragDropTarget() { if (IsOpen) ImGui::EndDragDropTarget(); }
    };

    struct ClipRect
    {
        ClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect) { 
            ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
        }
        ~ClipRect() { ImGui::PopClipRect(); }
    };

    struct ChildFrame : OpenableGui
    {
        ChildFrame(ImGuiID id, const ImVec2& size, ImGuiWindowFlags flags = 0) { IsOpen = ImGui::BeginChildFrame(id, size, flags); }
        ~ChildFrame() { ImGui::EndChildFrame(); }
    };

    // eu

    struct TabBarItem : OpenableGui
    {
        TabBarItem(const char* label, bool* p_open = nullptr, ImGuiTabBarFlags flags = 0) {
            IsOpen = ImGui::BeginTabItem(label, p_open, flags);
        }
        ~TabBarItem() { if (IsOpen) ImGui::EndTabItem(); }
    };

    struct TabBar : VisibleGui
    {
        using Item = TabBarItem;

        TabBar(const char* str_id, ImGuiTabBarFlags flags = 0) {
            IsContentVisible = ImGui::BeginTabBar(str_id, flags);
        }
        ~TabBar() { ImGui::EndTabBar(); }
    };

    struct Indent : BaseGui
    {
        explicit Indent(float width = 0.f) : Width(width) { ImGui::Indent(Width); }
        ~Indent() { ImGui::Unindent(Width); }

        float Width;
    };

    struct OverlayWindow : Window
    {
        explicit OverlayWindow(const char* name, bool* p_open = NULL)
            : Window(name, p_open, wflags)
        {}

    private:

        static constexpr ImGuiWindowFlags wflags =
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize
            | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing
            | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;
    };
} // namespace ImScoped


// dear imgui: wrappers for C++ standard library (STL) types (std::string, etc.)
// This is also an example of how you may wrap your own similar types.

#include <string>

namespace ImGui
{
    // ImGui::InputText() with std::string
    // Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
    IMGUI_API bool  InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    IMGUI_API bool  InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    IMGUI_API bool  InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
}