#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdlib.h>
#include <signal.h>
LONG nr;
LONG qw;
LONG qr;
BOOL wrt;
HANDLE c_write;
HANDLE c_read;
HANDLE mutex;
int fl = 1;
int v = 0;
void start_read()
{
    InterlockedIncrement(&qr);
    if (wrt || qw > 0)
        WaitForSingleObject(c_read, INFINITE);
    WaitForSingleObject(mutex, INFINITE);
    InterlockedIncrement(&nr);
    InterlockedDecrement(&qr);
    SetEvent(c_read);
    ReleaseMutex(mutex);
}
void stop_read()
{
    InterlockedDecrement(&nr);
    if (nr == 0)
        SetEvent(c_write);
}
void start_write()
{
    InterlockedIncrement(&qw);
    if (nr > 0 || wrt)
        WaitForSingleObject(c_write, INFINITE);
    InterlockedDecrement(&qw);
    wrt = true;
}
void stop_write()
{
    ResetEvent(c_write);
    wrt = false;
    if (qr > 0)
        SetEvent(c_read);
    else
        SetEvent(c_write);
}
DWORD WINAPI reader(CONST LPVOID lp_params)
{
    DWORD tid = GetCurrentThreadId();
    srand(time(NULL) + tid);
    while (fl)
    {
        Sleep((double)rand() / RAND_MAX * 1000 * 3);
        start_read();
        printf("reader %lu: read %d\n", tid, v);
        stop_read();
    }
    exit(0);
}
DWORD WINAPI writer(CONST LPVOID lp_params)
{
    DWORD tid = GetCurrentThreadId();
    srand(time(NULL) + tid);
    while (fl)
    {
        Sleep((double)rand() / RAND_MAX * 1000 * 3);
        start_write();
        printf("writer %lu: write %d\n", tid, ++v);
        stop_write();
    }
    exit(0);
}
void sig_handler(const int code)
{
    printf("%lu catch signal %d\n", GetCurrentThreadId(), code);
    fl = 0;
}
int main()
{
    HANDLE rthrd[5];
    HANDLE wthrd[3];
    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        perror("signal");
        exit(1);
    }
    if ((mutex = CreateMutex(NULL, FALSE, NULL)) == NULL)
    {
        perror("CreateMutex");
        exit(1);
    }
    if ((c_read = CreateEvent(NULL, FALSE, FALSE, NULL)) == NULL)
    {
        perror("CreateEvent c_read");
        exit(1);
    }
    if ((c_write = CreateEvent(NULL, TRUE, FALSE, NULL)) == NULL)
    {
        perror("CreateEvent c_write");
        exit(1);
    }
    for (int i = 0; i < 3; i++)
    {
        if ((wthrd[i] = CreateThread(NULL, 0, writer, (LPVOID)i, 0, NULL)) == NULL)
        {
            perror("CreateThread writer");
            exit(1);
        }
    }
    for (int i = 0; i < 5; i++)
    {
        if ((rthrd[i] = CreateThread(NULL, 0, reader, (LPVOID)i, 0, NULL)) == NULL)
        {
            perror("CreateThread reader");
            exit(1);
        }
    }
    WaitForMultipleObjects(3, wthrd, TRUE, INFINITE);
    WaitForMultipleObjects(5, rthrd, TRUE, INFINITE);
    for (int i = 0; i < 3; i++)
        CloseHandle(wthrd[i]);
    for (int i = 0; i < 5; i++)
        CloseHandle(rthrd[i]);
    CloseHandle(mutex);
    CloseHandle(c_read);
    CloseHandle(c_write);
    return 0;
}
