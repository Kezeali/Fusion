################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../FusionLoggerTests.cpp \
../FusionResourceManagerTests.cpp \
../main.cpp 

OBJS += \
./FusionLoggerTests.o \
./FusionResourceManagerTests.o \
./main.o 

CPP_DEPS += \
./FusionLoggerTests.d \
./FusionResourceManagerTests.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -DFSN_USE_ASSERT -DFSN_PROFILING_ENABLED -DBOOST_BIND_NO_PLACEHOLDERS -I"${FUSION_WORKSPACE}/Fusion/Common" -I"${FUSION_WORKSPACE}/Fusion/Common/include" -I"${FUSION_WORKSPACE}/Fusion/external" -I"${FUSION_WORKSPACE}/ScriptUtils/include" -I"${GTEST_ROOT}/include" -I"${BOOST_ROOT}/include" -O0 -g3 -c -fmessage-length=0 -std=c++0x -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


