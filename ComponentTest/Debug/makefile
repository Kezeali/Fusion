################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include source/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: ComponentTest

dependents:
	-cd "${FUSION_WORKSPACE}/Fusion/Common/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/Fusion/ComponentInterface/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/Fusion/EngineFramework/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/Fusion/Input/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/Fusion/Network/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/RAS/source/Core/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/RAS/source/Controls/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/ScriptUtils/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/Fusion/Systems/AngelScript/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/Fusion/Systems/Box2D/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/Fusion/Systems/ClanLibRendering/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/Fusion/Visual/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/Fusion/external/minizip/Debug" && $(MAKE) all
	-cd "${FUSION_WORKSPACE}/Fusion/external/tinyxml/Debug" && $(MAKE) all

# Tool invocations
ComponentTest: $(OBJS) $(USER_OBJS) "${FUSION_WORKSPACE}/Fusion/Common/Debug/libCommon.a" "${FUSION_WORKSPACE}/Fusion/ComponentInterface/Debug/libComponentInterface.a" "${FUSION_WORKSPACE}/Fusion/EngineFramework/Debug/libEngineFramework.a" "${FUSION_WORKSPACE}/Fusion/Input/Debug/libInput.a" "${FUSION_WORKSPACE}/Fusion/Network/Debug/libNetwork.a" "${FUSION_WORKSPACE}/RAS/source/Core/Debug/libRocketAngelScript.a" "${FUSION_WORKSPACE}/RAS/source/Controls/Debug/libRocketAngelScriptControls.a" "${FUSION_WORKSPACE}/ScriptUtils/Debug/libScriptUtils.a" "${FUSION_WORKSPACE}/Fusion/Systems/AngelScript/Debug/libSystemAngelScript.a" "${FUSION_WORKSPACE}/Fusion/Systems/Box2D/Debug/libSystemBox2D.a" "${FUSION_WORKSPACE}/Fusion/Systems/ClanLibRendering/Debug/libSystemClanLibRendering.a" "${FUSION_WORKSPACE}/Fusion/Visual/Debug/libVisual.a" "${FUSION_WORKSPACE}/Fusion/external/minizip/Debug/libminizip.a" "${FUSION_WORKSPACE}/Fusion/external/tinyxml/Debug/libtinyxml.a"
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++ -L"${FUSION_WORKSPACE}/Fusion/external/tinyxml/Debug" -L"${FUSION_WORKSPACE}/Fusion/external/minizip/Debug" -L"${FUSION_WORKSPACE}/Fusion/Visual/Debug" -L"${FUSION_WORKSPACE}/Fusion/Systems/ClanLibRendering/Debug" -L"${FUSION_WORKSPACE}/Fusion/Systems/Box2D/Debug" -L"${FUSION_WORKSPACE}/Fusion/Systems/AngelScript/Debug" -L"${FUSION_WORKSPACE}/Fusion/Network/Debug" -L"${FUSION_WORKSPACE}/Fusion/Input/Debug" -L"${FUSION_WORKSPACE}/Fusion/EngineFramework/Debug" -L"${FUSION_WORKSPACE}/Fusion/ComponentInterface/Debug" -L"${FUSION_WORKSPACE}/Fusion/Common/Debug" -L"${FUSION_WORKSPACE}/ScriptUtils/Debug" -L"${FUSION_WORKSPACE}/RAS/source/Core/Debug" -L"${FUSION_WORKSPACE}/RAS/source/Controls/Debug" -L"${RAKNET_ROOT}/Lib" -L"${BOOST_ROOT}/lib" -o "ComponentTest" $(OBJS) $(USER_OBJS) $(LIBS) `pkg-config --cflags --libs clanCore-2.3 clanDisplay-2.3 clanGL-2.3 clanApp-2.3 clanRegExp-2.3 clanSound-2.3 clanVorbis-2.3` -pthread
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C++_DEPS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) ComponentTest
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:
${FUSION_WORKSPACE}/Fusion/Common/Debug/libCommon.a:
${FUSION_WORKSPACE}/Fusion/ComponentInterface/Debug/libComponentInterface.a:
${FUSION_WORKSPACE}/Fusion/EngineFramework/Debug/libEngineFramework.a:
${FUSION_WORKSPACE}/Fusion/Input/Debug/libInput.a:
${FUSION_WORKSPACE}/Fusion/Network/Debug/libNetwork.a:
${FUSION_WORKSPACE}/RAS/source/Core/Debug/libRocketAngelScript.a:
${FUSION_WORKSPACE}/RAS/source/Controls/Debug/libRocketAngelScriptControls.a:
${FUSION_WORKSPACE}/ScriptUtils/Debug/libScriptUtils.a:
${FUSION_WORKSPACE}/Fusion/Systems/AngelScript/Debug/libSystemAngelScript.a:
${FUSION_WORKSPACE}/Fusion/Systems/Box2D/Debug/libSystemBox2D.a:
${FUSION_WORKSPACE}/Fusion/Systems/ClanLibRendering/Debug/libSystemClanLibRendering.a:
${FUSION_WORKSPACE}/Fusion/Visual/Debug/libVisual.a:
${FUSION_WORKSPACE}/Fusion/external/minizip/Debug/libminizip.a:
${FUSION_WORKSPACE}/Fusion/external/tinyxml/Debug/libtinyxml.a:

-include ../makefile.targets
