/**LCD**/

#include "stdio.h"
#include "device.h"


/*Fn writes on data*/
static int mydev_init(struct packet *p){
        p->size = 0;
        p->data = 0;
	return 1;
}

/*Fn writes on data*/
static void mydev_uninit(struct device_operations *devops){
	devops->devop_init_registered = 0;
}

/*Fn reads data*/
static int readData(struct device_operations *devops){
	if (devops->devop_init_registered>0){
		return 1;
	}
	else {
		return 0;
	}
}


//int static fn_n(struct device_operations *devops){
//	devops->device_info = "another device";
//	readAndWriteData(&devops);
//	return 0;
//}

/*Fn reads and writes data*/
static void readAndWriteData(struct device_operations *devops){
	
	if (devops->devop_init_registered>0){
		 devops->device_info = "new device";	
	}
}

int fn_n(struct device_operations *devops){

	 devops->device_info = "another device";

	// if (devops->devop_init_registered>0){
	//	 devops->device_info = "new device";	
	//}

	//plain fn call -> plain fn call, all in same translation unit
	/*If we have the following line, the write on device_info via readAndWriteData is reported in the projection of fn_n*/
//	readAndWriteData(devops); 

	//FP to function, in same translation unit
	/*If we have the following line, then the write on device_info via readAndWriteData is NOT reported in the projection of fn_n*/
//	devops->dop_read_write(devops);

	return 0;
}

int main() {
	
        /* 
         * struct device (LCD Data) is defined in device.h
         */	
		struct device my_printer;

        /* 
         * reg_device is an environment fn, defined in device.h
         * as a prototype
         */
        my_printer.set_device = reg_device;

	/*
	 * An invocation to the environment from the LCD
	 */
	my_printer.set_device(&my_printer,"PRINTER");

	/*
	 * initializing some fields of devops struct
	 */
	static struct device_operations devops = {
		.dop_init	= mydev_init,
		.dop_uninit	= mydev_uninit,
		.dop_read	= readData,
		.dop_read_write = readAndWriteData,
	};

	/* Below we make function calls. Since these are calls from main,		
	 * their Rs/Ws would be registered as bu.globals in the IDL. ( not all calls see below)
	 */

	/* 
         * reg_devops is an environment fn, defined in device.h
         * as a prototype (it is static)
     */
	/*Also dsa generator generates a segfault at 
	#0  getNodeForValue (V=0x0, this=0x109ee20) at /local/devel/DataStructureAnalysis/include/dsa/DSGraph.h:373
#1  dsa::DSAGenerator::runOnModule (this=0x1089860, m=...)
    at /local/devel/DataStructureAnalysis/lib/dsaGenerator/DSAGenerator.cpp:518
#2  0x0000000000c33880 in llvm::legacy::PassManagerImpl::run(llvm::Module&) ()
#3  0x000000000050a50a in main (argc=<optimized out>, argv=<optimized out>)
 if the following is not used. (most likely the use of &devops)*/
	
	//1. is not reported in globals since readAndWriteData is a fp.
	reg_devops(readAndWriteData,&devops);

	//2. 
	// fn_n(&devops);


}
