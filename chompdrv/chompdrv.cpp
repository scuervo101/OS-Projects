
#include <libusb.h>
#include <cstdio>
#include <linux/uinput.h>
#include <linux/input.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

union Byte
{
    unsigned char byte;

    struct
    {
        bool bit0 : 1;
        bool bit1 : 1;
        bool bit2 : 1;
        bool bit3 : 1;
        bool bit4 : 1;
        bool bit5 : 1;
        bool bit6 : 1;
        bool bit7 : 1;
    };
};

void emit(int fd, int type, int code, int val)
{
	struct input_event ie;

	ie.type = type;
	ie.code = code;
	ie.value = val;
 
	write(fd, &ie, sizeof(ie));
}

int main()
{
	struct uinput_setup usetup;
	libusb_device_handle* joystick;
	uint16_t vendorID = 0x9A7A;
	uint16_t productID = 0xBA17;
	libusb_context *ctx = NULL;
	
	libusb_init(&ctx);
	
	joystick = libusb_open_device_with_vid_pid(ctx, vendorID, productID);
	
	if(joystick == NULL)
	{
		printf("Device failed to open \n");
		return -1;
	}
	printf("Device has been opened! \n");

	
	if(libusb_claim_interface(joystick, 0) < 0)
	{
		printf("Cannot claim interface \n");
		return -1;
	}
	
	
	int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	
	ioctl(fd, UI_SET_EVBIT, EV_KEY);
	ioctl(fd, UI_SET_KEYBIT, BTN_JOYSTICK);
	
	ioctl(fd, UI_SET_EVBIT, EV_ABS);
	ioctl(fd, UI_SET_ABSBIT, ABS_X);
	ioctl(fd, UI_SET_ABSBIT, ABS_Y);
	
	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = vendorID;
	usetup.id.product = productID;
	strcpy(usetup.name, "ChompStick device");
	
	ioctl(fd, UI_DEV_SETUP, &usetup);
	ioctl(fd, UI_DEV_CREATE);
	
	int actual;
	unsigned char* data = new unsigned char[2];
	data[0] = '0';
	data[1] = '\0';
	
	int r =0;
	while(r == 0)
	{
		r = libusb_bulk_transfer(joystick, (0x81 | LIBUSB_ENDPOINT_OUT), data, 2, &actual, 0);
	
		Byte b;
	
		b.byte = data[0];
	
		int button = b.bit4;
	
		int yaxis = b.bit0;
		int xaxis = b.bit2;
	
		if(b.bit1 == 1)
			yaxis += 2;
	
		if(b.bit3 == 1)
			xaxis += 2;
		
		if(button == 1)
		{
			emit(fd, EV_KEY, BTN_JOYSTICK, 1);
			emit(fd, EV_SYN, SYN_REPORT, 0);
		}
		else
		{
			emit(fd, EV_KEY, BTN_JOYSTICK, 0);
			emit(fd, EV_SYN, SYN_REPORT, 0);
		}
		
		if(xaxis == 3)
		{
			emit(fd, EV_ABS, ABS_X, 32767);
			emit(fd, EV_SYN, SYN_REPORT, 0);	
		}
		else if(xaxis == 1)
		{
			emit(fd, EV_ABS, ABS_X, -32767);
			emit(fd, EV_SYN, SYN_REPORT, 0);
		}
		else
		{
			emit(fd, EV_ABS, ABS_X, 0);
			emit(fd, EV_SYN, SYN_REPORT, 0);
		}
		
		if(yaxis == 3)
		{
			emit(fd, EV_ABS, ABS_Y, -32767);
			emit(fd, EV_SYN, SYN_REPORT, 0);	
		}
		else if(yaxis == 1)
		{
			emit(fd, EV_ABS, ABS_Y, 32767);
			emit(fd, EV_SYN, SYN_REPORT, 0);
		}
		else
		{
			emit(fd, EV_ABS, ABS_Y, 0);
			emit(fd, EV_SYN, SYN_REPORT, 0);
		}
		
		
		printf("Button: %d YAxis: %d XAxis: %d \n", button, yaxis, xaxis);
	}
	
	
	if(libusb_release_interface(joystick, 0) != 0)
	{
		printf("Cannot release interface \n");
		return -1;
	}
	printf("Release \n");
	
	libusb_close(joystick);
	libusb_exit(ctx);
	ioctl(fd, UI_DEV_DESTROY);
	close(fd);
	
}
