/*
 * simple_watcher.c - Cross-platform File system watcher for Eiffel
 *
 * Windows: Uses ReadDirectoryChangesW with overlapped I/O
 * Linux: Uses inotify
 * macOS: Uses FSEvents
 *
 * Copyright (c) 2025 Larry Rix - MIT License
 */

#include "simple_watcher.h"
#include <stdlib.h>
#include <string.h>

#if !defined(__APPLE__)
static char last_error_msg[512] = {0};
#endif

#if defined(_WIN32) || defined(EIF_WINDOWS)
/* ============ WINDOWS IMPLEMENTATION ============ */

static void store_last_error(void) {
    DWORD err = GetLastError();
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, err,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        last_error_msg, sizeof(last_error_msg) - 1, NULL
    );
}

static DWORD flags_to_filter(int flags) {
    DWORD filter = 0;
    if (flags & SWF_FILE_NAME)  filter |= FILE_NOTIFY_CHANGE_FILE_NAME;
    if (flags & SWF_DIR_NAME)   filter |= FILE_NOTIFY_CHANGE_DIR_NAME;
    if (flags & SWF_ATTRIBUTES) filter |= FILE_NOTIFY_CHANGE_ATTRIBUTES;
    if (flags & SWF_SIZE)       filter |= FILE_NOTIFY_CHANGE_SIZE;
    if (flags & SWF_LAST_WRITE) filter |= FILE_NOTIFY_CHANGE_LAST_WRITE;
    if (flags & SWF_SECURITY)   filter |= FILE_NOTIFY_CHANGE_SECURITY;
    return filter;
}

static int action_to_event_type(DWORD action) {
    switch (action) {
        case FILE_ACTION_ADDED:            return SWE_FILE_ADDED;
        case FILE_ACTION_REMOVED:          return SWE_FILE_REMOVED;
        case FILE_ACTION_MODIFIED:         return SWE_FILE_MODIFIED;
        case FILE_ACTION_RENAMED_OLD_NAME: return SWE_FILE_RENAMED;
        case FILE_ACTION_RENAMED_NEW_NAME: return SWE_FILE_RENAMED;
        default:                           return SWE_FILE_MODIFIED;
    }
}

sw_watcher* sw_create(const char* path, int watch_subtree, int flags) {
    sw_watcher* w;

    w = (sw_watcher*)malloc(sizeof(sw_watcher));
    if (!w) return NULL;
    memset(w, 0, sizeof(sw_watcher));

    w->watch_path = _strdup(path);
    w->watch_subtree = watch_subtree;
    w->filter = flags_to_filter(flags);

    /* Open directory */
    w->directory_handle = CreateFileA(
        path,
        FILE_LIST_DIRECTORY,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
        NULL
    );

    if (w->directory_handle == INVALID_HANDLE_VALUE) {
        store_last_error();
        w->error_message = _strdup(last_error_msg);
        return w;
    }

    /* Create event for overlapped I/O */
    w->event_handle = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!w->event_handle) {
        store_last_error();
        w->error_message = _strdup(last_error_msg);
        CloseHandle(w->directory_handle);
        w->directory_handle = INVALID_HANDLE_VALUE;
        return w;
    }

    w->overlapped.hEvent = w->event_handle;

    return w;
}

int sw_start(sw_watcher* watcher) {
    BOOL result;

    if (!watcher || watcher->directory_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    ResetEvent(watcher->event_handle);

    result = ReadDirectoryChangesW(
        watcher->directory_handle,
        watcher->buffer,
        sizeof(watcher->buffer),
        watcher->watch_subtree,
        watcher->filter,
        NULL,
        &watcher->overlapped,
        NULL
    );

    if (!result && GetLastError() != ERROR_IO_PENDING) {
        store_last_error();
        if (watcher->error_message) free(watcher->error_message);
        watcher->error_message = _strdup(last_error_msg);
        return 0;
    }

    watcher->is_watching = 1;
    return 1;
}

static sw_event* process_notification(sw_watcher* watcher, FILE_NOTIFY_INFORMATION* info) {
    sw_event* event;
    int filename_len;
    char* filename;

    event = (sw_event*)malloc(sizeof(sw_event));
    if (!event) return NULL;
    memset(event, 0, sizeof(sw_event));

    event->event_type = action_to_event_type(info->Action);

    /* Convert wide filename to narrow */
    filename_len = WideCharToMultiByte(CP_UTF8, 0, info->FileName,
                                       info->FileNameLength / sizeof(WCHAR),
                                       NULL, 0, NULL, NULL);
    filename = (char*)malloc(filename_len + 1);
    if (filename) {
        WideCharToMultiByte(CP_UTF8, 0, info->FileName,
                           info->FileNameLength / sizeof(WCHAR),
                           filename, filename_len, NULL, NULL);
        filename[filename_len] = '\0';
        event->filename = filename;
    }

    return event;
}

sw_event* sw_poll(sw_watcher* watcher) {
    DWORD wait_result;
    DWORD bytes_returned;
    FILE_NOTIFY_INFORMATION* info;
    sw_event* event;

    if (!watcher || !watcher->is_watching) {
        return NULL;
    }

    /* Check if event signaled without waiting */
    wait_result = WaitForSingleObject(watcher->event_handle, 0);
    if (wait_result != WAIT_OBJECT_0) {
        return NULL;
    }

    /* Get result */
    if (!GetOverlappedResult(watcher->directory_handle, &watcher->overlapped,
                             &bytes_returned, FALSE)) {
        return NULL;
    }

    if (bytes_returned == 0) {
        /* Restart watching */
        sw_start(watcher);
        return NULL;
    }

    /* Process first notification */
    info = (FILE_NOTIFY_INFORMATION*)watcher->buffer;
    event = process_notification(watcher, info);

    /* Restart watching for next change */
    sw_start(watcher);

    return event;
}

sw_event* sw_wait(sw_watcher* watcher, int timeout_ms) {
    DWORD wait_result;
    DWORD bytes_returned;
    FILE_NOTIFY_INFORMATION* info;
    sw_event* event;
    DWORD timeout;

    if (!watcher || !watcher->is_watching) {
        return NULL;
    }

    timeout = (timeout_ms < 0) ? INFINITE : (DWORD)timeout_ms;

    wait_result = WaitForSingleObject(watcher->event_handle, timeout);
    if (wait_result != WAIT_OBJECT_0) {
        return NULL;
    }

    /* Get result */
    if (!GetOverlappedResult(watcher->directory_handle, &watcher->overlapped,
                             &bytes_returned, FALSE)) {
        return NULL;
    }

    if (bytes_returned == 0) {
        sw_start(watcher);
        return NULL;
    }

    /* Process first notification */
    info = (FILE_NOTIFY_INFORMATION*)watcher->buffer;
    event = process_notification(watcher, info);

    /* Restart watching */
    sw_start(watcher);

    return event;
}

int sw_is_watching(sw_watcher* watcher) {
    return watcher ? watcher->is_watching : 0;
}

const char* sw_get_path(sw_watcher* watcher) {
    return watcher ? watcher->watch_path : NULL;
}

const char* sw_get_error(sw_watcher* watcher) {
    return watcher ? watcher->error_message : NULL;
}

void sw_close(sw_watcher* watcher) {
    if (watcher) {
        if (watcher->directory_handle != INVALID_HANDLE_VALUE) {
            CancelIo(watcher->directory_handle);
            CloseHandle(watcher->directory_handle);
        }
        if (watcher->event_handle) {
            CloseHandle(watcher->event_handle);
        }
        if (watcher->watch_path) {
            free(watcher->watch_path);
        }
        if (watcher->error_message) {
            free(watcher->error_message);
        }
        if (watcher->pending_event) {
            sw_free_event(watcher->pending_event);
        }
        free(watcher);
    }
}

void sw_free_event(sw_event* event) {
    if (event) {
        if (event->filename) free(event->filename);
        if (event->old_filename) free(event->old_filename);
        free(event);
    }
}

#elif defined(__APPLE__)
/* ============ macOS FSEVENTS IMPLEMENTATION ============ */
/* FSEvents delivers on a dispatch queue, so events wait in a list for sw_poll
   and sw_wait. It watches whole subtrees natively; for a top-level watch the
   deeper events are dropped here. It merges a burst on one file into one event
   with several flags, so the file's present state settles which event it is. */

#include <CoreServices/CoreServices.h>
#include <dispatch/dispatch.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define SW_LATENCY_SECONDS 0.05
#define SW_SEEN_SLOTS 64

struct sw_queued_s {
    sw_event* event;
    struct sw_queued_s* next;
};

/* FSEvents flags are sticky: for a while after a file is created, every report
   on it still says "created". So each path remembers the flags already acted
   on, and only new ones count. Used on the dispatch queue only. */
struct sw_seen_s {
    char* name;
    FSEventStreamEventFlags flags;
};

static void set_error(sw_watcher* w, const char* a_text) {
    if (w->error_message) free(w->error_message);
    w->error_message = strdup(a_text);
}

/* The slot remembering `a_name', taking the oldest when it is new. */
static struct sw_seen_s* seen_slot(sw_watcher* w, const char* a_name) {
    struct sw_seen_s* l_seen = (struct sw_seen_s*)w->seen;
    struct sw_seen_s* l_slot;
    int l_i;
    for (l_i = 0; l_i < SW_SEEN_SLOTS; l_i++) {
        if (l_seen[l_i].name && strcmp(l_seen[l_i].name, a_name) == 0) return &l_seen[l_i];
    }
    l_slot = &l_seen[w->seen_next];
    w->seen_next = (w->seen_next + 1) % SW_SEEN_SLOTS;
    free(l_slot->name);
    l_slot->name = strdup(a_name);
    l_slot->flags = 0;
    return l_slot;
}

/* What to remember of `a_flags': a path that is gone starts afresh. */
static FSEventStreamEventFlags flags_to_remember(FSEventStreamEventFlags a_flags, int a_exists) {
    return a_exists ? a_flags : 0;
}

/* The sw_event for one FSEvents report, or NULL when it is filtered out. */
static sw_event* new_event(sw_watcher* w, const char* a_path, FSEventStreamEventFlags a_flags) {
    size_t l_root = strlen(w->real_path);
    const char* l_name;
    struct stat l_info;
    struct sw_seen_s* l_seen;
    FSEventStreamEventFlags l_new;
    sw_event* l_event;
    int l_type;
    int l_exists = lstat(a_path, &l_info) == 0;
    int l_is_dir = (a_flags & kFSEventStreamEventFlagItemIsDir) != 0;
    int l_names = w->filter & (l_is_dir ? SWF_DIR_NAME : SWF_FILE_NAME);

    if (strncmp(a_path, w->real_path, l_root) != 0 || a_path[l_root] != '/') return NULL;
    l_name = a_path + l_root + 1;
    if (!*l_name) return NULL;
    if (!w->watch_subtree && strchr(l_name, '/')) return NULL;

    l_seen = seen_slot(w, l_name);
    l_new = a_flags & ~l_seen->flags;
    l_seen->flags = flags_to_remember(a_flags, l_exists);

    if ((a_flags & kFSEventStreamEventFlagItemRemoved) && !l_exists) {
        if (!l_names) return NULL;
        l_type = SWE_FILE_REMOVED;
    } else if (l_new & kFSEventStreamEventFlagItemRenamed) {
        if (!l_names) return NULL;
        l_type = SWE_FILE_RENAMED;
    } else if (l_new & kFSEventStreamEventFlagItemCreated) {
        if (!l_names) return NULL;
        l_type = SWE_FILE_ADDED;
    } else if (a_flags & (kFSEventStreamEventFlagItemModified | kFSEventStreamEventFlagItemInodeMetaMod
                          | kFSEventStreamEventFlagItemChangeOwner | kFSEventStreamEventFlagItemXattrMod
                          | kFSEventStreamEventFlagItemFinderInfoMod)) {
        if (!(w->filter & (SWF_ATTRIBUTES | SWF_SIZE | SWF_LAST_WRITE | SWF_SECURITY))) return NULL;
        l_type = SWE_FILE_MODIFIED;
    } else {
        return NULL;
    }

    l_event = (sw_event*)calloc(1, sizeof(sw_event));
    if (!l_event) return NULL;
    l_event->event_type = l_type;
    l_event->filename = strdup(l_name);
    return l_event;
}

static void on_events(ConstFSEventStreamRef a_stream, void* a_watcher, size_t a_count, void* a_paths,
                      const FSEventStreamEventFlags a_flags[], const FSEventStreamEventId a_ids[]) {
    sw_watcher* w = (sw_watcher*)a_watcher;
    char** l_paths = (char**)a_paths;
    size_t l_i;
    (void)a_stream;
    (void)a_ids;

    for (l_i = 0; l_i < a_count; l_i++) {
        sw_event* l_event = new_event(w, l_paths[l_i], a_flags[l_i]);
        if (l_event) {
            struct sw_queued_s* l_node = (struct sw_queued_s*)malloc(sizeof(struct sw_queued_s));
            if (!l_node) {
                sw_free_event(l_event);
            } else {
                l_node->event = l_event;
                l_node->next = NULL;
                pthread_mutex_lock(&w->lock);
                if (w->last) w->last->next = l_node; else w->first = l_node;
                w->last = l_node;
                pthread_cond_broadcast(&w->ready);
                pthread_mutex_unlock(&w->lock);
            }
        }
    }
}

sw_watcher* sw_create(const char* path, int watch_subtree, int flags) {
    sw_watcher* w;
    char l_real[PATH_MAX];
    struct stat l_info;

    w = (sw_watcher*)calloc(1, sizeof(sw_watcher));
    if (!w) return NULL;
    pthread_mutex_init(&w->lock, NULL);
    pthread_cond_init(&w->ready, NULL);
    w->watch_path = strdup(path);
    w->watch_subtree = watch_subtree;
    w->filter = flags;
    w->seen = calloc(SW_SEEN_SLOTS, sizeof(struct sw_seen_s));

    /* FSEvents reports resolved paths: /tmp arrives as /private/tmp. */
    if (!realpath(path, l_real) || stat(l_real, &l_info) != 0) {
        set_error(w, strerror(errno));
        return w;
    }
    if (!S_ISDIR(l_info.st_mode)) {
        set_error(w, strerror(ENOTDIR));
        return w;
    }
    w->real_path = strdup(l_real);
    return w;
}

int sw_start(sw_watcher* watcher) {
    FSEventStreamContext l_context;
    FSEventStreamRef l_stream;
    CFStringRef l_path;
    CFArrayRef l_paths;
    dispatch_queue_t l_queue;

    if (!watcher || !watcher->real_path || !watcher->seen) return 0;
    if (watcher->is_watching) return 1;

    memset(&l_context, 0, sizeof(l_context));
    l_context.info = watcher;
    l_path = CFStringCreateWithCString(NULL, watcher->real_path, kCFStringEncodingUTF8);
    if (!l_path) { set_error(watcher, "The path is not valid UTF-8"); return 0; }
    l_paths = CFArrayCreate(NULL, (const void**)&l_path, 1, &kCFTypeArrayCallBacks);
    CFRelease(l_path);
    if (!l_paths) { set_error(watcher, "Out of memory"); return 0; }
    l_stream = FSEventStreamCreate(NULL, on_events, &l_context, l_paths, kFSEventStreamEventIdSinceNow,
                                   SW_LATENCY_SECONDS, kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagNoDefer);
    CFRelease(l_paths);
    if (!l_stream) { set_error(watcher, "FSEventStreamCreate failed"); return 0; }
    l_queue = dispatch_queue_create("simple_watcher", NULL);
    FSEventStreamSetDispatchQueue(l_stream, l_queue);
    if (!FSEventStreamStart(l_stream)) {
        FSEventStreamInvalidate(l_stream);
        FSEventStreamRelease(l_stream);
        dispatch_release(l_queue);
        set_error(watcher, "FSEventStreamStart failed");
        return 0;
    }
    watcher->stream = l_stream;
    watcher->queue = l_queue;
    watcher->is_watching = 1;
    return 1;
}

/* With the lock held: take the oldest queued event, if any. */
static sw_event* take_event(sw_watcher* watcher) {
    struct sw_queued_s* l_node = watcher->first;
    sw_event* l_event = NULL;
    if (l_node) {
        watcher->first = l_node->next;
        if (!watcher->first) watcher->last = NULL;
        l_event = l_node->event;
        free(l_node);
    }
    return l_event;
}

sw_event* sw_poll(sw_watcher* watcher) {
    sw_event* l_event;
    if (!watcher || !watcher->is_watching) return NULL;
    pthread_mutex_lock(&watcher->lock);
    l_event = take_event(watcher);
    pthread_mutex_unlock(&watcher->lock);
    return l_event;
}

sw_event* sw_wait(sw_watcher* watcher, int timeout_ms) {
    struct timespec l_until;
    sw_event* l_event;
    int l_result = 0;

    if (!watcher || !watcher->is_watching) return NULL;
    if (timeout_ms >= 0) {
        clock_gettime(CLOCK_REALTIME, &l_until);
        l_until.tv_sec += timeout_ms / 1000;
        l_until.tv_nsec += (long)(timeout_ms % 1000) * 1000000L;
        if (l_until.tv_nsec >= 1000000000L) {
            l_until.tv_sec += 1;
            l_until.tv_nsec -= 1000000000L;
        }
    }
    pthread_mutex_lock(&watcher->lock);
    while (!watcher->first && l_result == 0) {
        if (timeout_ms < 0) {
            pthread_cond_wait(&watcher->ready, &watcher->lock);
        } else {
            l_result = pthread_cond_timedwait(&watcher->ready, &watcher->lock, &l_until);
        }
    }
    l_event = take_event(watcher);
    pthread_mutex_unlock(&watcher->lock);
    return l_event;
}

int sw_is_watching(sw_watcher* watcher) {
    return watcher ? watcher->is_watching : 0;
}

const char* sw_get_path(sw_watcher* watcher) {
    return watcher ? watcher->watch_path : NULL;
}

const char* sw_get_error(sw_watcher* watcher) {
    return watcher ? watcher->error_message : NULL;
}

static void nothing(void* a_ignored) { (void)a_ignored; }

void sw_close(sw_watcher* watcher) {
    sw_event* l_event;
    if (watcher) {
        if (watcher->stream) {
            FSEventStreamRef l_stream = (FSEventStreamRef)watcher->stream;
            dispatch_queue_t l_queue = (dispatch_queue_t)watcher->queue;
            FSEventStreamStop(l_stream);
            FSEventStreamInvalidate(l_stream);
            /* A callback already on the queue finishes before the record goes. */
            dispatch_sync_f(l_queue, NULL, nothing);
            FSEventStreamRelease(l_stream);
            dispatch_release(l_queue);
        }
        while ((l_event = take_event(watcher)) != NULL) sw_free_event(l_event);
        if (watcher->seen) {
            struct sw_seen_s* l_seen = (struct sw_seen_s*)watcher->seen;
            int l_i;
            for (l_i = 0; l_i < SW_SEEN_SLOTS; l_i++) free(l_seen[l_i].name);
            free(l_seen);
        }
        pthread_cond_destroy(&watcher->ready);
        pthread_mutex_destroy(&watcher->lock);
        if (watcher->watch_path) free(watcher->watch_path);
        if (watcher->real_path) free(watcher->real_path);
        if (watcher->error_message) free(watcher->error_message);
        free(watcher);
    }
}

void sw_free_event(sw_event* event) {
    if (event) {
        if (event->filename) free(event->filename);
        if (event->old_filename) free(event->old_filename);
        free(event);
    }
}

#else
/* ============ LINUX INOTIFY IMPLEMENTATION ============ */

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/inotify.h>
#include <poll.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>

/* One watched folder. `relative' is "" for the root. */
struct sw_dir_s {
    int descriptor;
    char* relative;
};

static void store_last_error(void) {
    snprintf(last_error_msg, sizeof(last_error_msg) - 1, "%s", strerror(errno));
}

/* Convert our SWF_* flags to inotify mask */
static uint32_t flags_to_inotify_mask(int flags) {
    uint32_t mask = 0;
    if (flags & SWF_FILE_NAME)  mask |= IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO;
    if (flags & SWF_DIR_NAME)   mask |= IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | IN_ISDIR;
    if (flags & SWF_ATTRIBUTES) mask |= IN_ATTRIB;
    if (flags & SWF_SIZE)       mask |= IN_MODIFY;
    if (flags & SWF_LAST_WRITE) mask |= IN_MODIFY | IN_CLOSE_WRITE;
    if (flags & SWF_SECURITY)   mask |= IN_ATTRIB;
    return mask;
}

/* Convert inotify mask to our event type */
static int inotify_mask_to_event_type(uint32_t mask) {
    if (mask & IN_CREATE)      return SWE_FILE_ADDED;
    if (mask & IN_DELETE)      return SWE_FILE_REMOVED;
    if (mask & IN_MOVED_FROM)  return SWE_FILE_RENAMED;
    if (mask & IN_MOVED_TO)    return SWE_FILE_RENAMED;
    if (mask & (IN_MODIFY | IN_CLOSE_WRITE | IN_ATTRIB))
                               return SWE_FILE_MODIFIED;
    return SWE_FILE_MODIFIED;
}

static void remember_dir(sw_watcher* w, int descriptor, const char* relative) {
    int i;
    for (i = 0; i < w->dir_count; i++) {
        if (w->dirs[i].descriptor == descriptor) {
            free(w->dirs[i].relative);
            w->dirs[i].relative = strdup(relative);
            return;
        }
    }
    if (w->dir_count == w->dir_capacity) {
        int capacity = w->dir_capacity ? w->dir_capacity * 2 : 16;
        struct sw_dir_s* grown = (struct sw_dir_s*)realloc(w->dirs, (size_t)capacity * sizeof(struct sw_dir_s));
        if (!grown) return;
        w->dirs = grown;
        w->dir_capacity = capacity;
    }
    w->dirs[w->dir_count].descriptor = descriptor;
    w->dirs[w->dir_count].relative = strdup(relative);
    w->dir_count++;
}

static void forget_dir(sw_watcher* w, int descriptor) {
    int i;
    for (i = 0; i < w->dir_count; i++) {
        if (w->dirs[i].descriptor == descriptor) {
            free(w->dirs[i].relative);
            w->dirs[i] = w->dirs[w->dir_count - 1];
            w->dir_count--;
            return;
        }
    }
}

static const char* relative_of(sw_watcher* w, int descriptor) {
    int i;
    for (i = 0; i < w->dir_count; i++) {
        if (w->dirs[i].descriptor == descriptor) return w->dirs[i].relative;
    }
    return NULL;
}

/* `a' joined to `b' with a slash, leaving out an empty `a'. Caller frees. */
static char* join_path(const char* a, const char* b) {
    size_t size = strlen(a) + strlen(b) + 2;
    char* result = (char*)malloc(size);
    if (result) {
        if (a[0]) snprintf(result, size, "%s/%s", a, b);
        else snprintf(result, size, "%s", b);
    }
    return result;
}

static void note_error_once(sw_watcher* w) {
    if (!w->error_message) {
        store_last_error();
        w->error_message = strdup(last_error_msg);
    }
}

/* Watch the folder at `relative' under the root and every folder below it. Links
   are not followed, so a loop cannot trap this. A failure, such as running out of
   watches, is kept in `error_message' rather than passed over in silence. */
static void watch_tree(sw_watcher* w, const char* relative) {
    char* full = join_path(w->watch_path, relative);
    DIR* dir;
    struct dirent* entry;
    int descriptor;

    if (!full) return;
    descriptor = inotify_add_watch(w->inotify_fd, full, w->watch_mask | IN_ONLYDIR | IN_DONT_FOLLOW);
    if (descriptor < 0) {
        if (errno != ENOENT && errno != ENOTDIR) note_error_once(w);
        free(full);
        return;
    }
    remember_dir(w, descriptor, relative);
    dir = opendir(full);
    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            char* child_full;
            struct stat info;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
            child_full = join_path(full, entry->d_name);
            if (child_full && lstat(child_full, &info) == 0 && S_ISDIR(info.st_mode)) {
                char* child = join_path(relative, entry->d_name);
                if (child) {
                    watch_tree(w, child);
                    free(child);
                }
            }
            free(child_full);
        }
        closedir(dir);
    }
    free(full);
}

sw_watcher* sw_create(const char* path, int watch_subtree, int flags) {
    sw_watcher* w;
    uint32_t mask;

    w = (sw_watcher*)malloc(sizeof(sw_watcher));
    if (!w) return NULL;
    memset(w, 0, sizeof(sw_watcher));

    w->watch_path = strdup(path);
    w->watch_subtree = watch_subtree;
    w->filter = flags;
    w->inotify_fd = -1;
    w->watch_descriptor = -1;

    /* Create inotify instance */
    w->inotify_fd = inotify_init1(IN_NONBLOCK);
    if (w->inotify_fd < 0) {
        store_last_error();
        w->error_message = strdup(last_error_msg);
        return w;
    }

    /* Add watch. A subtree also needs to hear of new folders, whatever was asked. */
    mask = flags_to_inotify_mask(flags);
    w->user_mask = mask;
    w->watch_mask = watch_subtree ? (mask | IN_CREATE | IN_MOVED_TO | IN_MOVED_FROM | IN_DELETE) : mask;
    w->watch_descriptor = inotify_add_watch(w->inotify_fd, path, w->watch_mask);
    if (w->watch_descriptor < 0) {
        store_last_error();
        w->error_message = strdup(last_error_msg);
        close(w->inotify_fd);
        w->inotify_fd = -1;
        return w;
    }

    remember_dir(w, w->watch_descriptor, "");

    /* inotify watches one folder at a time, so a subtree is a watch per folder. */
    if (watch_subtree) {
        DIR* dir = opendir(path);
        struct dirent* entry;
        if (dir) {
            while ((entry = readdir(dir)) != NULL) {
                char* child_full;
                struct stat info;
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
                child_full = join_path(path, entry->d_name);
                if (child_full && lstat(child_full, &info) == 0 && S_ISDIR(info.st_mode)) {
                    watch_tree(w, entry->d_name);
                }
                free(child_full);
            }
            closedir(dir);
        }
    }

    return w;
}

int sw_start(sw_watcher* watcher) {
    if (!watcher || watcher->inotify_fd < 0) {
        return 0;
    }
    watcher->is_watching = 1;
    return 1;
}

/* The next event the caller asked for, keeping the folder table up to date. */
static sw_event* read_next_event(sw_watcher* watcher) {
    struct inotify_event* event;
    sw_event* result;
    const char* relative;
    char* name;

    for (;;) {
        if (watcher->buffer_pos >= watcher->buffer_len) {
            /* Need to read more data */
            ssize_t len = read(watcher->inotify_fd, watcher->buffer, sizeof(watcher->buffer));
            if (len <= 0) {
                return NULL;
            }
            watcher->buffer_len = (int)len;
            watcher->buffer_pos = 0;
        }

        event = (struct inotify_event*)(watcher->buffer + watcher->buffer_pos);
        watcher->buffer_pos += sizeof(struct inotify_event) + event->len;

        /* The kernel dropped this watch: a bookkeeping notice, not a change. */
        if (event->mask & IN_IGNORED) {
            forget_dir(watcher, event->wd);
            continue;
        }
        relative = relative_of(watcher, event->wd);
        if (!relative) relative = "";
        name = join_path(relative, event->len > 0 ? event->name : "");

        if (watcher->watch_subtree && (event->mask & IN_ISDIR) && (event->mask & (IN_CREATE | IN_MOVED_TO))) {
            if (name) watch_tree(watcher, name);
        }
        if (!(event->mask & watcher->user_mask)) {
            free(name);
            continue;
        }

        result = (sw_event*)malloc(sizeof(sw_event));
        if (!result) {
            free(name);
            return NULL;
        }
        memset(result, 0, sizeof(sw_event));
        result->event_type = inotify_mask_to_event_type(event->mask);
        if (event->len > 0) {
            result->filename = name;
        } else {
            free(name);
        }
        return result;
    }
}

sw_event* sw_poll(sw_watcher* watcher) {
    if (!watcher || !watcher->is_watching || watcher->inotify_fd < 0) {
        return NULL;
    }
    return read_next_event(watcher);
}

sw_event* sw_wait(sw_watcher* watcher, int timeout_ms) {
    struct pollfd pfd;
    int ret;

    if (!watcher || !watcher->is_watching || watcher->inotify_fd < 0) {
        return NULL;
    }

    /* First check if there's buffered data */
    if (watcher->buffer_pos < watcher->buffer_len) {
        return read_next_event(watcher);
    }

    /* Wait for data */
    pfd.fd = watcher->inotify_fd;
    pfd.events = POLLIN;
    ret = poll(&pfd, 1, timeout_ms);

    if (ret <= 0) {
        return NULL;
    }

    return read_next_event(watcher);
}

int sw_is_watching(sw_watcher* watcher) {
    return watcher ? watcher->is_watching : 0;
}

const char* sw_get_path(sw_watcher* watcher) {
    return watcher ? watcher->watch_path : NULL;
}

const char* sw_get_error(sw_watcher* watcher) {
    return watcher ? watcher->error_message : NULL;
}

void sw_close(sw_watcher* watcher) {
    if (watcher) {
        if (watcher->watch_descriptor >= 0 && watcher->inotify_fd >= 0) {
            inotify_rm_watch(watcher->inotify_fd, watcher->watch_descriptor);
        }
        if (watcher->inotify_fd >= 0) {
            close(watcher->inotify_fd);
        }
        if (watcher->watch_path) {
            free(watcher->watch_path);
        }
        if (watcher->error_message) {
            free(watcher->error_message);
        }
        if (watcher->pending_event) {
            sw_free_event(watcher->pending_event);
        }
        if (watcher->dirs) {
            int i;
            for (i = 0; i < watcher->dir_count; i++) free(watcher->dirs[i].relative);
            free(watcher->dirs);
        }
        free(watcher);
    }
}

void sw_free_event(sw_event* event) {
    if (event) {
        if (event->filename) free(event->filename);
        if (event->old_filename) free(event->old_filename);
        free(event);
    }
}

#endif /* _WIN32 */
