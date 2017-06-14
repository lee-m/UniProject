[section .data]
	str_ref_1	db "",0
	str_ref_2	db "Array contents after sorting:",0
	str_ref_0	db "Array contents before sorting:",0

[section .bss]
	array resd 40

[section .text]

global _main

extern _print_integer
extern _print_string

_quicksort:
	push	ebp
	mov	ebp,esp
	sub	esp,4
	mov	eax,dword [ebp + 8]
	sub	eax,dword [ebp + 12]
	cmp	eax,0
	jl 	L0
	jmp	L1
L0:
	push	eax
	mov	eax,dword [ebp + 8]
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	ecx
	push	edx
	mov	ecx,[esp+8]
	mov	edx,[ecx]
	mov	dword [ebp - 4],edx
	pop	edx
	pop	ecx
	add	esp, 4
	pop	eax
	mov	eax,dword [ebp + 12]
	add	eax,dword [ebp + 8]
	mov	eax,eax
	push	ebx
	mov	ebx,2
	push	edx
	xor	edx,edx
	idiv	ebx
	pop	edx
	pop	ebx
	mov	eax,eax
	push	eax
	mov	eax,dword [ebp + 8]
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,eax
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	ecx
	push	edx
	mov	ecx,[esp+8]
	mov	edx,[ecx]
	mov	ecx,[esp+16]
	mov	[ecx],edx
	pop	edx
	pop	ecx
	add	esp,12
	pop	eax
	mov	eax,dword [ebp + 12]
	add	eax,dword [ebp + 8]
	mov	eax,eax
	push	ebx
	mov	ebx,2
	push	edx
	xor	edx,edx
	idiv	ebx
	pop	edx
	pop	ebx
	push	eax
	mov	eax,eax
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	push	dword [ebp - 4]
	pop	dword [eax]
	pop	eax
	add	esp, 4
	pop	eax
	push	eax
	mov	eax,dword [ebp + 8]
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	ecx
	mov	ecx,[esp+4]
	mov	edx,[ecx]
	mov	edx,edx
	pop	ecx
	add	esp, 4
	pop	eax
	mov	ecx,dword [ebp + 8]
	mov	eax,dword [ebp + 8]
	add	eax,1
	mov	ebx,eax
L2:
	mov	eax,ebx
	sub	eax,dword [ebp + 12]
	cmp	eax,0
	jle 	L3
	jmp	L4
L3:
	mov	eax,ebx
	imul	eax,4
	add	eax,array
nop
	push	eax
	push	ecx
	push	edx
	mov	ecx,[esp+8]
	mov	edx,[ecx]
	mov	eax,edx
	pop	edx
	pop	ecx
	add	esp, 4
	mov	eax,eax
	sub	eax,edx
	cmp	eax,0
	jl 	L5
	jmp	L6
L5:
	mov	eax,ecx
	add	eax,1
	mov	ecx,eax
	push	eax
	mov	eax,ecx
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	ecx
	push	edx
	mov	ecx,[esp+8]
	mov	edx,[ecx]
	mov	dword [ebp - 4],edx
	pop	edx
	pop	ecx
	add	esp, 4
	pop	eax
	push	eax
	mov	eax,ecx
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,ebx
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	ecx
	push	edx
	mov	ecx,[esp+8]
	mov	edx,[ecx]
	mov	ecx,[esp+16]
	mov	[ecx],edx
	pop	edx
	pop	ecx
	add	esp,12
	pop	eax
	push	eax
	mov	eax,ebx
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	push	dword [ebp - 4]
	pop	dword [eax]
	pop	eax
	add	esp, 4
	pop	eax
L6:
	mov	eax,ebx
	add	eax,1
	mov	ebx,eax
	jmp	L2
L4:
	push	eax
	mov	eax,dword [ebp + 8]
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	ecx
	push	edx
	mov	ecx,[esp+8]
	mov	edx,[ecx]
	mov	dword [ebp - 4],edx
	pop	edx
	pop	ecx
	add	esp, 4
	pop	eax
	push	eax
	mov	eax,dword [ebp + 8]
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,ecx
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	ecx
	push	edx
	mov	ecx,[esp+8]
	mov	edx,[ecx]
	mov	ecx,[esp+16]
	mov	[ecx],edx
	pop	edx
	pop	ecx
	add	esp,12
	pop	eax
	push	eax
	mov	eax,ecx
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	push	dword [ebp - 4]
	pop	dword [eax]
	pop	eax
	add	esp, 4
	pop	eax
	mov	eax,ecx
	sub	eax,1
	pushad
	push	eax
	push	dword [ebp + 8]
	call	_quicksort
	add	esp,8
	popad
	mov	ebx,ecx
	add	ebx,1
	pushad
	push	dword [ebp + 12]
	push	ebx
	call	_quicksort
	add	esp,8
	popad
L1:
	mov	esp,ebp
	pop	ebp
	ret

_initialise_array:
	push	ebp
	mov	ebp,esp
	push	eax
	mov	eax,0
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	mov	dword [eax],9
	pop	eax
	add	esp, 4
	pop	eax
	push	eax
	mov	eax,1
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	mov	dword [eax],2
	pop	eax
	add	esp, 4
	pop	eax
	push	eax
	mov	eax,2
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	mov	dword [eax],4
	pop	eax
	add	esp, 4
	pop	eax
	push	eax
	mov	eax,3
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	mov	dword [eax],7
	pop	eax
	add	esp, 4
	pop	eax
	push	eax
	mov	eax,4
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	mov	dword [eax],10
	pop	eax
	add	esp, 4
	pop	eax
	push	eax
	mov	eax,5
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	mov	dword [eax],1
	pop	eax
	add	esp, 4
	pop	eax
	push	eax
	mov	eax,6
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	mov	dword [eax],5
	pop	eax
	add	esp, 4
	pop	eax
	push	eax
	mov	eax,7
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	mov	dword [eax],8
	pop	eax
	add	esp, 4
	pop	eax
	push	eax
	mov	eax,8
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	mov	dword [eax],3
	pop	eax
	add	esp, 4
	pop	eax
	push	eax
	mov	eax,9
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	eax
	mov	eax,dword [esp + 4]
	mov	dword [eax],6
	pop	eax
	add	esp, 4
	pop	eax
	mov	esp,ebp
	pop	ebp
	ret

_print_array:
	push	ebp
	mov	ebp,esp
	mov	eax,0
L7:
	mov	ebx,eax
	sub	ebx,10
	cmp	ebx,0
	jl 	L8
	jmp	L9
L8:
	push	eax
	mov	eax,eax
	imul	eax,4
	add	eax,array
nop
	push	eax
	mov	eax,[esp + 4]
	push	edx
	mov	ecx,[esp+4]
	mov	edx,[ecx]
	mov	ecx,edx
	pop	edx
	add	esp, 4
	pop	eax
	pushad
	push	ecx
	call	_print_integer
	add	esp,4
	popad
	mov	edx,eax
	add	edx,1
	mov	eax,edx
	jmp	L7
L9:
	mov	esp,ebp
	pop	ebp
	ret

_main:
	push	ebp
	mov	ebp,esp
	pushad
	call	_initialise_array
	add	esp,0
	popad
	pushad
	push	dword str_ref_0
	call	_print_string
	add	esp,4
	popad
	pushad
	call	_print_array
	add	esp,0
	popad
	pushad
	push	dword str_ref_1
	call	_print_string
	add	esp,4
	popad
	pushad
	push	9
	push	0
	call	_quicksort
	add	esp,8
	popad
	pushad
	push	dword str_ref_2
	call	_print_string
	add	esp,4
	popad
	pushad
	call	_print_array
	add	esp,0
	popad
	mov	esp,ebp
	pop	ebp
	ret
