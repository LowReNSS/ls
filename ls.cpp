#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
 
#include <string>
#include <vector>
#include <algorithm>
 
/*
struct stat {
    dev_t         st_dev;      // устройство
    ino_t         st_ino;      // inode
    mode_t        st_mode;     // режим доступа
    nlink_t       st_nlink;    // количество жестких ссылок
    uid_t         st_uid;      // идентификатор пользователя-владельца
    gid_t         st_gid;      // идентификатор группы-владельца 
    dev_t         st_rdev;     // тип устройства
                               // (если это устройство)
    off_t         st_size;     // общий размер в байтах
    blksize_t     st_blksize;  // размер блока ввода-вывода
                               // в файловой системе
    blkcnt_t      st_blocks;   // количество выделенных блоков
    time_t        st_atime;    // время последнего доступа
    time_t        st_mtime;    // время последней модификации
    time_t        st_ctime;    // время последнего изменения
};
 
*/
 
void print_human_size(uint64_t bytes);
 
void print_long_format(const std::string &file_name, bool print_readible);
 
 
int main(int argc, char *argv[])
{
    bool print_extended = false;
    bool print_reverse  = false;
    bool print_readible = false;
    
 
    // read arguments
    int optcmd = 0;
    while((optcmd  = getopt(argc, argv, "l::r::h::")) != -1)
    {
        if(0);
        else if(optcmd == 'l') print_extended    = true;
        else if(optcmd == 'r') print_reverse     = true;
        else if(optcmd == 'h') print_readible    = true;
 
        if(optarg) for(auto it : std::string(optarg))
        {
            if(0);
            else if(it == 'l') print_extended    = true;
            else if(it == 'r') print_reverse     = true;
            else if(it == 'h') print_readible    = true;
            else printf("%s: invalid argument -- '%c'\n", argv[0], (char)it);
        }
            
    }
 
    // read directory
    std::string directory = ".";
    if(optind < argc) directory = argv[optind];
   
    // open directory
    DIR *dir = opendir(directory.c_str());
 
    if(!dir) perror("Error");
    else
    {
        struct dirent *info;
        std::vector<std::string> files;
 
        while((info = readdir(dir)) != NULL)
            if(info->d_name[0] != '.') files.push_back(info->d_name);
 
        closedir(dir);
 
        if(print_reverse)   std::sort(files.rbegin(), files.rend());
        else                std::sort(files.begin(), files.end());
 
        for(std::string &it : files)
            if(print_extended) print_long_format(it.c_str(), print_readible);
            else printf(it.find(' ') == std::string::npos ? " %s" : " '%s'", it.c_str());
 
        if(!print_extended) printf("\n");
    }
 
    return 0;
}
 
 
void print_long_format(const std::string &file_name, bool print_readible)
{
    struct stat info;
 
    if (stat(file_name.c_str(), &info) == -1) 
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }
 
    auto file_type = info.st_mode & S_IFMT;
    switch(file_type) 
    {
        case S_IFBLK:  printf("b"); break;  // block device
        case S_IFCHR:  printf("c"); break;  // character device
        case S_IFDIR:  printf("d"); break;  // directory
        case S_IFIFO:  printf("p"); break;  // FIFO/pipe
        case S_IFLNK:  printf("l"); break;  // symlink
        case S_IFREG:  printf("-"); break;  // regular file
        case S_IFSOCK: printf("s"); break;  // socket
        default:       printf("?"); break;  // unknown
    }
        
    auto permission_user = info.st_mode & S_IRWXU;
    if(permission_user & S_IRUSR) printf("r"); else printf("-");    // read
    if(permission_user & S_IWUSR) printf("w"); else printf("-");    // write
    if(permission_user & S_IXUSR) printf("x"); else printf("-");    // execute
    
    auto permission_group = info.st_mode & S_IRWXG;
    if(permission_group & S_IRGRP) printf("r"); else printf("-");
    if(permission_group & S_IWGRP) printf("w"); else printf("-");
    if(permission_group & S_IXGRP) printf("x"); else printf("-");
    
    auto permission_other = info.st_mode & S_IRWXO;
    if(permission_other & S_IROTH) printf("r"); else printf("-");
    if(permission_other & S_IWOTH) printf("w"); else printf("-");
    if(permission_other & S_IXOTH) printf("x"); else printf("-");
    
    auto number_of_links = info.st_nlink;
    printf(" %lu", number_of_links);
    
    auto owner_user_id = info.st_uid;
    printf(" %s", getpwuid(owner_user_id)->pw_name);
 
    auto owner_group_id = info.st_gid;
    printf(" %s", getgrgid(owner_group_id)->gr_name);
 
    auto size_in_bytes = info.st_size;
    if(print_readible) print_human_size(size_in_bytes);
    else printf(" %8lu", size_in_bytes);
 
    auto date_of_last_modification = info.st_mtime;
    struct tm *timeinfo = localtime(&date_of_last_modification);
    char str[20]; 
    strftime(str, sizeof(str), "%b %d %H:%M", timeinfo); 
    printf(" %s", str);
 
    printf(file_name.find(' ') == std::string::npos ? " %s\n" : " '%s'\n", file_name.c_str());
}
 
void print_human_size(uint64_t bytes)
{
    char suffix[] = {'B', 'K', 'M', 'G', 'T'};
    int length = sizeof(suffix);
 
    int i = 0;
 
    double num = bytes;
    if(bytes > 1024)
    {
        for(i = 0; (bytes / 1024) > 0 && i<length-1; i++, bytes /= 1024) num = bytes / 1024.0;
    }
 
    printf(" %6.1lf%c", num, suffix[i]);
}