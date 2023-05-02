// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2020 Facebook */
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include "writesnoop.h"
#include "syscall.h"
#include "writesnoop.skel.h"

#define QUOTE(...) #__VA_ARGS__

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	if (level == LIBBPF_DEBUG)
		return 0;
	return vfprintf(stderr, format, args);
}

static volatile bool exiting = false;

static void sig_handler(int sig)
{
	exiting = true;
}

static int handle_event(void *ctx, void *data, size_t data_sz)
{
	u32 event_id = *((u32 *)data);
	// TODO: ADD SANITY CHECK OF EVENT ID
	data = data + sizeof(u32);
	switch(event_id)
	{
		case APP:
		{
			const struct applog_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);


			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"data":{
						"fd":%d,
						"lms":"%s"
					},
					"artifacts":{
						"exe":"%s"
						// "file_writen"
					}
				}
				),
				d->event.ts, ts, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->fd, d->msg, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_READ:
		{
			const struct read_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);


			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"read",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"fd":%d,
						"buf":"0x%08x",
						"count":%u
					},
					"artifacts":{
						"exe":"%s",
						"file_read":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->fd, (unsigned int)d->buf, d->count,
				d->event.task.exe_path, d->filepath
			);
			break;
		}
		case SYSCALL_WRITE:
		{
			const struct write_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"write",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"fd":%d,
						"buf":"0x%08x",
						"count":%u
					},
					"artifacts":{
						"exe":"%s",
						"file_written":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->fd, (unsigned int)d->buf, d->count,
				d->event.task.exe_path,d->filepath
			);
			break;
		}
		case SYSCALL_OPEN:
		{
			const struct open_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"open",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"filename":"%s",
						"flags":%d,
						"mode":%d
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->filename, d->flags, d->mode, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_CLOSE:
		{
			const struct close_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"close",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"fd":%d
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->fd, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_DUP:
		{
			const struct dup_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"dup",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"fildes":%d
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->fildes, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_DUP2:
		{
			const struct dup2_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"dup2",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"oldfd":%d,
						"newfd":%d
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->oldfd, d->newfd, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_CONNECT:
		{
			const struct connect_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"connect",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"fd":%d,
						"uservaddr":"0x%08x",
						"addrlen":%d
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->fd, d->uservaddr, d->addrlen, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_ACCEPT:
		{
			const struct accept_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"accept",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"fd":%d,
						"upeer_sockaddr":"0x%08x",
						"upeer_addrlen":"0x%08x"
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->fd, d->upeer_sockaddr, d->upeer_addrlen, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_BIND:
		{
			const struct bind_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"bind",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"fd":%d,
						"umyaddr":"0x%08x",
						"addrlen":%d
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->fd, d->umyaddr, d->addrlen, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_CLONE:
		{
			const struct clone_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"clone",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"flags":%lu,
						"newsp":"0x%08x",
						"parent_tid":"0x%08x",
						"child_tid":"0x%08x",
						"tls":%lu
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->flags, d->newsp, d->parent_tid,
				d->child_tid, d->tls, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_FORK:
		{
			const struct fork_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"fork",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_VFORK:
		{
			const struct vfork_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"vfork",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_EXECVE:
		{
			const struct execve_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"execve",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"filename":"0x%08x",
						"argv":"0x%08x"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->filename, d->argv
			);
			break;
		}
		case SYSCALL_EXIT:
		{
			const struct exit_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);


			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"exit",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"error_code":"%d"
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->error_code, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_EXIT_GROUP:
		{
			const struct exit_group_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);


			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"exit_group",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"error_code":"%d"
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->error_code, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_OPENAT:
		{
			const struct openat_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"openat",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"dfd":%d,
						"filename":"%s",
						"flags":%d,
						"mode":%d
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->dfd, d->filename, d->flags, d->mode, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_UNLINKAT:
		{
			const struct unlinkat_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"unlinkat",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"dfd":%d,
						"pathname":"%s",
						"flag":%d
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->dfd, d->pathname, d->flag, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_ACCEPT4:
		{
			const struct accept4_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"accept4",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"fd":%d,
						"upeer_sockaddr":"0x%08x",
						"upeer_addrlen":"0x%08x",
						"flags":%d
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->fd, d->upeer_sockaddr, d->upeer_addrlen, d->flags, d->event.task.exe_path
			);
			break;
		}
		case SYSCALL_DUP3:
		{
			const struct dup3_data_t *d = data;
			char ts[32]; time_t t;

			time(&t); struct tm *tmd = localtime(&t);
			strftime(ts, sizeof(ts), "%H:%M:%S", tmd);

			printf(
				QUOTE(
				{
					"event_context":{
						"ts":%llu,
						"datetime":"%s",
						"syscall_id":%d,
						"syscall_name":"dup3",
						"retval":%d,
						"task_context":{
							"host_pid":%d,
							"host_tid":%d,
							"host_ppid":%d,
							"pid":%d,
							"tid":%d,
							"ppid":%d,
							"cgroup_id":%llu,
							"mntns_id":%u,
							"pidns_id":%u,
							"task_command":"%s"
						}
					},
					"arguments":{
						"oldfd":%d,
						"newfd":%d,
						"flags":%d
					},
					"artifacts":{
						"exe":"%s"
					}
				}
				),
				d->event.ts, ts, d->event.syscall_id, d->retval, d->event.task.host_pid, d->event.task.host_tid,
				d->event.task.host_ppid, d->event.task.pid, d->event.task.tid, d->event.task.ppid, d->event.task.cgroup_id,
				d->event.task.mntns_id, d->event.task.pidns_id, d->event.task.comm, d->oldfd, d->newfd, d->flags, d->event.task.exe_path
			);
			break;
		}
		default:
		{
			break;
		}
	}
	printf(",\n");
	return 0;
}

int main(int argc, char **argv)
{
	struct ring_buffer *rb = NULL;
	struct writesnoop_bpf *skel;

	int err;

	libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
	/* Set up libbpf errors and debug info callback */
	libbpf_set_print(libbpf_print_fn);

	/* Cleaner handling of Ctrl-C */
	signal(SIGINT, sig_handler);
	signal(SIGTERM, sig_handler);

	/* Load and verify BPF application */
	skel = writesnoop_bpf__open();
	if (!skel) {
		fprintf(stderr, "Failed to open and load BPF skeleton\n");
		return 1;
	}

	/* ensure that BPF program only handles write() syscalls from other processes */
	skel->bss->mypid = getpid();

	/* Load & verify BPF programs */
	err = writesnoop_bpf__load(skel);
	if (err) {
		fprintf(stderr, "Failed to load and verify BPF skeleton\n");
		goto cleanup;
	}

	/* Attach tracepoints */
	err = writesnoop_bpf__attach(skel);
	if (err) {
		fprintf(stderr, "Failed to attach BPF skeleton\n");
		goto cleanup;
	}

	/* Set up ring buffer polling */
	rb = ring_buffer__new(bpf_map__fd(skel->maps.rb), handle_event, skel, NULL);
	if (!rb) {
		err = -1;
		fprintf(stderr, "Failed to create ring buffer\n");
		goto cleanup;
	}

	/* Process events */
	printf("{\n\"logs\":[\n");
	while (!exiting) {
		err = ring_buffer__poll(rb, 100 /* 1out, ms */);
		/* Ctrl-C will cause -EINTR */
		if (err == -EINTR) {
			err = 0;
			break;
		}
		if (err < 0) {
			printf("Error polling perf buffer: %d\n", err);
			break;
		}
	}
	printf("]\n}");

cleanup:
	/* Clean up */
	ring_buffer__free(rb);
	writesnoop_bpf__destroy(skel);

	return err < 0 ? -err : 0;
}
