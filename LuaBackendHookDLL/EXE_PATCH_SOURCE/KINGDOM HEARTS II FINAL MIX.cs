// LOCATION: EXE -> AxaFormBase -> BaseSimpleForm
// STARTING: N/A

using System.IO;
using System.Threading;
using System.Diagnostics;
using System.Threading.Tasks;

[DllImport("LuaBackendHook.dll", CallingConvention = CallingConvention.Cdecl)]
public static extern void HookGame(ulong ModuleAddress);

[DllImport("LuaBackendHook.dll", CallingConvention = CallingConvention.Cdecl)]
public static extern int EntryLUA(int InputID, IntPtr InputHandle, ulong InputAddress, string InputPath);

[DllImport("LuaBackendHook.dll", CallingConvention = CallingConvention.Cdecl)]
public static extern int ExecuteLUA();

[DllImport("LuaBackendHook.dll", CallingConvention = CallingConvention.Cdecl)]
public static extern bool CheckLUA();

[DllImport("LuaBackendHook.dll", CallingConvention = CallingConvention.Cdecl)]
public static extern int VersionLUA();

// LOCATION: EXE -> AxaFormBase -> BaseSimpleForm.createInstance()
// STARTING: Line #09

string _scrFolder = Environment.GetFolderPath(Environment.SpecialFolder.Personal) + "\\KINGDOM HEARTS HD 1.5+2.5 ReMIX\\scripts";

if (File.Exists("LuaBackendHook.dll"))
{
    if (!Directory.Exists(_scrFolder))
        MessageBox.Show("The \"" + _scrFolder + "\" folder was not found.\nLuaEngine will not operate.", 
                        "KINGDOM HEARTS II FINAL MIX", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);

    else
    {
        if (VersionLUA() > 128)
            MessageBox.Show("The LuaEngine DLL is too new for this executable.\nLuaEngine will not operate until this EXE is updated.", 
                            "KINGDOM HEARTS II FINAL MIX", MessageBoxButtons.OK, MessageBoxIcon.Error);

        else if (VersionLUA() < 128)
            MessageBox.Show("The LuaEngine DLL is too old for this executable.\nLuaEngine will not operate until the DLL is updated.", 
                "KINGDOM HEARTS II FINAL MIX", MessageBoxButtons.OK, MessageBoxIcon.Error);

        else
        {
            long _moduleAddress = Process.GetCurrentProcess().MainModule.BaseAddress.ToInt64();
            long _addr = _moduleAddress + 0x56450E;
            
            if (EntryLUA(Process.GetCurrentProcess().Id, Process.GetCurrentProcess().Handle, (ulong)_addr, _scrFolder) == 0)
            {
                HookGame((ulong)_moduleAddress);
            }
        }
    }
}
