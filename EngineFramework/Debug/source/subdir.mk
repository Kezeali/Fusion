################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/FusionContextMenu.cpp \
../source/FusionElementSelectableDataGrid.cpp \
../source/FusionEntityDecorator.cpp \
../source/FusionEntityDecoratorInstancer.cpp \
../source/FusionEntityManager.cpp \
../source/FusionEntitySerialisationUtils.cpp \
../source/FusionGUI.cpp \
../source/FusionGameMapLoader.cpp \
../source/FusionInstanceSynchroniser.cpp \
../source/FusionMessageBox.cpp \
../source/FusionRegionCellCache.cpp \
../source/FusionRegionMapLoader.cpp \
../source/FusionRocketInterface.cpp \
../source/FusionStreamingManager.cpp \
../source/FusionTaskManager.cpp \
../source/FusionTaskScheduler.cpp 

OBJS += \
./source/FusionContextMenu.o \
./source/FusionElementSelectableDataGrid.o \
./source/FusionEntityDecorator.o \
./source/FusionEntityDecoratorInstancer.o \
./source/FusionEntityManager.o \
./source/FusionEntitySerialisationUtils.o \
./source/FusionGUI.o \
./source/FusionGameMapLoader.o \
./source/FusionInstanceSynchroniser.o \
./source/FusionMessageBox.o \
./source/FusionRegionCellCache.o \
./source/FusionRegionMapLoader.o \
./source/FusionRocketInterface.o \
./source/FusionStreamingManager.o \
./source/FusionTaskManager.o \
./source/FusionTaskScheduler.o 

CPP_DEPS += \
./source/FusionContextMenu.d \
./source/FusionElementSelectableDataGrid.d \
./source/FusionEntityDecorator.d \
./source/FusionEntityDecoratorInstancer.d \
./source/FusionEntityManager.d \
./source/FusionEntitySerialisationUtils.d \
./source/FusionGUI.d \
./source/FusionGameMapLoader.d \
./source/FusionInstanceSynchroniser.d \
./source/FusionMessageBox.d \
./source/FusionRegionCellCache.d \
./source/FusionRegionMapLoader.d \
./source/FusionRocketInterface.d \
./source/FusionStreamingManager.d \
./source/FusionTaskManager.d \
./source/FusionTaskScheduler.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -DFSN_USE_ASSERT -DFSN_PROFILING_ENABLED -DBOOST_BIND_NO_PLACEHOLDERS -I"${FUSION_WORKSPACE}/Fusion/Common/include" -I"${FUSION_WORKSPACE}/Fusion/Visual/include" -I"${FUSION_WORKSPACE}/Fusion/Systems/ClanLibRendering/include" -I"${FUSION_WORKSPACE}/Fusion/Systems/AngelScript/include" -I"${FUSION_WORKSPACE}/RAS/include" -I"${FUSION_WORKSPACE}/Fusion/Network/include" -I"${RAKNET_ROOT}/include/raknet" -I"${FUSION_WORKSPACE}/Fusion/Input/include" -I"${FUSION_WORKSPACE}/Fusion/EngineFramework" -I"${FUSION_WORKSPACE}/ScriptUtils/include" -I"${FUSION_WORKSPACE}/Fusion/external" -I"${FUSION_WORKSPACE}/Fusion/ComponentInterface/include" -I"${FUSION_WORKSPACE}/Fusion/EngineFramework/include" -I"${BOOST_ROOT}/include" -O0 -g3 -c -fmessage-length=0 -std=c++0x -pthread -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


