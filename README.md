# Intel 8080
This repo contains my implementaion of the [I8080](https://en.wikipedia.org/wiki/Intel_8080) CPU
for the purposes of testing i added a makefile and a main.c file

## Testing
i used the amazing [cpudiag.asm](http://www.emulator101.com/full-8080-emulation.html) to find most of the bugs in the code
and [TST8080.com](https://altairclone.com/downloads/cpu_tests/) was pretty helpful for some stuff i didnt notice with cpudiag
![cpudiag output](cpudiag.png)


## Note
i implemented this cpu for the sole purpose of using it to emulate [the space invaders arcade machine](https://en.wikipedia.org/wiki/Space_Invaders), which according to [r/emudev](https://www.reddit.com/r/EmuDev/) bros doesnt use neither of the half carry flag nor the DAA instruction, so for now they are not implemented in this emulator(for now tho)


# Resources 
i used alot of resources for alot of problems i stumpled upon in this project, but the most helpful were:
* [Intel 8080 Programmers manual](https://altairclone.com/downloads/manuals/8080%20Programmers%20Manual.pdf) not only it was helpful for this project, but i also learned alot of general stuff from it.
* [r/emudev](https://www.reddit.com/r/EmuDev/) on reddit and on discord too
* [full 8080 emulation](http://www.emulator101.com/full-8080-emulation.html) this was very helpful in setting up cpudiag test
* [I8080 opcode table](https://pastraiser.com/cpu/i8080/i8080_opcodes.html)
