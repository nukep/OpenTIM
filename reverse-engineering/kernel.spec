1   stub FatalExit
2   pascal -ret16 ExitKernel() ExitKernel16
3   pascal GetVersion() GetVersion16
4   pascal -ret16 LocalInit(word word word) LocalInit16
5   pascal -ret16 LocalAlloc(word word) LocalAlloc16
6   pascal -ret16 LocalReAlloc(word word word) LocalReAlloc16
7   pascal -ret16 LocalFree(word) LocalFree16
8   pascal LocalLock(word) LocalLock16
9   pascal -ret16 LocalUnlock(word) LocalUnlock16
10  pascal -ret16 LocalSize(word) LocalSize16
11  pascal -ret16 LocalHandle(word) LocalHandle16
12  pascal -ret16 LocalFlags(word) LocalFlags16
13  pascal -ret16 LocalCompact(word) LocalCompact16
14  pascal LocalNotify(long) LocalNotify16
15  pascal -ret16 GlobalAlloc(word long) GlobalAlloc16
16  pascal -ret16 GlobalReAlloc(word long word) GlobalReAlloc16
17  pascal -ret16 GlobalFree(word) GlobalFree16
18  pascal GlobalLock(word) WIN16_GlobalLock16
19  pascal -ret16 GlobalUnlock(word) GlobalUnlock16
20  pascal GlobalSize(word) GlobalSize16
21  pascal GlobalHandle(word) GlobalHandle16
22  pascal -ret16 GlobalFlags(word) GlobalFlags16
23  pascal -ret16 LockSegment(word) LockSegment16
24  pascal -ret16 UnlockSegment(word) UnlockSegment16
25  pascal GlobalCompact(long) GlobalCompact16
26  pascal -ret16 GlobalFreeAll(word) GlobalFreeAll16
27  pascal -ret16 GetModuleName(word ptr word) GetModuleName16 # W1.1: SETSWAPHOOK, W2.0: nothing !
28  pascal   GlobalMasterHandle() GlobalMasterHandle16
29  pascal -ret16 Yield() Yield16
30  pascal -ret16 WaitEvent(word) WaitEvent16
31  pascal -ret16 PostEvent(word) PostEvent16
32  pascal -ret16 SetPriority(word s_word) SetPriority16
33  pascal -ret16 LockCurrentTask(word) LockCurrentTask16
34  pascal -ret16 SetTaskQueue(word word) SetTaskQueue16
35  pascal -ret16 GetTaskQueue(word) GetTaskQueue16
36  pascal   GetCurrentTask() WIN16_GetCurrentTask
37  pascal GetCurrentPDB() GetCurrentPDB16
38  pascal   SetTaskSignalProc(word segptr) SetTaskSignalProc
39  stub     SetTaskSwitchProc      # W1.1, W2.0
40  stub     SetTaskInterchange     # W1.1, W2.0
41  pascal -ret16 EnableDos() KERNEL_nop
42  pascal -ret16 DisableDos() KERNEL_nop
43  stub     IsScreenGrab           # W1.1, W2.0
44  stub     BuildPDB               # W1.1, W2.0
45  pascal -ret16 LoadModule(str ptr) LoadModule16
46  pascal -ret16 FreeModule(word) FreeModule16
47  pascal   GetModuleHandle(segstr) WIN16_GetModuleHandle
48  pascal -ret16 GetModuleUsage(word) GetModuleUsage16
49  pascal -ret16 GetModuleFileName(word ptr s_word) GetModuleFileName16
50  pascal GetProcAddress(word str) GetProcAddress16
51  pascal MakeProcInstance(segptr word) MakeProcInstance16
52  pascal -ret16 FreeProcInstance(segptr) FreeProcInstance16
53  stub CallProcInstance
54  pascal -ret16 GetInstanceData(word word word) GetInstanceData16
55  pascal -register Catch(ptr) Catch16
56  pascal -register Throw(ptr word) Throw16
57  pascal -ret16 GetProfileInt(str str s_word) GetProfileInt16
58  pascal -ret16 GetProfileString(str str str ptr word) GetProfileString16
59  pascal -ret16 WriteProfileString(str str str) WriteProfileString16
60  pascal -ret16 FindResource(word str str) FindResource16
61  pascal -ret16 LoadResource(word word) LoadResource16
62  pascal LockResource(word) WIN16_LockResource16
63  pascal -ret16 FreeResource(word) FreeResource16
64  pascal -ret16 AccessResource(word word) AccessResource16
65  pascal SizeofResource(word word) SizeofResource16
66  pascal -ret16 AllocResource(word word long) AllocResource16
67  pascal SetResourceHandler(word str segptr) SetResourceHandler16
68  pascal -ret16 InitAtomTable(word) InitAtomTable16
69  pascal -ret16 FindAtom(str) FindAtom16
70  pascal -ret16 AddAtom(str) AddAtom16
71  pascal -ret16 DeleteAtom(word) DeleteAtom16
72  pascal -ret16 GetAtomName(word ptr word) GetAtomName16
73  pascal -ret16 GetAtomHandle(word) GetAtomHandle16
74  pascal -ret16 OpenFile(str ptr word) OpenFile16
75  stub OpenPathName
76  stub DeletePathName
77  pascal Reserved1(segptr) KERNEL_AnsiNext16
78  pascal Reserved2(segptr segptr) KERNEL_AnsiPrev16
79  pascal Reserved3(segstr) KERNEL_AnsiUpper16
80  pascal Reserved4(segstr) KERNEL_AnsiLower16
81  pascal -ret16 _lclose(word) _lclose16
82  pascal -ret16 _lread(word segptr word) WIN16_lread
83  pascal -ret16 _lcreat(str word) _lcreat16
84  pascal   _llseek(word long word) _llseek16
85  pascal -ret16 _lopen(str word) _lopen16
86  pascal -ret16 _lwrite(word ptr word) _lwrite16
87  pascal -ret16 Reserved5(str str) KERNEL_lstrcmp16
88  pascal   lstrcpy(segptr str) lstrcpy16
89  pascal   lstrcat(segstr str) lstrcat16
90  pascal -ret16 lstrlen(str) lstrlen16
91  pascal -register InitTask() InitTask16
92  pascal   GetTempDrive(word) GetTempDrive
93  pascal GetCodeHandle(segptr) GetCodeHandle16
94  pascal -ret16 DefineHandleTable(word) DefineHandleTable16
95  pascal -ret16 LoadLibrary(str) LoadLibrary16
96  pascal -ret16 FreeLibrary(word) FreeLibrary16
97  pascal -ret16 GetTempFileName(word str word ptr) GetTempFileName16
98  pascal -ret16 GetLastDiskChange() KERNEL_nop
99  stub GetLPErrMode
100 pascal -ret16 ValidateCodeSegments() KERNEL_nop
101 stub NoHookDosCall
102 pascal -register DOS3Call() DOS3Call
103 pascal -register NetBIOSCall() NetBIOSCall16
104 pascal -ret16 GetCodeInfo(segptr ptr) GetCodeInfo16
105 pascal -ret16 GetExeVersion() GetExeVersion16
106 pascal SetSwapAreaSize(word) SetSwapAreaSize16
107 pascal -ret16 SetErrorMode(word) SetErrorMode16
108 pascal -ret16 SwitchStackTo(word word word) SwitchStackTo16 # STO in W2.0
109 pascal -register SwitchStackBack() SwitchStackBack16 # SBACK in W2.0
110 pascal   PatchCodeHandle(word) PatchCodeHandle16
111 pascal   GlobalWire(word) GlobalWire16
112 pascal -ret16 GlobalUnWire(word) GlobalUnWire16
113 equate __AHSHIFT 3
114 equate __AHINCR 8
115 pascal -ret16 OutputDebugString(str) OutputDebugString16
116 stub InitLib
117 pascal -ret16 OldYield() OldYield16
118 pascal -ret16 GetTaskQueueDS() GetTaskQueueDS16
119 pascal -ret16 GetTaskQueueES() GetTaskQueueES16
120 stub UndefDynLink
121 pascal -ret16 LocalShrink(word word) LocalShrink16
122 pascal -ret16 IsTaskLocked() IsTaskLocked16
123 pascal -ret16 KbdRst() KERNEL_nop
124 pascal -ret16 EnableKernel() KERNEL_nop
125 pascal -ret16 DisableKernel() KERNEL_nop
126 stub MemoryFreed
127 pascal -ret16 GetPrivateProfileInt(str str s_word str) GetPrivateProfileInt16
128 pascal -ret16 GetPrivateProfileString(str str str ptr word str) GetPrivateProfileString16
129 pascal -ret16 WritePrivateProfileString(str str str str) WritePrivateProfileString16
130 pascal FileCDR(ptr) FileCDR16
131 pascal GetDOSEnvironment() GetDOSEnvironment16
132 pascal GetWinFlags() GetWinFlags16
133 pascal -ret16 GetExePtr(word) WIN16_GetExePtr
134 pascal -ret16 GetWindowsDirectory(ptr word) GetWindowsDirectory16
135 pascal -ret16 GetSystemDirectory(ptr word) GetSystemDirectory16
136 pascal -ret16 GetDriveType(word) GetDriveType16
137 pascal -ret16 FatalAppExit(word str) FatalAppExit16
138 pascal GetHeapSpaces(word) GetHeapSpaces16
139 stub DoSignal
140 pascal -ret16 SetSigHandler(segptr ptr ptr word word) SetSigHandler16
141 stub InitTask1
142 pascal -ret16 GetProfileSectionNames(ptr word) GetProfileSectionNames16
143 pascal -ret16 GetPrivateProfileSectionNames(ptr word str) GetPrivateProfileSectionNames16
144 pascal -ret16 CreateDirectory(ptr ptr) CreateDirectory16
145 pascal -ret16 RemoveDirectory(ptr) RemoveDirectory16
146 pascal -ret16 DeleteFile(ptr) DeleteFile16
147 pascal -ret16 SetLastError(long) SetLastError16
148 pascal GetLastError() GetLastError16
149 pascal -ret16 GetVersionEx(ptr) GetVersionEx16
150 pascal -ret16 DirectedYield(word) DirectedYield16
151 stub WinOldApCall
152 pascal -ret16 GetNumTasks() GetNumTasks16
154 pascal -ret16 GlobalNotify(segptr) GlobalNotify16
155 pascal -ret16 GetTaskDS() GetTaskDS16
156 pascal   LimitEMSPages(long) LimitEMSPages16
157 pascal   GetCurPID(long) GetCurPID16
158 pascal -ret16 IsWinOldApTask(word) IsWinOldApTask16
159 pascal GlobalHandleNoRIP(word) GlobalHandleNoRIP16
160 stub EMSCopy
161 pascal -ret16 LocalCountFree() LocalCountFree16
162 pascal -ret16 LocalHeapSize() LocalHeapSize16
163 pascal -ret16 GlobalLRUOldest(word) GlobalLRUOldest16
164 pascal -ret16 GlobalLRUNewest(word) GlobalLRUNewest16
165 pascal -ret16 A20Proc(word) A20Proc16
166 pascal -ret16 WinExec(str word) WinExec16
167 pascal -ret16 GetExpWinVer(word) GetExpWinVer16
168 pascal -ret16 DirectResAlloc(word word word) DirectResAlloc16
169 pascal GetFreeSpace(word) GetFreeSpace16
170 pascal -ret16 AllocCStoDSAlias(word) AllocCStoDSAlias16
171 pascal -ret16 AllocDStoCSAlias(word) AllocDStoCSAlias16
172 pascal -ret16 AllocAlias(word) AllocCStoDSAlias16
173 equate __ROMBIOS 0
174 equate __A000H 0
175 pascal -ret16 AllocSelector(word) AllocSelector16
176 pascal -ret16 FreeSelector(word) FreeSelector16
177 pascal -ret16 PrestoChangoSelector(word word) PrestoChangoSelector16
178 equate __WINFLAGS 0x413
179 equate __D000H 0
180 pascal -ret16 LongPtrAdd(long long) LongPtrAdd16
181 equate __B000H 0
182 equate __B800H 0
183 equate __0000H 0
184 pascal GlobalDOSAlloc(long) GlobalDOSAlloc16
185 pascal -ret16 GlobalDOSFree(word) GlobalDOSFree16
186 pascal GetSelectorBase(word) GetSelectorBase
187 pascal -ret16 SetSelectorBase(word long) SetSelectorBase
188 pascal GetSelectorLimit(word) GetSelectorLimit16
189 pascal -ret16 SetSelectorLimit(word long) SetSelectorLimit16
190 equate __E000H 0
191 pascal -ret16 GlobalPageLock(word) GlobalPageLock16
192 pascal -ret16 GlobalPageUnlock(word) GlobalPageUnlock16
193 equate __0040H 0
194 equate __F000H 0
195 equate __C000H 0
196 pascal -ret16 SelectorAccessRights(word word word) SelectorAccessRights16
197 pascal -ret16 GlobalFix(word) GlobalFix16
198 pascal -ret16 GlobalUnfix(word) GlobalUnfix16
199 pascal -ret16 SetHandleCount(word) SetHandleCount16
200 pascal -ret16 ValidateFreeSpaces() KERNEL_nop
201 stub ReplaceInst
202 stub RegisterPtrace
203 pascal -register DebugBreak() DebugBreak16
204 stub SwapRecording
205 stub CVWBreak
206 pascal -ret16 AllocSelectorArray(word) AllocSelectorArray16
207 pascal -ret16 IsDBCSLeadByte(word) IsDBCSLeadByte
310 pascal -ret16 LocalHandleDelta(word) LocalHandleDelta16
311 pascal GetSetKernelDOSProc(ptr) GetSetKernelDOSProc16
314 stub DebugDefineSegment
315 pascal -ret16 WriteOutProfiles() WriteOutProfiles16
316 pascal GetFreeMemInfo() GetFreeMemInfo16
318 stub FatalExitHook
319 stub FlushCachedFileHandle
320 pascal -ret16 IsTask(word) IsTask16
323 pascal -ret16 IsRomModule(word) IsRomModule16
324 pascal -ret16 LogError(word ptr) LogError16
325 pascal -ret16 LogParamError(word ptr ptr) LogParamError16
326 pascal -ret16 IsRomFile(word) IsRomFile16
327 pascal -register K327() HandleParamError
328 varargs -ret16 _DebugOutput(word str) _DebugOutput
329 pascal -ret16 K329(str word) DebugFillBuffer
332 variable THHOOK(0 0 0 0 0 0 0 0)
334 pascal -ret16 IsBadReadPtr(segptr word) IsBadReadPtr16
335 pascal -ret16 IsBadWritePtr(segptr word) IsBadWritePtr16
336 pascal -ret16 IsBadCodePtr(segptr) IsBadCodePtr16
337 pascal -ret16 IsBadStringPtr(segptr word) IsBadStringPtr16
338 pascal -ret16 HasGPHandler(segptr) HasGPHandler16
339 pascal -ret16 DiagQuery() DiagQuery16
340 pascal -ret16 DiagOutput(str) DiagOutput16
341 pascal ToolHelpHook(ptr) ToolHelpHook16
342 variable __GP(0 0)
343 stub RegisterWinOldApHook
344 stub GetWinOldApHooks
345 pascal -ret16 IsSharedSelector(word) IsSharedSelector16
346 pascal -ret16 IsBadHugeReadPtr(segptr long) IsBadHugeReadPtr16
347 pascal -ret16 IsBadHugeWritePtr(segptr long) IsBadHugeWritePtr16
348 pascal -ret16 hmemcpy(ptr ptr long) hmemcpy16
349 pascal   _hread(word segptr long) WIN16_hread
350 pascal   _hwrite(word ptr long) _hwrite16
351 pascal -ret16 BUNNY_351() KERNEL_nop
352 pascal   lstrcatn(segstr str word) lstrcatn16
353 pascal   lstrcpyn(segptr str word) lstrcpyn16
354 pascal   GetAppCompatFlags(word) GetAppCompatFlags16
355 pascal -ret16 GetWinDebugInfo(ptr word) GetWinDebugInfo16
356 pascal -ret16 SetWinDebugInfo(ptr) SetWinDebugInfo16
403 pascal -ret16 FarSetOwner(word word) FarSetOwner16 # aka K403
404 pascal -ret16 FarGetOwner(word) FarGetOwner16 # aka K404