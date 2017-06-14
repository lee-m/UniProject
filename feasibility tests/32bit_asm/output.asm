[section .data]
	
	instruction_msg db "Please enter a string and press enter: ", 0
	entered_msg db "You entered: ", 0
	blank_line db 0, 13, 10
	
[section .bss]

	in_string resb 64
	
[section .text]

extern _puts
extern _gets

global _main

_main:
	
	;output instructions
	push dword instruction_msg
	call _puts
	add esp, 4
	
	;get the text from stdin and stick the address in in_string
	push dword in_string
	call _gets
	add esp, 4
	
	;print a blank line
	push dword blank_line
	call _puts
	add esp, 4
	
	;print our message
	push dword entered_msg
	call _puts
	add esp, 4
	
	;print the message we got from the user and return
	push dword in_string
	call _puts
	add esp, 4 
	ret