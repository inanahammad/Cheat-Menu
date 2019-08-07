-- This module contains functions for visuals tab

local module = {}

local tvisuals =
{
    show_coordinates = imgui.new.bool(fconfig.get('tvisuals.show_coordinates',false)),
    zone_names = imgui.new.bool(true),
    car_names  = imgui.new.bool(true),
}

module.tvisuals = tvisuals

function module.VisualsMain()
    if imgui.BeginTabBar("Visuals") then
        imgui.Spacing()
        if imgui.BeginTabItem(flanguage.GetText('common.Checkboxes')) then
            imgui.Spacing()

            imgui.Columns(2,nil,false)

            fcommon.CheckBox({ address = 0xA444A0,name = flanguage.GetText('visuals.EnableHud')})
            fcommon.CheckBox({ address = 0xA444A4,name = flanguage.GetText('visuals.GrayRadar')})
            fcommon.CheckBox({ address = 0xBA676C,name = flanguage.GetText('visuals.EnableRadar'),value = 0,value2 = 2})
            fcommon.CheckBox({ address = 0x58F58D,name = flanguage.GetText('visuals.MoneyFontBorder')})
            fcommon.CheckBox({ address = 0x589123,name = flanguage.GetText('visuals.BodyArmourBorder')})
            fcommon.CheckBox({ address = 0x589207,name = flanguage.GetText('visuals.BreathMeterBorder')})


            imgui.NextColumn()
            fcommon.CheckBox({ name = flanguage.GetText('visuals.ShowCoordinates'),var = tvisuals.show_coordinates,show_help_popups = true,help_text = "Draws coordinates on-screen.\n\nOptions:\nPressing (Left Ctrl + C) stores coords to clipboard."})
            fcommon.CheckBox({ address = 0x589355,name = flanguage.GetText('visuals.HealthBarPercentage')})
            fcommon.CheckBox({ address = 0x589125,name = flanguage.GetText('visuals.BodyArmourPercentage')})
            fcommon.CheckBox({ address = 0x589209,name = flanguage.GetText('visuals.BreathMeterPercentage')})
            fcommon.CheckBox({ address = 0x58DD1B,name = flanguage.GetText('visuals.HideWantedLevel'),value = 0x90})

            if imgui.Checkbox(flanguage.GetText('visuals.DisplayZoneNames'),tvisuals.zone_names) then
                displayZoneNames(tvisuals.zone_names[0])
                if tvisuals.zone_names[0] then fcommon.CheatActivated() else fcommon.CheatDeactivated() end
            end

            if imgui.Checkbox(flanguage.GetText('visuals.DisplayCarNames'),tvisuals.car_names) then
                displayCarNames(tvisuals.car_names[0])
                if tvisuals.car_names[0] then fcommon.CheatActivated() else fcommon.CheatDeactivated() end
            end

            imgui.Columns(1)
            imgui.EndTabItem()
        end

        if imgui.BeginTabItem(flanguage.GetText('common.Menus')) then
            if imgui.BeginChild("Menus") then
                imgui.Spacing()
                imgui.Text(flanguage.GetText('visuals.TweakWithCaution'))
                imgui.Spacing()
                fcommon.UpdateAddress({ name = flanguage.GetText('visuals.RadarXPosition'),address = 0x858A10,size = 4,min=-999,max = 999,is_float = true,help_text = "Changes radar vertical position"})
                fcommon.UpdateAddress({ name = flanguage.GetText('visuals.RadarYPosition'),address = 0x866B70,size = 4,min=-999,max = 999,is_float = true,help_text = "Changes radar horizantal position"})
                fcommon.UpdateAddress({ name = flanguage.GetText('visuals.WantedStarBorder'),address = 0x58DD41,size = 1,max = 3})
                fcommon.UpdateAddress({ name = flanguage.GetText('visuals.ActiveWantedStarY'),address = 0x858CCC,size = 4,is_float = true,min=-999,max = 999})
                fcommon.UpdateAddress({ name = flanguage.GetText('visuals.WantedStarColor'),address = 0x58DDC9,size = 1,max = 30})
                fcommon.UpdateAddress({ name = flanguage.GetText('visuals.MoneyColor'),address = 0x58F492 ,size = 1,max = 30})
                fcommon.UpdateAddress({ name = flanguage.GetText('visuals.DebtColor'),address = 0x58F4D4 ,size = 1,max = 30})
                imgui.EndChild()
            end
            imgui.EndTabItem()
        end
        imgui.EndTabBar()
    end
end

return module
