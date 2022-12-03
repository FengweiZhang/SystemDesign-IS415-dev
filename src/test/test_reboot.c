#include <unistd.h>
#include <sys/reboot.h>
int main()
{
    sync(); //  同步磁盘数据,将缓存数据回写到硬盘,以防数据丢失[luther.gliethttp]
    return reboot(RB_AUTOBOOT);
}