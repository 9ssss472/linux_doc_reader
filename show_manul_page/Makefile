CROSSCOMPILE := arm-linux-gnueabihf-

CFLAGS 	:= -Wall -O2 -c
CFLAGS  += -I$(PWD)/include 

# for 100ask imx6ull
CFLAGS += -I /home/book/100ask_imx6ull-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/arm-buildroot-linux-gnueabihf/sysroot/usr/include/freetype2

# for 100ask stm32mp157
CFLAGS += -I /home/book/100ask_stm32mp157_pro-sdk/ToolChain/arm-buildroot-linux-gnueabihf_sdk-buildroot/arm-buildroot-linux-gnueabihf/sysroot/usr/include/freetype2

LDFLAGS := -lm -lfreetype -lts -lpthread -ljpeg

CC 	:= $(CROSSCOMPILE)gcc
LD 	:= $(CROSSCOMPILE)ld

OBJS := main.o \
			dsp/display.o        \
			dsp/fb.o                  \
			encoding/ascil.o              \
			encoding/utf16be.o           \
			encoding/encoding_manager.o   \
			encoding/utf-8.o              \
			encoding/utf16le.o           \
			draw/draw.o                   \
			fonts/ascil.o                 \
			fonts/GBK.o                   \
			fonts/freetype.o              \
			fonts/fonts_manager.o		\
			input/input_manager.o		\
			input/stdin.o				\
			input/touchScreen.o			\
			debug/debug_manager.o		\
			debug/netprint.o			\
			debug/stdout.o				\
			pic/bmp.o					\
			pic/merge.o					\
			pic/zoom.o					\
			page/file.o					\
			page/main_page.o				\
			page/page_manager.o			\
			page/render.o				\
			page/setting_page.o			\
			page/interval_page.o		\
			page/browse_page.o			\
			page/auto_page.o			\
			pic/jpg.o					\
			pic/picfmt_manager.o		\
			page/manul_page.o

all: $(OBJS)
	$(CC) $(LDFLAGS) -o show_file $^

clean:
	rm -f show_file
	rm -f $(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) -o $@ $<

