################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/utnso/git/tp-2016-1c-WorstPractices/Commons/commons/parser/metadata_program.c \
/home/utnso/git/tp-2016-1c-WorstPractices/Commons/commons/parser/parser.c 

OBJS += \
./commons/parser/metadata_program.o \
./commons/parser/parser.o 

C_DEPS += \
./commons/parser/metadata_program.d \
./commons/parser/parser.d 


# Each subdirectory must supply rules for building sources it contributes
commons/parser/metadata_program.o: /home/utnso/git/tp-2016-1c-WorstPractices/Commons/commons/parser/metadata_program.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2016-1c-WorstPractices/Commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

commons/parser/parser.o: /home/utnso/git/tp-2016-1c-WorstPractices/Commons/commons/parser/parser.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2016-1c-WorstPractices/Commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


