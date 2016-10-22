################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Serializable.c \
../cmhmr_fuse_main.c \
../mockPodexCliente.c \
../protocoloCliente.c 

OBJS += \
./Serializable.o \
./cmhmr_fuse_main.o \
./mockPodexCliente.o \
./protocoloCliente.o 

C_DEPS += \
./Serializable.d \
./cmhmr_fuse_main.d \
./mockPodexCliente.d \
./protocoloCliente.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0  -DFUSE_USE_VERSION=27 -D_FILE_OFFSET_BITS=64 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


