# RBAC访问控制实验

[TOC]

> Author：l1b0
## 实验环境

Ubuntu 18.04 64bits
Linux 4.4

## 实验过程

### 1. 创建用户

> sudo adduser user_name # 新建用户user1和user2
> cat /etc/passwd # 查看用户信息

<img src="https://i.loli.net/2021/04/28/CaSXlKgh1BAD3cn.png" alt="image-20210428154711853" style="zoom:67%;" />

### 2. 用户、角色和权限的设计

本次作业我设计了两个用户，分别为user1和user2；以及两个角色fish1和fish2，支持的三种操作为执行、删除和修改文件，其中fish1具有创建和重命名文件的权限，fish2只有删除文件的权限。

<img src="https://i.loli.net/2021/04/28/ORSImVjWpgslPXh.png" alt="image-20210428154720273" style="zoom:67%;" />

<img src="https://i.loli.net/2021/04/28/Bs5ZULjpH8SvEGQ.png" alt="image-20210428153535228" style="zoom:67%;" />

### 3. 用户层程序：角色、用户的管理

角色的信息存储在/etc/lsm/roleconfig，用户的信息存储在/etc/lsm/userconfig，权限只支持三种，即MKDIR、RENAME和RENAME，如下图。

<img src="https://i.loli.net/2021/04/28/tgJVGXOCEFvsfrZ.png" alt="image-20210428153613084" style="zoom:67%;" />

由于这两个文件需要管理员权限编辑，故执行lsm_init程序需要 sudo。
用户层程序lsm_init通过对文件/etc/lsm/roleconfig和/etc/lsm/userconfig进行读写，该程序具有以下功能：

1. 输出系统的所有用户信息（name，uid，gid）；
2. 输出userconfig；
3. 更新userconfig；
4. 删除userconfig的一个用户；
5. 添加userconfig支持的一个用户；
6. 输出roleconfig；
7. 更新roleconfig；
8. 删除roleconfig的一个角色；
9. 添加一个角色；

<img src="https://i.loli.net/2021/04/28/SnuiFWTtIBGsb5P.png" alt="image-20210428153628335" style="zoom:67%;" />

#### 3.1 输出系统的所有用户信息

调用system函数执行命令实现，使用者通过该功能查看当前系统的用户信息。
`system("cat /etc/passwd|grep -v nologin|grep -v halt|grep -v shutdown|awk -F\":\" '{ print $1\"|\"$3\"|\"$4 }'|more");`
<img src="https://i.loli.net/2021/04/28/Wl4sTr5fKqGSM7m.png" alt="image-20210428153652789" style="zoom:67%;" />

#### 3.2 输出userconfig

通过该功能查看lsm_module支持的用户id及其拥有的角色。
<img src="https://i.loli.net/2021/04/28/ftB9nJoywzXvKxh.png" alt="image-20210428153726542" style="zoom:67%;" />


#### 3.3 更新userconfig

可以看到用户user1的角色由fishone更新为fishtwo。
<img src="https://i.loli.net/2021/04/28/tIG8dNwUKVx6TBL.png" alt="image-20210428153745036" style="zoom:67%;" />

#### 3.4 删除lsm支持的一个用户

删除uid为1001的用户。
<img src="https://i.loli.net/2021/04/28/7ELl4dj82AiJDRm.png" alt="image-20210428153802307" style="zoom:67%;" />

#### 3.5 添加lsm支持的一个用户

添加uid为1001的用户，赋予角色fishone。
<img src="https://i.loli.net/2021/04/28/ERvJV3K9hsjLgpF.png" alt="image-20210428153817994" style="zoom:67%;" />

#### 3.6 输出roleconfig

通过该功能查看lsm_module支持的角色及其拥有的权限。
<img src="https://i.loli.net/2021/04/28/nO4xGDbSTcsYIZH.png" alt="image-20210428153828417" style="zoom:67%;" />

#### 3.7 更新roleconfig

角色fishone的权限由可创建和可重命名变更为仅可重命名。
<img src="https://i.loli.net/2021/04/28/UYFMteim1GhLaOB.png" alt="image-20210428153840493" style="zoom:67%;" />

#### 3.8 删除lsm的一个角色

删除角色fishone。
<img src="https://i.loli.net/2021/04/28/kyXhZcRKAjoIOFq.png" alt="image-20210428153851076" style="zoom:67%;" />

#### 3.9 添加lsm的一个角色

添加角色fishone，并赋予权限mkdir和rename。
<img src="https://i.loli.net/2021/04/28/hziYE69kIb8WAVL.png" alt="image-20210428153914260" style="zoom:67%;" />

### 4. LSM内核模块的实现

#### 安全功能的开关

通过文件/etc/lsm/config的内容进行判断，为T时表示开启，为F时表示关闭。
<img src="https://i.loli.net/2021/04/28/fMikPRhpQ32xTSN.png" alt="image-20210428153936450" style="zoom:67%;" />


#### hook操作原理

为了不用每次导入模块时都重新编译内核，在内核源码/security/security.c的结尾处中添加如下代码，将符号security_add_hooks和security_hook_heads导出。 
<img src="https://i.loli.net/2021/04/28/4PZLlzKENnox5HA.png" alt="image-20210428153950808" style="zoom:67%;" />

本次实验支持三种操作，创建文件、删除文件以及重命名文件，对应角色结构体的syscall变量的第1位、第3位和第2位。
<img src="https://i.loli.net/2021/04/28/RWHrV6qPXgwQthB.png" alt="image-20210428154035908" style="zoom:67%;" />

<img src="https://i.loli.net/2021/04/28/utwPCqWsdSBAkbR.png" alt="image-20210428154019938" style="zoom:67%;" />

三种操作分别对应lsm hook的函数inode_mkdir、inode_rmdir和inode_rename，其函数定义可以在/security/security.c中查看，如下图。
<img src="https://i.loli.net/2021/04/28/DRaKXdcHOAhb6Ni.png" alt="image-20210428154047733" style="zoom:67%;" />

首先定义一个security_hooks_list类型的结构体数组，存放要hook的函数以及自定义的函数。
<img src="https://i.loli.net/2021/04/28/Vn1meMoEP8X6QdA.png" alt="image-20210428154056966" style="zoom:67%;" />

##### 模块注册函数lsm_init

调用security_add_hooks函数，添加hook。
<img src="https://i.loli.net/2021/04/28/pmtg3J1AvsoR8cu.png" alt="image-20210428154110132" style="zoom:67%;" />

##### 模块卸载函数lsm_exit

在模块卸载时需要实现对hook的钩子删除，否则在模块卸载后lsm安全功能仍有效。
由于lsm提供的security_delete_hook（位于/include/linux/lsm_hook.h）是一个内联函数，无法导出，所以这里将其源码直接复制过来。

<img src="https://i.loli.net/2021/04/28/VyYUc15kaRf2ODl.png" alt="image-20210428154142576" style="zoom:67%;" />

##### 自定义函数my_inode_mkdir

> 由于mkdir、rmdir和rename的自定义函数流程相似，所以仅对my_inode_mkdir函数进行描述。

函数首先检查安全功能是否开启（读取文件/etc/lsm/config内容判断）；
若开启则获取当前用户的uid，并读取user_config和role_config获取到其对应的角色和权限，如果当前用户不在user_config中，说明lsm尚未支持该用户，则直接返回0表示校验通过；
最后判断权限中mkdir操作对应的位是否为1，若为1则表示具有该操作权限，函数返回0（表示通过），否则返回EACCES（表示Permission denied）。

### 5. 功能性测试

> 切换用户：su - username

#### 5.1 关闭lsm安全功能

user1在lsm中不具有删除文件夹的权限，但是在关闭lsm功能时可以删除文件夹。
<img src="https://i.loli.net/2021/04/28/zX2IHpFBDM3ESoG.png" alt="image-20210428154158140" style="zoom:67%;" />

<img src="https://i.loli.net/2021/04/28/VCsPBAxdtH1QRln.png" alt="image-20210428154213559" style="zoom:67%;" />

#### 5.2 具有MKDIR权限对应角色fishone的用户user1进行MKDIR

结果如下图，在主目录下创建文件夹aaa成功。
<img src="https://i.loli.net/2021/04/28/pCsR31NdnZOBbXQ.png" alt="image-20210428154235390" style="zoom:67%;" />

dmesg查看内核输出，首先校验lsm安全开关，然后校验当前用户uid及拥有的角色，最后判断该角色的权限是否能够mkdir。
<img src="https://i.loli.net/2021/04/28/RvDzp9QtOMU3FAB.png" alt="image-20210428154248901" style="zoom:67%;" />

#### 5.3 具有RENAME权限对应角色fishone的用户user1进行RENAME

将文件夹aaa重命名为bbb。
<img src="https://i.loli.net/2021/04/28/1adRE8DPWJygAbV.png" alt="image-20210428154301390" style="zoom:67%;" />
dmesg查看内核输出
<img src="https://i.loli.net/2021/04/28/PadDHyvX8r1oxK4.png" alt="image-20210428154313255" style="zoom:67%;" />

#### 5.4 具有RMDIR权限对应角色fishtwo的用户user2进行RMDIR

删除文件夹aaa。
<img src="https://i.loli.net/2021/04/28/jrzvTdqGxI29FNQ.png" alt="image-20210428154326380" style="zoom:67%;" />
dmesg查看内核输出。
<img src="https://i.loli.net/2021/04/28/JvTLQdsFolxK6NS.png" alt="image-20210428154343173" style="zoom:67%;" />

#### 5.5 不具有RMDIR权限对应角色fishtwo的用户user1进行RMDIR

删除文件夹aaa。
<img src="https://i.loli.net/2021/04/28/ICqlHBuFwUjZaPe.png" alt="image-20210428154353686" style="zoom:67%;" />
dmesg查看内核输出。
<img src="https://i.loli.net/2021/04/28/kp1i3cB5T28qfoE.png" alt="image-20210428154407874" style="zoom:67%;" />

#### 5.6 不具有MKDIR权限对应角色fishone的用户user2进行MKDIR

创建文件夹aaa。
<img src="https://i.loli.net/2021/04/28/qQtE3aKrLcdnYOX.png" alt="image-20210428154418144" style="zoom:67%;" />
dmesg查看内核输出。
<img src="https://i.loli.net/2021/04/28/5r1xsU7AZh6ywN9.png" alt="image-20210428154430369" style="zoom:67%;" />

#### 5.7 不具有RENAME权限对应角色fishone的用户user2进行RENAME

重命名文件夹aaa为bbb。
<img src="https://i.loli.net/2021/04/28/kvgxmK7iBCaAs3N.png" alt="image-20210428154442312" style="zoom:67%;" />
dmesg查看内核输出。
<img src="https://i.loli.net/2021/04/28/WgRo7pOVeYEhm2Q.png" alt="image-20210428154455872" style="zoom:67%;" />

#### 5.8 lsm不支持的用户r11t

用户r11t进行删除文件夹操作，由于lsm的user_config中没有该用户，所以直接通过。
<img src="https://i.loli.net/2021/04/28/dTK3YwHVXeCFvAj.png" alt="image-20210428154509994" style="zoom:67%;" />
dmesg查看内核输出。
<img src="https://i.loli.net/2021/04/28/pR4hE7ODMJsQUuL.png" alt="image-20210428154520411" style="zoom:80%;" />


### 6. 安全性测试

1. 用户user1拥有rename权限，尝试修改/etc/lsm/config直接关闭lsm安全功能，失败。
   <img src="https://i.loli.net/2021/04/28/Kg8fr4O9Qd6Gi7A.png" alt="image-20210428154535249" style="zoom:67%;" />

  <img src="https://i.loli.net/2021/04/28/jx5sRtmVeWXCAaS.png" alt="image-20210428154548173" style="zoom:67%;" />

2. 用户user2拥有删除文件夹权限，尝试直接删除/etc/lsm文件夹，失败；
   <img src="https://i.loli.net/2021/04/28/vrQOVop3lHMen92.png" alt="image-20210428154607831" style="zoom:67%;" />

3. /etc/lsm/config文件不存在时，不执行lsm功能；
   <img src="https://i.loli.net/2021/04/28/67NhFzJovQHEseU.png" alt="image-20210428154616062" style="zoom:67%;" />

  <img src="https://i.loli.net/2021/04/28/4YlC38TVQ9MgbtW.png" alt="image-20210428154626011" style="zoom:67%;" />


### 7. 待修复的bug

1. 虚拟机OS禁用CPU

加载模块lsm_module一段时间后，出现下图弹窗，只能通过恢复快照解决。
<img src="https://i.loli.net/2021/04/28/FDMKBsHA7htogNm.png" alt="image-20210428154642292" style="zoom:67%;" />


## 附录

### 内核编译过程

```shell
r11t@ubuntu:/usr/src/linux-4.4$ cp /boot/
config-5.4.0-42-generic      memtest86+.elf
config-5.4.0-70-generic      memtest86+_multiboot.bin
config-5.4.0-72-generic      System.map-5.4.0-42-generic
grub/                        System.map-5.4.0-70-generic
initrd.img-5.4.0-42-generic  System.map-5.4.0-72-generic
initrd.img-5.4.0-70-generic  vmlinuz-5.4.0-42-generic
initrd.img-5.4.0-72-generic  vmlinuz-5.4.0-70-generic
memtest86+.bin               vmlinuz-5.4.0-72-generic
r11t@ubuntu:~/Desktop$ uname -r # 查看内核版本    
5.4.0-70-generic
r11t@ubuntu:/usr/src/linux-4.4$ sudo cp /boot/config-5.4.0-70-generic ./config # 复制当前内核的配置
r11t@ubuntu:/usr/src/linux-4.4$ sudo make menuconfig # 直接退出
r11t@ubuntu:/usr/src/linux-4.4$ vim config # set CONFIG_MODULE_SIG=n，关闭内核模块签名验证
r11t@ubuntu:/usr/src/linux-4.4$ sudo make clean
r11t@ubuntu:/usr/src/linux-4.4$ sudo make modules -j3
r11t@ubuntu:/usr/src/linux-4.4$ sudo make modules_install
r11t@ubuntu:/usr/src/linux-4.4$ sudo make -j 3
r11t@ubuntu:/usr/src/linux-4.4$ sudo make install
```

### 遇到的问题及解决方法

1. 【已解决】编译内核出现：cc1: error: code model kernel does not support PIC mode
   * 方法：sudo vim Makefile # 添加-fno-pie
   * <img src="https://i.loli.net/2021/04/28/cmCA7PixZMuzvy3.png" alt="image-20210428155430096" style="zoom:67%;" />
2. 【已解决】make modules_install出现sign-file: certs/signing_key.pem: No such file or directory
   * 方法：https://github.com/andikleen/simple-pt/issues/8
3. 【已解决】内核签名验证关闭：CONFIG_MODULE_SIG=n
4. 【已解决】make[1]: *** No rule to make target 'debian/canonical-certs.pem', needed by 'certs/x509_certificate_list'.  Stop.
   * 方法：在.config 中把CONFIG_SYSTEM_TRUSTED_KEYS=""设置为空值，重新make
5. 【待解决】linux-4.12.1，编译内核完成并加载内核后，加载lsm_module模块，demsg出现`security_add_hooks: kernel tried to execute NX-protected page - exploit attempt? (uid: 0）`
   * 方法：更换为版本4.4的内核源码。


## 参考资料

* 基于Linux Security Module的基于角色的权限管理模块：https://blog.csdn.net/jmh1996/article/details/88935907?ops_request_misc=%257B%2522request%255Fid%2522%253A%2522161891473116780264053893%2522%252C%2522scm%2522%253A%252220140713.130102334.pc%255Fblog.%2522%257D&request_id=161891473116780264053893&biz_id=0&utm_medium=distribute.pc_search_result.none-task-blog-2~blog~first_rank_v2~rank_v29-1-88935907.pc_v2_rank_blog_default&utm_term=%E8%A7%92%E8%89%B2
* 如何实现自己的lsm hook函数：https://stackoverflow.com/questions/10428212/how-can-i-implement-my-own-hook-function-with-lsm
* LSM(linux security module) Security钩子技术(linux原生机制)：https://www.cnblogs.com/LittleHann/p/3854977.html#_lab2_3_4
* LSM HOOK 学习及踩坑：https://blog.csdn.net/weixin_45574485/article/details/108132539
* 错误值含义：https://elixir.bootlin.com/linux/latest/source/include/uapi/asm-generic/errno-base.h#L26
* 内核模块签名：http://lishiwen4.github.io/linux-kernel/linux-kernel-module-signing
