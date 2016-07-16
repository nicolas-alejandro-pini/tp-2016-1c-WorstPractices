################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/nico/git/tp-2016-1c-WorstPractices/Commons/commons/collections/dictionary.c \
/home/nico/git/tp-2016-1c-WorstPractices/Commons/commons/collections/list.c \
/home/nico/git/tp-2016-1c-WorstPractices/Commons/commons/collections/list_mutex.c \
/home/nico/git/tp-2016-1c-WorstPractices/Commons/commons/collections/queue.c 

OBJS += \
./commons/collections/dictionary.o \
./commons/collections/list.o \
./commons/collections/list_mutex.o \
./commons/collections/queue.o 

C_DEPS += \
./commons/collections/dictionary.d \
./commons/collections/list.d \
./commons/collections/list_mutex.d \
./commons/collections/queue.d 


# Each subdirectory must supply rules for building sources it contributes
commons/collections/dictionary.o: /home/nico/git/tp-2016-1c-WorstPractices/Commons/commons/collections/dictionary.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/nico/git/tp-2016-1c-WorstPractices/Commons" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

commons/collections/list.o: /home/nico/git/tp-2016-1c-WorstPractices/Commons/commons/collections/list.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/nico/git/tp-2016-1c-WorstPractices/Commons" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

commons/collections/list_mutex.o: /home/nico/git/tp-2016-1c-WorstPractices/Commons/commons/collections/list_mutex.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/nico/git/tp-2016-1c-WorstPractices/Commons" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

commons/collections/queue.o: /home/nico/git/tp-2016-1c-WorstPractices/Commons/commons/collections/queue.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/nico/git/tp-2016-1c-WorstPractices/Commons" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


