all: clean main.exe

main.exe: main.obj

main.obj: main.c

clean:
	del *.exe *.obj salida.txt
