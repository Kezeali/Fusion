################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/FusionAngelScriptComponent.cpp \
../source/FusionAngelScriptSystem.cpp 

OBJS += \
./source/FusionAngelScriptComponent.o \
./source/FusionAngelScriptSystem.o 

CPP_DEPS += \
./source/FusionAngelScriptComponent.d \
./source/FusionAngelScriptSystem.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -DFSN_USE_ASSERT -DFSN_PROFILING_ENABLED -DBOOST_BIND_NO_PLACEHOLDERS -I/home/kezeali/tbb/include -I"/home/kezeali/FusionGit/Fusion/Network/include" -I"/home/kezeali/FusionGit/Fusion/Common/include" -I"/home/kezeali/FusionGit/Fusion/Input/include" -I"/home/kezeali/FusionGit/Fusion/external" -I"/home/kezeali/FusionGit/Fusion/ComponentInterface/include" -I"/home/kezeali/FusionGit/Fusion/Systems/AngelScript/include" -I"/home/kezeali/FusionGit/Fusion/Systems/AngelScript" -I"/home/kezeali/FusionGit/ScriptUtils/include" -I"${RAKNET_ROOT}/include/raknet" -I"${BOOST_ROOT}/include" -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


