################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Nucleo.c \
../src/consumidor_cpu.c \
../src/nucleo_config.c \
../src/planificador.c \
../src/semaforos.c \
../src/shared_vars.c 

OBJS += \
./src/Nucleo.o \
./src/consumidor_cpu.o \
./src/nucleo_config.o \
./src/planificador.o \
./src/semaforos.o \
./src/shared_vars.o 

C_DEPS += \
./src/Nucleo.d \
./src/consumidor_cpu.d \
./src/nucleo_config.d \
./src/planificador.d \
./src/semaforos.d \
./src/shared_vars.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/nico/git/tp-2016-1c-WorstPractices/Commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


