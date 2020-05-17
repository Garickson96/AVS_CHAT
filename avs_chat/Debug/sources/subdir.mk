################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../sources/chat_tcp.c \
../sources/cli.c \
../sources/discovery_udp.c \
../sources/doubly_linked_list.c \
../sources/files_processor.c \
../sources/hudba_chat.c \
../sources/main.c \
../sources/universal.c 

OBJS += \
./sources/chat_tcp.o \
./sources/cli.o \
./sources/discovery_udp.o \
./sources/doubly_linked_list.o \
./sources/files_processor.o \
./sources/hudba_chat.o \
./sources/main.o \
./sources/universal.o 

C_DEPS += \
./sources/chat_tcp.d \
./sources/cli.d \
./sources/discovery_udp.d \
./sources/doubly_linked_list.d \
./sources/files_processor.d \
./sources/hudba_chat.d \
./sources/main.d \
./sources/universal.d 


# Each subdirectory must supply rules for building sources it contributes
sources/%.o: ../sources/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


