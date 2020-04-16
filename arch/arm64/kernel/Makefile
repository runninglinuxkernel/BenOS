#
# Makefile for the linux kernel.
#

arm64-obj-y		:= uart.o mm.o utils.o


obj-y					+= $(arm64-obj-y) 
obj-m					+= $(arm64-obj-m)
head-y					:= head.o
extra-y					:= $(head-y) vmlinux.lds

# vDSO - this must be built first to generate the symbol offsets
