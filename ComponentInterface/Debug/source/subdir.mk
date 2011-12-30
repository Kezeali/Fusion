################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/FusionComponentFactory.cpp \
../source/FusionDeltaTime.cpp \
../source/FusionEntity.cpp \
../source/FusionEntityComponent.cpp 

OBJS += \
./source/FusionComponentFactory.o \
./source/FusionDeltaTime.o \
./source/FusionEntity.o \
./source/FusionEntityComponent.o 

CPP_DEPS += \
./source/FusionComponentFactory.d \
./source/FusionDeltaTime.d \
./source/FusionEntity.d \
./source/FusionEntityComponent.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -DFSN_USE_ASSERT -DFSN_PROFILING_ENABLED -DBOOST_BIND_NO_PLACEHOLDERS -I/home/kezeali/tbb/include -I"/home/kezeali/FusionGit/Fusion/ComponentInterface" -I"/home/kezeali/FusionGit/Fusion/ComponentInterface/include" -I"/home/kezeali/FusionGit/Fusion/ComponentInterface/../external" -I"${RAKNET_ROOT}/include/raknet" -I"/home/kezeali/FusionGit/Fusion/Common/include" -I"/home/kezeali/FusionGit/Fusion/Input/include" -I"/home/kezeali/FusionGit/ScriptUtils/include" -I"/home/kezeali/FusionGit/Fusion/Network/include" -I"${BOOST_ROOT}/include" -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


