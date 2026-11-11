#include "common.hpp"
#include <netdb.h>
#include <fstream>
#include <iostream>
#include <vector>
int main(int argc,char **argv) {
    if(argc<4) { std::cerr<<"Usage: ./QRClient HOST PORT FILE [FILE ...]\n"; return 1; }
    try { number(argv[2],2000,3000); } catch(const std::exception &e) { std::cerr<<e.what()<<'\n'; return 1; }
    addrinfo hints{}, *results=nullptr; hints.ai_family=AF_INET; hints.ai_socktype=SOCK_STREAM;
    int error=getaddrinfo(argv[1],argv[2],&hints,&results);
    if(error) { std::cerr<<gai_strerror(error)<<'\n'; return 1; }
    int fd=-1;
    for(auto *p=results;p;p=p->ai_next) {
        fd=socket(p->ai_family,p->ai_socktype|SOCK_CLOEXEC,p->ai_protocol);
        if(fd<0) continue;
        if(connect(fd,p->ai_addr,p->ai_addrlen)==0) break;
        close(fd); fd=-1;
    }
    freeaddrinfo(results);
    if(fd<0) { perror("connect"); return 1; }
    int exitcode=0;
    for(int i=3;i<argc;++i) {
        std::ifstream file(argv[i],std::ios::binary|std::ios::ate);
        auto length=file.tellg();
        if(!file || length<0 || length>MAX_IMAGE) { std::cerr<<"Cannot read file or file exceeds 4 MiB: "<<argv[i]<<'\n'; exitcode=1; break; }
        std::vector<char> bytes(static_cast<size_t>(length)); file.seekg(0);
        if(!file.read(bytes.data(),length)) { std::cerr<<"File read failed\n"; exitcode=1; break; }
        uint32_t wire_length=htonl(static_cast<uint32_t>(bytes.size()));
        bool sent=transfer(fd,&wire_length,sizeof(wire_length),true,90)==1 && transfer(fd,bytes.data(),bytes.size(),true,90)==1;
        uint32_t response[2]{};
        if(transfer(fd,response,8,false,90)!=1) { std::cerr<<"No complete response"<<(sent?"":" after send failure")<<'\n'; exitcode=1; break; }
        uint32_t code=ntohl(response[0]), size=ntohl(response[1]);
        if(code>3 || size>MAX_IMAGE || (code==1 && size!=0)) { std::cerr<<"Invalid response\n"; exitcode=1; break; }
        std::string message(size,'\0');
        if(transfer(fd,message.data(),size,false,90)!=1) { std::cerr<<"Incomplete response\n"; exitcode=1; break; }
        std::cout<<argv[i]<<": code="<<code<<" "<<message<<'\n';
        if(code) exitcode=1;
        if(code==2 || !sent) break;
    }
    close(fd); return exitcode;
}
