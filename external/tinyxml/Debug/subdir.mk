################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../ticpp.cpp \
../tinyxml.cpp \
../tinyxmlerror.cpp \
../tinyxmlparser.cpp 

OBJS += \
./ticpp.o \
./tinyxml.o \
./tinyxmlerror.o \
./tinyxmlparser.o 

CPP_DEPS += \
./ticpp.d \
./tinyxml.d \
./tinyxmlerror.d \
./tinyxmlparser.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -O0 -g3 -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


