all: build test

build: src/*.c
	gcc src/*.c -o build/lbodc -std=c90

debug: src/*.c
	gcc src/*.c -o build/lbodc -std=c90 -fsanitize=address -g
	gdb build/lbodc

test: all
	build/lbodc build/test/test.lb -o build/test/test.asm -d
	fasm build/test/test.asm build/test/test
	qemu-system-x86_64 build/test/test
test2: all
	build/lbodc build/test/test2.lb -o build/test/test2.asm -d
	fasm build/test/test2.asm build/test/test2
	qemu-system-x86_64 build/test/test2

install: build
	cp build/lbodc /usr/bin/lbodc
install_vim:
	sudo cp -rf "vimfiles/lbodsyn.vim" "$(HOME)/.vim/syntax/lbodsyn.vim"
	sudo cp -rf "vimfiles/lbodftd.vim" "$(HOME)/.vim/ftdetect/lbodfdt.vim"
