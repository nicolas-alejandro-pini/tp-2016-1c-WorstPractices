################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Consola.c \
../src/ICPU.c \
../src/ISwap.c \
../src/Memoria.c \
../src/TLB.c \
../src/TablaMarcos.c \
../src/UMC.c 

OBJS += \
./src/Consola.o \
./src/ICPU.o \
./src/ISwap.o \
./src/Memoria.o \
./src/TLB.o \
./src/TablaMarcos.o \
./src/UMC.o 

C_DEPS += \
./src/Consola.d \
./src/ICPU.d \
./src/ISwap.d \
./src/Memoria.d \
./src/TLB.d \
./src/TablaMarcos.d \
./src/UMC.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/nico/git/tp-2016-1c-WorstPractices/Commons" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


