function [image size] = streamCreateVideo()
%remote host (character string ip address with .'s)
rh = '192.168.1.141';
%remote port (integer value)
port = 8855;
imagesize = 19200;
% set(u,'Timeout',10);
mssg = judp('receive',port,imagesize+20);
packet = mssg;

size = length(packet);
if (size > 13)
width = typecast(int8(packet(13:16)),'uint32');
height = typecast(int8(packet(17:20)),'uint32');
packet = typecast(int8(packet(21:end)), 'uint8');
image = reshape(packet, width, height)';

size = width*height;
else
image = 0;
size = 0;
end

end