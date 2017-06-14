tools\nasmw .\prog.asm -f win32 -o prog.obj
tools\link /entry:main /subsystem:console prog.obj .\tools\libs\stdlib.lib
pause 