#if !defined(__DEVICE_H__)
#define __DEVICE_H__ 

struct packet {
   int size;
   char *data;
};

struct device {
	char *name;
	char status[10];
	//void (*set_device)(char *dev_name, struct device *dev);	
	void (*set_device)(struct device *dev, char *dev_name);	
	float id;
	float system_id;
};

struct device_operations {
	
	/*FP arg is a struct var of struct packet*/
	int	(*dop_init)(struct packet *p);

	/*FP arg is a struct var of FP's parent struct*/
	void	(*dop_uninit)(struct device_operations *dev);
	void	(*dop_read_write)(struct device_operations *dev);
	int	(*dop_read)(struct device_operations *dev);
	
	int	devop_init_registered;
	char 	*device_info;
};


//Current DSA does not report projections of these functions, if they are static.

void reg_device(struct device *dev, char *name);

int reg_devops(void(*fp)(struct device_operations *),struct device_operations *devops);

int fn_n(struct device_operations *devops);

#endif
