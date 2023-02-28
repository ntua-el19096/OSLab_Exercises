#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xf1fbc9ab, "module_layout" },
	{ 0x2cc4be63, "cdev_del" },
	{ 0xd1ed3dd3, "kmalloc_caches" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x247a366f, "cdev_init" },
	{ 0xf09b5d9a, "get_zeroed_page" },
	{ 0xe2ec6b65, "param_ops_int" },
	{ 0x3fd78f3b, "register_chrdev_region" },
	{ 0x6bd0e573, "down_interruptible" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x953e1b9e, "ktime_get_real_seconds" },
	{ 0x83fc992e, "pv_ops" },
	{ 0xa1d71d3d, "nonseekable_open" },
	{ 0xa120d33c, "tty_unregister_ldisc" },
	{ 0xd9a5ea54, "__init_waitqueue_head" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0xc5850110, "printk" },
	{ 0xa1c76e0a, "_cond_resched" },
	{ 0xfe487975, "init_wait_entry" },
	{ 0x65b3f06, "cdev_add" },
	{ 0xc6cbbc89, "capable" },
	{ 0xc959d152, "__stack_chk_fail" },
	{ 0x1000e51, "schedule" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5c599162, "kmem_cache_alloc_trace" },
	{ 0xba8fbd64, "_raw_spin_lock" },
	{ 0x4302d0eb, "free_pages" },
	{ 0x3eeb2322, "__wake_up" },
	{ 0x8c26d495, "prepare_to_wait_event" },
	{ 0x37a0cba, "kfree" },
	{ 0xcf2a6966, "up" },
	{ 0x92540fbf, "finish_wait" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xd91b6d54, "tty_register_ldisc" },
	{ 0x88db9f48, "__check_object_size" },
};

MODULE_INFO(depends, "");

