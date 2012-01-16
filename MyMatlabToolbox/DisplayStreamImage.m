close all
clear all
clc

while(1)
[img size] = streamCreateVideo();
if size ~= 0
imshow(img,'DisplayRange',[0 255])
end
pause(0.1)
end