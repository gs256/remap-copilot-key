main:
	gcc main.c -O3 -static -mwindows -municode -s -o out.exe
	cp out.exe remap-copilot-to-ctrl.exe
	cp out.exe remap-copilot-to-menu.exe
	rm out.exe
