################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
/home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src/FatFSAccessor.c \
../src/FatFSTestHelper.c \
../src/diskio.c \
../src/fatfs_test.c \
/home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src/ff.c 

OBJS += \
./src/FatFSAccessor.o \
./src/FatFSTestHelper.o \
./src/diskio.o \
./src/fatfs_test.o \
./src/ff.o 

C_DEPS += \
./src/FatFSAccessor.d \
./src/FatFSTestHelper.d \
./src/diskio.d \
./src/fatfs_test.d \
./src/ff.d 


# Each subdirectory must supply rules for building sources it contributes
src/FatFSAccessor.o: /home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src/FatFSAccessor.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_LARGEFILE64_SOURCE=1 -D_PCSIM=1 -D_LARGE_FILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -I"/home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_LARGEFILE64_SOURCE=1 -D_PCSIM=1 -D_LARGE_FILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -I"/home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/ff.o: /home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src/ff.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -D_LARGEFILE64_SOURCE=1 -D_PCSIM=1 -D_LARGE_FILE_SOURCE=1 -D_FILE_OFFSET_BITS=64 -I"/home/cromwelldev/workspace/lpcmod_os/fs/ff12b/src" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


