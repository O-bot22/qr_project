#include "common.hpp"
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <ctime>
#include <iostream>
#include <vector>
#include <deque>
#include <set>
#include <cstring>
struct Config { int port=2012, requests=3, window=60, users=3, timeout=80; };
volatile sig_atomic_t stopping=0;
volatile sig_atomic_t client_socket=-1;
void stop(int) {
    int saved_errno=errno;
    stopping=1;
    // Wake a worker blocked in recv so shutdown does not wait for its timeout.
    if (client_socket>=0) shutdown(client_socket,SHUT_RDWR);
    errno=saved_errno;
}
void wake(int) {}
// Each process opens its own log descriptor so flock serializes whole records.
void log_event(const std::string &ip, const char *event, int code=0, size_t bytes=0) {
    time_t now=time(nullptr); tm date{}; localtime_r(&now, &date);
    char timestamp[32]; strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &date);
    std::string line="{\"timestamp\":\""+std::string(timestamp)+"\",\"client_ip\":\""+ip+
        "\",\"client_id\":"+std::to_string(getpid())+",\"event\":\""+event+"\",\"error_code\":"+std::to_string(code)+"}\n";
    auto epoch_us=std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    line.resize(line.size()-2);
    line+=",\"epoch_us\":"+std::to_string(epoch_us)+",\"bytes\":"+std::to_string(bytes)+"}\n";
    int fd=open("server.jsonl", O_WRONLY|O_CREAT|O_APPEND|O_CLOEXEC, 0600);
    if (fd<0) { perror("open log"); return; }
    int locked; do { locked=flock(fd, LOCK_EX); } while (locked<0 && errno==EINTR);
    if (!locked) {
        size_t done=0;
        while (done<line.size()) {
            ssize_t n=write(fd,line.data()+done,line.size()-done);
            if (n<0 && errno==EINTR) continue;
            if (n<=0) { perror("write log"); break; }
            done+=static_cast<size_t>(n);
        }
        if (flock(fd, LOCK_UN)<0) perror("unlock log");
    }
    close(fd);
}
// Discard a bounded snapshot, never loop indefinitely on an attacker streaming data.
void discard_queued(int fd, const std::string &ip) {
    int queued=0;
    if (ioctl(fd, FIONREAD, &queued)<0) { log_event(ip,"drain_error",errno); return; }
    if (!queued) return;
    log_event(ip,"extraneous_bytes",1);
    char buffer[8192];
    while (queued>0) {
        ssize_t n=recv(fd,buffer,std::min<int>(queued,sizeof(buffer)),MSG_DONTWAIT);
        if (n<0 && errno==EINTR) continue;
        if (n<=0) break;
        queued-=static_cast<int>(n);
    }
}
std::string decode(const std::vector<unsigned char> &bytes) {
    char path[]="/tmp/cs3516-qr-XXXXXX";
    int fd=mkstemp(path);
    if (fd<0) return {};
    size_t done=0;
    while (done<bytes.size()) {
        ssize_t n=write(fd, bytes.data()+done, bytes.size()-done);
        if (n<0 && errno==EINTR) continue;
        if (n<=0) { close(fd); unlink(path); return {}; }
        done+=static_cast<size_t>(n);
    }
    close(fd);
    int pipes[2];
    if (pipe2(pipes,O_CLOEXEC)<0) { unlink(path); return {}; }
    pid_t child=fork();
    if (child==0) {
        // Isolate the image parser; no shell and no client-controlled command arguments.
        rlimit memory{256*1024*1024,256*1024*1024}, cpu{3,3};
        if (setrlimit(RLIMIT_AS,&memory)<0 || setrlimit(RLIMIT_CPU,&cpu)<0 || dup2(pipes[1],STDOUT_FILENO)<0) _exit(127);
        close(pipes[0]); close(pipes[1]);
        execl("./QRDecode","QRDecode",path,static_cast<char *>(nullptr));
        _exit(127);
    }
    close(pipes[1]);
    if (child<0) { close(pipes[0]); unlink(path); return {}; }
    std::string output;
    auto deadline=Clock::now()+std::chrono::seconds(5);
    int status=0; bool reaped=false;
    while (Clock::now()<deadline) {
        pollfd p{pipes[0],POLLIN,0};
        int ready=poll(&p,1,50);
        if (ready<0) { if(errno==EINTR) continue; break; }
        if (ready>0) {
            char buffer[4096]; ssize_t n=read(pipes[0],buffer,sizeof(buffer));
            if (n>0) { output.append(buffer,static_cast<size_t>(n)); if(output.size()>65536) break; }
            else if (n<0 && errno!=EINTR) break;
        }
        pid_t result=waitpid(child,&status,WNOHANG);
        if (result==child) { reaped=true; break; }
        if (result<0 && errno!=EINTR) break;
    }
    if (!reaped) { kill(child,SIGKILL); while(waitpid(child,&status,0)<0 && errno==EINTR) {} output.clear(); }
    else {
        // Drain the final pipe bytes after the child exits.
        char buffer[4096]; ssize_t n;
        while ((n=read(pipes[0],buffer,sizeof(buffer)))>0) {
            output.append(buffer,static_cast<size_t>(n));
            if (output.size()>65536) { output.clear(); break; }
        }
        if (!WIFEXITED(status) || WEXITSTATUS(status)!=0) output.clear();
    }
    close(pipes[0]); unlink(path); return output;
}
// TCP can return fewer bytes than requested, so keep reading the remaining bytes.
bool receive_all(int fd, void *buffer, size_t length, const std::string &ip) {
    char *data=static_cast<char *>(buffer);
    size_t received=0;
    while (received<length && !stopping) {
        ssize_t count=recv(fd,data+received,length-received,0);
        if (count>0) {
            received+=static_cast<size_t>(count);
            continue;
        }
        if (stopping) return false;
        if (count==0) {
            log_event(ip,"peer_closed_or_incomplete");
            return false;
        }
        if (errno==EINTR) continue;
        if (errno==EAGAIN || errno==EWOULDBLOCK) {
            log_event(ip,"timeout",2);
            if (!reply(fd,2,"Connection timed out",1)) log_event(ip,"send_error",errno);
        } else {
            log_event(ip,"receive_error",errno);
        }
        return false;
    }
    return received==length;
}

void session(int fd, const std::string &ip, const Config &cfg) {
    log_event(ip,"connected");
    // Each blocking recv waits at most this long for the next incoming bytes.
    timeval timeout{cfg.timeout,0};
    if (setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout))<0) {
        log_event(ip,"timeout_setup_error",errno);
        log_event(ip,"disconnected");
        return;
    }
    std::deque<Clock::time_point> requests;
    while (!stopping) {
        uint32_t wire_length=0;
        if (!receive_all(fd,&wire_length,sizeof(wire_length),ip)) break;
        uint32_t length=ntohl(wire_length);
        if (length>MAX_IMAGE) {
            discard_queued(fd,ip); log_event(ip,"oversize",1);
            if(!reply(fd,1,"",cfg.timeout)) break;
            continue;
        }
        std::vector<unsigned char> bytes(length);
        if (!receive_all(fd,bytes.data(),bytes.size(),ip)) break;
        log_event(ip,"image_received",0,bytes.size());
        auto now=Clock::now();
        while(!requests.empty() && now-requests.front()>=std::chrono::seconds(cfg.window)) requests.pop_front();
        uint32_t code=0; std::string message;
        if (requests.size()>=static_cast<size_t>(cfg.requests)) { code=3; message="Rate limit exceeded"; log_event(ip,"rate_limit",3); }
        else {
            requests.push_back(now);
            if(!bytes.empty()) { message=decode(bytes); if(message.empty()) code=1; }
            else code=1;
            log_event(ip,code?"invalid_request":"success",static_cast<int>(code));
        }
        discard_queued(fd,ip);
        if (!reply(fd,code,message,cfg.timeout)) { log_event(ip,"send_error",errno); break; }
    }
    log_event(ip,"disconnected");
}
int main(int argc,char **argv) {
    io_cancel=&stopping;
    Config cfg;
    try {
        for(int i=1;i<argc;++i) {
            std::string key=argv[i];
            auto arg=[&]() { if(++i>=argc) throw std::runtime_error("missing option value"); return argv[i]; };
            if(key=="PORT") cfg.port=number(arg(),2000,3000);
            else if(key=="RATE") { cfg.requests=number(arg(),1,100000); cfg.window=number(arg(),1,86400); }
            else if(key=="MAX_USERS" || key=="MAX USERS") cfg.users=number(arg(),1,1000);
            else if(key=="TIME_OUT" || key=="TIME OUT") cfg.timeout=number(arg(),1,86400);
            else if(key=="MAX") { if(std::string(arg())!="USERS") throw std::runtime_error("expected USERS"); cfg.users=number(arg(),1,1000); }
            else if(key=="TIME") { if(std::string(arg())!="OUT") throw std::runtime_error("expected OUT"); cfg.timeout=number(arg(),1,86400); }
            else throw std::runtime_error("unknown option: "+key);
        }
    } catch(const std::exception &e) { std::cerr<<e.what()<<'\n'; return 1; }
    struct sigaction action{}; sigemptyset(&action.sa_mask); action.sa_handler=stop;
    sigaction(SIGINT,&action,nullptr); sigaction(SIGTERM,&action,nullptr);
    action.sa_handler=wake; sigaction(SIGCHLD,&action,nullptr); sigaction(SIGUSR1,&action,nullptr);
    int listener=socket(AF_INET,SOCK_STREAM|SOCK_CLOEXEC,0);
    if(listener<0) { perror("socket"); return 1; }
    int one=1;
    sockaddr_in address{}; address.sin_family=AF_INET; address.sin_addr.s_addr=htonl(INADDR_ANY); address.sin_port=htons(cfg.port);
    if(setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one))<0 || bind(listener,reinterpret_cast<sockaddr *>(&address),sizeof(address))<0 || listen(listener,32)<0) {
        perror("listen setup"); close(listener); return 1;
    }
    std::set<pid_t> children;
    log_event("0.0.0.0","startup");
    while(!stopping) {
        int status; pid_t dead;
        while((dead=waitpid(-1,&status,WNOHANG))>0) children.erase(dead);
        pollfd p{listener,POLLIN,0}; int ready=poll(&p,1,200);
        if(ready<0) { if(errno==EINTR) continue; perror("poll"); break; }
        if(!ready) continue;
        sockaddr_in peer{}; socklen_t size=sizeof(peer);
        int fd=accept4(listener,reinterpret_cast<sockaddr *>(&peer),&size,SOCK_CLOEXEC);
        if(fd<0) { if(errno==EINTR) continue; perror("accept"); continue; }
        char ip[INET_ADDRSTRLEN]{};
        if(!inet_ntop(AF_INET,&peer.sin_addr,ip,sizeof(ip))) { close(fd); continue; }
        while((dead=waitpid(-1,&status,WNOHANG))>0) children.erase(dead);
        if(children.size()>=static_cast<size_t>(cfg.users)) {
            log_event(ip,"server_busy",1);
            // Original protocol requires an empty message for code 1; no busy code exists.
            if(!reply(fd,1,"",1)) log_event(ip,"busy_send_error",errno);
            close(fd); continue;
        }
        pid_t child=fork();
        if(child==0) {
            close(listener);
            client_socket=fd;
            try { session(fd,ip,cfg); } catch(const std::exception &) { log_event(ip,"session_exception",1); }
            client_socket=-1;
            close(fd); return 0;
        }
        if(child<0) { log_event(ip,"fork_error",errno); if(!reply(fd,1,"",1)) log_event(ip,"send_error",errno); }
        else children.insert(child);
        close(fd);
    }
    close(listener);
    for(pid_t child:children) kill(child,SIGTERM);
    // Workers finish their bounded receive/decode; always reap them before exiting.
    for(pid_t child:children) while(waitpid(child,nullptr,0)<0 && errno==EINTR) {}
    log_event("0.0.0.0","shutdown");
    return 0;
}
