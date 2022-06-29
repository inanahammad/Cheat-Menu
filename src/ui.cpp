#include "pch.h"
#include "util.h"
#include "ui.h"
#include "widget.h"
#include "../depend/imgui/imgui_internal.h"
#include "menu.h"

ImVec2 Ui::GetSize(short count, bool spacing)
{
    if (count == 1)
    {
        spacing = false;
    }

    // manually tested values
    float factor = ImGui::GetStyle().ItemSpacing.x / 2.0f;
    float x;

    if (count == 3)
    {
        factor = ImGui::GetStyle().ItemSpacing.x / 1.403f;
    }

    if (spacing)
    {
        x = ImGui::GetWindowContentRegionWidth() / count - factor;
    }
    else
    {
        x = ImGui::GetWindowContentRegionWidth() / count;
    }

    return ImVec2(x, ImGui::GetFrameHeight() * 1.3f);
}

// Really messy code, cleanup someday
bool Ui::DrawTitleBar()
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("#CLOSE");

    ImGui::PushFont(FontMgr::Get("title"));
    Widget::TextCentered(MENU_TITLE);

    if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_RootWindow | ImGuiHoveredFlags_ChildWindows
                                | ImGuiHoveredFlags_AllowWhenBlockedByPopup | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem))
    {
        ImGui::PopFont();
        return false;
    }

    ImVec2 rectMin = ImGui::GetItemRectMin(); // get pos of title text
    ImGuiStyle& Style = ImGui::GetStyle();
    float framePadding = Style.FramePadding.x;
    float fontSize = ImGui::GetFontSize();
    ImRect title_bar_rect = window->TitleBarRect();
    ImVec2 pos = ImVec2(title_bar_rect.Max.x - framePadding*2 - fontSize, title_bar_rect.Min.y);

    // drawing the close button
    const ImRect bb(pos, pos + ImVec2(g.FontSize, g.FontSize) + g.Style.FramePadding * 2.0f);
    ImRect bb_interact = bb;
    const float area_to_visible_ratio = window->OuterRectClipped.GetArea() / bb.GetArea();
    if (area_to_visible_ratio < 1.5f)
    {
        bb_interact.Expand(ImFloor(bb_interact.GetSize() * -0.25f));
    }

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb_interact, id, &hovered, &held);

    float cross_extent = (fontSize * 0.3f) - 1.0f;
    ImVec2 closePos = ImVec2(bb.GetCenter().x - cross_extent, rectMin.y);
    ImU32 closeCol = ImGui::GetColorU32(held || hovered ? ImVec4(0.80f, 0.0f, 0.0f, 1.0f) : ImVec4(0.80f, 0.80f, 0.80f, 1.00f));
    window->DrawList->AddText(closePos, closeCol, "X");
    ImGui::PopFont();

    return pressed;
}

bool Ui::ListBox(const char* label, const std::vector<std::string>& all_items, int& selected)
{
    bool rtn = false;
    if (ImGui::BeginCombo(label, all_items[selected].c_str()))
    {
        for (size_t index = 0; index < all_items.size(); index++)
        {
            if (selected != index)
            {
                if (ImGui::MenuItem(all_items[index].c_str()))
                {
                    selected = index;
                    rtn = true;
                }
            }
        }
        ImGui::EndCombo();
    }
    return rtn;
}

bool Ui::ListBoxStr(const char* label, const std::vector<std::string>& all_items, std::string& selected)
{
    bool rtn = false;
    if (ImGui::BeginCombo(label, selected.c_str()))
    {
        for (std::string current_item : all_items)
        {
            if (ImGui::MenuItem(current_item.c_str()))
            {
                selected = current_item;
                rtn = true;
            }
        }
        ImGui::EndCombo();
    }

    return rtn;
}

bool Ui::CheckboxWithHint(const char* label, bool* v, const char* hint, bool is_disabled)
{
    // set things up
    bool pressed = false;
    const ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 textSize = ImGui::CalcTextSize(label, nullptr, true);
    float square_sz = ImGui::GetFrameHeight();
    ImDrawList* drawlist = ImGui::GetWindowDrawList();
    ImU32 color = ImGui::GetColorU32(ImGuiCol_FrameBg);
    std::string slabel = "##InvCheckboxBtn" + std::string(label);

    ImGui::BeginDisabled(is_disabled);

    // process the button states
    if (ImGui::InvisibleButton(slabel.c_str(), ImVec2(square_sz, square_sz)) && !is_disabled)
    {
        pressed = true;
        *v = !*v;
    }

    if (ImGui::IsItemHovered() && !is_disabled)
    {
        color = ImGui::GetColorU32(ImGuiCol_FrameBgHovered);
    }

    // draw the button
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    drawlist->AddRectFilled(min, max, color, ImGui::GetStyle().FrameRounding);

    int pad = static_cast<int>(square_sz / 6.0);
    pad = (pad < 1) ? 1 : pad;

    if (*v)
    {
        // draw the checkmark
        float sz = (square_sz - pad * 2.0);
        float thickness = sz / 5.0;
        thickness = (thickness < 1.0) ? 1.0 : thickness;
        sz = sz - thickness * 0.5;

        auto pos = ImVec2(min.x + pad, min.y + pad);
        pos.x = pos.x + thickness * 0.25;
        pos.y = pos.y + thickness * 0.25;

        float third = sz / 3.0;
        float bx = pos.x + third;
        float by = pos.y + sz - third * 0.5;

        drawlist->PathLineTo(ImVec2(bx - third, by - third));
        drawlist->PathLineTo(ImVec2(bx, by));
        drawlist->PathLineTo(ImVec2(bx + third * 2.0, by - third * 2.0));
        drawlist->PathStroke(ImGui::GetColorU32(ImGuiCol_CheckMark), false, thickness);
    }

    // draw label
    ImGui::SameLine(0, style.ItemInnerSpacing.x);
    if (ImGui::InvisibleButton(label, ImVec2(ImGui::CalcTextSize(label, nullptr, true).x, square_sz)) && !is_disabled)
    {
        pressed = true;
        *v = !*v;
    }
    min = ImGui::GetItemRectMin();
    drawlist->AddText(ImVec2(min.x, min.y + style.ItemInnerSpacing.y / 2), ImGui::GetColorU32(ImGuiCol_Text), label);

    // draw hint
    if (hint != nullptr)
    {
        ImGui::SameLine(0, style.ItemInnerSpacing.x);
        ImGui::InvisibleButton("?", ImGui::CalcTextSize("?", nullptr, true));
        min = ImGui::GetItemRectMin();
        drawlist->AddText(ImVec2(min.x, min.y + style.ItemInnerSpacing.y / 2), ImGui::GetColorU32(ImGuiCol_TextDisabled),
                          "?");

        if (ImGui::IsItemHovered() && !is_disabled)
        {
            ImGui::BeginTooltip();
            ImGui::Text(hint);
            ImGui::Spacing();
            ImGui::EndTooltip();
        }
    }

    ImGui::EndDisabled();

    return pressed;
}

bool Ui::CheckboxAddress(const char* label, const int addr, const char* hint)
{
    bool rtn = false;
    bool state = patch::Get<bool>(addr, false);

    if (CheckboxWithHint(label, &state, hint) && addr != NULL)
    {
        patch::Set<bool>(addr, state, false);
        rtn = true;
    }

    return rtn;
}

bool Ui::CheckboxAddressEx(const char* label, const int addr, int enabled_val, int disabled_val, const char* hint)
{
    bool rtn = false;

    bool state = false;
    int val = 0;
    patch::GetRaw(addr, &val, 1, false);

    state = (val == enabled_val);
    if (CheckboxWithHint(label, &state, hint) && addr != NULL)
    {
        if (state)
        {
            patch::SetRaw(addr, &enabled_val, 1, false);
        }
        else
        {
            patch::SetRaw(addr, &disabled_val, 1, false);
        }
        rtn = true;
    }

    return rtn;
}

bool Ui::CheckboxBitFlag(const char* label, uint flag, const char* hint)
{
    bool rtn = false;
    bool state = (flag == 1) ? true : false;
    if (CheckboxWithHint(label, &state, hint))
    {
        flag = state ? 1 : 0;
        rtn = true;
    }

    return rtn;
}

#ifdef GTASA
void Ui::EditStat(const char* label, const int stat_id, const int min, const int def, const int max)
{
    if (ImGui::CollapsingHeader(label))
    {
        int val = static_cast<int>(CStats::GetStatValue(stat_id));

        ImGui::Columns(3, nullptr, false);
        ImGui::Text("Min: %d", min);
        ImGui::NextColumn();
        ImGui::Text("Def: %d", def);
        ImGui::NextColumn();
        ImGui::Text("Max: %d", max);
        ImGui::Columns(1);

        ImGui::Spacing();

        if (ImGui::InputInt(("Set value##" + std::string(label)).c_str(), &val))
            CStats::SetStatValue(stat_id, static_cast<float>(val));

        ImGui::Spacing();

        if (ImGui::Button(("Minimum##" + std::string(label)).c_str(), GetSize(3)))
            CStats::SetStatValue(stat_id, static_cast<float>(min));

        ImGui::SameLine();

        if (ImGui::Button(("Default##" + std::string(label)).c_str(), GetSize(3)))
            CStats::SetStatValue(stat_id, static_cast<float>(def));

        ImGui::SameLine();

        if (ImGui::Button(("Maximum##" + std::string(label)).c_str(), GetSize(3)))
            CStats::SetStatValue(stat_id, static_cast<float>(max));

        ImGui::Spacing();
        ImGui::Separator();
    }
}
#endif

void Ui::RadioButtonAddress(const char* label, std::vector<NamedMemory>& named_mem)
{
    size_t btn_in_column = named_mem.size() / 2 - 1;

    ImGui::Text(label);
    ImGui::Columns(2, nullptr, false);

    bool state = true;

    for (size_t i = 0; i < named_mem.size(); i++)
    {
        if (patch::Get<bool>(named_mem[i].addr, false))
            state = false;
    }

    if (ImGui::RadioButton((std::string("None##") + label).c_str(), state))
    {
        for (size_t i = 0; i < named_mem.size(); i++)
            patch::Set<bool>(named_mem[i].addr, false);
    }

    for (size_t i = 0; i < named_mem.size(); i++)
    {
        state = patch::Get<bool>(named_mem[i].addr, false);

        if (ImGui::RadioButton(named_mem[i].name.c_str(), state))
        {
            for (size_t i = 0; i < named_mem.size(); i++)
                patch::Set<bool>(named_mem[i].addr, false);

            patch::Set<bool>(named_mem[i].addr, true);
        }

        if (i == btn_in_column)
            ImGui::NextColumn();
    }
    ImGui::Columns(1);
}

void Ui::RadioButtonAddressEx(const char* label, int addr, std::vector<NamedValue>& named_val)
{
    size_t btn_in_column = named_val.size() / 2;

    ImGui::Text(label);
    ImGui::Columns(2, nullptr, false);

    int mem_val = 0;
    patch::GetRaw(addr, &mem_val, 1, false);

    for (size_t i = 0; i < named_val.size(); i++)
    {
        if (ImGui::RadioButton(named_val[i].name.c_str(), &mem_val, named_val[i].value))
            patch::SetRaw(addr, &named_val[i].value, 1, false);

        if (i == btn_in_column)
            ImGui::NextColumn();
    }
    ImGui::Columns(1);
}

void Ui::EditRadioButtonAddress(const char* label, std::vector<NamedMemory>& named_mem)
{
    if (ImGui::CollapsingHeader(label))
    {
        RadioButtonAddress(label, named_mem);
        ImGui::Spacing();
        ImGui::Separator();
    }
}

void Ui::EditRadioButtonAddressEx(const char* label, int addr, std::vector<NamedValue>& named_val)
{
    if (ImGui::CollapsingHeader(label))
    {
        RadioButtonAddressEx(label, addr, named_val);
        ImGui::Spacing();
        ImGui::Separator();
    }
}

void Ui::ColorPickerAddress(const char* label, int base_addr, ImVec4&& default_color)
{
    if (ImGui::CollapsingHeader(label))
    {
        float cur_color[4];
        cur_color[0] = patch::Get<BYTE>(base_addr, false);
        cur_color[1] = patch::Get<BYTE>(base_addr + 1, false);
        cur_color[2] = patch::Get<BYTE>(base_addr + 2, false);
        cur_color[3] = patch::Get<BYTE>(base_addr + 3, false);

        // 0-255 -> 0-1
        cur_color[0] /= 255;
        cur_color[1] /= 255;
        cur_color[2] /= 255;
        cur_color[3] /= 255;

        if (ImGui::ColorPicker4(std::string("Pick color##" + std::string(label)).c_str(), cur_color))
        {
            // 0-1 -> 0-255
            cur_color[0] *= 255;
            cur_color[1] *= 255;
            cur_color[2] *= 255;
            cur_color[3] *= 255;

            patch::Set<BYTE>(base_addr, cur_color[0], false);
            patch::Set<BYTE>(base_addr + 1, cur_color[1], false);
            patch::Set<BYTE>(base_addr + 2, cur_color[2], false);
            patch::Set<BYTE>(base_addr + 3, cur_color[3], false);
        }
        ImGui::Spacing();

        if (ImGui::Button("Reset to default", GetSize()))
        {
            patch::Set<BYTE>(base_addr, default_color.x, false);
            patch::Set<BYTE>(base_addr + 1, default_color.y, false);
            patch::Set<BYTE>(base_addr + 2, default_color.z, false);
            patch::Set<BYTE>(base_addr + 3, default_color.w, false);
        }

        ImGui::Spacing();
        ImGui::Separator();
    }
}

void Ui::EditBits(const char* label, const int address, const std::vector<std::string>& names)
{
    auto mem_val = (int*)address;

    if (ImGui::CollapsingHeader(label))
    {
        ImGui::Columns(2, nullptr, false);

        for (int i = 0; i < 32; ++i)
        {
            int mask = 1 << i;
            bool state = *mem_val & mask;

            if (ImGui::Checkbox(names[i].c_str(), &state))
            {
                *mem_val ^= mask;
            }

            if (i + 1 == 32 / 2)
            {
                ImGui::NextColumn();
            }
        }
        ImGui::Columns(1);

        ImGui::Spacing();
        ImGui::Separator();
    }
}

void Ui::EditFloat(const char* label, const int address, const float min, const float def, const float max,
                   const float mul, const float change)
{
    if (ImGui::CollapsingHeader(label))
    {
        float val = patch::Get<float>(address, false) * mul;

        int items = 3;

        if (min == def)
            items = 2;

        ImGui::Columns(items, nullptr, false);

        ImGui::Text("Min: %f", min);

        if (items == 3)
        {
            ImGui::NextColumn();
            ImGui::Text("Def: %f", def);
        }

        ImGui::NextColumn();
        ImGui::Text("Max: %f", max);
        ImGui::Columns(1);

        ImGui::Spacing();

        int size = ImGui::GetFrameHeight();

        if (ImGui::InputFloat(("##" + std::string(label)).c_str(), &val))
            patch::SetFloat(address, val / mul, false);

        ImGui::SameLine(0.0, 4.0);
        if (ImGui::Button("-", ImVec2(size, size)) && (val - change) > min)
        {
            val -= change;
            patch::SetFloat(address, val / mul, false);
        }
        ImGui::SameLine(0.0, 4.0);
        if (ImGui::Button("+", ImVec2(size, size)) && (val + change) < max)
        {
            val += change;
            patch::SetFloat(address, val / mul, false);
        }
        ImGui::SameLine(0.0, 4.0);
        ImGui::Text("Set");


        ImGui::Spacing();

        if (ImGui::Button(("Minimum##" + std::string(label)).c_str(), GetSize(items)))
            patch::Set<float>(address, min / mul, false);

        if (items == 3)
        {
            ImGui::SameLine();

            if (ImGui::Button(("Default##" + std::string(label)).c_str(), GetSize(items)))
                patch::Set<float>(address, def / mul, false);
        }

        ImGui::SameLine();

        if (ImGui::Button(("Maximum##" + std::string(label)).c_str(), GetSize(items)))
            patch::Set<float>(address, max / mul, false);

        ImGui::Spacing();
        ImGui::Separator();
    }
}

bool Ui::ColorButton(int color_id, std::vector<float>& color, ImVec2 size)
{
    bool rtn = false;
    std::string label = "Color " + std::to_string(color_id);

    if (ImGui::ColorButton(label.c_str(), ImVec4(color[0], color[1], color[2], 1), 0, size))
    {
        rtn = true;
    }

    if (ImGui::IsItemHovered())
    {
        ImDrawList* drawlist = ImGui::GetWindowDrawList();
        drawlist->AddRectFilled(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
                                ImGui::GetColorU32(ImGuiCol_ModalWindowDimBg));
    }

    return rtn;
}
