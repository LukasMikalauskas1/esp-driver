#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

#define MIN(a, b) (((a) <= (b)) ? (a) : (b))
#define BULK_EP_OUT 0x01
#define BULK_EP_IN 0x81
#define MAX_PKT_SIZE 512

#define VID 0x10C4
#define PID 0xEA60

static struct usb_device *device;
static struct usb_class_driver class;
static unsigned char bulk_buf[MAX_PKT_SIZE];

// Test writing - echo "Test data" | sudo tee /dev/esp_0
// Test reading - sudo cat /dev/esp_0

static int esp_open(struct inode *inode, struct file *file)
{
        return 0;
}
static int esp_close(struct inode *inode, struct file *file)
{
        return 0;
}

static ssize_t esp_read(struct file *file, char __user *buf, size_t cnt, loff_t *off)
{
        int retval;
        int read_cnt;

        printk(KERN_INFO ">>>>>>>>>>READING<<<<<<<<<<");

        /* Read the data from the bulk endpoint */
        retval = usb_bulk_msg(device, usb_rcvbulkpipe(device, BULK_EP_IN), bulk_buf, MAX_PKT_SIZE, &read_cnt, 100);
        if (retval)
        {
                printk(KERN_ERR "Bulk message returned %d\n", retval);
                return retval;
        }
        if (copy_to_user(buf, bulk_buf, MIN(cnt, read_cnt)))
        {
                return -EFAULT;
        }

        return MIN(cnt, read_cnt);
}

static ssize_t esp_write(struct file *f, const char __user *buf, size_t cnt,
                         loff_t *off)
{
        printk(KERN_INFO ">>>>>>>>>>WRITING<<<<<<<<<<");

        int retval;
        int wrote_cnt = MIN(cnt, MAX_PKT_SIZE);

        if (copy_from_user(bulk_buf, buf, MIN(cnt, MAX_PKT_SIZE)))
        {
                return -EFAULT;
        }

        /* Write the data into the bulk endpoint */
        retval = usb_bulk_msg(device, usb_sndbulkpipe(device, BULK_EP_OUT), bulk_buf, MIN(cnt, MAX_PKT_SIZE), &wrote_cnt, 5000);
        if (retval)
        {
                printk(KERN_ERR "Bulk message returned %d\n", retval);
                return retval;
        }

        return wrote_cnt;
}

static struct file_operations fops =
    {
        .owner = THIS_MODULE,
        .open = esp_open,
        .release = esp_close,
        .read = esp_read,
        .write = esp_write,
};

static int esp_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
        int retval = 0;

        device = interface_to_usbdev(interface);

        class.name = "esp_%d";
        class.fops = &fops;

        if ((retval = usb_register_dev(interface, &class)) < 0)
        {
                /* Something prevented us from registering this driver */
                printk(KERN_ERR "Not able to get a minor for this device.");
        }
        else
        {
                printk(KERN_INFO "Minor obtained: %d\n", interface->minor);
        }

        printk(KERN_INFO "USB-TESTING-MODULE: device connected: Vendor ID = 0x%04X, Product ID = 0x%04X\n", id->idVendor, id->idProduct);
        printk(KERN_INFO "USB-TESTING-MODULE: DEVICE IS NOT ESP\n");

        return retval;
}

static void esp_disconnect(struct usb_interface *interface)
{
        usb_deregister_dev(interface, &class);
}

/* Table of devices that work with this driver */
static struct usb_device_id esp_table[] =
    {
        {USB_DEVICE(VID, PID)},
        {} /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, esp_table);

static struct usb_driver esp_driver =
    {
        .name = "esp_driver",
        .probe = esp_probe,
        .disconnect = esp_disconnect,
        .id_table = esp_table,
};

static int __init esp_init(void)
{
        int result;

        /* Register this driver with the USB subsystem */
        if ((result = usb_register(&esp_driver)))
        {
                printk(KERN_ERR "usb_register failed. Error number %d", result);
        }
        return result;
}

static void __exit esp_exit(void)
{
        /* Deregister this driver with the USB subsystem */
        usb_deregister(&esp_driver);
}

module_init(esp_init);
module_exit(esp_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("");
MODULE_DESCRIPTION("");