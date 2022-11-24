################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL := cmd.exe
RM := rm -rf

USER_OBJS :=

LIBS := 
PROJ := 

O_SRCS := 
C_SRCS := 
S_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
PREPROCESSING_SRCS := 
OBJS := 
OBJS_AS_ARGS := 
C_DEPS := 
C_DEPS_AS_ARGS := 
EXECUTABLES := 
OUTPUT_FILE_PATH :=
OUTPUT_FILE_PATH_AS_ARGS :=
AVR_APP_PATH :=$$$AVR_APP_PATH$$$
QUOTE := "
ADDITIONAL_DEPENDENCIES:=
OUTPUT_FILE_DEP:=
LIB_DEP:=
LINKER_SCRIPT_DEP:=

# Every subdirectory with source files must be described here
SUBDIRS := 


# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS +=  \
../action.cpp \
../BATTLE.cpp \
../bikerace.cpp \
../button.cpp \
../Glcd_function.cpp \
../JUNPGAME.cpp \
../main.cpp \
../move.cpp \
../setumei.cpp \
../Shooting.cpp \
../snake.cpp \
../sound.cpp \
../TETRIS.cpp \
../TETRIS_2.cpp


PREPROCESSING_SRCS += 


ASM_SRCS += 


OBJS +=  \
action.o \
BATTLE.o \
bikerace.o \
button.o \
Glcd_function.o \
JUNPGAME.o \
main.o \
move.o \
setumei.o \
Shooting.o \
snake.o \
sound.o \
TETRIS.o \
TETRIS_2.o

OBJS_AS_ARGS +=  \
action.o \
BATTLE.o \
bikerace.o \
button.o \
Glcd_function.o \
JUNPGAME.o \
main.o \
move.o \
setumei.o \
Shooting.o \
snake.o \
sound.o \
TETRIS.o \
TETRIS_2.o

C_DEPS +=  \
action.d \
BATTLE.d \
bikerace.d \
button.d \
Glcd_function.d \
JUNPGAME.d \
main.d \
move.d \
setumei.d \
Shooting.d \
snake.d \
sound.d \
TETRIS.d \
TETRIS_2.d

C_DEPS_AS_ARGS +=  \
action.d \
BATTLE.d \
bikerace.d \
button.d \
Glcd_function.d \
JUNPGAME.d \
main.d \
move.d \
setumei.d \
Shooting.d \
snake.d \
sound.d \
TETRIS.d \
TETRIS_2.d

OUTPUT_FILE_PATH +=GLCD_SYSTEM.elf

OUTPUT_FILE_PATH_AS_ARGS +=GLCD_SYSTEM.elf

ADDITIONAL_DEPENDENCIES:=

OUTPUT_FILE_DEP:= ./makedep.mk

LIB_DEP+= 

LINKER_SCRIPT_DEP+= 


# AVR32/GNU C Compiler





























./%.o: .././%.cpp
	@echo Building file: $<
	@echo Invoking: AVR8/GNU C Compiler : 4.9.2
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-g++.exe$(QUOTE) -funsigned-char -funsigned-bitfields -DDEBUG  -I"C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\include"  -O1 -ffunction-sections -fdata-sections -fpack-struct -fshort-enums -mrelax -g2 -Wall -mmcu=atmega1284p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega1284p" -c -MD -MP -MF "$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -MT"$(@:%.o=%.o)"   -o "$@" "$<" 
	@echo Finished building: $<
	



# AVR32/GNU Preprocessing Assembler



# AVR32/GNU Assembler




ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(OUTPUT_FILE_PATH) $(ADDITIONAL_DEPENDENCIES)

$(OUTPUT_FILE_PATH): $(OBJS) $(USER_OBJS) $(OUTPUT_FILE_DEP) $(LIB_DEP) $(LINKER_SCRIPT_DEP)
	@echo Building target: $@
	@echo Invoking: AVR8/GNU Linker : 4.9.2
	$(QUOTE)C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-g++.exe$(QUOTE) -o$(OUTPUT_FILE_PATH_AS_ARGS) $(OBJS_AS_ARGS) $(USER_OBJS) $(LIBS) -Wl,-Map="GLCD_SYSTEM.map" -Wl,--start-group -Wl,-lm  -Wl,--end-group -Wl,--gc-sections -mrelax -mmcu=atmega1284p -B "C:\Program Files (x86)\Atmel\Studio\7.0\Packs\atmel\ATmega_DFP\1.0.106\gcc\dev\atmega1284p"  
	@echo Finished building target: $@
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O ihex -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures  "GLCD_SYSTEM.elf" "GLCD_SYSTEM.hex"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -j .eeprom  --set-section-flags=.eeprom=alloc,load --change-section-lma .eeprom=0  --no-change-warnings -O ihex "GLCD_SYSTEM.elf" "GLCD_SYSTEM.eep" || exit 0
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objdump.exe" -h -S "GLCD_SYSTEM.elf" > "GLCD_SYSTEM.lss"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-objcopy.exe" -O srec -R .eeprom -R .fuse -R .lock -R .signature -R .user_signatures "GLCD_SYSTEM.elf" "GLCD_SYSTEM.srec"
	"C:\Program Files (x86)\Atmel\Studio\7.0\toolchain\avr8\avr8-gnu-toolchain\bin\avr-size.exe" "GLCD_SYSTEM.elf"
	
	





# Other Targets
clean:
	-$(RM) $(OBJS_AS_ARGS) $(EXECUTABLES)  
	-$(RM) $(C_DEPS_AS_ARGS)   
	rm -rf "GLCD_SYSTEM.elf" "GLCD_SYSTEM.a" "GLCD_SYSTEM.hex" "GLCD_SYSTEM.lss" "GLCD_SYSTEM.eep" "GLCD_SYSTEM.map" "GLCD_SYSTEM.srec" "GLCD_SYSTEM.usersignatures"
	