#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#define MAX_PATH_LENGTH 4096
#define MAX_DATE_BUFFER_LENGTH 64

void print_permissions(mode_t file_mode) {
  char permissions[11];

  permissions[0] = S_ISDIR(file_mode) ? 'd' : '-';
  permissions[1] = (file_mode & S_IRUSR) ? 'r' : '-';
  permissions[2] = (file_mode & S_IWUSR) ? 'w' : '-';
  permissions[3] = (file_mode & S_IXUSR) ? 'x' : '-';
  permissions[4] = (file_mode & S_IRGRP) ? 'r' : '-';
  permissions[5] = (file_mode & S_IWGRP) ? 'w' : '-';
  permissions[6] = (file_mode & S_IXGRP) ? 'x' : '-';
  permissions[7] = (file_mode & S_IROTH) ? 'r' : '-';
  permissions[8] = (file_mode & S_IWOTH) ? 'w' : '-';
  permissions[9] = (file_mode & S_IXOTH) ? 'x' : '-';
  permissions[10] = '\0';

  printf("%s", permissions);
}

int main(int argc, char const** argv) {
  const char* directory_path = (argc > 1) ? argv[1] : "./";

  DIR* directory_pointer = opendir(directory_path);
  if (directory_pointer == NULL) {
    perror("Error opendir(directory_path)");
    return EXIT_FAILURE;
  }

  struct dirent* directory_entry;
  struct stat file_stat;
  char full_file_path[MAX_PATH_LENGTH];

  while ((directory_entry = readdir(directory_pointer)) != NULL) {
    if (strcmp(directory_entry->d_name, ".") == 0 || strcmp(directory_entry->d_name, "..") == 0) {
      continue;
    }

    snprintf(full_file_path, sizeof(full_file_path), "%s/%s", directory_path, directory_entry->d_name);

    if (stat(full_file_path, &file_stat) == -1) {
      perror("Error stat()");
      continue;
    }

    struct passwd* password_entry = getpwuid(file_stat.st_uid);
    struct group* group_entry = getgrgid(file_stat.st_gid);

    char date_buffer[MAX_DATE_BUFFER_LENGTH];
    struct tm* time_information = localtime(&file_stat.st_mtime);
    strftime(date_buffer, sizeof(date_buffer), "%d%m%y @ %Hh%M", time_information);

    printf("%-20s - ", directory_entry->d_name);
    print_permissions(file_stat.st_mode);
    printf(" %s : %s - %ld - %s\n", (password_entry != NULL ? password_entry->pw_name : "unknown"), (group_entry != NULL ? group_entry->gr_name : "unknown"), file_stat.st_size, date_buffer);
  }

  closedir(directory_pointer);
  return EXIT_SUCCESS;
}
