function sendpacket(msg)
%remote host (character string ip address with .'s)
rh='192.168.1.141';
%remote port (integer value)
port=8866;
disp(msg);
packetType = 0;
if (strcmp(msg(0),'INIT') == 1)
    packetType = 1;
elseif (strcmp(msg(0),'END') == 1)
    packetType = 2;
elseif (strcmp(msg(0),'CTRL') == 1)
    packetType = 3;
elseif (strcmp(msg(0),'DATA') == 1)
    packetType = 4;
elseif (strcmp(msg(0),'ERROR') == 1)
    packetType = 5;
elseif (strcmp(msg(0),'SHUTDOWN') == 1)
    packetType = 6;
else
    packetType = 7;
end
packet = [packetType msg(1:end)];
%get UDP packet from the remote host
u=udp(rh,port,'LocalPort',port);
fopen(u);
fwrite(u,packet);
fclose(u);
end
