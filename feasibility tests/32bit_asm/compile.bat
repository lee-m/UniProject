nasmw -f win32 output.asm -o output.obj
link /ENTRY:main /SUBSYSTEM:CONSOLE output.obj msvcrt.lib
pause