function tag = streamCreateARtag()
warning off all;
rh='192.168.1.141';
port=8844;
%get UDP packet from the remote host
u=udp(rh,port,'LocalPort',port);
set(u,'Timeout',0.2)
tag.isValid = 0;
fopen(u);
[packet size] = fread(u);
if size > 15
    tag.isValid = 1;
    tag.id = typecast(uint8(packet(13:16)),'int32');
    tag.x = typecast(uint8(packet(17:20)),'single');
    tag.y = typecast(uint8(packet(21:24)),'single');
    tag.z = typecast(uint8(packet(25:28)),'single');
    tag.yaw = typecast(uint8(packet(29:32)),'single');
end
fclose(u);
end