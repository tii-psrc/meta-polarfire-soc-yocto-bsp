#include <linux/module.h>
#include <linux/uio_driver.h>
#include <linux/platform_device.h>
#include <linux/of.h> /* For struct of_device_id */

#define RINGBUF_BASE 0xC0000000UL
#define RINGBUF_SIZE 0x4000000UL

static struct uio_info ringbuf_info = {
    .name = "polarfire-ringbuf",
    .version = "1.0",
    .mem[0].name = "ringbuf-mem",
    .mem[0].addr = RINGBUF_BASE,
    .mem[0].size = RINGBUF_SIZE,
    .mem[0].memtype = UIO_MEM_PHYS,
};

static int ringbuf_probe(struct platform_device *pdev)
{
    return uio_register_device(&pdev->dev, &ringbuf_info);
}

static void ringbuf_remove(struct platform_device *pdev)
{
    uio_unregister_device(&ringbuf_info);
}

#ifdef CONFIG_OF
static const struct of_device_id ringbuf_of_match[] = {
    { .compatible = "microchip,uio-ringbuf" },
    { }
};
MODULE_DEVICE_TABLE(of, ringbuf_of_match);
#else
#define ringbuf_of_match NULL
#endif

static struct platform_driver ringbuf_driver = {
    .probe = ringbuf_probe,
    .remove = ringbuf_remove,
    .driver = {
        .name = "uio-ringbuf",
        .of_match_table = ringbuf_of_match,
    },
};

static int __init ringbuf_init(void)
{
    return platform_driver_register(&ringbuf_driver);
}

static void __exit ringbuf_exit(void)
{
    platform_driver_unregister(&ringbuf_driver);
}

module_init(ringbuf_init);
module_exit(ringbuf_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mukesh Jha");
MODULE_DESCRIPTION("UIO driver for DPU ring buffer");