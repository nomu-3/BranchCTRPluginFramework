#include "3ds.h"
#include "csvc.h"
#include "CTRPluginFramework.hpp"
#include "Cheats.hpp"
#include <vector>
#include <ctime>

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
            case 0x004000000000000:    //titleID1
            menu_1(menu);
            break;
            case 0x004000000000000:    //titleID2
            menu_2(menu);
            break;
            case 0x004000000000000:    //titleID3
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

        // Init our menu entries & folders
        ChoiceMenu(*menu);

        // Launch menu and mainloop
        menu->Run();

        delete menu;

        // Exit plugin
        return (0);
    }
}
