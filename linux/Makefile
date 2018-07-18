INCLUDES = -I$(VULKAN_SDK)/x86_64/include -I../contrib -I../src
CFLAGS = $(OPTFLAGS) $(shell pkg-config --cflags sdl2)
CXXFLAGS = $(CFLAGS) -std=c++14
DEFINES = -D_DEBUG
LDFLAGS = -L$(VULKAN_SDK)/x86_64/lib -lvulkan $(shell pkg-config --libs sdl2)
LD_LIBRARY_PATH = $(VULKAN_SDK)/x86_64/lib
BUILDDIR = ./build
TARGET = VkPlayground

SOURCES = \
	../contrib/stb_image/stb_image.c \
	../src/renderer/vulkan/Base.cpp \
	../src/renderer/vulkan/Buffers.cpp \
	../src/renderer/vulkan/CmdBuffer.cpp \
	../src/renderer/vulkan/Device.cpp \
	../src/renderer/vulkan/Image.cpp \
	../src/renderer/vulkan/Pipeline.cpp \
	../src/renderer/vulkan/Validation.cpp \
	../src/renderer/vulkan/VkMemAlloc.cpp \
	../src/renderer/Camera.cpp \
	../src/renderer/CameraDirector.cpp \
	../src/renderer/Font.cpp \
	../src/renderer/GameTexture.cpp \
	../src/renderer/RenderContext.cpp \
	../src/renderer/TextureManager.cpp \
	../src/Application.cpp \
	../src/DebugOverlay.cpp \
	../src/InputHandlers.cpp \
	../src/main.cpp \
	../src/Math.cpp \
	../src/Utils.cpp

TMPOBJS = $(SOURCES:.cpp=.o)
OBJS = $(TMPOBJS:.c=.o)

OBJS := $(patsubst %,$(BUILDDIR)/%,$(OBJS))

debug: $(OBJS)
	$(CXX) $(OBJS) $(INCLUDES) $(CXXFLAGS) $(LDFLAGS) -o $(TARGET)

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) -c $(INCLUDES) $(CXXFLAGS) $(DEFINES) $< -o $@

$(BUILDDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) -c $(INCLUDES) $(CFLAGS) $(DEFINES) $< -o $@

$(BUILDDIR)/%.d: %.cpp
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CXX) -MM $(INCLUDES) $(CXXFLAGS) $(DEFINES) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(BUILDDIR)/$(<:.cpp=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

$(BUILDDIR)/%.d: %.c
	@echo Resolving dependencies of $<
	@mkdir -p $(@D)
	@$(CC) -MM $(INCLUDES) $(CFLAGS) $(DEFINES) $< > $@.$$$$; \
	sed 's,.*\.o[ :]*,$(BUILDDIR)/$(<:.c=.o) $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

ifneq "$(MAKECMDGOALS)" "clean"
-include $(OBJS:.o=.d)
endif

.PHONY: clean debug release
