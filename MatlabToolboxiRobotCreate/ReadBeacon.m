function out = ReadBeacon(ports)
%READBEACON  Retrieve the most recent artag reading from a Beagleboard.
%   ReadBeacon(ports) returns
%   out.id = The id of the beacon
%   out.x = The x position of the beacon
%   out.y = The y position of the beacon
%   out.z = The z position of the beacon
%   out.yaw = The orientation of the beacon
%
%   The udp port object 'ports' must first be initialized with the 
%   CreateBeagleInit command (available as part of the Matlab Toolbox for 
%   the iRobot Create).
%
%   NOTE: If a beacon requested is not seen in the view then values of NaN 
%   are substituted for real data.
%
% By: Chuck Yang, ty244, 2012

warning off all;

global BEACON_OFFSET;
if isempty(BEACON_OFFSET)
BEACON_OFFSET = 0;
end

MAXARTAGSEEN = 10;
HEADER = 12;
% Initialize preliminary return value
out.id(1:MAXARTAGSEEN) = NaN;
out.x(1:MAXARTAGSEEN) = 0;
out.y(1:MAXARTAGSEEN) = 0;
out.z(1:MAXARTAGSEEN) = 0;
out.yaw(1:MAXARTAGSEEN) = 0;
try
    fclose(ports.beacon);
    pause(.01);
    fopen(ports.beacon);

    %read in packet and get size
    [packet size] = fread(ports.beacon);
    if size > 15
        for i = 1:MAXARTAGSEEN
            idIndex = HEADER+(i-1)*4+1;
            out.id(i) = typecast(uint8(packet(idIndex:idIndex+3)),'int32');
            if out.id(i) == 0
                out.id(i) = NaN;
                break;
            end
            xIndex = idIndex+4*MAXARTAGSEEN;
            out.x(i) = typecast(uint8(packet(xIndex:xIndex+3)),'single');
            yIndex = xIndex+4*MAXARTAGSEEN;
            out.y(i) = typecast(uint8(packet(yIndex:yIndex+3)),'single');
            zIndex = yIndex+4*MAXARTAGSEEN;
            out.z(i) = typecast(uint8(packet(zIndex:zIndex+3)),'single') + BEACON_OFFSET;
            yawIndex = zIndex+4*MAXARTAGSEEN;
            out.yaw(i) = typecast(uint8(packet(yawIndex:yawIndex+3)),'single');
        end
    end
catch
    disp('WARNING:  Function did not terminate correctly.  Output may be unreliable.')
end

end