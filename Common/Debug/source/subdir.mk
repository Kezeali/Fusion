################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../source/FusionAnyFS.cpp \
../source/FusionAssert.cpp \
../source/FusionAudioLoader.cpp \
../source/FusionBinaryStream.cpp \
../source/FusionClientOptions.cpp \
../source/FusionCmdOptions.cpp \
../source/FusionConsole.cpp \
../source/FusionConsoleStdOutWriter.cpp \
../source/FusionException.cpp \
../source/FusionIDStack.cpp \
../source/FusionImageLoader.cpp \
../source/FusionLog.cpp \
../source/FusionLogFileConsole.cpp \
../source/FusionLogPhysFS.cpp \
../source/FusionLogger.cpp \
../source/FusionPhysFS.cpp \
../source/FusionPhysFSIODeviceProvider.cpp \
../source/FusionPhysFSIOStream.cpp \
../source/FusionPlayerRegistry.cpp \
../source/FusionPrecompiledHeaders.cpp \
../source/FusionProfiling.cpp \
../source/FusionRefCounted.cpp \
../source/FusionResource.cpp \
../source/FusionResourceManager.cpp \
../source/FusionScriptManager.cpp \
../source/FusionScriptModule.cpp \
../source/FusionScriptReference.cpp \
../source/FusionScriptSound.cpp \
../source/FusionScriptVector.cpp \
../source/FusionScriptedConsoleCommand.cpp \
../source/FusionScriptedSlots.cpp \
../source/FusionSpriteDefinition.cpp \
../source/FusionStreamedResourceUser.cpp \
../source/FusionTimer.cpp \
../source/FusionVirtualFileSource_PhysFS.cpp \
../source/FusionXML.cpp \
../source/FusionXMLLoader.cpp \
../source/FusionZipArchive.cpp \
../source/PhysFS++.cpp \
../source/scriptany.cpp \
../source/scriptarray.cpp \
../source/scriptmath.cpp \
../source/scriptstdstring.cpp \
../source/scriptstdstring_utils.cpp \
../source/scriptstring.cpp \
../source/scriptstring_utils.cpp 

OBJS += \
./source/FusionAnyFS.o \
./source/FusionAssert.o \
./source/FusionAudioLoader.o \
./source/FusionBinaryStream.o \
./source/FusionClientOptions.o \
./source/FusionCmdOptions.o \
./source/FusionConsole.o \
./source/FusionConsoleStdOutWriter.o \
./source/FusionException.o \
./source/FusionIDStack.o \
./source/FusionImageLoader.o \
./source/FusionLog.o \
./source/FusionLogFileConsole.o \
./source/FusionLogPhysFS.o \
./source/FusionLogger.o \
./source/FusionPhysFS.o \
./source/FusionPhysFSIODeviceProvider.o \
./source/FusionPhysFSIOStream.o \
./source/FusionPlayerRegistry.o \
./source/FusionPrecompiledHeaders.o \
./source/FusionProfiling.o \
./source/FusionRefCounted.o \
./source/FusionResource.o \
./source/FusionResourceManager.o \
./source/FusionScriptManager.o \
./source/FusionScriptModule.o \
./source/FusionScriptReference.o \
./source/FusionScriptSound.o \
./source/FusionScriptVector.o \
./source/FusionScriptedConsoleCommand.o \
./source/FusionScriptedSlots.o \
./source/FusionSpriteDefinition.o \
./source/FusionStreamedResourceUser.o \
./source/FusionTimer.o \
./source/FusionVirtualFileSource_PhysFS.o \
./source/FusionXML.o \
./source/FusionXMLLoader.o \
./source/FusionZipArchive.o \
./source/PhysFS++.o \
./source/scriptany.o \
./source/scriptarray.o \
./source/scriptmath.o \
./source/scriptstdstring.o \
./source/scriptstdstring_utils.o \
./source/scriptstring.o \
./source/scriptstring_utils.o 

CPP_DEPS += \
./source/FusionAnyFS.d \
./source/FusionAssert.d \
./source/FusionAudioLoader.d \
./source/FusionBinaryStream.d \
./source/FusionClientOptions.d \
./source/FusionCmdOptions.d \
./source/FusionConsole.d \
./source/FusionConsoleStdOutWriter.d \
./source/FusionException.d \
./source/FusionIDStack.d \
./source/FusionImageLoader.d \
./source/FusionLog.d \
./source/FusionLogFileConsole.d \
./source/FusionLogPhysFS.d \
./source/FusionLogger.d \
./source/FusionPhysFS.d \
./source/FusionPhysFSIODeviceProvider.d \
./source/FusionPhysFSIOStream.d \
./source/FusionPlayerRegistry.d \
./source/FusionPrecompiledHeaders.d \
./source/FusionProfiling.d \
./source/FusionRefCounted.d \
./source/FusionResource.d \
./source/FusionResourceManager.d \
./source/FusionScriptManager.d \
./source/FusionScriptModule.d \
./source/FusionScriptReference.d \
./source/FusionScriptSound.d \
./source/FusionScriptVector.d \
./source/FusionScriptedConsoleCommand.d \
./source/FusionScriptedSlots.d \
./source/FusionSpriteDefinition.d \
./source/FusionStreamedResourceUser.d \
./source/FusionTimer.d \
./source/FusionVirtualFileSource_PhysFS.d \
./source/FusionXML.d \
./source/FusionXMLLoader.d \
./source/FusionZipArchive.d \
./source/PhysFS++.d \
./source/scriptany.d \
./source/scriptarray.d \
./source/scriptmath.d \
./source/scriptstdstring.d \
./source/scriptstdstring_utils.d \
./source/scriptstring.d \
./source/scriptstring_utils.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DTIXML_USE_TICPP -DTIXML_USE_STL -D_DEBUG -DFSN_USE_ASSERT -DFSN_PROFILING_ENABLED -DBOOST_BIND_NO_PLACEHOLDERS -I"/home/kezeali/FusionGit/ScriptUtils/include" -I"${RAKNET_ROOT}/include/raknet" -I/home/kezeali/tbb/include -I"/home/kezeali/FusionGit/Fusion/Common/../external" -I"/home/kezeali/FusionGit/Fusion/Common" -I"/home/kezeali/FusionGit/Fusion/Common/include" -I"${BOOST_ROOT}/include" -O0 -g3 -c -fmessage-length=0 -std=c++0x -fpermissive -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


