/*  
Copyright 2010 Rorschach <r0rschach@lavabit.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. 
*/

#include <libnotify/notify.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define NUMBER_OUT 30
#define LIBNOTIFY_TIMEOUT 3600000 /* 60min */

int main(int argc, char **argv)
{
	typedef int bool;

	/* those are needed by libnotify */
	char *name = "arch_update_check";
	char *category = "update";
        NotifyNotification *my_notify;
        GError *error = NULL;

        /* those are needed for the output */
	char *output_string=malloc(23); 
	sprintf(output_string,"There are updates for:\n");
	bool got_updates = FALSE;

	/* those are needed for getting the list of updates */
	FILE *pac_out;
	int llen = 0;
	char line[BUFSIZ];

  
        pac_out = popen("/usr/bin/pacman -Qu","r");

	while (fgets(line,BUFSIZ,pac_out) != NULL) 
	{
		got_updates = TRUE;

		llen = strlen(line);
		output_string = (char *)realloc(output_string,strlen(output_string)+llen);
		strncat(output_string,"- ",2);
		strncat(output_string,line,llen);
	}

	pclose(pac_out);


	if (got_updates == TRUE)
	{
		
		notify_init(name);
		my_notify = notify_notification_new("New updates for Archlinux available!",output_string,NULL,NULL);
		notify_notification_set_timeout(my_notify,LIBNOTIFY_TIMEOUT);
		
		notify_notification_set_category(my_notify,category);
		notify_notification_set_urgency (my_notify,NOTIFY_URGENCY_CRITICAL);
		
		notify_notification_show(my_notify,&error);
		notify_uninit();
	}

	return(0);
}
