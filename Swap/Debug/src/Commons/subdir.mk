################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/Commons/Fcomunes.c \
../src/Commons/bitarray.c \
../src/Commons/config.c \
../src/Commons/error.c \
../src/Commons/listas.c \
../src/Commons/log.c \
../src/Commons/process.c \
../src/Commons/sockets.c \
../src/Commons/socketsIPCIRC.c \
../src/Commons/string.c \
../src/Commons/temporal.c \
../src/Commons/txt.c 

OBJS += \
./src/Commons/Fcomunes.o \
./src/Commons/bitarray.o \
./src/Commons/config.o \
./src/Commons/error.o \
./src/Commons/listas.o \
./src/Commons/log.o \
./src/Commons/process.o \
./src/Commons/sockets.o \
./src/Commons/socketsIPCIRC.o \
./src/Commons/string.o \
./src/Commons/temporal.o \
./src/Commons/txt.o 

C_DEPS += \
./src/Commons/Fcomunes.d \
./src/Commons/bitarray.d \
./src/Commons/config.d \
./src/Commons/error.d \
./src/Commons/listas.d \
./src/Commons/log.d \
./src/Commons/process.d \
./src/Commons/sockets.d \
./src/Commons/socketsIPCIRC.d \
./src/Commons/string.d \
./src/Commons/temporal.d \
./src/Commons/txt.d 


# Each subdirectory must supply rules for building sources it contributes
src/Commons/%.o: ../src/Commons/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/nico/git/tp-2016-1c-WorstPractices/Commons" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


