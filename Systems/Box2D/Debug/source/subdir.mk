################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/FusionBox2DComponent.cpp \
../source/FusionBox2DSystem.cpp 

OBJS += \
./source/FusionBox2DComponent.o \
./source/FusionBox2DSystem.o 

CPP_DEPS += \
./source/FusionBox2DComponent.d \
./source/FusionBox2DSystem.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -DFSN_USE_ASSERT -DFSN_PROFILING_ENABLED -DBOOST_BIND_NO_PLACEHOLDERS -I"${RAKNET_ROOT}/include/raknet" -I"/home/kezeali/FusionGit/Fusion/Network/include" -I"/home/kezeali/FusionGit/Fusion/Common/include" -I"/home/kezeali/FusionGit/Fusion/Input/include" -I"/home/kezeali/FusionGit/Fusion/external" -I"/home/kezeali/FusionGit/ScriptUtils/include" -I"/home/kezeali/FusionGit/Fusion/ComponentInterface/include" -I"/home/kezeali/FusionGit/Fusion/Systems/Box2D" -I"/home/kezeali/FusionGit/Fusion/Systems/Box2D/include" -I"${BOOST_ROOT}/include" -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


