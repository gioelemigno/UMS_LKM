#pragma once
/// @file 
/// This file contains functions used to manage /proc filesystem
///

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>


#include "ums_hashtable.h"
#include "ums_process.h"
#include "ums_context.h"
#include "ums_completion_lsit.h"


extern struct proc_dir_entry* ums_proc_ums_folder; /** entry in /proc of "ums" folder*/

/**
 * @brief snprintf used to print info about scheduler in /proc
 * 
 * @param tgid tgid of the process
 * @param sched_pid pid of the scheduler
 */
ssize_t ums_scheduler_snprintf_info(pid_t tgid, pid_t sched_pid, char* buff, size_t buff_size);

/**
 * @brief snprintf used to print info about a worker thread in /proc
 * 
 * @param tgid tgid of the process
 * @param sched_pid pid of the scheduler that manages the ums_context
 * @param ucd descriptor of the ums_context
 */
ssize_t ums_scheduler_snprintf_worker(pid_t tgid, pid_t sched_pid, ums_context_descriptor_t ucd, char* buff, size_t buff_size);


/**
 * @brief function used when user write a file in /proc/ums 
 * 
 */
static ssize_t ums_proc_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos){
        printk(KERN_ALERT  "Sorry, this operation isn't supported.\n");
        return -EINVAL;
}


/**
 * @brief get pid of the scheduler to which "file" refers to, in /proc/ums
 * 
 * @param p_file pointer to a struct file
 * @param p_pid_out output, pointer to a pid_t
 */
#define __sched_file_to_sched_pid(p_file, p_pid_out)    \
    do{ \
        long tmp;   \
        int res = kstrtol((p_file)->f_path.dentry->d_parent->d_iname, 10, &tmp); \
        if(likely(res==0))   \
            *(p_pid_out) = (pid_t)tmp;  \
        else    \
            *(p_pid_out) = -1;  \
    }while(0)

/**
 * @brief get tgid of the process to which "file" refers to, in /proc/ums
 * 
 */
#define __sched_file_to_tgid(p_file, p_tgid_out)    \
    do{ \
        long tmp;   \
        int res = kstrtol((p_file)->f_path.dentry->d_parent->d_parent->d_parent->d_iname, 10, &tmp);    \
        if(likely(res==0))  \
            *(p_tgid_out) = (pid_t)tmp; \
        else \
            *(p_tgid_out) = -1; \
    }while(0)


#define __INFO_FILE_BUFF_SIZE   128
/**
 * @brief function used when user reads "info" file in /proc/ums
 * 
 */
static ssize_t ums_proc_read_scheduler(struct file *file, char __user *ubuf, size_t count, loff_t *ppos){

    pid_t sched_pid;
    pid_t tgid;

    char buff[__INFO_FILE_BUFF_SIZE];
    ssize_t len;
    len = 0;

    __sched_file_to_sched_pid(file, &sched_pid);
    __sched_file_to_tgid(file, &tgid);

    if (*ppos > 0 || count < __INFO_FILE_BUFF_SIZE)
        return 0;

    len += ums_scheduler_snprintf_info(tgid, sched_pid, buff, __INFO_FILE_BUFF_SIZE);

    if (copy_to_user(ubuf, buff, len))
        return -EFAULT;

    *ppos = len;
    return len;
}
static struct proc_ops sched_info_ops = {
        .proc_read = ums_proc_read_scheduler,
        .proc_write = ums_proc_write,
};


/**
 * @brief get ums_context_descriptor of the ums_context to which "p_file" refers to, in /proc/ums
 * 
 * @param p_file pointer to a struct file
 * @param p_ucd output, pointer to a ums_context_descriptor
 */
#define __worker_file_to_ucd(p_file, p_ucd)    \
    do{ \
        long tmp;   \
        int res = kstrtol((p_file)->f_path.dentry->d_iname, 10, &tmp); \
        if(likely(res==0))   \
            *(p_ucd) = (int)tmp;  \
        else    \
            *(p_ucd) = -1;  \
    }while(0)

/**
 * @brief get pid of the scheduler to which "file" refers to, in /proc/ums
 * 
 * @param p_file pointer to a struct file
 * @param p_pid_out output, pointer to a pid_t
 */
#define __worker_file_to_sched_pid(p_file, p_pid_out)    \
    do{ \
        long tmp;   \
        int res = kstrtol((p_file)->f_path.dentry->d_parent->d_parent->d_iname, 10, &tmp); \
        if(likely(res==0))   \
            *(p_pid_out) = (pid_t)tmp;  \
        else    \
            *(p_pid_out) = -1;  \
    }while(0)

/**
 * @brief get tgid of the process to which "file" refers to, in /proc/ums
 * 
 * @param p_file pointer to a struct file
 * @param p_tgid_out output, pointer to a pid_t
 */
#define __worker_file_to_tgid(p_file, p_tgid_out)    \
    do{ \
        long tmp;   \
        int res = kstrtol((p_file)->f_path.dentry->d_parent->d_parent->d_parent->d_parent->d_iname, 10, &tmp);    \
        if(likely(res==0))  \
            *(p_tgid_out) = (pid_t)tmp; \
        else \
            *(p_tgid_out) = -1; \
    }while(0)




#define __INFO_WORKER_BUFF_SIZE     128
/**
 * @brief function used when user reads a file in "workers" folder in /proc/ums..
 * 
 */
static ssize_t ums_proc_read_thread(struct file *file, char __user *ubuf, size_t count, loff_t *ppos){
        
        pid_t tgid;
        pid_t sched_pid;
        ums_context_descriptor_t ucd;

        char buff[__INFO_WORKER_BUFF_SIZE];
        int len = 0;

        if(*ppos > 0 || count < __INFO_WORKER_BUFF_SIZE)
            return 0;
 
        __worker_file_to_tgid(file, &tgid);
        __worker_file_to_sched_pid(file, &sched_pid);
        __worker_file_to_ucd(file, &ucd);

        /*
        len += snprintf(buff, __INFO_WORKER_BUFF_SIZE,
                        "tgid=%d"
                        "sched_pid=%d"
                        "ucd=%d",
                        tgid,
                        sched_pid,
                        ucd);
        */
        len += ums_scheduler_snprintf_worker(tgid, sched_pid, ucd, buff, __INFO_WORKER_BUFF_SIZE);

        if(copy_to_user(ubuf, buff, len))
                return -EFAULT;
        *ppos = len;

        return len;
}
static struct proc_ops thread_ops = {
        .proc_read = ums_proc_read_thread,
        .proc_write = ums_proc_write,
};

/**
 * @brief make /proc/ums directory
 * 
 */
#define ums_proc_mount()    \
    do{ \
        ums_proc_ums_folder = proc_mkdir("ums", NULL);  \
    }while(0)

/**
 * @brief remove /proc/ums directory
 * 
 */
#define ums_proc_unmount()  \
    do{ \
        proc_remove(ums_proc_ums_folder);   \
    }while(0)

/**
 * @brief add a process in /proc/ums
 * 
 * @param p_pd_proc_out output, pointer proc_dir_entry of the new directory /proc/ums/<tgid>
 * @param p_pd_scheds_out output, pointer proc_dir_entry of the new directory /proc/ums/<tgid>/schedulers
 * @param pid pid of the process (tgid)
 */
#define ums_proc_add_process(p_pd_proc_out, p_pd_scheds_out, pid)   \
    do{ \
        char __buff[32];    \
        sprintf(__buff, "%d", pid); \
        p_pd_proc_out = proc_mkdir(__buff, ums_proc_ums_folder);  \
        p_pd_scheds_out = proc_mkdir("schedulers", p_pd_proc_out);  \
    }while(0)

/**
 * @brief remove a process in /proc/ums
 * 
 * @param p_pd_proc_in pointer proc_dir_entry of the directory /proc/ums/<tgid> to remove
 * @param p_pd_scheds_in pointer proc_dir_entry of the directory /proc/ums/<tgid>/schedulers to remove
 */
#define ums_proc_remove_process(p_pd_proc_in, p_pd_sched_in)    \
    do{ \
        proc_remove(p_pd_sched_in); \
        proc_remove(p_pd_proc_in);  \
    }while(0)

/**
 * @brief add a scheduler in /proc/ums/<tgid>
 * 
 * @param p_pd_scheds_main_in pointer proc_dir_entry of the directory /proc/ums/<tgid>/schedulers
 * @param sched_id pid of the scheduler
 * @param p_pd_sched_out output, pointer proc_dir_entry of the directory /proc/ums/<tgid>/schedulers/<sched_pid> 
 * @param p_info_out output, pointer proc_dir_entry of the file "info" in /proc/ums/<tgid>/schedulers/<sched_pid> 
 * @param p_pd_workers_main_out output, pointer proc_dir_entry of the folder /proc/ums/<tgid>/schedulers/<sched_pid>/workers 
 * 
 */
#define ums_proc_add_scheduler(p_pd_scheds_main_in, sched_id, p_pd_sched_out, p_info_out, p_pd_workers_main_out)    \
    do{ \
        char __buff[32];    \
        sprintf(__buff, "%d", sched_id);    \
        \
        p_pd_sched_out = proc_mkdir(__buff, p_pd_scheds_main_in);   \
        p_info_out = proc_create("info", S_IALLUGO, p_pd_sched_out, &sched_info_ops);    \
        p_pd_workers_main_out = proc_mkdir("workers", p_pd_sched_out);  \
    }while(0)

/**
 * @brief remove a scheduler in /proc/ums/<tgid>
 * 
 * @param p_pd_sched_in pointer proc_dir_entry of the directory /proc/ums/<tgid>/schedulers/<sched_pid> 
 * @param p_info_in pointer proc_dir_entry of the file "info" in /proc/ums/<tgid>/schedulers/<sched_pid> 
 * @param p_pd_workers_main_in pointer proc_dir_entry of the folder /proc/ums/<tgid>/schedulers/<sched_pid>/workers 
 * 
 */
#define ums_proc_remove_scheduler(p_pd_sched_in, p_info_in, p_pd_workers_main_in)  \
    do{ \
        proc_remove(p_pd_workers_main_in);  \
        proc_remove(p_info_in);    \
        proc_remove(p_pd_sched_in); \
    }while(0)


/**
 * @brief add a ums_context in /proc/ums/tgid/schedulers/<pid>/workers
 * 
 * @param p_pd_workers_main_in pointer proc_dir_entry of the folder /proc/ums/<tgid>/schedulers/<sched_pid>/workers 
 * @param id_thread_in ums_context_descriptor
 * @param p_pd_thread_out output, proc_dir_entry of the file associated to ums_context in /proc/ums/tgid/schedulers/<pid>/workers
 * 
 */
#define ums_proc_add_thread(p_pd_workers_main_in, id_thread_in, p_pd_thread_out) \
    do{ \
        char __buff[32];    \
        sprintf(__buff, "%d", id_thread_in);    \
        p_pd_thread_out = proc_create(__buff, S_IALLUGO, p_pd_workers_main_in, &thread_ops);    \
    }while(0)

/**
 * @brief remove a ums_context from /proc/ums/tgid/schedulers/<pid>/workers
 * 
 * @param p_pd_thread_in proc_dir_entry of the file associated to ums_context in /proc/ums/tgid/schedulers/<pid>/workers
 * 
 */
#define ums_proc_remove_thread(p_pd_thread_in) \
    do{ \
        proc_remove(p_pd_thread_in);    \
    }while(0)
