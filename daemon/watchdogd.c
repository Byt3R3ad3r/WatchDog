#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <syslog.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct {
    char canary_path[256];
    int canary_count;
    char category[32];
    char email_to[256];
} Config;

#define MAX_DECoys 6
#define MAX_TYPES 4

#define ALERT_COOLDOWN 60
typedef struct {
    char file[512];
    time_t last_alert;
} AlertEntry;

#define MAX_ALERTS 32
AlertEntry alert_log[MAX_ALERTS];
int alert_count = 0;

int should_alert(const char *file) {
    time_t now = time(NULL);

    for (int i = 0; i < alert_count; i++) {
        if (strcmp(alert_log[i].file, file) == 0) {
            if (now - alert_log[i].last_alert < ALERT_COOLDOWN)
                return 0;

            alert_log[i].last_alert = now;
            return 1;
        }
    }

    // new file
    if (alert_count < MAX_ALERTS) {
        strncpy(alert_log[alert_count].file, file,
                sizeof(alert_log[alert_count].file)-1);
        alert_log[alert_count].last_alert = now;
        alert_count++;
    }

    return 1;
}

void fake_ownership(int fd, const char *path) {
    struct stat st;
    if (stat(path, &st) == 0) {
        if (fchown(fd, st.st_uid, st.st_gid) < 0) {
            syslog(LOG_WARNING, "fchown failed: %s", strerror(errno));
        }
    }
}

//Decoy file names, need to add more accurate data within
const char *decoyTypes[MAX_TYPES][MAX_DECoys] = {
    {"Tax_Deduction_2023.excel", "Tax_Deduction_2024.excel", "Tax_Deduction_2025.excel", "Car Loan schedule (Email to Victor).excel", "Hawaii vac spending.excel", "Work_cheques_2022.pdf"},
    {"Dr-Olsen_biopsy_report.pdf", "DiclofenacPrescription.pdf", "MRI-Reading-June.png", "MRI-Reading-July.png", "MRI-Reading-August.png", "MRI-Reading-September.png"},
    {"Lounge 6E access code.txt", "managementcontact.txt", "memodraft.txt", "Calvin-rejection-letter.worddocx", "HardwareCorporateRequest.worddocx", "memodraft2.txt"},
    {"Module B MATEN.pdf", "SDHolder.obj", "ScannedDocuments.pdf", "Loan-proposal-draft.txt", "Pet sitting schedule.txt", "coretightingabworkout.txt"}
};

int category_index(const char *cat) {
    if (strcasecmp(cat, "financial") == 0) return 0;
    if (strcasecmp(cat, "medical")   == 0) return 1;
    if (strcasecmp(cat, "company")   == 0) return 2;
    if (strcasecmp(cat, "personal")  == 0) return 3;
    return -1;
}






static volatile sig_atomic_t running = 1;

void handle_sigterm(int sig) { (void)sig; running = 0; }

int parse_config(Config *cfg) {
    cfg->canary_count = 1; // default values
    strncpy(cfg->category, "financial", sizeof(cfg->category) - 1);
    cfg->category[sizeof(cfg->category) - 1] = '\0';
    //Need to add default file


    FILE *f = fopen("/etc/watchdog/watchdog.conf", "r");
    if (!f) return -1;

    char line[512];
    char section[64] = {0};

    while (fgets(line, sizeof(line), f)) {
        char *s = line;
        while (*s == ' ' || *s == '\t') s++;

        if (*s == ';' || *s == '#' || *s == '\n') continue;

        if (*s == '[') {
            char *end = strchr(s, ']');
            if (end) { *end = '\0'; strcpy(section, s + 1); }
            continue;
        }

        char *eq = strchr(s, '=');
        if (!eq) continue;
        *eq = '\0';
        char *key = s;
        char *val = eq + 1;

        key[strcspn(key, "\r\n")] = 0;
        val[strcspn(val, "\r\n")] = 0;

        if (strcmp(section, "canary") == 0) {
            if (strcmp(key, "path") == 0)
                strncpy(cfg->canary_path, val, sizeof(cfg->canary_path)-1);
            else if (strcmp(key, "count") == 0)
                cfg->canary_count = atoi(val);
            else if (strcmp(key, "category") == 0)
                     strncpy(cfg->category, val, sizeof(cfg->category)-1);
        }

        else if (strcmp(section, "alert") == 0) {
            if (strcmp(key, "email") == 0)
                strncpy(cfg->email_to, val, sizeof(cfg->email_to)-1);
        }
    }

    fclose(f);
    return 0;
}

//Obselete canaries
/*void cleanup_canaries(const char *path) {
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "rm -f %s/decoy_*.txt", path);
    system(cmd);
}*/


void create_canaries(const char *path, int count, const char *category) {
    int idx = category_index(category);
    if (idx < 0) {
        syslog(LOG_ERR, "Invalid decoy category");
        return;
    }

    if (mkdir(path, 0755) < 0 && errno != EEXIST) {
        syslog(LOG_ERR, "mkdir failed: %s", strerror(errno));
        }

    for (int i = 0; i < count && i < MAX_DECoys; i++) {
        char file[512];
        snprintf(file, sizeof(file), "%s/%s", path, decoyTypes[idx][i]);
        int fd = open(file, O_CREAT | O_WRONLY | O_TRUNC, 0600);
        if (fd < 0) continue;

        fake_ownership(fd, path);

        dprintf(fd,
            "CONFIDENTIAL\n\n"
            "Internal document\n"
            "Last reviewed: %s\n\n"
            "Do not distribute.\n",
            ctime(&(time_t){time(NULL) - (rand() % 86400 * 30)})
        );

        for (int j = 0; j < rand() % 200; j++)
            dprintf(fd, " \n");

        close(fd);

        struct timespec ts[2];
        ts[0].tv_sec = ts[1].tv_sec = time(NULL) - (rand() % 86400 * 60);
        utimensat(AT_FDCWD, file, ts, 0);
    }
}

//Part of email alert, todo
void trigger_alert(const char *file, const char *event) {
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "/usr/local/lib/watchdog/alert.sh \"%s\" \"%s\"",
        file, event
    );
    system(cmd);
}


int main(void) {
    Config cfg = {0};
    if (parse_config(&cfg) != 0) {
        fprintf(stderr, "Failed to read config\n");
        return EXIT_FAILURE;
    }

    srand(time(NULL));

    
    time_t arm_time = 0;
    
    //cleanup_canaries(cfg.canary_path);

    //create_canaries(cfg.canary_path, cfg.canary_count);
    create_canaries(cfg.canary_path, cfg.canary_count, cfg.category);

    signal(SIGTERM, handle_sigterm);
    signal(SIGINT, handle_sigterm);

    openlog("watchdog", LOG_PID | LOG_CONS, LOG_DAEMON);


    int fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0) {
        syslog(LOG_ERR, "inotify_init failed: %s", strerror(errno));
        return EXIT_FAILURE;
    }

    #define MAX_CANARIES 128
        typedef struct {
        int wd;
        char file[512];
    } Watch;

    Watch canary_watches[MAX_CANARIES];
    int canary_watch_count = 0;

    
    for (int i = 0; i < cfg.canary_count && i < MAX_CANARIES; i++) {
        char file[512];
        int idx = category_index(cfg.category);
        snprintf(file, sizeof(file), "%s/%s", cfg.canary_path, decoyTypes[idx][i]);

        int w = inotify_add_watch(fd, file,
            IN_OPEN | IN_MODIFY | IN_DELETE_SELF);
        
            if (w < 0) {
            syslog(LOG_ERR, "inotify_add_watch failed for %s: %s", file, strerror(errno));
            continue;
    }

        
        canary_watches[canary_watch_count].wd = w;
        
        strncpy(canary_watches[canary_watch_count].file, file, sizeof(canary_watches[canary_watch_count].file) - 1);
        canary_watches[canary_watch_count].file[sizeof(canary_watches[canary_watch_count].file) - 1] = '\0';

        canary_watch_count++;
    
    
    }

    arm_time = time(NULL);  
    syslog(LOG_INFO, "WatchDog armed on %s", cfg.canary_path);

    char buffer[4096];

    while (running) {
        int len = read(fd, buffer, sizeof(buffer));
        if (len < 0) {
            if (errno == EAGAIN) {
                usleep(200000);
                continue;
            }
            syslog(LOG_ERR, "read failed: %s", strerror(errno));
            break;
        }

        for (char *ptr = buffer; ptr < buffer + len; ) {
            struct inotify_event *event = (struct inotify_event *)ptr;

            // Ignore startup noise
            if (time(NULL) - arm_time < 10) {
                ptr += sizeof(struct inotify_event) + event->len;
                continue;
            }

            // find filename
            char *filename = NULL;
            for (int i = 0; i < canary_watch_count; i++) {
                if (canary_watches[i].wd == event->wd) {
                    filename = canary_watches[i].file;
                    break;
                }
            }

            if ((event->mask & IN_OPEN) && filename) {
                if (should_alert(filename)) {
                syslog(LOG_ALERT, "WATCHDOG ALERT: OPENED %s", filename);
                trigger_alert(filename, "OPENED");
                }
                else {
                syslog(LOG_INFO, "WatchDog cooldown active for %s", filename);
                }
                }
    

            if (event->mask & IN_DELETE_SELF) {
            syslog(LOG_ALERT, "WATCHDOG ALERT: DELETED %s",
                   filename ? filename : "unknown");
            }

        ptr += sizeof(struct inotify_event) + event->len;
        }
    }

    for (int i = 0; i < canary_watch_count; i++) {
    inotify_rm_watch(fd, canary_watches[i].wd);
    }

    close(fd);

    closelog();
    return EXIT_SUCCESS;
}
