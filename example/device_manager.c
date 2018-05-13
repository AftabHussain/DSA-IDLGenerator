/**ENVIRONMENT (simulates the kernel)**/

#include "device.h"
#include "string.h"

/* 
 * Environment Data
 */
typedef struct device_manager {
	char *log [10];
	int dev_req_count;
	int dev_reg_count;
} device_manager;

device_manager dm;

/*
 * Invoked by the LCD
 * It updates both Environment and LCD data
 */
//void reg_device(char *name, struct device *dev){
 void reg_device(struct device *dev, char *name){

	//read/write LCD data (cross-domain)
	dev->name = name;
	
	//write Env data (intra-domain)
	dm.log[dm.dev_req_count] = name;	

	//read/write LCD data (cross-domain)
//	strcpy(dev->status,"ready");

	//write Env data (intra-domain)
//	dm.dev_req_count++;

	//read/write LCD data (cross-domain)
//	dev->system_id = (dev->id * dm.dev_req_count)/1234;
//	dev->system_id = dev->id * 88;

}

int foobar(struct device_operations *devop) {
   devop->device_info = "write sth";
   return 0;
}

//void acceptFP(int (*my_fp)(struct device_operations *)){
//   int (*local_fp)(struct device_operations *);
//   local_fp=my_fp;
//}


/*
 * Also invoked by the LCD.
 * It calls LCD functions via the *devops struct function
 * pointers which updates the LCD data (other fields of devops struct).
 * It also updates Environment data dev_reg_count	
 */
int reg_devops(void(*fp)(struct device_operations *),struct device_operations *devops){
	
//	struct packet p; 
//	dm.dev_reg_count++;
//	devops->devop_init_registered = 1;
//	devops->dop_init(&p);

	/*FAILS TO REPORT - FP IS STRUCT FIELD*/
	//CASE 1 - FP to functin outside translation unit - FP is a field of a struct var
	/*The following calls readAndWriteData in my_device.c, which writes on device_info
	field of devops. However, dsa generator is unable to report it, inside the projection
	of reg_devops. Instead it reports an rpc to the function pointer in the read section (regardless of whether the devops struct field inside the function pointed to by this FP is being written/read) -- calls the function via accessing function pointer field of devops*/
	// devops->dop_read_write(devops);	
	// devops->dop_read(devops);

	/*FAILS TO REPORT - FP IS PARAM*/
	//CASE 2 - FP to function outside translation unit (fp is supplied as an arg) - Use FP directly
	/*The following calls readAndWriteData in my_device.c, which writes on device_info
	field of devops. However, dsa generator is unable to report it, inside the projection
	of reg_devops -- calls the function directly via the function pointer */
	// fp(devops);

	/*SUCCESSFULLY REPORTS - DIRECT FN CALL INSIDE TRANSLATION UNIT*/
	//CASE 3 - Plain Function call inside the translation unit
	/*The following calls an internal function, foobar, in the same compilation unit.
	DSA correctly reports device_info being written on inside the projection of reg_devops.*/
//        foobar(devops); 	
	
	/*SUCCESSFULLY REPORTS - FP CALL INSIDE THE TRANSLATION UNIT*/
	//CASE 4 - FP to function inside the translation unit - Use FP directly
	/*The following calls an internal function, foobar, in the same compilation unit.
	DSA correctly reports that device_info is being written into inside reg_devops's projection*/
	int (*new_fp)(struct device_operations *);
	// new_fp = &foobar;
	// new_fp(devops);

	//CASE 5 - FP to function inside the translation unit - FP is a struct field
	/*same as case 1, see simple.c*/

	//CASE 6 - Plain Function call to function outside the translation unit.
	//Correctly reports whatever fn_n is doing - the above cases are recursively observed.
	//fn_n(devops);

	//CASE 7  - FP call to function outside the translation unit - use FP directly(FP assigned as an argument here)
	/*DSA Correctly reports whatever fn_n is doing| note that new_fp is assigned
	to fn_n in this translation unit. */

	new_fp = &fn_n;
	new_fp(devops);
		

	return 1;
};
