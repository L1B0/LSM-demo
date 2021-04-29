#include <linux/security.h>
#include <linux/module.h>
#include <linux/lsm_hooks.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/dcache.h>
#include <asm/fcntl.h>
#include <asm/processor.h>
#include <asm/uaccess.h>

#include <linux/uidgid.h>
#include <linux/cred.h>
#include <linux/sched.h>

#define LSMConfigPath "/etc/lsm/config"

#define PASS 0
#define NOPASS EINVAL

// copy from lsm_init.c 
// start
#define UserConfigPath "/etc/lsm/user_config"
#define RoleConfigPath "/etc/lsm/role_config"

#define SYSCALL_MKDIR 1
#define SYSCALL_RENAME 2
#define SYSCALL_RMDIR   4

char role_list[3][10] = { "MKDIR\0", "RENAME\0", "RMDIR\0"};

// end

int mystrlen(char *s)
{
	int ret = 0;
	while(s[ret] != '\0') ret++;
	
	return ret;
}

int str2int(char *s)
{
	int i, ret=0;
	for(i=0;i<mystrlen(s);i++)
	{
		ret = ret*10 + (s[i]-'0');
	}
	
	//printk("str2int-uid: %s %d\n",s, ret);
	return ret;
}

int check_user_and_role(void)
{
	printk(KERN_WARNING "[Check User and Role]\n");
	
	// get current user id
 	
	struct user_struct *user=get_current_user();
	int cur_user = user->uid.val;
	printk("current uid: %d\n", cur_user);

	// check user
        int i=0;
	char flag=0, buf[1024], cur_role[50]={0};
	char *linestart;	

	mm_segment_t oldfs;

	struct file *f=filp_open(UserConfigPath,O_RDONLY,0);
	if( IS_ERR(f) || (f==NULL))
	{
		printk(KERN_WARNING "get user config error.\n");
		return 0;
	}

	oldfs =get_fs();
	set_fs(KERNEL_DS);
	
	linestart = buf;
	while(vfs_read(f,buf+i,1,&f->f_pos)==1)
	{
		if(buf[i] == ':')
		{
			buf[i] = '\0'; // cut it
			
			if(str2int(linestart) == cur_user)
			{
				flag = 1;
				linestart = buf+i+1;
			}
		}
		else if(buf[i] == '\n')
		{
			if(flag)
			{
				buf[i-1] = '\0';
				printk("cur_role: %s\n",linestart);
				strcpy(cur_role, linestart); 
			}
			else
			{
				linestart = buf+i+1;
			}
		}
		i ++;	
	}
	set_fs(oldfs);
	filp_close(f,0);

	// unsupported user id
	if(flag == 0) return 0;
	
	// Check role and calc auth
	i = 0;
	flag = 0;
        int cur_auth=0;

       	f =filp_open(RoleConfigPath,O_RDONLY,0);
        if( IS_ERR(f) || (f==NULL))
        {
                printk(KERN_WARNING "get role config error.\n");
                return 0;
        }

        oldfs =get_fs();
        set_fs(KERNEL_DS);

        linestart = buf;
        while(vfs_read(f,buf+i,1,&f->f_pos)==1)
	{
		if(buf[i] == ':')
		{
			buf[i] = '\0';
			
			
			if(strcmp(cur_role,linestart) == 0)
			{	
				flag = 1;
				linestart = buf+i+1;
				printk("find cur_role.\n");
			}
		}
		else if(flag == 1 && (buf[i] == ',' || buf[i] == ';'))
		{
			if(buf[i] == ';') flag = 2;

			buf[i] = '\0';
			
			if(strcmp(linestart, "MKDIR") == 0)
			{
				printk("You can mkdir.\n");
				cur_auth |= SYSCALL_MKDIR;
			}
			else if(strcmp(linestart, "RENAME") == 0)
			{
				printk("You can rename.\n");
				cur_auth |= SYSCALL_RENAME;
			}
			else if(strcmp(linestart, "RMDIR") == 0)
			{
				printk("You can rmdir.\n");
				cur_auth |= SYSCALL_RMDIR;
			}
			else
			{
				printk(KERN_WARNING "roleconfig's contents have errors.\n");
			}

			if(flag == 2) break;

			linestart = buf+i+1;

		}
		else if(buf[i] == '\n') linestart = buf+i+1;
		i ++;
	}
	set_fs(oldfs);
        filp_close(f,0);
	
	printk("[Current Auth]: %d\n",cur_auth);	
	return cur_auth;	
}

int check_mkdir_auth(int cur_auth)
{
	printk(KERN_WARNING "[Check Auth]: %d\n",cur_auth & SYSCALL_MKDIR);

	if(cur_auth & SYSCALL_MKDIR)
		return 1;
	else
		return 0;
}

int check_rmdir_auth(int cur_auth)
{
	printk(KERN_WARNING "[Check Auth]: %d\n",cur_auth & SYSCALL_RMDIR);
	
	if(cur_auth & SYSCALL_RMDIR)
                return 1;
        else
                return 0;
}

int check_rename_auth(int cur_auth)
{
	printk(KERN_WARNING "[Check Auth]: %d\n",cur_auth & SYSCALL_RENAME);
	
	if(cur_auth & SYSCALL_RENAME)
                return 1;
        else
                return 0;
}

int check_lsm_switch(void)
{
	printk("[CHECK LSM SWITCH]\n");
	
	char buf;	

	mm_segment_t oldfs;

	struct file *f =filp_open(LSMConfigPath,O_RDONLY,0);
	if( IS_ERR(f) || (f==NULL))
	{
		printk(KERN_WARNING "get lsm config error.\n");
		return 0;
	}

	oldfs =get_fs();
	set_fs(KERNEL_DS);
	
	if(vfs_read(f,&buf,1,&f->f_pos)==1)
	{
		printk("Read LSM config: %c\n",buf);
		if(buf == 'T') return 1;
		else return 0;	
	}
	else
	{
		printk("Read LSM config error.\n");
		return 0;
	}
}

int my_inode_mkdir(struct inode *dir, struct dentry *dentry,
				umode_t mode)
{
	int f = check_lsm_switch();
	if(f == 0) return 0;
	
	printk("[Hook mkdir]\n");
	int cur_auth=0;

	// get and check cur_user_name, then get the role and auth.
	cur_auth = check_user_and_role();
	
	int flag = check_mkdir_auth(cur_auth);
	if(flag)
	{
		printk("You can pass.\n");
		return PASS; /* Check Pass */
	}
	else
	{
		printk("You have no permission.\n");
		return NOPASS;/* Permission denied */
	}
}

int my_inode_rmdir(struct inode *dir, struct dentry *dentry)
{
        int f = check_lsm_switch();
        if(f == 0) return 0;

	printk("[Hook rmdir]\n");
	int cur_auth=0;

        // get and check cur_user_name, then get the role and auth.
        cur_auth = check_user_and_role();

	int flag = check_rmdir_auth(cur_auth);
        if(flag)
        {
		printk("You can pass.\n");
                return PASS; /* Check Pass */
        }
        else
        {
		printk("You have no permission.\n");
                return NOPASS;/* Permission denied */
        }
}

int my_inode_rename(struct inode *old_dir, struct dentry *old_dentry,
				struct inode *new_dir,
				struct dentry *new_dentry)
{
        int f = check_lsm_switch();
        if(f == 0) return 0;

	printk("[Hook rename]\n");
	int cur_auth=0;

        // get and check cur_user_name, then get the role and auth.
        cur_auth = check_user_and_role();

	int flag = check_rename_auth(cur_auth);
        if(flag)
        {
		printk("You can pass.\n");
                return PASS; /* Check Pass */
        }
        else
        {
		printk("You have no permission.\n");
                return NOPASS;/* Permission denied */
        }
}

struct security_hook_list hooks[] =
{
        //LSM_HOOK_INIT(path_rename,my_rename)
	LSM_HOOK_INIT(inode_mkdir, my_inode_mkdir),
	LSM_HOOK_INIT(inode_rmdir, my_inode_rmdir),
	LSM_HOOK_INIT(inode_rename, my_inode_rename)
};

static int lsm_init(void)
{

/*
Ref: https://www.kernel.org/doc/htmldocs/kernel-api/API-security-add-hooks.html
Name
	security_add_hooks — Add a modules hooks to the hook lists.

Synopsis
	void security_add_hooks (	struct security_hook_list * hooks,
 		int count,
 		char * lsm);
 
Arguments
	struct security_hook_list * hooks
		the hooks to add

	int count
		the number of hooks to add

	char * lsm
		the name of the security module	
*/
	printk(KERN_WARNING "[LSM_INIT]\n");
        security_add_hooks(hooks,ARRAY_SIZE(hooks));
	
	return 0;
}

static void lsm_exit(void)
{
        printk(KERN_WARNING "[LSM_EXIT]\n");

	//security_delete_hooks(hooks,1);
	int i, count=ARRAY_SIZE(hooks);
        for (i = 0; i < count; i++)
                list_del_rcu(&hooks[i].list);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("l1b0");
MODULE_DESCRIPTION("A lsm security module demo.");

module_init(lsm_init);
module_exit(lsm_exit);
