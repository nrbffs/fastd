/*
  Copyright (c) 2012-2014, Matthias Schiffer <mschiffer@universe-factory.net>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice,
       this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright notice,
       this list of conditions and the following disclaimer in the documentation
       and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "fastd.h"
#include "config.h"
#include "peer.h"
#include <fastd_version.h>

#include <arpa/inet.h>


static void print_usage(const char *options, const char *message) {
	/* 28 spaces */
	static const char spaces[] = {[0 ... 27] = ' ', [28] = 0};

	int len = strlen(options);

	printf("%s", options);

	if (len < 28)
		printf("%s", spaces+len);
	else
		printf("\n%s", spaces);

	puts(message);
}

static void usage(void) {
	puts("fastd (Fast and Secure Tunnelling Daemon) " FASTD_VERSION " usage:\n");

#define OR ", "
#define SEPARATOR puts("")
#define OPTION(func, options, message) print_usage("  " options, message)
#define OPTION_ARG(func, options, arg, message) print_usage("  " options " " arg, message)
#include "options.def.h"
#undef OR
#undef SEPARATOR
#undef OPTION
#undef OPTION_ARG

	exit(0);
}

static void version(void) {
	puts("fastd " FASTD_VERSION);
	exit(0);
}

static void option_daemon(void) {
	conf.daemon = true;
}

static void option_pid_file(const char *arg) {
	free(conf.pid_file);
	conf.pid_file = strdup(arg);
}


static void option_config(const char *arg) {
	if (!strcmp(arg, "-"))
		arg = NULL;

	if (!fastd_read_config(arg, false, 0))
		exit(1);
}

static void option_config_peer(const char *arg) {
	fastd_peer_config_new();

	if(!fastd_read_config(arg, true, 0))
		exit(1);
}

static void option_config_peer_dir(const char *arg) {
	fastd_add_peer_dir(arg);
}


#ifdef WITH_CMDLINE_USER

static void option_user(const char *arg) {
	free(conf.user);
	conf.user = strdup(arg);
}

static void option_group(const char *arg) {
	free(conf.group);
	conf.group = strdup(arg);
}

#endif

#ifdef WITH_CMDLINE_LOGGING

static int parse_log_level(const char *arg) {
	if (!strcmp(arg, "fatal"))
		return LL_FATAL;
	else if (!strcmp(arg, "error"))
		return LL_ERROR;
	else if (!strcmp(arg, "warn"))
		return LL_WARN;
	else if (!strcmp(arg, "info"))
		return LL_INFO;
	else if (!strcmp(arg, "verbose"))
		return LL_VERBOSE;
	else if (!strcmp(arg, "debug"))
		return LL_DEBUG;
	else if (!strcmp(arg, "debug2"))
		return LL_DEBUG2;
	else
		exit_error("invalid log level `%s'", arg);
}

static void option_log_level(const char *arg) {
	conf.log_stderr_level = parse_log_level(arg);
}

static void option_syslog_level(const char *arg) {
	conf.log_syslog_level = parse_log_level(arg);
}

static void option_syslog_ident(const char *arg) {
	free(conf.log_syslog_ident);
	conf.log_syslog_ident = strdup(arg);
}

static void option_hide_ip_addresses(void) {
	conf.hide_ip_addresses = true;
}

static void option_hide_mac_addresses(void) {
	conf.hide_mac_addresses = true;
}

#endif

#ifdef WITH_CMDLINE_OPERATION

static void option_mode(const char *arg) {
	if (!strcmp(arg, "tap"))
		conf.mode = MODE_TAP;
	else if (!strcmp(arg, "tun"))
		conf.mode = MODE_TUN;
	else
		exit_error("invalid mode `%s'", arg);
}

static void option_interface(const char *arg) {
	free(conf.ifname);
	conf.ifname = strdup(arg);
}

static void option_mtu(const char *arg) {
	char *endptr;
	long mtu = strtol(arg, &endptr, 10);

	if (*endptr || mtu < 576 || mtu > 65535)
		exit_error("invalid mtu `%s'", arg);

	conf.mtu = mtu;
}

static void option_bind(const char *arg) {
	long l;
	char *charptr;
	char *endptr;
	char *addrstr;
	char *ifname = NULL;

	if (arg[0] == '[') {
		charptr = strrchr(arg, ']');
		if (!charptr || (charptr[1] != ':' && charptr[1] != '\0'))
			exit_error("invalid bind address `%s'", arg);

		addrstr = strndup(arg+1, charptr-arg-1);

		if (charptr[1] == ':')
			charptr++;
		else
			charptr = NULL;
	}
	else {
		charptr = strrchr(arg, ':');
		if (charptr) {
			addrstr = strndup(arg, charptr-arg);
		}
		else {
			addrstr = strdup(arg);
		}
	}

	if (charptr) {
		l = strtol(charptr+1, &endptr, 10);
		if (*endptr || l < 1 || l > 65535)
			exit_error("invalid bind port `%s'", charptr+1);
	}
	else {
		l = 0;
	}

	fastd_peer_address_t addr = {};

	if (arg[0] == '[') {
		addr.in6.sin6_family = AF_INET6;
		addr.in6.sin6_port = htons(l);

		ifname = strchr(addrstr, '%');
		if (ifname) {
			*ifname = '\0';
			ifname++;
		}

		if (inet_pton(AF_INET6, addrstr, &addr.in6.sin6_addr) != 1)
			exit_error("invalid bind address `[%s]'", addrstr);
	}
	else if (strcmp(addrstr, "any") == 0) {
		addr.in.sin_family = AF_UNSPEC;
		addr.in.sin_port = htons(l);
	}
	else {
		addr.in.sin_family = AF_INET;
		addr.in.sin_port = htons(l);

		if (inet_pton(AF_INET, addrstr, &addr.in.sin_addr) != 1)
			exit_error("invalid bind address `%s'", addrstr);
	}

	free(addrstr);

	fastd_config_bind_address(&addr, ifname, false, false);
}

static void option_protocol(const char *arg) {
	fastd_config_protocol(arg);
}

static void option_method(const char *arg) {
	fastd_config_method(arg);
}

static void option_forward(void) {
	conf.forward = true;
}

#endif

#ifdef WITH_CMDLINE_COMMANDS

static void option_on_pre_up(const char *arg) {
	fastd_shell_command_set(&conf.on_pre_up, arg, true);
}

static void option_on_up(const char *arg) {
	fastd_shell_command_set(&conf.on_up, arg, true);
}

static void option_on_down(const char *arg) {
	fastd_shell_command_set(&conf.on_down, arg, true);
}

static void option_on_post_down(const char *arg) {
	fastd_shell_command_set(&conf.on_post_down, arg, true);
}

static void option_on_connect(const char *arg) {
	fastd_shell_command_set(&conf.on_connect, arg, false);
}

static void option_on_establish(const char *arg) {
	fastd_shell_command_set(&conf.on_establish, arg, false);
}

static void option_on_disestablish(const char *arg) {
	fastd_shell_command_set(&conf.on_disestablish, arg, false);
}

#ifdef WITH_VERIFY

static void option_on_verify(const char *arg) {
	fastd_shell_command_set(&conf.on_verify, arg, false);
}

#endif

#endif

static void option_verify_config(void) {
	conf.verify_config = true;
}

static void option_generate_key(void) {
	conf.generate_key = true;
	conf.show_key = false;
}

static void option_show_key(void) {
	conf.generate_key = false;
	conf.show_key = true;
}

static void option_machine_readable(void) {
	conf.machine_readable = true;
}


static bool config_match(const char *opt, ...) {
	va_list ap;
	bool match = false;
	const char *str;

	va_start(ap, opt);

	while((str = va_arg(ap, const char*)) != NULL) {
		if (strcmp(opt, str) == 0) {
			match = true;
			break;
		}
	}

	va_end(ap);

	return match;
}

void fastd_config_handle_options(int argc, char *const argv[]) {
	int i = 1;

	while (i < argc) {
#define OR ,
#define SEPARATOR do {} while (false)
#define OPTION(func, options, message)					\
		({							\
			if(config_match(argv[i], options, NULL)) {	\
				i++;					\
				func();			\
				continue;				\
			}						\
		})
#define OPTION_ARG(func, options, arg, message)				\
		({							\
			if(config_match(argv[i], options, NULL)) {	\
				i+=2;					\
				if (i > argc)				\
					exit_error("command line error: option `%s' needs an argument; see --help for usage", argv[i-2]); \
				func(argv[i-1]);		\
				continue;				\
			}						\
		})
#include "options.def.h"
#undef OR
#undef SEPARATOR
#undef OPTION
#undef OPTION_ARG

		exit_error("command line error: unknown option `%s'; see --help for usage", argv[i]);
	}
}
