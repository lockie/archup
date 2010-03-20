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

#define NUMBER_OUT 30
#define LIBNOTIFY_TIMEOUT 3600000 /* 60min */
#define MAX_LINE_LENGTH 50

int main(int argc, char **argv)
{
	typedef int bool;

	/* those are needed by libnotify */
	char *name = "arch_update_check";
	char *category = "update";
        NotifyNotification *my_notify;
        GError *error = NULL;

	bool got_updates = FALSE;
	char output_string[NUMBER_OUT*MAX_LINE_LENGTH+100] = "There are updates for:\n";
	char line[MAX_LINE_LENGTH];
	int  i=0;
	FILE *pac_out;

        pac_out = popen("/usr/bin/pacman -Qu","r");
        while ( fgets( line, sizeof line, pac_out) != NULL )
        {
		i++;
		if (i >= NUMBER_OUT)
		{
			strncat(output_string," - ...\0 ",7);
			break;
		}
		got_updates = TRUE;
		strncat(output_string," - ",3);
		strncat(output_string,line,50);
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
