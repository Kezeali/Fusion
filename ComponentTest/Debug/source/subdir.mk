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
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -DFSN_USE_ASSERT -DFSN_PROFILING_ENABLED -DBOOST_BIND_NO_PLACEHOLDERS -I"${FUSION_WORKSPACE}/Fusion/Visual" -I"${FUSION_WORKSPACE}/Fusion/Systems/ClanLibRendering" -I"${FUSION_WORKSPACE}/Fusion/Systems/Box2D" -I"${FUSION_WORKSPACE}/Fusion/Systems/AngelScript" -I"${FUSION_WORKSPACE}/Fusion/Network" -I"${FUSION_WORKSPACE}/Fusion/Input" -I"${FUSION_WORKSPACE}/Fusion/EngineFramework" -I"${FUSION_WORKSPACE}/Fusion/ComponentInterface" -I"${FUSION_WORKSPACE}/Fusion/Common" -I"${FUSION_WORKSPACE}/Fusion/ComponentTest" -I"${FUSION_WORKSPACE}/Fusion/ComponentTest/include" -I"${FUSION_WORKSPACE}/Fusion/Common/include" -I"${FUSION_WORKSPACE}/Fusion/Input/include" -I"${FUSION_WORKSPACE}/Fusion/Network/include" -I"${FUSION_WORKSPACE}/Fusion/Visual/include" -I"${FUSION_WORKSPACE}/RAS/include" -I"${FUSION_WORKSPACE}/ScriptUtils/include" -I"${FUSION_WORKSPACE}/Fusion/external" -I"${RAKNET_ROOT}/include/raknet" -I"${FUSION_WORKSPACE}/Fusion/ComponentInterface/include" -I"${FUSION_WORKSPACE}/Fusion/EngineFramework/include" -I"${FUSION_WORKSPACE}/Fusion/Systems/AngelScript/include" -I"${FUSION_WORKSPACE}/Fusion/Systems/Box2D/include" -I"${FUSION_WORKSPACE}/Fusion/Systems/ClanLibRendering/include" -I"${BOOST_ROOT}/include" -O0 -g3 -c -fmessage-length=0 -std=c++0x -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


