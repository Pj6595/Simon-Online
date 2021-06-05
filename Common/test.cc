#include "SimonMessage.h"
#include <iostream>
#include <fcntl.h>

int main(){
    SimonMessage c("User1","12913718629653283587263482648");
    c.type = 2;
    c.to_bin();
    int file = creat("./chatFile", 0666);
    write(file, c.data(), c.MESSAGE_SIZE);
    close(file);
    file = open("./chatFile", O_RDONLY);
    char* buf = (char*)malloc(c.MESSAGE_SIZE);
    read(file, buf, c.MESSAGE_SIZE);
    c.from_bin(buf);
    close(file);
    std::cout << "Type: " << unsigned(c.type) << std::endl;
    std::cout << "Nick: " << c.nick << std::endl;
    std::cout << "Sequence: " << c.sequence << std::endl;
    return 0;
}