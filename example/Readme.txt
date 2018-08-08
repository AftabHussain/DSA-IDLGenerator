#General Toy Example:
chengli.c

#Kernel Toy Example:
device.h
my_device.c
device_manager.c

Generate a single .bc file for above:
clang -O1 -g -emit-llvm device_manager.c -c -o device_manager.bc && clang -O1 -g -emit-llvm my_device.c -c -o my_device.bc && llvm-link my_device.bc device_manager.bc -o merged_mgr_mydvc.bc 

