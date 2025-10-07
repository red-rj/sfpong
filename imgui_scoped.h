// scope guard helpers for imgui
// based on https://github.com/sethk/imgui/blob/raii/misc/cpp/imgui_scoped.h
// but using base classes instead of macros
#pragma once
#include <imgui.h>


namespace ImScoped
{
    // suppress copy and move ctors in derived class
    struct NonCopiable
    {
        NonCopiable(NonCopiable&&) = delete;
        NonCopiable& operator=(NonCopiable&&) = delete;

    protected:
        NonCopiable() = default;
    };
    // common bases
    struct Parent : NonCopiable
    {
        bool IsContentVisible;
        explicit operator bool() const { return IsContentVisible; }
    };
    struct Widget : NonCopiable
    {
        bool IsOpen;
        explicit operator bool() const { return IsOpen; }
    };
    using Property = NonCopiable;


    struct Window : Parent
    {
        Window(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0) {
            IsContentVisible = ImGui::Begin(name, p_open, flags);
        }
        ~Window() { ImGui::End(); }
    };

    struct Child : Parent
    {
        Child(const char* str_id, const ImVec2& size = ImVec2(0,0), bool border = false, ImGuiWindowFlags flags = 0) {
            IsContentVisible = ImGui::BeginChild(str_id, size, border, flags);
        }
        Child(ImGuiID id, const ImVec2& size = ImVec2(0,0), bool border = false, ImGuiWindowFlags flags = 0) {
            IsContentVisible = ImGui::BeginChild(id, size, border, flags);
        }
        ~Child() { ImGui::EndChild(); }
    };

    struct Font : Property
    {
        Font(ImFont* font) { ImGui::PushFont(font); }
        ~Font() { ImGui::PopFont(); }
    };

    struct StyleColor : Property
    {
        StyleColor(ImGuiCol idx, ImU32 col) { ImGui::PushStyleColor(idx, col); }
        StyleColor(ImGuiCol idx, const ImVec4& col) { ImGui::PushStyleColor(idx, col); }
        ~StyleColor() { ImGui::PopStyleColor(); }
    };

    struct StyleVar : Property
    {
        StyleVar(ImGuiStyleVar idx, float val) { ImGui::PushStyleVar(idx, val); }
        StyleVar(ImGuiStyleVar idx, const ImVec2& val) { ImGui::PushStyleVar(idx, val); }
        ~StyleVar() { ImGui::PopStyleVar(); }
    };

    struct ItemWidth : Property
    {
        ItemWidth(float item_width) { ImGui::PushItemWidth(item_width); }
        ~ItemWidth() { ImGui::PopItemWidth(); }
    };

    struct TextWrapPos : Property
    {
        TextWrapPos(float wrap_pos_x = 0.0f) { ImGui::PushTextWrapPos(wrap_pos_x); }
        ~TextWrapPos() { ImGui::PopTextWrapPos(); }
    };

    struct AllowKeyboardFocus : Property
    {
        AllowKeyboardFocus(bool allow_keyboard_focus) { ImGui::PushAllowKeyboardFocus(allow_keyboard_focus); }
        ~AllowKeyboardFocus() { ImGui::PopAllowKeyboardFocus(); }
    };

    struct ButtonRepeat : Property
    {
        ButtonRepeat(bool repeat) { ImGui::PushButtonRepeat(repeat); }
        ~ButtonRepeat() { ImGui::PopButtonRepeat(); }
    };

    struct Group : Property
    {
        Group() { ImGui::BeginGroup(); }
        ~Group() { ImGui::EndGroup(); }
    };

    struct ID : Property
    {
        ID(const char* str_id) { ImGui::PushID(str_id); }
        ID(const char* str_id_begin, const char* str_id_end) { ImGui::PushID(str_id_begin, str_id_end); }
        ID(const void* ptr_id) { ImGui::PushID(ptr_id); }
        ID(int int_id) { ImGui::PushID(int_id); }
        ~ID() { ImGui::PopID(); }
    };

    struct Combo : Widget
    {
        Combo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0) {
            IsOpen = ImGui::BeginCombo(label, preview_value, flags);
        }
        ~Combo() { if (IsOpen) ImGui::EndCombo(); }
    };

    struct TreeNode : Widget
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
            IsOpen = ImGui::TreeNodeV(ptr_id, fmt, ap);
            va_end(ap);
        }
        ~TreeNode() { if (IsOpen) ImGui::TreePop(); }
    };

    struct TreeNodeV : Widget
    {
        TreeNodeV(const char* str_id, const char* fmt, va_list args) IM_FMTLIST(3) { IsOpen = ImGui::TreeNodeV(str_id, fmt, args); }
        TreeNodeV(const void* ptr_id, const char* fmt, va_list args) IM_FMTLIST(3) { IsOpen = ImGui::TreeNodeV(ptr_id, fmt, args); }
        ~TreeNodeV() { if (IsOpen) ImGui::TreePop(); }
    };

    struct TreeNodeEx : Widget
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

    struct TreeNodeExV : Widget
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

    struct MainMenuBar : Widget
    {
        MainMenuBar() { IsOpen = ImGui::BeginMainMenuBar(); }
        ~MainMenuBar() { if (IsOpen) ImGui::EndMainMenuBar(); }
    };

    struct MenuBar : Widget
    {
        MenuBar() { IsOpen = ImGui::BeginMenuBar(); }
        ~MenuBar() { if (IsOpen) ImGui::EndMenuBar(); }
    };

    struct Menu : Widget
    {
        Menu(const char* label, bool enabled = true) { IsOpen = ImGui::BeginMenu(label, enabled); }
        ~Menu() { if (IsOpen) ImGui::EndMenu(); }
    };

    struct Tooltip : Property
    {
        Tooltip() { ImGui::BeginTooltip(); }
        ~Tooltip() { ImGui::EndTooltip(); }
    };

    struct Popup : Widget
    {
        Popup(const char* str_id, ImGuiWindowFlags flags = 0) { IsOpen = ImGui::BeginPopup(str_id, flags); }
        ~Popup() { if (IsOpen) ImGui::EndPopup(); }
    };

    struct PopupContextItem : Widget
    {
        PopupContextItem(const char* str_id = NULL, ImGuiPopupFlags popup_flags = ImGuiPopupFlags_MouseButtonRight) {
            IsOpen = ImGui::BeginPopupContextItem(str_id, popup_flags);
        }
        ~PopupContextItem() { if (IsOpen) ImGui::EndPopup(); }
    };

    struct PopupContextWindow : Widget
    {
        PopupContextWindow(const char* str_id = NULL, ImGuiPopupFlags popup_flags = ImGuiPopupFlags_MouseButtonRight) {
            IsOpen = ImGui::BeginPopupContextWindow(str_id, popup_flags);
        }
        ~PopupContextWindow() { if (IsOpen) ImGui::EndPopup(); }
    };

    struct PopupContextVoid : Widget
    {
        PopupContextVoid(const char* str_id = NULL, ImGuiPopupFlags popup_flags = ImGuiPopupFlags_MouseButtonRight) {
            IsOpen = ImGui::BeginPopupContextVoid(str_id, popup_flags);
        }
        ~PopupContextVoid() { if (IsOpen) ImGui::EndPopup(); }
    };

    struct PopupModal : Widget
    {
        PopupModal(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0) {
            IsOpen = ImGui::BeginPopupModal(name, p_open, flags);
        }
        ~PopupModal() { if (IsOpen) ImGui::EndPopup(); }

        void Close() { ImGui::CloseCurrentPopup(); }
    };

    struct DragDropSource : Widget
    {
        DragDropSource(ImGuiDragDropFlags flags = 0) { IsOpen = ImGui::BeginDragDropSource(flags); }
        ~DragDropSource() { if (IsOpen) ImGui::EndDragDropSource(); }
    };

    struct DragDropTarget : Widget
    {
        DragDropTarget() { IsOpen = ImGui::BeginDragDropTarget(); }
        ~DragDropTarget() { if (IsOpen) ImGui::EndDragDropTarget(); }
    };

    struct ClipRect : Property
    {
        ClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max, bool intersect_with_current_clip_rect) { 
            ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
        }
        ~ClipRect() { ImGui::PopClipRect(); }
    };

    struct ChildFrame : Widget
    {
        ChildFrame(ImGuiID id, const ImVec2& size, ImGuiWindowFlags flags = 0) { IsOpen = ImGui::BeginChildFrame(id, size, flags); }
        ~ChildFrame() { ImGui::EndChildFrame(); }
    };

    // eu

    struct TabBarItem : Widget
    {
        TabBarItem(const char* label, bool* p_open = nullptr, ImGuiTabBarFlags flags = 0) {
            IsOpen = ImGui::BeginTabItem(label, p_open, flags);
        }
        ~TabBarItem() { if (IsOpen) ImGui::EndTabItem(); }
    };

    struct TabBar : Widget
    {
        using Item = TabBarItem;

        TabBar(const char* str_id, ImGuiTabBarFlags flags = 0) {
            IsOpen = ImGui::BeginTabBar(str_id, flags);
        }
        ~TabBar() { if (IsOpen) ImGui::EndTabBar(); }
    };

    struct Indent : Property
    {
        explicit Indent(float width = 0.f) : Width(width) { ImGui::Indent(Width); }
        ~Indent() { ImGui::Unindent(Width); }

        float Width;
    };

    struct GroupID : ID
    {
        using ID::ID;

    private:
        Group _g_;
    };

} // namespace ImScoped
