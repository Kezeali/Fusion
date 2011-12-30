################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/EntityScriptBinding.cpp \
../source/main.cpp 

OBJS += \
./source/EntityScriptBinding.o \
./source/main.o 

CPP_DEPS += \
./source/EntityScriptBinding.d \
./source/main.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -DFSN_USE_ASSERT -DFSN_PROFILING_ENABLED -DBOOST_BIND_NO_PLACEHOLDERS -I"/home/kezeali/FusionGit/Fusion/external/tinyxml" -I"/home/kezeali/FusionGit/Fusion/external/minizip" -I"/home/kezeali/FusionGit/Fusion/Visual" -I"/home/kezeali/FusionGit/Fusion/Systems/ClanLibRendering" -I"/home/kezeali/FusionGit/Fusion/Systems/Box2D" -I"/home/kezeali/FusionGit/Fusion/Systems/AngelScript" -I"/home/kezeali/FusionGit/ScriptUtils" -I"/home/kezeali/FusionGit/RAS/source/Controls" -I"/home/kezeali/FusionGit/RAS/source/Core" -I"/home/kezeali/FusionGit/Fusion/Network" -I"/home/kezeali/FusionGit/Fusion/Input" -I"/home/kezeali/FusionGit/Fusion/EngineFramework" -I"/home/kezeali/FusionGit/Fusion/ComponentInterface" -I"/home/kezeali/FusionGit/Fusion/Common" -I"/home/kezeali/FusionGit/Fusion/ComponentTest" -I"/home/kezeali/FusionGit/Fusion/Common/include" -I"/home/kezeali/FusionGit/Fusion/Input/include" -I"/home/kezeali/FusionGit/Fusion/Network/include" -I"/home/kezeali/FusionGit/Fusion/Visual/include" -I"/home/kezeali/FusionGit/RAS/include" -I"/home/kezeali/FusionGit/ScriptUtils/include" -I/home/kezeali/FusionGit/Fusion/ComponentTest/../external -I"${RAKNET_ROOT}/include/raknet" -I"/home/kezeali/FusionGit/Fusion/ComponentInterface/include" -I"/home/kezeali/FusionGit/Fusion/EngineFramework/include" -I"/home/kezeali/FusionGit/Fusion/Systems/AngelScript/include" -I"/home/kezeali/FusionGit/Fusion/Systems/Box2D/include" -I"/home/kezeali/FusionGit/Fusion/Systems/ClanLibRendering/include" -I/home/kezeali/tbb/include -I"${BOOST_ROOT}/include" -O0 -g3 -c -fmessage-length=0 -std=c++0x -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


