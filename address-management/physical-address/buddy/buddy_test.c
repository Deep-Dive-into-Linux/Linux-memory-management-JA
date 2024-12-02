#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

static struct page *page;
static unsigned int order = 10;

static int __int buddy_test_init(void)
{
	unsigned int flags = 0;
	flags |= __GFP_DMA32;
	page = alloc_pages(flags, order);
	return 0;
}

static void __exit buddy_test_exit(void)
{
	__free_pages(page, order);
}

module_init(buddy_test_init);
module_exit(buddy_test_exit);
MODULE_AUTHOR("YOSHI");
MODULE_DESCRIPTION("Buddy System");
MODULE_LICENSE("GPL");
