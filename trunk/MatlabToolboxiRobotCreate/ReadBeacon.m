function beacons = ReadBeacon(beaconPort)
%READBEACON  Retrieve the most recent ARtag reading from a Beagleboard.
%   ReadBeacon(beaconPort) returns
%   beacons(i).id = The id of the i'th beacon in camera view
%   beacons(i).x = The x position of the i'th beacon in camera view
%   beacons(i).y = The y position of the i'th beacon in camera view
%   beacons(i).z = The z position of the i'th beacon in camera view
%   beacons(i).yaw = The orientation of the i'th beacon in camera view
%
%   The total number of beacon detected is length(beacons) and it should
%   not be more than 10.
%   
%   When there is no ARtag detected, isempty(beacons) return 1.
%
%   The udp port object 'beaconPort' must first be initialized with the 
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

% the maximum number of ARtags that will get detected in one camera view.
MAXARTAGSEEN = 10;
HEADER = 12;
% Initialize preliminary return value
beacons = [];
beacon.id = NaN;
beacon.x = 0;
beacon.y = 0;
beacon.z = 0;
beacon.yaw = 0;
try
    fclose(beaconPort);
    pause(.01);
    fopen(beaconPort);

    %read in packet and get size
    [packet size] = fread(beaconPort);
    if size > 15
        for i = 1:MAXARTAGSEEN
            idIndex = HEADER+(i-1)*4+1;
            beacon.id = typecast(uint8(packet(idIndex:idIndex+3)),'int32');
            if beacon.id == 0 || beacon.id == -1
                break;
            end
            xIndex = idIndex+4*MAXARTAGSEEN;
            beacon.x = typecast(uint8(packet(xIndex:xIndex+3)),'single');
            yIndex = xIndex+4*MAXARTAGSEEN;
            beacon.y = typecast(uint8(packet(yIndex:yIndex+3)),'single');
            zIndex = yIndex+4*MAXARTAGSEEN;
            beacon.z = typecast(uint8(packet(zIndex:zIndex+3)),'single') + BEACON_OFFSET;
            yawIndex = zIndex+4*MAXARTAGSEEN;
            beacon.yaw = typecast(uint8(packet(yawIndex:yawIndex+3)),'single');

            beacons = [beacons beacon];
        end
    end
catch
    disp('WARNING:  Function did not terminate correctly.  Output may be unreliable.')
end

end