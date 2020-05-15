#
# Makefile for the linux kernel.
#

arm64-obj-$(CONFIG_PI_MINI_UART)        += mini_uart.o
arm64-obj-$(CONFIG_PI_PL_UART)        += uart.o

obj-y					+= $(arm64-obj-y) 
obj-m					+= $(arm64-obj-m)
