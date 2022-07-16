#include "3ds.h"
#include "csvc.h"
#include "CTRPluginFramework.hpp"

#include <vector>

namespace CTRPluginFramework
{
    //HotKey
    static MenuEntry* EntryWithHotkey(MenuEntry* entry, const Hotkey& hotkey)
    {
        if (entry != nullptr)
        {
            entry->Hotkeys += hotkey;
            entry->SetArg(new std::string(entry->Name()));
            entry->Hotkeys.OnHotkeyChangeCallback([](MenuEntry* entry, int index)
            {
                std::string* name = reinterpret_cast<std::string*>(entry->GetArg());
            });
        }
        return (entry);
    }

    static MenuEntry* EntryWithHotkey(MenuEntry* entry,const std::vector<Hotkey>& hotkeys)
    {
        if (entry != nullptr)
        {
            for (const Hotkey& hotkey : hotkeys) entry->Hotkeys += hotkey;
        }

        return (entry);
    }

    static MenuEntry* EnableEntry(MenuEntry* entry)
    {
        if (entry != nullptr)
        {
            entry->SetArg(new std::string(entry->Name()));
            entry->Enable();
        }
        return (entry);
    }
    
    // This patch the NFC disabling the touchscreen when scanning an amiibo, which prevents ctrpf to be used
    static void    ToggleTouchscreenForceOn(void)
    {
        static u32 original = 0;
        static u32 *patchAddress = nullptr;

        if (patchAddress && original)
        {
            *patchAddress = original;
            return;
        }

        static const std::vector<u32> pattern = { 0xE59F10C0, 0xE5840004, 0xE5841000, 0xE5DD0000, 0xE5C40008, 0xE28DD03C, 0xE8BD80F0, 0xE5D51001, 0xE1D400D4, 0xE3510003, 0x159F0034, 0x1A000003 };

        Result  res;
        Handle  processHandle;
        s64     textTotalSize = 0;
        s64     startAddress = 0;
        u32 *   found;

        if (R_FAILED(svcOpenProcess(&processHandle, 16)))
            return;

        svcGetProcessInfo(&textTotalSize, processHandle, 0x10002);
        svcGetProcessInfo(&startAddress, processHandle, 0x10005);
        if(R_FAILED(svcMapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, processHandle, (u32)startAddress, textTotalSize)))
            goto exit;

        found = (u32 *)Utils::Search<u32>(0x14000000, (u32)textTotalSize, pattern);

        if (found != nullptr)
        {
            original = found[13];
            patchAddress = (u32 *)PA_FROM_VA((found + 13));
            found[13] = 0xE1A00000;
        }

        svcUnmapProcessMemoryEx(CUR_PROCESS_HANDLE, 0x14000000, textTotalSize);
        exit:
        svcCloseHandle(processHandle);
    }
    
    std::string SystemInfomation()
    {
        char date[6400];
        std::time_t unix_time = std::time(nullptr);
        struct tm *time_struct = gmtime((const time_t *)&unix_time);
        std::string am_or_pm;
        const std::vector<std::string> kListTimeOfTheWeek{"Sun", "Mon", "Tues", "Wed", "Thurs", "Fri", "Satur"};
        int time_of_the_week = time_struct->tm_wday;
        const int kStartYear = 1900;
        const int kStartMonth = 1;
        int year = time_struct->tm_year + kStartYear;
        int month = time_struct->tm_mon + kStartMonth;
        int day = time_struct->tm_mday;
        int hour_24 = time_struct->tm_hour;
        int hour_12;
        int minute = time_struct->tm_min;
        int second = time_struct->tm_sec;
        if (hour_24 / 12)
        {
            am_or_pm = "Pm";
        }
        else
        {
            am_or_pm = "Am";
        }
        if (hour_24 % 12)
        {
            hour_12 = hour_24 % 12;
        }
        else
        {
            hour_12 = 12;
        }

        std::string systeminfo = Utils::Format("%d/%02d/%02d(%s) %s:%02d:%02d:%02d", year, month, day, kListTimeOfTheWeek[time_of_the_week].c_str(), am_or_pm.c_str(), hour_12, minute, second);
        return systeminfo;
    }
    
    void DrawCallBack(Time time)
    {
        const Screen& scr = OSD::GetTopScreen();
        scr.DrawRect(30, 4, 340, 18, Color::Black, true);
        scr.DrawRect(32, 6, 336, 14, Color::White, false);
        scr.DrawSysfont(SystemInfomation(), 35, 5, Color::White);
    }
    
    static const std::string About = Color(255,255,255) << "Create library: Nanquitas\nCreate nomu-3:";

    // This function is called before main and before the game starts
    // Useful to do code edits safely
    void    PatchProcess(FwkSettings &settings)
    {
        ToggleTouchscreenForceOn();
    }

    // This function is called when the process exits
    // Useful to save settings, undo patchs or clean up things
    void    OnProcessExit(void)
    {
        ToggleTouchscreenForceOn();
    }
    
    void menu_1(PluginMenu &menu)
    {
    }
    
    void menu_2(PluginMenu &menu)
    {
    }
    
    void menu_3(PluginMenu &menu)
    {
    }
    
    void ChoiceMenu(PluginMenu &menu)
    {
        u64 title_id =Process::GetTitleID();
        switch (title_id)
        {
            case 0x004000000000000:
            menu_1(menu);
            break;
            case 0x004000000000000:
            menu_2(menu);
            break;
            case 0x004000000000000:
            menu_3(menu);
            break;
            default:
            break;
        }
    }

    int main(void)
    {
        auto* menu = new PluginMenu(Color::White << "ChoiceCTRPF", 0,5,1, About);

        // Synnchronize the menu with frame event
        menu->SynchronizeWithFrame(true);
        
        // Add NewFrame
        menu->OnNewFrame = DrawCallBack;

        // Init our menu entries & folders
        ChoiceMenu(*menu);

        // Launch menu and mainloop
        menu->Run();

        delete menu;

        // Exit plugin
        return (0);
    }
}
