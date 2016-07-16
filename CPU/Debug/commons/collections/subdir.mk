################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/utnso/git/tp-2016-1c-WorstPractices/Commons/commons/collections/dictionary.c \
/home/utnso/git/tp-2016-1c-WorstPractices/Commons/commons/collections/list.c \
/home/utnso/git/tp-2016-1c-WorstPractices/Commons/commons/collections/list_mutex.c \
/home/utnso/git/tp-2016-1c-WorstPractices/Commons/commons/collections/queue.c 

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
commons/collections/dictionary.o: /home/utnso/git/tp-2016-1c-WorstPractices/Commons/commons/collections/dictionary.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2016-1c-WorstPractices/Commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

commons/collections/list.o: /home/utnso/git/tp-2016-1c-WorstPractices/Commons/commons/collections/list.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2016-1c-WorstPractices/Commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

commons/collections/list_mutex.o: /home/utnso/git/tp-2016-1c-WorstPractices/Commons/commons/collections/list_mutex.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2016-1c-WorstPractices/Commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

commons/collections/queue.o: /home/utnso/git/tp-2016-1c-WorstPractices/Commons/commons/collections/queue.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/utnso/git/tp-2016-1c-WorstPractices/Commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


