################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/FusionEasyPacket.cpp \
../source/FusionNetworkManager.cpp \
../source/FusionNetworkSystem.cpp \
../source/FusionPacketDispatcher.cpp \
../source/FusionPlayerManager.cpp \
../source/FusionRakNetwork.cpp 

OBJS += \
./source/FusionEasyPacket.o \
./source/FusionNetworkManager.o \
./source/FusionNetworkSystem.o \
./source/FusionPacketDispatcher.o \
./source/FusionPlayerManager.o \
./source/FusionRakNetwork.o 

CPP_DEPS += \
./source/FusionEasyPacket.d \
./source/FusionNetworkManager.d \
./source/FusionNetworkSystem.d \
./source/FusionPacketDispatcher.d \
./source/FusionPlayerManager.d \
./source/FusionRakNetwork.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -DFSN_USE_ASSERT -DFSN_PROFILING_ENABLED -DBOOST_BIND_NO_PLACEHOLDERS -I"${FUSION_WORKSPACE}/Fusion/Common/include" -I"${FUSION_WORKSPACE}/Fusion/external" -I"${FUSION_WORKSPACE}/Fusion/Network" -I"${FUSION_WORKSPACE}/Fusion/Network/include" -I"${RAKNET_ROOT}/include/raknet" -I"${BOOST_ROOT}/include" -O0 -g3 -Wall -c -fmessage-length=0 -std=c++0x -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


