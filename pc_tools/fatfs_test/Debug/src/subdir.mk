################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/fatfs_test.cpp 

C_SRCS += \
/home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src/diskio.c \
/home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src/ff.c 

OBJS += \
./src/diskio.o \
./src/fatfs_test.o \
./src/ff.o 

CPP_DEPS += \
./src/fatfs_test.d 

C_DEPS += \
./src/diskio.d \
./src/ff.d 


# Each subdirectory must supply rules for building sources it contributes
src/diskio.o: /home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src/diskio.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ff.o: /home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src/ff.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


