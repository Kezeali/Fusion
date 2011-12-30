################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../FusionLoggerTests.cpp \
../main.cpp 

OBJS += \
./FusionLoggerTests.o \
./main.o 

CPP_DEPS += \
./FusionLoggerTests.d \
./main.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -DFSN_USE_ASSERT -DFSN_PROFILING_ENABLED -DBOOST_BIND_NO_PLACEHOLDERS -I"/home/kezeali/FusionGit/Fusion/external/tinyxml" -I"/home/kezeali/FusionGit/Fusion/external/minizip" -I"/home/kezeali/FusionGit/Fusion/Common" -I"/home/kezeali/FusionGit/Fusion/Common/include" -I/home/kezeali/FusionGit/Fusion/UnitTests/CommonTests/../../external -I"${GTEST_ROOT}/include" -I/home/kezeali/tbb/include -I"${BOOST_ROOT}/include" -O0 -g3 -c -fmessage-length=0 -std=c++0x -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


