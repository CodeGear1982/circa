# GNU Make project makefile autogenerated by Premake
ifndef config
  config=debug
endif

ifndef verbose
  SILENT = @
endif

ifndef CC
  CC = gcc
endif

ifndef CXX
  CXX = g++
endif

ifndef AR
  AR = ar
endif

ifeq ($(config),debug)
  OBJDIR     = ../build/obj/Debug/unit_tests
  TARGETDIR  = ../build
  TARGET     = $(TARGETDIR)/circa_test
  DEFINES   += -DDEBUG
  INCLUDES  += -I../include -I. -I../3rdparty -I/usr/local/include
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -L/usr/local/lib -L../build
  LIBS      += -lcirca_d -ldl -luv
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += ../build/libcirca_d.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

ifeq ($(config),release)
  OBJDIR     = ../build/obj/Release/unit_tests
  TARGETDIR  = ../build
  TARGET     = $(TARGETDIR)/circa_test_r
  DEFINES   += 
  INCLUDES  += -I../include -I. -I../3rdparty -I/usr/local/include
  CPPFLAGS  += -MMD -MP $(DEFINES) $(INCLUDES)
  CFLAGS    += $(CPPFLAGS) $(ARCH) -g -O3
  CXXFLAGS  += $(CFLAGS) 
  LDFLAGS   += -L/usr/local/lib -L../build
  LIBS      += -lcirca -ldl -luv
  RESFLAGS  += $(DEFINES) $(INCLUDES) 
  LDDEPS    += ../build/libcirca.a
  LINKCMD    = $(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(ARCH) $(LIBS)
  define PREBUILDCMDS
  endef
  define PRELINKCMDS
  endef
  define POSTBUILDCMDS
  endef
endif

OBJECTS := \
	$(OBJDIR)/block_test.o \
	$(OBJDIR)/building_test.o \
	$(OBJDIR)/cascading_test.o \
	$(OBJDIR)/code_iterator_test.o \
	$(OBJDIR)/compound_type_test.o \
	$(OBJDIR)/control_flow_test.o \
	$(OBJDIR)/fakefs_test.o \
	$(OBJDIR)/file_test.o \
	$(OBJDIR)/file_watch_test.o \
	$(OBJDIR)/function_test.o \
	$(OBJDIR)/hashtable_test.o \
	$(OBJDIR)/if_block.o \
	$(OBJDIR)/importing_test.o \
	$(OBJDIR)/interpreter_test.o \
	$(OBJDIR)/list_test.o \
	$(OBJDIR)/loop_test.o \
	$(OBJDIR)/main.o \
	$(OBJDIR)/migration_test.o \
	$(OBJDIR)/modules_test.o \
	$(OBJDIR)/names_test.o \
	$(OBJDIR)/native_patch_test.o \
	$(OBJDIR)/parser_test.o \
	$(OBJDIR)/path_expression_test.o \
	$(OBJDIR)/stack_test.o \
	$(OBJDIR)/state_test.o \
	$(OBJDIR)/string_test.o \
	$(OBJDIR)/symbol_test.o \
	$(OBJDIR)/tagged_value_test.o \
	$(OBJDIR)/tokenizer_test.o \
	$(OBJDIR)/type_test.o \

RESOURCES := \

SHELLTYPE := msdos
ifeq (,$(ComSpec)$(COMSPEC))
  SHELLTYPE := posix
endif
ifeq (/bin,$(findstring /bin,$(SHELL)))
  SHELLTYPE := posix
endif

.PHONY: clean prebuild prelink

all: $(TARGETDIR) $(OBJDIR) prebuild prelink $(TARGET)
	@:

$(TARGET): $(GCH) $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking unit_tests
	$(SILENT) $(LINKCMD)
	$(POSTBUILDCMDS)

$(TARGETDIR):
	@echo Creating $(TARGETDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(TARGETDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(TARGETDIR))
endif

$(OBJDIR):
	@echo Creating $(OBJDIR)
ifeq (posix,$(SHELLTYPE))
	$(SILENT) mkdir -p $(OBJDIR)
else
	$(SILENT) mkdir $(subst /,\\,$(OBJDIR))
endif

clean:
	@echo Cleaning unit_tests
ifeq (posix,$(SHELLTYPE))
	$(SILENT) rm -f  $(TARGET)
	$(SILENT) rm -rf $(OBJDIR)
else
	$(SILENT) if exist $(subst /,\\,$(TARGET)) del $(subst /,\\,$(TARGET))
	$(SILENT) if exist $(subst /,\\,$(OBJDIR)) rmdir /s /q $(subst /,\\,$(OBJDIR))
endif

prebuild:
	$(PREBUILDCMDS)

prelink:
	$(PRELINKCMDS)

ifneq (,$(PCH))
$(GCH): $(PCH)
	@echo $(notdir $<)
	-$(SILENT) cp $< $(OBJDIR)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
endif

$(OBJDIR)/block_test.o: unit_tests/block_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/building_test.o: unit_tests/building_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/cascading_test.o: unit_tests/cascading_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/code_iterator_test.o: unit_tests/code_iterator_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/compound_type_test.o: unit_tests/compound_type_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/control_flow_test.o: unit_tests/control_flow_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/fakefs_test.o: unit_tests/fakefs_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/file_test.o: unit_tests/file_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/file_watch_test.o: unit_tests/file_watch_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/function_test.o: unit_tests/function_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/hashtable_test.o: unit_tests/hashtable_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/if_block.o: unit_tests/if_block.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/importing_test.o: unit_tests/importing_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/interpreter_test.o: unit_tests/interpreter_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/list_test.o: unit_tests/list_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/loop_test.o: unit_tests/loop_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/main.o: unit_tests/main.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/migration_test.o: unit_tests/migration_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/modules_test.o: unit_tests/modules_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/names_test.o: unit_tests/names_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/native_patch_test.o: unit_tests/native_patch_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/parser_test.o: unit_tests/parser_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/path_expression_test.o: unit_tests/path_expression_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/stack_test.o: unit_tests/stack_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/state_test.o: unit_tests/state_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/string_test.o: unit_tests/string_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/symbol_test.o: unit_tests/symbol_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/tagged_value_test.o: unit_tests/tagged_value_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/tokenizer_test.o: unit_tests/tokenizer_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"
$(OBJDIR)/type_test.o: unit_tests/type_test.cpp
	@echo $(notdir $<)
	$(SILENT) $(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
