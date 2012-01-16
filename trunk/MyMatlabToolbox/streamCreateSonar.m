function dist = streamCreateSonar()
warning off all;
rh='192.168.1.141';
port=8833;
%get UDP packet from the remote host
u=udp(rh,port,'LocalPort',port);
set(u,'Timeout',0.2)
dist.isValid = 0;
dist.sonar1 = 0;
dist.sonar2 = 0;
dist.sonar3 = 0;
fopen(u);
[packet size] = fread(u);
if size > 14
    dist.isValid = 1;
    dist.sonar1 = typecast(uint8(packet(13:16)),'single');
    dist.sonar2 = typecast(uint8(packet(17:20)),'single');
    dist.sonar3 = typecast(uint8(packet(21:24)),'single');
end
fclose(u);
end