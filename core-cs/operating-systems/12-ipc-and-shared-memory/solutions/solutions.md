# Solutions: Inter-Process Communication & Shared Memory

---

### Solution 2: File Descriptor Passing via Unix Domain Sockets (`SCM_RIGHTS`)

This complete C++ program demonstrates transferring an open file descriptor from a parent process to a child process over a Unix Domain Socket using `sendmsg()` and `recvmsg()`.

```cpp
#include <iostream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

bool send_fd(int socket_fd, int fd_to_send) {
    struct msghdr msg{};
    char dummy = '*';
    struct iovec io = { .iov_base = &dummy, .iov_len = sizeof(dummy) };

    // Allocate control message buffer for the file descriptor
    char cmsg_buf[CMSG_SPACE(sizeof(int))];
    memset(cmsg_buf, 0, sizeof(cmsg_buf));

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg_buf;
    msg.msg_controllen = sizeof(cmsg_buf);

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    memcpy(CMSG_DATA(cmsg), &fd_to_send, sizeof(int));

    return sendmsg(socket_fd, &msg, 0) >= 0;
}

int recv_fd(int socket_fd) {
    struct msghdr msg{};
    char dummy;
    struct iovec io = { .iov_base = &dummy, .iov_len = sizeof(dummy) };

    char cmsg_buf[CMSG_SPACE(sizeof(int))];
    memset(cmsg_buf, 0, sizeof(cmsg_buf));

    msg.msg_iov = &io;
    msg.msg_iovlen = 1;
    msg.msg_control = cmsg_buf;
    msg.msg_controllen = sizeof(cmsg_buf);

    if (recvmsg(socket_fd, &msg, 0) <= 0) {
        return -1;
    }

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    if (!cmsg || cmsg->cmsg_type != SCM_RIGHTS) {
        return -1;
    }

    int received_fd = -1;
    memcpy(&received_fd, CMSG_DATA(cmsg), sizeof(int));
    return received_fd;
}

int main() {
    int sv[2];
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) < 0) {
        perror("socketpair");
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Child: Waits for received FD, reads from it
        close(sv[1]);
        int received_fd = recv_fd(sv[0]);
        if (received_fd < 0) {
            std::cerr << "Child failed to receive fd\n";
            _exit(1);
        }

        char buffer[128] = {0};
        lseek(received_fd, 0, SEEK_SET);
        ssize_t n = read(received_fd, buffer, sizeof(buffer) - 1);
        std::cout << "[Child] Received FD " << received_fd << ", read content: \"" << buffer << "\"\n";
        close(received_fd);
        close(sv[0]);
        _exit(0);
    } else {
        // Parent: Creates a temporary file, writes secret, sends FD to child
        close(sv[0]);
        char temp_path[] = "/tmp/test_fd_pass_XXXXXX";
        int temp_fd = mkstemp(temp_path);
        const char* secret = "Confidential Token: FAANG_SECRET_99";
        write(temp_fd, secret, strlen(secret));

        std::cout << "[Parent] Created file with FD " << temp_fd << ", passing to child...\n";
        send_fd(sv[1], temp_fd);

        close(temp_fd);
        unlink(temp_path);
        close(sv[1]);
        waitpid(pid, nullptr, 0);
    }
    return 0;
}
```
