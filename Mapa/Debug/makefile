################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: Mapa

dependents:
	-cd /home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/CompuMundoHiperMegaRed-commons/Debug && $(MAKE) all
	-cd /home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/nivel/Debug && $(MAKE) all

# Tool invocations
Mapa: $(OBJS) $(USER_OBJS) /home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/CompuMundoHiperMegaRed-commons/Debug/libCompuMundoHiperMegaRed-commons.so /home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/nivel/Debug/libnivel.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/CompuMundoHiperMegaRed-commons/Debug" -L"/home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/nivel/Debug" -o "Mapa" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) Mapa
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/CompuMundoHiperMegaRed-commons/Debug/libCompuMundoHiperMegaRed-commons.so:
/home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/nivel/Debug/libnivel.so:

-include ../makefile.targets
