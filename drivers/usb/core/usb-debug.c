/*
 * debug.c - USB debug helper routines.
 *
 * I just want these out of the way where they aren't in your
 * face, but so that you can still use them..
 */
#define CONFIG_USB_DEBUG
#if 0
#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/slab.h>
#ifdef CONFIG_USB_DEBUG
    #define DEBUG
#else
    #undef DEBUG
#endif
#include <linux/usb.h>
#else
#include "../usb_wrapper.h"
#endif

#include "lib/LPCMod/xblastDebug.h"

static void usb_show_endpoint(struct usb_host_endpoint *endpoint)
{
    usb_show_endpoint_descriptor(&endpoint->desc);
}

static void usb_show_interface(struct usb_host_interface *altsetting)
{
    int i;

    usb_show_interface_descriptor(&altsetting->desc);

    for (i = 0; i < altsetting->desc.bNumEndpoints; i++)
        usb_show_endpoint(altsetting->endpoint + i);
}

static void usb_show_config(struct usb_host_config *config)
{
    int i, j;
    struct usb_interface *ifp;

    usb_show_config_descriptor(&config->desc);
    for (i = 0; i < config->desc.bNumInterfaces; i++) {
        ifp = config->interface + i;

        if (!ifp)
            break;

        debugSPIPrint("\n  Interface: %d", i);
        for (j = 0; j < ifp->num_altsetting; j++)
            usb_show_interface(ifp->altsetting + j);
    }
}

void usb_show_device(struct usb_device *dev)
{
    int i;

    usb_show_device_descriptor(&dev->descriptor);
    for (i = 0; i < dev->descriptor.bNumConfigurations; i++)
        usb_show_config(dev->config + i);
}

/*
 * Parse and show the different USB descriptors.
 */
void usb_show_device_descriptor(struct usb_device_descriptor *desc)
{
    if (!desc)
    {
        debugSPIPrint("Invalid USB device descriptor (NULL POINTER)");
        return;
    }
    debugSPIPrint("  Length              = %2d%s", desc->bLength,
        desc->bLength == USB_DT_DEVICE_SIZE ? "" : " (!!!)");
    debugSPIPrint("  DescriptorType      = %02x", desc->bDescriptorType);

    debugSPIPrint("  USB version         = %x.%02x",
        desc->bcdUSB >> 8, desc->bcdUSB & 0xff);
    debugSPIPrint("  Vendor:Product      = %04x:%04x",
        desc->idVendor, desc->idProduct);
    debugSPIPrint("  MaxPacketSize0      = %d", desc->bMaxPacketSize0);
    debugSPIPrint("  NumConfigurations   = %d", desc->bNumConfigurations);
    debugSPIPrint("  Device version      = %x.%02x",
        desc->bcdDevice >> 8, desc->bcdDevice & 0xff);

    debugSPIPrint("  Device Class:SubClass:Protocol = %02x:%02x:%02x",
        desc->bDeviceClass, desc->bDeviceSubClass, desc->bDeviceProtocol);
    switch (desc->bDeviceClass) {
    case 0:
        debugSPIPrint("    Per-interface classes");
        break;
    case USB_CLASS_AUDIO:
        debugSPIPrint("    Audio device class");
        break;
    case USB_CLASS_COMM:
        debugSPIPrint("    Communications class");
        break;
    case USB_CLASS_HID:
        debugSPIPrint("    Human Interface Devices class");
        break;
    case USB_CLASS_PRINTER:
        debugSPIPrint("    Printer device class");
        break;
    case USB_CLASS_MASS_STORAGE:
        debugSPIPrint("    Mass Storage device class");
        break;
    case USB_CLASS_HUB:
        debugSPIPrint("    Hub device class");
        break;
    case USB_CLASS_VENDOR_SPEC:
        debugSPIPrint("    Vendor class");
        break;
    default:
        debugSPIPrint("    Unknown class");
    }
}

void usb_show_config_descriptor(struct usb_config_descriptor *desc)
{
    debugSPIPrint("Configuration:");
    debugSPIPrint("  bLength             = %4d%s", desc->bLength,
        desc->bLength == USB_DT_CONFIG_SIZE ? "" : " (!!!)");
    debugSPIPrint("  bDescriptorType     =   %02x", desc->bDescriptorType);
    debugSPIPrint("  wTotalLength        = %04x", desc->wTotalLength);
    debugSPIPrint("  bNumInterfaces      =   %02x", desc->bNumInterfaces);
    debugSPIPrint("  bConfigurationValue =   %02x", desc->bConfigurationValue);
    debugSPIPrint("  iConfiguration      =   %02x", desc->iConfiguration);
    debugSPIPrint("  bmAttributes        =   %02x", desc->bmAttributes);
    debugSPIPrint("  bMaxPower            = %4dmA", desc->bMaxPower * 2);
}

void usb_show_interface_descriptor(struct usb_interface_descriptor *desc)
{
    debugSPIPrint("  Alternate Setting: %2d", desc->bAlternateSetting);
    debugSPIPrint("    bLength             = %4d%s", desc->bLength,
        desc->bLength == USB_DT_INTERFACE_SIZE ? "" : " (!!!)");
    debugSPIPrint("    bDescriptorType     =   %02x", desc->bDescriptorType);
    debugSPIPrint("    bInterfaceNumber    =   %02x", desc->bInterfaceNumber);
    debugSPIPrint("    bAlternateSetting   =   %02x", desc->bAlternateSetting);
    debugSPIPrint("    bNumEndpoints       =   %02x", desc->bNumEndpoints);
    debugSPIPrint("    bInterface Class:SubClass:Protocol =   %02x:%02x:%02x",
        desc->bInterfaceClass, desc->bInterfaceSubClass, desc->bInterfaceProtocol);
    debugSPIPrint("    iInterface          =   %02x", desc->iInterface);
}

void usb_show_endpoint_descriptor(struct usb_endpoint_descriptor *desc)
{
    char *LengthCommentString = (desc->bLength ==
        USB_DT_ENDPOINT_AUDIO_SIZE) ? " (Audio)" : (desc->bLength ==
        USB_DT_ENDPOINT_SIZE) ? "" : " (!!!)";
    char *EndpointType[4] = { "Control", "Isochronous", "Bulk", "Interrupt" };

    debugSPIPrint("    Endpoint:");
    debugSPIPrint("      bLength             = %4d%s",
        desc->bLength, LengthCommentString);
    debugSPIPrint("      bDescriptorType     =   %02x", desc->bDescriptorType);
    debugSPIPrint("      bEndpointAddress    =   %02x (%s)", desc->bEndpointAddress,
        (desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) ==
            USB_ENDPOINT_XFER_CONTROL ? "i/o" :
        (desc->bEndpointAddress & USB_ENDPOINT_DIR_MASK) ? "in" : "out");
    debugSPIPrint("      bmAttributes        =   %02x (%s)", desc->bmAttributes,
        EndpointType[USB_ENDPOINT_XFERTYPE_MASK & desc->bmAttributes]);
    debugSPIPrint("      wMaxPacketSize      = %04x", desc->wMaxPacketSize);
    debugSPIPrint("      bInterval           =   %02x", desc->bInterval);

    /* Audio extensions to the endpoint descriptor */
    if (desc->bLength == USB_DT_ENDPOINT_AUDIO_SIZE) {
        debugSPIPrint("      bRefresh            =   %02x", desc->bRefresh);
        debugSPIPrint("      bSynchAddress       =   %02x", desc->bSynchAddress);
    }
}

void usb_show_string(struct usb_device *dev, char *id, int index)
{
    char *buf;

    if (!index)
        return;
    if (!(buf = kmalloc(256, GFP_KERNEL)))
        return;
    if (usb_string(dev, index, buf, 256) > 0)
        dev_printk(KERN_INFO, &dev->dev, "%s: %s", id, buf);
    kfree(buf);
}

void usb_dump_urb (struct urb *urb)
{
    debugSPIPrint ("urb                   :%p", urb);
    debugSPIPrint ("dev                   :%p", urb->dev);
    debugSPIPrint ("pipe                  :%08X", urb->pipe);
    debugSPIPrint ("status                :%d", urb->status);
    debugSPIPrint ("transfer_flags        :%08X", urb->transfer_flags);
    debugSPIPrint ("transfer_buffer       :%p", urb->transfer_buffer);
    debugSPIPrint ("transfer_buffer_length:%d", urb->transfer_buffer_length);
    debugSPIPrint ("actual_length         :%d", urb->actual_length);
    debugSPIPrint ("setup_packet          :%p", urb->setup_packet);
    debugSPIPrint ("start_frame           :%d", urb->start_frame);
    debugSPIPrint ("number_of_packets     :%d", urb->number_of_packets);
    debugSPIPrint ("interval              :%d", urb->interval);
    debugSPIPrint ("error_count           :%d", urb->error_count);
    debugSPIPrint ("context               :%p", urb->context);
    debugSPIPrint ("complete              :%p", urb->complete);
}

