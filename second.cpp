#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <time.h>
//textbase ui
#include <ncurses.h>

#define MAX_PROCESSES 1024
#define MAX_STRING_LENGTH 128
#define MAX_FILE_SYSTEMS 10

// Struct to hold system information
struct SystemInfo
{
  double total_memory;
  double free_memory;
  double used_memory;
  double cpu_usage;
  double disk_usage;
  double network_usage;
};

// Structure to hold process information
struct ProcessInfo
{
  int pid;
  char name[MAX_STRING_LENGTH];
  double cpu_usage;
  double mem_usage;
};

// Structure to hold file system information
struct FileSystemInfo
{
  char name[MAX_STRING_LENGTH];
  double total_space;
  double free_space;
  double used_space;
};

// Function to get system information
void
get_system_info (struct SystemInfo *sys_info)
{
  struct sysinfo s_info;
  if (sysinfo (&s_info) != 0)
    {
      perror ("sysinfo");
      exit (EXIT_FAILURE);
    }
//convert into GB from megabytes
  sys_info->total_memory = (double) s_info.totalram / (1024 * 1024);
  sys_info->free_memory = (double) s_info.freeram / (1024 * 1024);
  sys_info->used_memory = sys_info->total_memory - sys_info->free_memory;

  FILE *fp;
  char buffer[MAX_STRING_LENGTH];

  fp = popen ("top -b -n 1 | grep \"Cpu(s)\" | awk '{print $2+$4}'", "r");
  fgets (buffer, sizeof (buffer), fp);
  pclose (fp);

  sys_info->cpu_usage = atof (buffer);

  fp = popen ("df -h | awk '$NF==\"/\"{printf \"%d\", $5}'", "r");
  fgets (buffer, sizeof (buffer), fp);
  pclose (fp);

  sys_info->disk_usage = atof (buffer);

  fp = popen ("cat /proc/net/dev | awk 'NR==3{print $2+$10}'", "r");
  fgets (buffer, sizeof (buffer), fp);
  pclose (fp);

  sys_info->network_usage = atof (buffer);
}

// Function to get process information
void
get_process_info (struct ProcessInfo *proc_info, int *num_procs)
{
  FILE *fp;
  char buffer[MAX_STRING_LENGTH];
  char command[MAX_STRING_LENGTH];

  sprintf (command, "ps -eo pid,comm,pcpu,pmem | sort -k 3 -r | head -n %d",
  MAX_PROCESSES);

  fp = popen (command, "r");

  int i = 0;
  while (fgets (buffer, sizeof (buffer), fp) != NULL)
    {
      if (i == 0)
{
 // Skip header row
 i++;
 continue;
}

      char *pid_str = strtok (buffer, " ");
      char *name_str = strtok (NULL, " ");
      char *cpu_str = strtok (NULL, " ");
      char *mem_str = strtok (NULL, " ");

      proc_info[i - 1].pid = atoi (pid_str);
      strncpy (proc_info[i - 1].name, name_str,
      sizeof (proc_info[i - 1].name));
      proc_info[i - 1].cpu_usage = atof (cpu_str);
      proc_info[i - 1].mem_usage = atof (mem_str);

      i++;
      if (i > MAX_PROCESSES)
{
 break;
}
    }

  *num_procs = i - 1;

  pclose (fp);
}

// Function to get file system information
void
get_file_system_info (struct FileSystemInfo *fs_info, int *num_fs)
{
  FILE *fp;
  char buffer[MAX_STRING_LENGTH];
  char command[MAX_STRING_LENGTH];

  sprintf (command,
  "df -h | awk '{print $1,$2,$3,$4,$5}' | sed '1d' | head -n %d",
  MAX_FILE_SYSTEMS);

  fp = popen (command, "r");

  int i = 0;
  while (fgets (buffer, sizeof (buffer), fp) != NULL)
    {
      char *name_str = strtok (buffer, " ");
      char *total_str = strtok (NULL, " ");
      char *used_str = strtok (NULL, " ");
      char *free_str = strtok (NULL, " ");
      char *usage_str = strtok (NULL, " ");

      strncpy (fs_info[i].name, name_str, sizeof (fs_info[i].name));
      fs_info[i].total_space = atof (total_str);
      fs_info[i].used_space = atof (used_str);
      fs_info[i].free_space = atof (free_str);

      i++;
      if (i > MAX_FILE_SYSTEMS)
{
 break;
}
    }

  *num_fs = i;

  pclose (fp);
}

// Function to display system information on console
void
display_system_info (struct SystemInfo sys_info)
{
  time_t current_time = time (NULL);
  char *time_str = ctime (&current_time);

  clear ();
  mvprintw (0, 0, "System Resource Monitor");
  mvprintw (1, 0, "Generated at: %s", time_str);
  mvprintw (3, 0, "CPU Usage: %.2f%%", sys_info.cpu_usage);
  mvprintw (4, 0, "Memory Usage: %.2f/%.2f GB (%.2f%%)", sys_info.used_memory,
   sys_info.total_memory,
   (sys_info.used_memory / sys_info.total_memory) * 100);
  mvprintw (5, 0, "Disk Usage: %.2f%%", sys_info.disk_usage);
  mvprintw (6, 0, "Network Usage: %.2f bytes/s", sys_info.network_usage);

  refresh ();
}

// Function to display process information on console
void
display_process_info (struct ProcessInfo *proc_info, int num_procs)
{
  int row = 8;

  mvprintw (row, 0, "Processes (Top %d)", num_procs);
  row += 2;

  mvprintw (row, 0, "%5s  %-30s  %10s  %10s", "PID", "Name", "CPU Usage",
   "Memory Usage");
  row += 1;

  for (int i = 0; i < num_procs; i++)
    {
      mvprintw (row, 0, "%5d  %-30s  %10.2f%%  %10.2f MB", proc_info[i].pid,
proc_info[i].name, proc_info[i].cpu_usage,
proc_info[i].mem_usage);
      row += 1;
    }

  refresh ();
}

// Function to display file system information on console
void
display_file_system_info (struct FileSystemInfo *fs_info, int num_fs)
{
  int row = 8;

  mvprintw (row, 40, "File Systems (Top %d)", num_fs);
  row += 2;

  mvprintw (row, 40, "%-20s  %10s  %10s  %10s", "Name", "Total Space",
   "Free Space", "Used Space");
  row += 1;

  for (int i = 0; i < num_fs; i++)
    {
      mvprintw (row, 40, "%-20s  %10.2f GB  %10.2f GB  %10.2f GB",
fs_info[i].name, fs_info[i].total_space,
fs_info[i].free_space, fs_info[i].used_space);
      row += 1;
    }

  refresh ();
}

int
main ()
{
  struct SystemInfo sys_info;
  struct ProcessInfo proc_info[MAX_PROCESSES];
  struct FileSystemInfo fs_info[MAX_FILE_SYSTEMS];
  int num_procs = 0;
  int num_fs = 0;

  // Initialize ncurses
  initscr ();
  cbreak ();
  noecho ();
  curs_set (0);

  while (1)
    {
      get_system_info (&sys_info);
      get_process_info (proc_info, &num_procs);
      get_file_system_info (fs_info, &num_fs);

      display_system_info (sys_info);
      display_process_info (proc_info, num_procs);
      display_file_system_info (fs_info, num_fs);

      // Wait for 1 second
      sleep (1);
    }

  // End ncurses
  endwin ();

  return 0;
}
