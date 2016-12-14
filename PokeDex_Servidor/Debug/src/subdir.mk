################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/PokeDex_Servidor.c 

OBJS += \
./src/PokeDex_Servidor.o 

C_DEPS += \
./src/PokeDex_Servidor.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/CompuMundoHiperMegaRed-commons" -include"/home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/CompuMundoHiperMegaRed-commons/sockets.c" -include"/home/utnso/Documentos/Projects/SO_2016/Github/CompuMundoHiperMegaRed/CompuMundoHiperMegaRed-commons/osada.c" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


