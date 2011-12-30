################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/FusionCamera.cpp \
../source/FusionPhysicsDebugDraw.cpp \
../source/FusionRenderer.cpp \
../source/FusionViewport.cpp 

OBJS += \
./source/FusionCamera.o \
./source/FusionPhysicsDebugDraw.o \
./source/FusionRenderer.o \
./source/FusionViewport.o 

CPP_DEPS += \
./source/FusionCamera.d \
./source/FusionPhysicsDebugDraw.d \
./source/FusionRenderer.d \
./source/FusionViewport.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -DFSN_USE_ASSERT -DFSN_PROFILING_ENABLED -DBOOST_BIND_NO_PLACEHOLDERS -I"${FUSION_WORKSPACE}/Fusion/Common/include" -I"${FUSION_WORKSPACE}/ScriptUtils/include" -I"${FUSION_WORKSPACE}/Fusion/Visual" -I"${FUSION_WORKSPACE}/Fusion/external" -I"${FUSION_WORKSPACE}/Fusion/Visual/include" -I"${BOOST_ROOT}/include" -O0 -g3 -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


