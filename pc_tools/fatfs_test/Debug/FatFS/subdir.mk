################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src/diskio.c \
/home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src/ff.c 

OBJS += \
./FatFS/diskio.o \
./FatFS/ff.o 

C_DEPS += \
./FatFS/diskio.d \
./FatFS/ff.d 


# Each subdirectory must supply rules for building sources it contributes
FatFS/diskio.o: /home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src/diskio.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

FatFS/ff.o: /home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src/ff.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


