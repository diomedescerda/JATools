#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef PATH_MAX
#endif

int count_files(const char *path) {
  DIR *dir = opendir(path);
  if (dir == NULL) {
    perror("opendir failed");
    return -1;
  }

  int count = 0;
  struct dirent *entry;

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
      count++;
    }
  }

  closedir(dir);
  return count;
}

void process_make_command(const char *watched_path) {
  printf("Executing mp3forme in: %s\n", watched_path);
  if (chdir(watched_path)) {
    perror("Failed to change directory");
    return;
  }
  if (system("mp3forme") != 0) {
    fprintf(stderr, "Command failed in %s\n", watched_path);
  }
}

void process_move_command(const char *watched_path) {
  printf("Moving MP3 files from: %s\n", watched_path);
  if (chdir(watched_path)) {
    perror("Failed to change directory");
    return;
  }

  system("mkdir -p ../../../../聴聞");
  if (system("cp *.mp3 ../../../../聴聞/") != 0) {
    fprintf(stderr, "Move command failed in %s\n", watched_path);
  }

  system("mkdir -p ../../../../共有");
  if (system("mv *.mkv *.mp3 ../../../../共有/") != 0) {
    fprintf(stderr, "Move command failed in %s\n", watched_path);
  }
}

void process_clean_command() {
  printf("Cleaning target directories\n");
  char *target[] = {"/home/kenma/personal/AJATT/聴聞",
                    "/home/kenma/personal/AJATT/共有"};

  for (int i = 0; i < 2; i++) {
    if (count_files(target[i]) > 0) {
      char cmd[256];
      snprintf(cmd, sizeof(cmd), "rm %s/*", target[i]);
      system(cmd);
    } else {
      printf("Empty directory [%s]\n", target[i]);
    }
  }
}

int main(int argc, char *argv[]) {
  if (argc != 2 || (strcmp(argv[1], "make") != 0 && strcmp(argv[1], "move") &&
                    strcmp(argv[1], "clean") != 0)) {
    printf("Usage: %s [make|move]\n", argv[0]);
    printf("    make - run mp3forme in each watched directory\n");
    printf("    move - move mp3 files to ../../../../聴聞\n");
    printf("    clean - delete files from 聴聞 and 共有\n");
    return 1;
  }

  const char *base_dir = "/home/kenma/personal/AJATT/内容/見る";
  const char *target_dir = "watched";

  DIR *dir = opendir(base_dir);
  if (!dir) {
    perror("Failed to open base directory");
    return 1;
  }

  struct dirent *entry;
  int total_processed = 0;

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    char entry_path[PATH_MAX];
    int full_path = snprintf(entry_path, sizeof(entry_path), "%s/%s", base_dir,
                             entry->d_name);
    if (full_path < 0 || (size_t)full_path >= sizeof(entry_path)) {
      fprintf(stderr, "Skipping: Path too long\n");
      continue;
    }

    DIR *subdir = opendir(entry_path);
    if (!subdir) {
      continue;
    }

    struct dirent *subentry;
    int found_watched = 0;
    while ((subentry = readdir(subdir)) != NULL) {
      if (strcmp(subentry->d_name, target_dir) == 0) {
        found_watched = 1;
        break;
      }
    }
    closedir(subdir);

    if (found_watched) {
      char watched_path[PATH_MAX];
      full_path = snprintf(watched_path, sizeof(watched_path), "%s/%s",
                           entry_path, target_dir);
      if (full_path < 0 || (size_t)full_path >= sizeof(watched_path)) {
        fprintf(stderr, "Skipping: Watched path too long\n");
        continue;
      }

      if (strcmp(argv[1], "make") == 0) {
        process_make_command(watched_path);
      } else if (strcmp(argv[1], "move") == 0) {
        process_move_command(watched_path);
      }
      total_processed++;
    }
  }

  if (strcmp(argv[1], "clean") == 0) {
    process_clean_command();
  }

  closedir(dir);
  printf("Done. Total processed: %d\n", total_processed);
  return 0;
}
