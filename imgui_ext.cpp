#include "imgui_ext.h"

struct InputTextUserData_string
{
    std::string* Str;
    ImGuiInputTextCallback ChainCallback;
    void* ChainCallbackUserData;

    InputTextUserData_string(std::string* str, ImGuiInputTextCallback chain_cb, void* chain_data)
        : Str(str), ChainCallback(chain_cb), ChainCallbackUserData(chain_data) {}
};

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    auto user_data = (InputTextUserData_string*)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
        std::string* str = user_data->Str;
        IM_ASSERT(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = str->data();
    }
    else if (user_data->ChainCallback)
    {
        // Forward to user callback, if any
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

bool ImGui::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    auto cb_user_data = InputTextUserData_string(str, callback, user_data);
    return InputText(label, str->data(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

bool ImGui::InputTextMultiline(const char* label, std::string* str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    auto cb_user_data = InputTextUserData_string(str, callback, user_data);
    return InputTextMultiline(label, str->data(), str->capacity() + 1, size, flags, InputTextCallback, &cb_user_data);
}

bool ImGui::InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    auto cb_user_data = InputTextUserData_string(str, callback, user_data);
    return InputTextWithHint(label, hint, str->data(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}
