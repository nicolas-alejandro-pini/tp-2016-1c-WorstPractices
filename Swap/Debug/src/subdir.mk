################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Swap.c \
../src/gestionAsignacion.c \
../src/particionSwap.c 

OBJS += \
./src/Swap.o \
./src/gestionAsignacion.o \
./src/particionSwap.o 

C_DEPS += \
./src/Swap.d \
./src/gestionAsignacion.d \
./src/particionSwap.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/nico/git/tp-2016-1c-WorstPractices/Commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


