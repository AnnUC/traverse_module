#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
MODULE_LICENSE("GPL");
//#include "traverse_module_1.h"
#include <linux/dram_interface.h>

struct free_chunk_info* traverse_func (void* arena_start_ptr, size_t VpageNO, size_t* len) {
	return NULL;
}

static int __init traverse_module_init(void)
{
	//traverse (void* arena_start_ptr, size_t VpageNO, size_t* len);
	printk(KERN_INFO "traverse_init");
	register_traverse(0,traverse_func);
	printk(KERN_INFO "error in traverse");
	return 0;
}

static void __exit traverse_module_exit(void)
{
	return 0;
}

module_init(traverse_module_init);
module_exit(traverse_module_exit);
