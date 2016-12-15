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
all: PokeDex_Servidor

dependents:
	-cd /home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/CompuMundoHiperMegaRed-commons/Debug && $(MAKE) all

# Tool invocations
PokeDex_Servidor: $(OBJS) $(USER_OBJS) /home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/CompuMundoHiperMegaRed-commons/Debug/libCompuMundoHiperMegaRed-commons.so
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C Linker'
	gcc -L"/home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/CompuMundoHiperMegaRed-commons/Debug" -o "PokeDex_Servidor" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) PokeDex_Servidor
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
/home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/CompuMundoHiperMegaRed-commons/Debug/libCompuMundoHiperMegaRed-commons.so:

-include ../makefile.targets
