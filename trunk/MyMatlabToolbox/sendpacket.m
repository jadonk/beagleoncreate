function sendpacket(msg)
%remote host (character string ip address with .'s)

rh='192.168.1.141';
%remote port (integer value)
port=8866;
disp(msg);
packet = msg;
if length(msg) > 1
    packet = [msg(1) 0 0 0 '00000000' msg(2:end)];
end

%get UDP packet from the remote host
u=udp(rh,port,'LocalPort',port);
fopen(u);
fwrite(u,packet);
fclose(u);
end
