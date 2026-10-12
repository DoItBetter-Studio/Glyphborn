#include "scripting/xeno_runtime.h"

#if (defined(__linux__) && !defined(__YGGDRASIL__)) || defined(_WIN64)

#include "platform/platform.h"
#include "vm.h"
#include "stdlib_register.h"

#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN64
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#define XENO_RUNTIME_PATH_MAX 1024
#define XENO_RUNTIME_MOD_MAX 64

static XenoVM s_vm;
static bool s_vm_initialized;

static XenoResult xeno_host_print(XenoVM *vm, int argc, Value *argv,
                                  Value *out)
{
    (void)vm;
    (void)out;
    if (argc != 1)
    {
        xeno_vm_error(vm, "print host function expects one argument");
        return XENO_RUNTIME_ERROR;
    }

    printf("[XenoScript] %s\n",
           argv[0].is_null || !argv[0].s ? "null" : argv[0].s);
    return XENO_OK;
}

static bool mod_is_loaded(const char *name)
{
    for (int i = 0; i < s_vm.stdlib_module_count; i++)
        if (strcmp(s_vm.stdlib_loaded_names[i], name) == 0)
            return true;
    return false;
}

static bool valid_mod_name(const char *name)
{
    if (!name[0] || strcmp(name, ".") == 0 || strcmp(name, "..") == 0)
        return false;
    for (const unsigned char *p = (const unsigned char *)name; *p; p++)
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z') ||
              (*p >= '0' && *p <= '9') || *p == '_' || *p == '-' ||
              *p == '.'))
            return false;
    return true;
}

static bool load_mod_archive(const char *mods_directory, const char *path,
                             char loading[XENO_RUNTIME_MOD_MAX][XAR_MAX_NAME],
                             int depth)
{
    if (depth >= XENO_RUNTIME_MOD_MAX)
    {
        fprintf(stderr, "[XenoScript] mod dependency chain exceeds %d archives\n",
                XENO_RUNTIME_MOD_MAX);
        return false;
    }

    XarArchive archive;
    XarResult read_result = xar_read(&archive, path);
    if (read_result != XAR_OK)
    {
        fprintf(stderr, "[XenoScript] failed to read mod '%s': %s\n",
                path, xar_result_str(read_result));
        return false;
    }
    if (!valid_mod_name(archive.manifest.name))
    {
        fprintf(stderr, "[XenoScript] mod archive '%s' has an invalid manifest name\n",
                path);
        xar_archive_free(&archive);
        return false;
    }

    const char *name = archive.manifest.name;
    if (mod_is_loaded(name))
    {
        xar_archive_free(&archive);
        return true;
    }
    for (int i = 0; i < depth; i++)
    {
        if (strcmp(loading[i], name) == 0)
        {
            fprintf(stderr, "[XenoScript] cyclic mod dependency involving '%s'\n",
                    name);
            xar_archive_free(&archive);
            return false;
        }
    }
    memcpy(loading[depth], name, strlen(name) + 1);

    for (int i = 0; i < archive.manifest.dep_count; i++)
    {
        if (!valid_mod_name(archive.manifest.dependencies[i]))
        {
            fprintf(stderr, "[XenoScript] mod '%s' has an invalid dependency name\n",
                    name);
            xar_archive_free(&archive);
            return false;
        }
        char dependency_path[XENO_RUNTIME_PATH_MAX];
        int length = snprintf(dependency_path, sizeof(dependency_path),
                              "%s/%s.xar", mods_directory,
                              archive.manifest.dependencies[i]);
        if (length < 0 || (size_t)length >= sizeof(dependency_path))
        {
            fprintf(stderr, "[XenoScript] path for dependency '%s' is too long\n",
                    archive.manifest.dependencies[i]);
            xar_archive_free(&archive);
            return false;
        }
        if (!load_mod_archive(mods_directory, dependency_path, loading, depth + 1))
        {
            xar_archive_free(&archive);
            return false;
        }
    }

    if (!xeno_vm_load_xar(&s_vm, &archive))
    {
        fprintf(stderr, "[XenoScript] failed to load mod '%s' from '%s'\n",
                name, path);
        xar_archive_free(&archive);
        return false;
    }
    fprintf(stderr, "[XenoScript] loaded mod '%s'\n", name);
    xar_archive_free(&archive);
    return true;
}

static int compare_mod_names(const void *left, const void *right)
{
    const char *const *a = left;
    const char *const *b = right;
    return strcmp(*a, *b);
}

static bool load_mods_from_directory(const char *mods_directory)
{
    char mod_names[XENO_RUNTIME_MOD_MAX][XENO_RUNTIME_PATH_MAX];
    int mod_count = 0;
#ifdef _WIN64
    char pattern[XENO_RUNTIME_PATH_MAX];
    int pattern_length = snprintf(pattern, sizeof(pattern), "%s\\*.xar",
                                  mods_directory);
    if (pattern_length < 0 || (size_t)pattern_length >= sizeof(pattern))
    {
        fprintf(stderr, "[XenoScript] mods directory path is too long\n");
        return false;
    }
    struct _finddata_t entry;
    intptr_t search = _findfirst(pattern, &entry);
    if (search == -1)
    {
        if (errno == ENOENT)
            return true;
        fprintf(stderr, "[XenoScript] cannot scan '%s': %s\n",
                mods_directory, strerror(errno));
        return false;
    }
    do
    {
        if (entry.attrib & _A_SUBDIR)
            continue;
        if (mod_count >= XENO_RUNTIME_MOD_MAX)
        {
            fprintf(stderr, "[XenoScript] too many mod archives in '%s'\n",
                    mods_directory);
            _findclose(search);
            return false;
        }
        snprintf(mod_names[mod_count++], XENO_RUNTIME_PATH_MAX, "%s",
                 entry.name);
        errno = 0;
    } while (_findnext(search, &entry) == 0);
    int search_error = errno;
    _findclose(search);
    if (search_error != 0 && search_error != ENOENT)
    {
        fprintf(stderr, "[XenoScript] error scanning '%s': %s\n",
                mods_directory, strerror(search_error));
        return false;
    }
#else
    DIR *directory = opendir(mods_directory);
    if (!directory)
    {
        if (errno == ENOENT)
            return true;
        fprintf(stderr, "[XenoScript] cannot scan '%s': %s\n",
                mods_directory, strerror(errno));
        return false;
    }
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL)
    {
        size_t name_length = strlen(entry->d_name);
        if (name_length <= 4 ||
            strcmp(entry->d_name + name_length - 4, ".xar") != 0)
            continue;

        char full_path[XENO_RUNTIME_PATH_MAX];
        int path_length = snprintf(full_path, sizeof(full_path), "%s/%s",
                                   mods_directory, entry->d_name);
        if (path_length < 0 || (size_t)path_length >= sizeof(full_path))
        {
            fprintf(stderr, "[XenoScript] mod path is too long in '%s'\n",
                    mods_directory);
            closedir(directory);
            return false;
        }
        struct stat status;
        if (stat(full_path, &status) != 0 || !S_ISREG(status.st_mode))
            continue;
        if (mod_count >= XENO_RUNTIME_MOD_MAX)
        {
            fprintf(stderr, "[XenoScript] too many mod archives in '%s'\n",
                    mods_directory);
            closedir(directory);
            return false;
        }
        snprintf(mod_names[mod_count++], XENO_RUNTIME_PATH_MAX, "%s",
                 entry->d_name);
    }
    closedir(directory);
#endif

    char *sorted_names[XENO_RUNTIME_MOD_MAX];
    for (int i = 0; i < mod_count; i++)
        sorted_names[i] = mod_names[i];
    qsort(sorted_names, (size_t)mod_count, sizeof(sorted_names[0]),
          compare_mod_names);

    int first_mod = s_vm.stdlib_module_count;
    for (int i = 0; i < mod_count; i++)
    {
        char path[XENO_RUNTIME_PATH_MAX];
        int path_length = snprintf(path, sizeof(path), "%s/%s",
                                   mods_directory, sorted_names[i]);
        if (path_length < 0 || (size_t)path_length >= sizeof(path))
        {
            fprintf(stderr, "[XenoScript] mod path is too long: '%s'\n",
                    sorted_names[i]);
            return false;
        }
        char loading[XENO_RUNTIME_MOD_MAX][XAR_MAX_NAME] = {{0}};
        if (!load_mod_archive(mods_directory, path, loading, 0))
            return false;
    }

    for (int i = first_mod; i < s_vm.stdlib_module_count; i++)
    {
        XenoResult run_result =
            xeno_vm_run_mod(&s_vm, s_vm.stdlib_loaded_names[i]);
        if (run_result != XENO_OK)
        {
            fprintf(stderr, "[XenoScript] mod '%s' failed: ",
                    s_vm.stdlib_loaded_names[i]);
            xeno_vm_print_error(&s_vm);
            return false;
        }
    }
    return true;
}

void xeno_runtime_shutdown(void)
{
    if (s_vm_initialized)
    {
        xeno_vm_free(&s_vm);
        s_vm_initialized = false;
    }
}

bool xeno_runtime_init(void)
{
    xeno_vm_init(&s_vm);
    s_vm_initialized = true;

    int print_param_kind[] = {TYPE_ANY};
    if (xeno_register_fn_typed(&s_vm, "print", xeno_host_print,
                               TYPE_VOID, 1, print_param_kind) < 0)
    {
        fprintf(stderr, "[XenoScript] failed to register print host function\n");
        xeno_runtime_shutdown();
        return false;
    }
    stdlib_register_host_fns(&s_vm);
    xeno_vm_load_stdlib(&s_vm);

    char executable_dir[XENO_RUNTIME_PATH_MAX];
    if (!platform_get_executable_directory(executable_dir,
                                           sizeof(executable_dir)))
    {
        fprintf(stderr, "[XenoScript] failed to determine executable directory\n");
        xeno_runtime_shutdown();
        return false;
    }
    char mods_directory[XENO_RUNTIME_PATH_MAX];
    int mods_path_length = snprintf(mods_directory, sizeof(mods_directory),
                                    "%s/mods", executable_dir);
    if (mods_path_length < 0 ||
        (size_t)mods_path_length >= sizeof(mods_directory) ||
        mods_path_length >= (int)sizeof(s_vm.mod_path) - 1)
    {
        fprintf(stderr, "[XenoScript] mods directory path is too long\n");
        xeno_runtime_shutdown();
        return false;
    }
    char mods_search_path[XENO_RUNTIME_PATH_MAX];
    snprintf(mods_search_path, sizeof(mods_search_path), "%s/", mods_directory);
    xeno_vm_set_mod_path(&s_vm, mods_search_path);

    if (!load_mods_from_directory(mods_directory))
    {
        xeno_runtime_shutdown();
        return false;
    }
    return true;
}

#else

void xeno_runtime_shutdown(void)
{
}

bool xeno_runtime_init(void)
{
    return true;
}

#endif
