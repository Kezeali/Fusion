################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/FusionInputDefinitionLoader.cpp \
../source/FusionInputHandler.cpp \
../source/FusionPlayerInput.cpp 

OBJS += \
./source/FusionInputDefinitionLoader.o \
./source/FusionInputHandler.o \
./source/FusionPlayerInput.o 

CPP_DEPS += \
./source/FusionInputDefinitionLoader.d \
./source/FusionInputHandler.d \
./source/FusionPlayerInput.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -DFSN_USE_ASSERT -DFSN_PROFILING_ENABLED -DBOOST_BIND_NO_PLACEHOLDERS -DBOOST_MULTI_INDEX_DISABLE_SERIALIZATION -I"/home/kezeali/FusionGit/Fusion/Input/include" -I/home/kezeali/tbb/include -I"/home/kezeali/FusionGit/Fusion/Common/include" -I"/home/kezeali/FusionGit/Fusion/Input" -I"/home/kezeali/FusionGit/Fusion/Input/../external" -I"${RAKNET_ROOT}/include/raknet" -I"/home/kezeali/FusionGit/ScriptUtils/include" -I"${BOOST_ROOT}/include" -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


