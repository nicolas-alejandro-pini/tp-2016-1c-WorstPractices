################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/tests/test_nucleo.c 

OBJS += \
./src/tests/test_nucleo.o 

C_DEPS += \
./src/tests/test_nucleo.d 


# Each subdirectory must supply rules for building sources it contributes
src/tests/%.o: ../src/tests/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/nico/git/tp-2016-1c-WorstPractices/Commons" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


