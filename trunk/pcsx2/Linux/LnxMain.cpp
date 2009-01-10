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

#include <sys/mman.h>
#include <sys/stat.h>

#include "LnxMain.h"

// these namespace directives globalize the new namespaces and make the code behave basically
// like it used to (with cpuReset() and other cpu* vars and functions global).
// Not sure if the Dynarec ones are needed, so uncomment them if you get unresolved external
// link errors.  Should solve some of them (maybe not all...) - (air)

//  Definitely not all. Most of them are coming from assembly files. :(

using namespace R5900;
using namespace Dynarec;

DIR *dir;

#ifdef PCSX2_DEVBUILD
TESTRUNARGS g_TestRun;
#endif

GtkWidget *MsgDlg;

static int sinit=0;

int main(int argc, char *argv[]) {
	char *file = NULL;
	char elfname[g_MaxPath];
	int i = 1;

	efile = 0;
#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, "Langs");
	textdomain(PACKAGE);
#endif

	printf("\n");
	mkdir(CONFIG_DIR, 0755);

	strcpy(cfgfile, CONFIG_DIR "/pcsx2.cfg");

#ifdef PCSX2_DEVBUILD
	memset(&g_TestRun, 0, sizeof(g_TestRun));
#endif

	while(i < argc) {
		char* token = argv[i++];

		if( stricmp(token, "-help") == 0 || stricmp(token, "--help") == 0 || stricmp(token, "-h") == 0 ) {
			//Msgbox::Alert("%s", params phelpmsg);
			return 0;
		}
		else if( stricmp(token, "-efile") == 0 ) {
			token = argv[i++];
			if( token != NULL ) {
				efile = atoi(token);
			}
		}
		else if( stricmp(token, "-nogui") == 0 ) {
			UseGui = FALSE;
		}
		else if( stricmp(token, "-loadgs") == 0 ) {
			g_pRunGSState = argv[i++];
		}
#ifdef PCSX2_DEVBUILD
		else if( stricmp(token, "-image") == 0 ) {
			g_TestRun.pimagename = argv[i++];
		}
		else if( stricmp(token, "-log") == 0 ) {
			g_TestRun.plogname = argv[i++];
		}
		else if( stricmp(token, "-logopt") == 0 ) {
			token = argv[i++];
			if( token != NULL ) {
				if( token[0] == '0' && token[1] == 'x' ) token += 2;
				sscanf(token, "%x", &varLog);
			}
		}
		else if( stricmp(token, "-frame") == 0 ) {
			token = argv[i++];
			if( token != NULL ) {
				g_TestRun.frame = atoi(token);
			}
		}
		else if( stricmp(token, "-numimages") == 0 ) {
			token = argv[i++];
			if( token != NULL ) {
				g_TestRun.numimages = atoi(token);
			}
		}
		else if( stricmp(token, "-jpg") == 0 ) {
			g_TestRun.jpgcapture = 1;
		}
		else if( stricmp(token, "-gs") == 0 ) {
			token = argv[i++];
			g_TestRun.pgsdll = token;
		}
		else if( stricmp(token, "-cdvd") == 0 ) {
			token = argv[i++];
			g_TestRun.pcdvddll = token;
		}
		else if( stricmp(token, "-spu") == 0 ) {
			token = argv[i++];
			g_TestRun.pspudll = token;
		}
		else if( stricmp(token, "-test") == 0 ) {
			g_TestRun.enabled = 1;
		}
#endif
		else if( stricmp(token, "-pad") == 0 ) {
			token = argv[i++];
			printf("-pad ignored\n");
		}
		else if( stricmp(token, "-loadgs") == 0 ) {
			token = argv[i++];
			g_pRunGSState = token;
		}
		else {
			file = token;
			printf("opening file %s\n", file);
		}
	}

#ifdef PCSX2_DEVBUILD
	g_TestRun.efile = efile;
	g_TestRun.ptitle = file;
#endif

	// make gtk thread safe if using MTGS
	if( CHECK_MULTIGS ) {
		g_thread_init(NULL);
		gdk_threads_init();
	}

	if (UseGui) {
		gtk_init(NULL, NULL);
	}
	
	if (LoadConfig() == -1) {

		memset(&Config, 0, sizeof(Config));
		strcpy(Config.BiosDir,    DEFAULT_BIOS_DIR "/");
		strcpy(Config.PluginsDir, DEFAULT_PLUGINS_DIR "/");
		Config.Patch = 1;
		Config.Options = PCSX2_EEREC | PCSX2_VU0REC | PCSX2_VU1REC | PCSX2_COP2REC;
		Config.sseMXCSR = DEFAULT_sseMXCSR;
		Config.sseVUMXCSR = DEFAULT_sseVUMXCSR;

		Msgbox::Alert("Pcsx2 needs to be configured");
		Pcsx2Configure();

		return 0;
	}

	InitLanguages(); 
	
	if( Config.PsxOut ) {
		// output the help commands
		Console::WriteLn("\tF1 - save state");
		Console::WriteLn("\t(Shift +) F2 - cycle states");
		Console::WriteLn("\tF3 - load state");

#ifdef PCSX2_DEVBUILD
		Console::WriteLn("\tF10 - dump performance counters");
		Console::WriteLn("\tF11 - save GS state");
		Console::WriteLn("\tF12 - dump hardware registers");
#endif
	}

	if (!SysInit()) return 1;

#ifdef PCSX2_DEVBUILD
	if( g_pRunGSState ) {
		LoadGSState(g_pRunGSState);
		SysClose();
		return 0;
	}
#endif

	if (UseGui && (file == NULL)) {
		StartGui();
		return 0;
	}

	if (OpenPlugins(file) == -1) return -1;

	SysReset();

	FixCPUState();
	cpuExecuteBios();
	if (file) strcpy(elfname, file);
	if (!efile) efile=GetPS2ElfName(elfname);
	loadElfFile(elfname);

	if( GSsetGameCRC != NULL ) 
		GSsetGameCRC(ElfCRC, g_ZeroGSOptions);

	ExecuteCpu();

	return 0;
}

void InitLanguages() {
	char *lang;
	int i = 1;
	
	if (Config.Lang[0] == 0) {
		strcpy(Config.Lang, "en");
	}

	langs = (_langs*)malloc(sizeof(_langs));
	strcpy(langs[0].lang, "en");
	dir = opendir(LANGS_DIR);
	
	while ((lang = GetLanguageNext()) != NULL) {
		langs = (_langs*)realloc(langs, sizeof(_langs)*(i+1));
		strcpy(langs[i].lang, lang);
		i++;
	}
	
	CloseLanguages();
	langsMax = i;
}

char *GetLanguageNext() {
	struct dirent *ent;

	if (dir == NULL) return NULL;
	for (;;) {
		ent = readdir(dir);
		if (ent == NULL) return NULL;

		if (!strcmp(ent->d_name, ".")) continue;
		if (!strcmp(ent->d_name, "..")) continue;
		break;
	}

	return ent->d_name;
}

void CloseLanguages() {
	if (dir) closedir(dir);
}

void ChangeLanguage(char *lang) {
    strcpy(Config.Lang, lang);
	SaveConfig();
}

/* Quick macros for checking shift, control, alt, and caps lock. */
#define SHIFT_EVT(evt) ((evt == XK_Shift_L) || (evt == XK_Shift_R))
#define CTRL_EVT(evt) ((evt == XK_Control_L) || (evt == XK_Control_L))
#define ALT_EVT(evt) ((evt == XK_Alt_L) || (evt == XK_Alt_R))
#define CAPS_LOCK_EVT(evt) (evt == XK_Caps_Lock)

void KeyEvent(keyEvent* ev) {
	static int shift = 0;

	if (ev == NULL) return;
	
	if( GSkeyEvent != NULL ) GSkeyEvent(ev);

	if (ev->evt == KEYPRESS)
	{
		if (SHIFT_EVT(ev->key)) 
			shift = 1;
		if (CAPS_LOCK_EVT(ev->key))
		{
			//Set up anything we want to happen while caps lock is down.
			//Config_hacks_backup = Config.Hacks;
		}
		
		switch (ev->key) {
			case XK_F1: ProcessFKeys(1, shift); break;
			case XK_F2: ProcessFKeys(2, shift); break;
			case XK_F3: ProcessFKeys(3, shift); break;
			case XK_F4: ProcessFKeys(4, shift); break;
			case XK_F5: ProcessFKeys(5, shift); break;
			case XK_F6: ProcessFKeys(6, shift); break;
			case XK_F7: ProcessFKeys(7, shift); break;
			case XK_F8: ProcessFKeys(8, shift); break;
			case XK_F9: ProcessFKeys(9, shift); break;
			case XK_F10: ProcessFKeys(10, shift); break;
			case XK_F11: ProcessFKeys(11, shift); break;
			case XK_F12: ProcessFKeys(12, shift); break;

			case XK_Escape:
				signal(SIGINT, SIG_DFL);
				signal(SIGPIPE, SIG_DFL);

				#ifdef PCSX2_DEVBUILD
				if( g_SaveGSStream >= 3 ) {
					g_SaveGSStream = 4;// gs state
					break;
				}
				#endif

				ClosePlugins();
				if (!UseGui) exit(0);
				RunGui();
				break;
			default:
				GSkeyEvent(ev);
				break;
		}
	}
	else if (ev->evt == KEYRELEASE)
	{
		if (SHIFT_EVT(ev->key)) 
			shift = 0;
		if (CAPS_LOCK_EVT(ev->key))
		{
			//Release caps lock
			//Config_hacks_backup = Config.Hacks;
		}
	}
	
	return;
}

void OnMsg_Ok() {
	gtk_widget_destroy(MsgDlg);
	gtk_main_quit();
}

void SysMessage(const char *fmt, ...) {
	va_list list;
	char msg[512];

	va_start(list,fmt);
	vsnprintf(msg,511,fmt,list);
	msg[511] = '\0';
	va_end(list);

	Msgbox::Alert(msg);
}

bool SysInit() 
{
	sinit=0;
	
	mkdir(SSTATES_DIR, 0755);
	mkdir(MEMCARDS_DIR, 0755);

	mkdir(LOGS_DIR, 0755);

#ifdef PCSX2_DEVBUILD
	if( g_TestRun.plogname != NULL )
		emuLog = fopen(g_TestRun.plogname, "w");
	if( emuLog == NULL ) 
		emuLog = fopen(LOGS_DIR "/emuLog.txt","wb");
#endif

	if( emuLog != NULL )
		setvbuf(emuLog, NULL, _IONBF, 0);

	SysDetect();

	while (LoadPlugins() == -1) {
		if (Pcsx2Configure() == FALSE)
		{
			Msgbox::Alert("Configuration failed. Exiting.");
			exit(1);
		}
	}

	if( !cpuInit() )
		return false;

	sinit = 1;
	return true;
}

void SysRestorableReset()
{
	// already reset? and saved?
	if( !g_GameInProgress ) return;
	if( g_RecoveryState != NULL ) return;

	try
	{
		g_RecoveryState = new MemoryAlloc<u8>( "Memory Savestate Recovery" );
		memSavingState( *g_RecoveryState ).FreezeAll();
		cpuShutdown();
		g_GameInProgress = false;
	}
	catch( std::runtime_error& ex )
	{
		Msgbox::Alert(
			"Pcsx2 gamestate recovery failed. Some options may have been reverted to protect your game's state.\n"
			"Error: %s", params ex.what() );
		safe_delete( g_RecoveryState );
	}
}

void SysReset()
{
	if (!sinit) return;

	g_GameInProgress = false;
	safe_free( g_RecoveryState );

	ResetPlugins();
}

void SysClose() {
	if (sinit == 0) return;
	cpuShutdown();
	ReleasePlugins();

	if (emuLog != NULL) 
	{
        fclose(emuLog);
        emuLog = NULL;
	}
	sinit=0;
}

void SysPrintf(const char *fmt, ...) {
	va_list list;
	char msg[512];

	va_start(list,fmt);
	vsnprintf(msg,511,fmt,list);
	msg[511] = '\0';
	va_end(list);

	Console::Write( msg );
}

void *SysLoadLibrary(const char *lib) {
	return dlopen(lib, RTLD_NOW);
}

void *SysLoadSym(void *lib, const char *sym) {
	return dlsym(lib, sym);
}

const char *SysLibError() {
	return dlerror();
}

void SysCloseLibrary(void *lib) {
	dlclose(lib);
}

void SysUpdate() {
	KeyEvent(PAD1keyEvent());
	KeyEvent(PAD2keyEvent());
}

void SysRunGui() {
	RunGui();
}

void *SysMmap(uptr base, u32 size) {
	return mmap((uptr*)base, size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
}

void SysMunmap(uptr base, u32 size) {
	munmap((uptr*)base, size);
}
