/*  Pcsx2 - Pc Ps2 Emulator
 *  Copyright (C) 2002-2008  Pcsx2 Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *  
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <windows.h>
#include <windowsx.h>
#include <shlobj.h>
#include <stdio.h>
#include <sys/stat.h>	//2002-09-22 (Florin)
#include "common.h"
#include "plugins.h"
#include "resource.h"
#include "Win32.h"

#define ComboAddPlugin(hw, str) { \
	sprintf(tmpStr, "%s %d.%d.%d", PS2E_GetLibName(), (version>>8)&0xff, version&0xff, (version>>24)&0xff); \
	lp = (char *)malloc(strlen(FindData.cFileName)+8); \
	sprintf(lp, "%s", FindData.cFileName); \
	i = ComboBox_AddString(hw, tmpStr); \
	ComboBox_SetItemData(hw, i, lp); \
	if (_stricmp(str, lp)==0) \
		ComboBox_SetCurSel(hw, i); \
}

BOOL OnConfigureDialog(HWND hW) {
	WIN32_FIND_DATA FindData;
	HANDLE Find;
	HANDLE Lib;
	_PS2EgetLibType     PS2E_GetLibType;
	_PS2EgetLibName     PS2E_GetLibName;
	_PS2EgetLibVersion2 PS2E_GetLibVersion2;
	HWND hWC_GS=GetDlgItem(hW,IDC_LISTGS);
	HWND hWC_PAD1=GetDlgItem(hW,IDC_LISTPAD1);
	HWND hWC_PAD2=GetDlgItem(hW,IDC_LISTPAD2);
	HWND hWC_SPU2=GetDlgItem(hW,IDC_LISTSPU2);
	HWND hWC_CDVD=GetDlgItem(hW,IDC_LISTCDVD);
	HWND hWC_DEV9=GetDlgItem(hW,IDC_LISTDEV9);
	HWND hWC_USB=GetDlgItem(hW,IDC_LISTUSB);
	HWND hWC_FW=GetDlgItem(hW,IDC_LISTFW); 
	HWND hWC_BIOS=GetDlgItem(hW,IDC_LISTBIOS);
	char tmpStr[g_MaxPath];
	char *lp;
	int i;

	strcpy(tmpStr, Config.PluginsDir);
	strcat(tmpStr, "*.dll");
	Find = FindFirstFile(tmpStr, &FindData);

	do {
		if (Find==INVALID_HANDLE_VALUE) break;
		CombinePaths( tmpStr, Config.PluginsDir, FindData.cFileName );
		Lib = LoadLibrary(tmpStr);
		if (Lib == NULL) { SysPrintf("%s: %d\n", tmpStr, GetLastError()); continue; }

		PS2E_GetLibType = (_PS2EgetLibType) GetProcAddress((HMODULE)Lib,"PS2EgetLibType");
		PS2E_GetLibName = (_PS2EgetLibName) GetProcAddress((HMODULE)Lib,"PS2EgetLibName");
		PS2E_GetLibVersion2 = (_PS2EgetLibVersion2) GetProcAddress((HMODULE)Lib,"PS2EgetLibVersion2");

		if (PS2E_GetLibType != NULL && PS2E_GetLibName != NULL && PS2E_GetLibVersion2 != NULL) {
			u32 version;
			long type;

			type = PS2E_GetLibType();
			if (type & PS2E_LT_GS) {
				version = PS2E_GetLibVersion2(PS2E_LT_GS);
				if ( ((version >> 16)&0xff) == PS2E_GS_VERSION) {
					ComboAddPlugin(hWC_GS, winConfig.GS);
				} else SysPrintf("Plugin %s: Version %x != %x\n", FindData.cFileName, 0xff&(version >> 16), PS2E_GS_VERSION);
			}
			if (type & PS2E_LT_PAD) {
				_PADquery query;

				query = (_PADquery)GetProcAddress((HMODULE)Lib, "PADquery");
				version = PS2E_GetLibVersion2(PS2E_LT_PAD);
				if (((version >> 16)&0xff) == PS2E_PAD_VERSION && query) {
					if (query() & 0x1)
						ComboAddPlugin(hWC_PAD1, winConfig.PAD1);
					if (query() & 0x2)
						ComboAddPlugin(hWC_PAD2, winConfig.PAD2);
				} else SysPrintf("Plugin %s: Version %x != %x\n", FindData.cFileName, (version >> 16)&0xff, PS2E_PAD_VERSION);
			}
			if (type & PS2E_LT_SPU2) {
				version = PS2E_GetLibVersion2(PS2E_LT_SPU2);
				if ( ((version >> 16)&0xff) == PS2E_SPU2_VERSION) {
					ComboAddPlugin(hWC_SPU2, winConfig.SPU2);
				} else SysPrintf("Plugin %s: Version %x != %x\n", FindData.cFileName, (version >> 16)&0xff, PS2E_SPU2_VERSION);
			}
			if (type & PS2E_LT_CDVD) {
				version = PS2E_GetLibVersion2(PS2E_LT_CDVD);
				if (((version >> 16)&0xff) == PS2E_CDVD_VERSION) {
					ComboAddPlugin(hWC_CDVD, winConfig.CDVD);
				} else SysPrintf("Plugin %s: Version %x != %x\n", FindData.cFileName, (version >> 16)&0xff, PS2E_CDVD_VERSION);
			}
			if (type & PS2E_LT_DEV9) {
				version = PS2E_GetLibVersion2(PS2E_LT_DEV9);
				if (((version >> 16)&0xff) == PS2E_DEV9_VERSION) {
					ComboAddPlugin(hWC_DEV9, winConfig.DEV9);
				} else SysPrintf("Plugin %s: Version %x != %x\n", FindData.cFileName, (version >> 16)&0xff, PS2E_DEV9_VERSION);
			}
			if (type & PS2E_LT_USB) {
				version = PS2E_GetLibVersion2(PS2E_LT_USB);
				if (((version >> 16)&0xff) == PS2E_USB_VERSION) {
					ComboAddPlugin(hWC_USB, winConfig.USB);
				} else SysPrintf("Plugin %s: Version %x != %x\n", FindData.cFileName, (version >> 16)&0xff, PS2E_USB_VERSION);
			}
			if (type & PS2E_LT_FW) {
				version = PS2E_GetLibVersion2(PS2E_LT_FW);
				if (((version >> 16)&0xff) == PS2E_FW_VERSION) {
					ComboAddPlugin(hWC_FW, winConfig.FW);
				} else SysPrintf("Plugin %s: Version %x != %x\n", FindData.cFileName, (version >> 16)&0xff, PS2E_FW_VERSION);
			}
		}
	} while (FindNextFile(Find,&FindData));

	if (Find!=INVALID_HANDLE_VALUE) FindClose(Find);

// BIOS

	/*lp=(char *)malloc(strlen("HLE") + 1);
	sprintf(lp, "HLE");
	i=ComboBox_AddString(hWC_BIOS, _("Internal HLE Bios"));
	ComboBox_SetItemData(hWC_BIOS, i, lp);
	if (_stricmp(Config.Bios, lp)==0)
		ComboBox_SetCurSel(hWC_BIOS, i);*/

	strcpy(tmpStr, Config.BiosDir);
	strcat(tmpStr, "*");
	Find=FindFirstFile(tmpStr, &FindData);

	do {
		char description[50];								//2002-09-22 (Florin)
		if (Find==INVALID_HANDLE_VALUE) break;
		if (!strcmp(FindData.cFileName, ".")) continue;
		if (!strcmp(FindData.cFileName, "..")) continue;
		if (FindData.nFileSizeLow > 1024 * 4096) continue;	//2002-09-22 (Florin)
		if (!IsBIOS(FindData.cFileName, description)) continue;//2002-09-22 (Florin)
		lp = (char *)malloc(strlen(FindData.cFileName)+8);
		sprintf(lp, "%s", (char *)FindData.cFileName);
		i = ComboBox_AddString(hWC_BIOS, description);		//2002-09-22 (Florin) modified
		ComboBox_SetItemData(hWC_BIOS, i, lp);
		if (_stricmp(Config.Bios, FindData.cFileName)==0)
			ComboBox_SetCurSel(hWC_BIOS, i);
	} while (FindNextFile(Find,&FindData));
    
	if (Find!=INVALID_HANDLE_VALUE) FindClose(Find);

	if (ComboBox_GetCurSel(hWC_GS) == -1)
		ComboBox_SetCurSel(hWC_GS,  0);
	if (ComboBox_GetCurSel(hWC_PAD1) == -1)
		ComboBox_SetCurSel(hWC_PAD1,  0);
	if (ComboBox_GetCurSel(hWC_PAD2) == -1)
		ComboBox_SetCurSel(hWC_PAD2,  0);
	if (ComboBox_GetCurSel(hWC_SPU2) == -1)
		ComboBox_SetCurSel(hWC_SPU2,  0);
	if (ComboBox_GetCurSel(hWC_CDVD) == -1)
		ComboBox_SetCurSel(hWC_CDVD,  0);
	if (ComboBox_GetCurSel(hWC_DEV9) == -1)
		ComboBox_SetCurSel(hWC_DEV9,  0);
	if (ComboBox_GetCurSel(hWC_USB) == -1)
		ComboBox_SetCurSel(hWC_USB,  0);
	if (ComboBox_GetCurSel(hWC_FW) == -1)
		ComboBox_SetCurSel(hWC_FW,  0);
	if (ComboBox_GetCurSel(hWC_BIOS) == -1)
		ComboBox_SetCurSel(hWC_BIOS,  0);

	return TRUE;
}
	
#define CleanCombo(item) \
	hWC = GetDlgItem(hW, item); \
	iCnt = ComboBox_GetCount(hWC); \
	for (i=0; i<iCnt; i++) { \
		lp = (char *)ComboBox_GetItemData(hWC, i); \
		if (lp) free(lp); \
	} \
	ComboBox_ResetContent(hWC);

void CleanUpCombos(HWND hW) {
	int i,iCnt;HWND hWC;char * lp;

	CleanCombo(IDC_LISTGS);
	CleanCombo(IDC_LISTPAD1);
	CleanCombo(IDC_LISTPAD2);
	CleanCombo(IDC_LISTSPU2);
	CleanCombo(IDC_LISTCDVD);
	CleanCombo(IDC_LISTDEV9);
	CleanCombo(IDC_LISTUSB);
	CleanCombo(IDC_LISTFW);
	CleanCombo(IDC_LISTBIOS);
}


void OnCancel(HWND hW) {
	CleanUpCombos(hW);
	EndDialog(hW,FALSE);
}


char *GetComboSel(HWND hW, int id) {
	HWND hWC = GetDlgItem(hW,id);
	int iSel;
	iSel = ComboBox_GetCurSel(hWC);
	if (iSel<0) return NULL;
	return (char *)ComboBox_GetItemData(hWC, iSel);
}

#define CheckComboSel(cfg, idc) { \
	char *str = GetComboSel(hW, idc); \
 \
	if (str != NULL) strcpy(cfg, str); \
}

void OnOK(HWND hW) {
	CheckComboSel(winConfig.Bios, IDC_LISTBIOS);
	CheckComboSel(winConfig.GS,   IDC_LISTGS);
	CheckComboSel(winConfig.PAD1, IDC_LISTPAD1);
	CheckComboSel(winConfig.PAD2, IDC_LISTPAD2);
	CheckComboSel(winConfig.SPU2, IDC_LISTSPU2);
	CheckComboSel(winConfig.CDVD, IDC_LISTCDVD);
	CheckComboSel(winConfig.DEV9, IDC_LISTDEV9);
	CheckComboSel(winConfig.USB, IDC_LISTUSB);
	CheckComboSel(winConfig.FW, IDC_LISTFW);
	CleanUpCombos(hW);

	EndDialog(hW, TRUE);

	// Apply winConfig settings:
	#define ApplyPluginPath( name ) strcpy( Config.name, winConfig.name )
	ApplyPluginPath( Bios );
	ApplyPluginPath( GS );
	ApplyPluginPath( PAD1 );
	ApplyPluginPath( PAD2 );
	ApplyPluginPath( SPU2 );
	ApplyPluginPath( CDVD );
	ApplyPluginPath( DEV9 );
	ApplyPluginPath( USB );
	ApplyPluginPath( FW );

	SaveConfig();
	needReset = 1;
}


static void ConfPlugin( HWND hW, int confs, const char* name )
{
	void *drv;
	void (*conf)();
	char * pDLL = GetComboSel(hW, confs);
	char file[g_MaxPath];

	if(pDLL==NULL) return;
	CombinePaths( file, Config.PluginsDir, pDLL );

	drv = SysLoadLibrary(file);
	if (drv == NULL) return;

	conf = (void (*)()) SysLoadSym(drv, name);
	if (SysLibError() == NULL) conf();
	SysCloseLibrary(drv);
}

void ConfigureGS(HWND hW) {
	ConfPlugin(hW, IDC_LISTGS, "GSconfigure");
}

void ConfigurePAD1(HWND hW) {
	ConfPlugin(hW, IDC_LISTPAD1, "PADconfigure");
}

void ConfigurePAD2(HWND hW) {
	ConfPlugin(hW, IDC_LISTPAD2, "PADconfigure");
}

void ConfigureSPU2(HWND hW) {
	ConfPlugin(hW, IDC_LISTSPU2, "SPU2configure");
}

void ConfigureCDVD(HWND hW) {
	ConfPlugin(hW, IDC_LISTCDVD, "CDVDconfigure");
}

void ConfigureDEV9(HWND hW) {
	ConfPlugin(hW, IDC_LISTDEV9, "DEV9configure");
}

void ConfigureUSB(HWND hW) {
	ConfPlugin(hW, IDC_LISTUSB, "USBconfigure");
}
void ConfigureFW(HWND hW) {
	ConfPlugin(hW, IDC_LISTFW, "FWconfigure");
}

void AboutGS(HWND hW) {
	ConfPlugin(hW, IDC_LISTGS, "GSabout");
}

void AboutPAD1(HWND hW) {
	ConfPlugin(hW, IDC_LISTPAD1, "PADabout");
}

void AboutPAD2(HWND hW) {
	ConfPlugin(hW, IDC_LISTPAD2, "PADabout");
}

void AboutSPU2(HWND hW) {
	ConfPlugin(hW, IDC_LISTSPU2, "SPU2about");
}

void AboutCDVD(HWND hW) {
	ConfPlugin(hW, IDC_LISTCDVD, "CDVDabout");
}

void AboutDEV9(HWND hW) {
	ConfPlugin(hW, IDC_LISTDEV9, "DEV9about");
}

void AboutUSB(HWND hW) {
	ConfPlugin(hW, IDC_LISTUSB, "USBabout");
}
void AboutFW(HWND hW) {
	ConfPlugin(hW, IDC_LISTFW, "FWabout");
}

static void TestPlugin( HWND hW, int confs, const char* name )
{
	void *drv;
	int (*conf)();
	int ret = 0;
	char * pDLL = GetComboSel(hW, confs);
	char file[g_MaxPath];

	if (pDLL== NULL) return;
	CombinePaths( file, Config.PluginsDir, pDLL );

	drv = SysLoadLibrary(file);
	if (drv == NULL) return;

	conf = (int (*)()) SysLoadSym(drv, name);
	if (SysLibError() == NULL) ret = conf();
	SysCloseLibrary(drv);

	if (ret == 0)
		SysMessage(_("This plugin reports that should work correctly"));
	else
		SysMessage(_("This plugin reports that should not work correctly"));
}

void TestGS(HWND hW) {
	TestPlugin(hW, IDC_LISTGS, "GStest");
}

void TestPAD1(HWND hW) {
	TestPlugin(hW, IDC_LISTPAD1, "PADtest");
}

void TestPAD2(HWND hW) {
	TestPlugin(hW, IDC_LISTPAD2, "PADtest");
}

void TestSPU2(HWND hW) {
	TestPlugin(hW, IDC_LISTSPU2, "SPU2test");
}

void TestCDVD(HWND hW) {
	TestPlugin(hW, IDC_LISTCDVD, "CDVDtest");
}

void TestDEV9(HWND hW) {
	TestPlugin(hW, IDC_LISTDEV9, "DEV9test");
}

void TestUSB(HWND hW) {
	TestPlugin(hW, IDC_LISTUSB, "USBtest");
}
void TestFW(HWND hW) {
	TestPlugin(hW, IDC_LISTFW, "FWtest");
}

int SelectPath(HWND hW, char *Title, char *Path) {
	LPITEMIDLIST pidl;
	BROWSEINFO bi;
	char Buffer[g_MaxPath];

	bi.hwndOwner = hW;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = Buffer;
	bi.lpszTitle = Title;
	bi.ulFlags = BIF_RETURNFSANCESTORS | BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	bi.lParam = 0;
	if ((pidl = SHBrowseForFolder(&bi)) != NULL) {
		if (SHGetPathFromIDList(pidl, Path)) {
			int len = strlen(Path);

			if (Path[len - 1] != '\\') { strcat(Path,"\\"); }
			return 0;
		}
	}
	return -1;
}

void SetPluginsDir(HWND hW) {
	char Path[g_MaxPath];

	if (SelectPath(hW, _("Select Plugins Directory"), Path) == -1) return;
	strcpy(Config.PluginsDir, Path);
	CleanUpCombos(hW);
	OnConfigureDialog(hW);
}

void SetBiosDir(HWND hW) {
	char Path[g_MaxPath];

	if (SelectPath(hW, _("Select Bios Directory"), Path) == -1) return;
	strcpy(Config.BiosDir, Path);
	CleanUpCombos(hW);
	OnConfigureDialog(hW);
}

BOOL CALLBACK ConfigureDlgProc(HWND hW, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch(uMsg) {
		case WM_INITDIALOG:
			SetWindowText(hW, _("Configuration"));

			Button_SetText(GetDlgItem(hW, IDOK), _("OK"));
			Button_SetText(GetDlgItem(hW, IDCANCEL), _("Cancel"));
			Static_SetText(GetDlgItem(hW, IDC_GRAPHICS), _("Graphics"));
			Static_SetText(GetDlgItem(hW, IDC_FIRSTCONTROLLER), _("First Controller"));
			Static_SetText(GetDlgItem(hW, IDC_SECONDCONTROLLER), _("Second Controller"));
			Static_SetText(GetDlgItem(hW, IDC_SOUND), _("Sound"));
			Static_SetText(GetDlgItem(hW, IDC_CDVDROM), _("Cdvdrom"));
			Static_SetText(GetDlgItem(hW, IDC_BIOS), _("Bios"));
			Static_SetText(GetDlgItem(hW, IDC_USB), _("Usb"));
			Static_SetText(GetDlgItem(hW, IDC_FW), _("FireWire"));
			Static_SetText(GetDlgItem(hW, IDC_DEV9), _("Dev9"));
			Button_SetText(GetDlgItem(hW, IDC_BIOSDIR), _("Set Bios Directory"));
			Button_SetText(GetDlgItem(hW, IDC_PLUGINSDIR), _("Set Plugins Directory"));
			Button_SetText(GetDlgItem(hW, IDC_CONFIGGS), _("Configure..."));
			Button_SetText(GetDlgItem(hW, IDC_TESTGS), _("Test..."));
			Button_SetText(GetDlgItem(hW, IDC_ABOUTGS), _("About..."));
			Button_SetText(GetDlgItem(hW, IDC_CONFIGSPU2), _("Configure..."));
			Button_SetText(GetDlgItem(hW, IDC_TESTSPU2), _("Test..."));
			Button_SetText(GetDlgItem(hW, IDC_ABOUTSPU2), _("About..."));
			Button_SetText(GetDlgItem(hW, IDC_CONFIGCDVD), _("Configure..."));
			Button_SetText(GetDlgItem(hW, IDC_TESTCDVD), _("Test..."));
			Button_SetText(GetDlgItem(hW, IDC_ABOUTCDVD), _("About..."));
			Button_SetText(GetDlgItem(hW, IDC_CONFIGPAD1), _("Configure..."));
			Button_SetText(GetDlgItem(hW, IDC_TESTPAD1), _("Test..."));
			Button_SetText(GetDlgItem(hW, IDC_ABOUTPAD1), _("About..."));
			Button_SetText(GetDlgItem(hW, IDC_CONFIGPAD2), _("Configure..."));
			Button_SetText(GetDlgItem(hW, IDC_TESTPAD2), _("Test..."));
			Button_SetText(GetDlgItem(hW, IDC_ABOUTPAD2), _("About..."));
			Button_SetText(GetDlgItem(hW, IDC_CONFIGDEV9), _("Configure..."));
			Button_SetText(GetDlgItem(hW, IDC_TESTDEV9), _("Test..."));
			Button_SetText(GetDlgItem(hW, IDC_ABOUTDEV9), _("About..."));
			Button_SetText(GetDlgItem(hW, IDC_CONFIGUSB), _("Configure..."));
			Button_SetText(GetDlgItem(hW, IDC_TESTUSB), _("Test..."));
			Button_SetText(GetDlgItem(hW, IDC_ABOUTUSB), _("About..."));
			Button_SetText(GetDlgItem(hW, IDC_CONFIGFW), _("Configure..."));
			Button_SetText(GetDlgItem(hW, IDC_TESTFW), _("Test..."));
			Button_SetText(GetDlgItem(hW, IDC_ABOUTFW), _("About..."));
			return OnConfigureDialog(hW);

		case WM_COMMAND:
			switch(LOWORD(wParam)) {
				case IDC_CONFIGGS:   ConfigureGS(hW);  return TRUE;
				case IDC_CONFIGPAD1: ConfigurePAD1(hW); return TRUE;
				case IDC_CONFIGPAD2: ConfigurePAD2(hW); return TRUE;
				case IDC_CONFIGSPU2: ConfigureSPU2(hW); return TRUE;
				case IDC_CONFIGCDVD: ConfigureCDVD(hW); return TRUE;
				case IDC_CONFIGDEV9: ConfigureDEV9(hW); return TRUE;
				case IDC_CONFIGUSB:  ConfigureUSB(hW); return TRUE;
				case IDC_CONFIGFW: ConfigureFW(hW); return TRUE;

				case IDC_TESTGS:    TestGS(hW);   return TRUE;
				case IDC_TESTPAD1:  TestPAD1(hW);  return TRUE;
				case IDC_TESTPAD2:  TestPAD2(hW);  return TRUE;
				case IDC_TESTSPU2:  TestSPU2(hW);  return TRUE;
				case IDC_TESTCDVD:  TestCDVD(hW);  return TRUE;
				case IDC_TESTDEV9:  TestDEV9(hW);  return TRUE;
				case IDC_TESTUSB:   TestUSB(hW);  return TRUE;
				case IDC_TESTFW: TestFW(hW); return TRUE;

				case IDC_ABOUTGS:   AboutGS(hW);  return TRUE;
				case IDC_ABOUTPAD1: AboutPAD1(hW); return TRUE;
				case IDC_ABOUTPAD2: AboutPAD2(hW); return TRUE;
				case IDC_ABOUTSPU2: AboutSPU2(hW); return TRUE;
				case IDC_ABOUTCDVD: AboutCDVD(hW); return TRUE;
				case IDC_ABOUTDEV9: AboutDEV9(hW); return TRUE;
				case IDC_ABOUTUSB:  AboutUSB(hW); return TRUE;
				case IDC_ABOUTFW: AboutFW(hW); return TRUE;

				case IDC_PLUGINSDIR: SetPluginsDir(hW); return TRUE;
				case IDC_BIOSDIR:	 SetBiosDir(hW);	return TRUE;

				case IDCANCEL: OnCancel(hW); return TRUE;
				case IDOK:     OnOK(hW);     return TRUE;
			}
	}
	return FALSE;
}


BOOL Pcsx2Configure(HWND hWnd) {
    return DialogBox(gApp.hInstance,
                     MAKEINTRESOURCE(IDD_CONFIG),
                     hWnd,  
                     (DLGPROC)ConfigureDlgProc);
}

